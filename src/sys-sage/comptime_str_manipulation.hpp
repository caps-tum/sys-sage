/*
 * This file provides the functionality for manipulating string literals at compile time.
 * In particular, it enables compile time string literal concatenation and compile time substring literal extraction for type name reflection.
 *
 * The former functionality was originally authored by user "cristi1990an" and shared on
 * https://www.reddit.com/r/cpp/comments/15z14hh/compile_time_string_literal_concatenation_or_how/
 * and
 * https://godbolt.org/z/Gdfnsf8Pa
 *
 * We have made slight modifications to the code (such as removing the delimiter and changing the namespace) and further extended it with the latter functionality.
 */

#include <algorithm>
#include <cstddef>
#include <ranges>

namespace sys_sage {
    /**
     * @private
     *
     * @brief Represents a string at compile time and is meant to be used in template metaprogramming.
     *        The string may result from copying a string literal, concatenating two strings or extracting a substring.
     */
    template <std::size_t N>
    struct CompStr {
        /**
         * @private
         *
         * @brief Copies a string literal into a CompStr object.
         */
        consteval CompStr(const char (&literal)[N]) noexcept
        {
            std::ranges::copy(literal, buffer);
        }
    
        /**
         * @private
         *
         * @brief Copies a CompStr object into another CompStr object.
         */
        template <std::size_t N1>
        consteval CompStr(const CompStr<N1> &compStr, std::size_t begin) noexcept
        {
            std::ranges::copy(compStr.buffer | std::ranges::views::drop(begin) | std::ranges::views::take(N - 1), buffer);
            buffer[N - 1] = '\0';
        }
    
        /**
         * @private
         *
         * @brief Creates a new CompStr object as the concatenation of two CompStr objects.
         */
        template <std::size_t N1, std::size_t N2> requires (N == N1 + N2 - 1)
        consteval CompStr(const CompStr<N1> &compStr1, const CompStr<N2> &compStr2) noexcept
        {
            std::ranges::copy(compStr2.buffer, std::ranges::copy(compStr1.buffer | std::ranges::views::take(N1 - 1), buffer).out);
        }
    
        char buffer[N];
    };

    /**
     * @private
     *
     * @brief deduction hint for the compiler
     */
    template <std::size_t N1, std::size_t N2>
    CompStr(const CompStr<N1> &, const CompStr<N2> &) -> CompStr<N1 + N2 - 1>;

    /**
     * @private
     *
     * @brief Effectively casts an internal buffer to a string literal.
     */
    template <CompStr compStr>
    consteval auto CompStrToLiteral() -> const char (&)[std::size(compStr.buffer)]
    {
        return compStr.buffer;
    }

    /**
     * @private
     *
     * @brief Simply returns the string literal back.
     */
    template <CompStr compStr>
    consteval decltype(auto) CompStrCat() noexcept
    {
        return CompStrToLiteral<compStr>();
    }

    /**
     * @private
     *
     * @brief Returns the concatenation of two string literals at compile time.
     *        It forms the base case of the recursive string literal concatenation.
     */
    template <CompStr compStr1, CompStr compStr2>
    consteval decltype(auto) CompStrCat() noexcept
    {
        return CompStrToLiteral<CompStr(compStr1, compStr2)>();
    }

    /**
     * @private
     *
     * @brief Recursively concatenates string literals at compile time.
     */
    template <CompStr compStr1, CompStr compStr2, CompStr... compStrs> requires (sizeof...(compStrs) > 0)
    consteval decltype(auto) CompStrCat() noexcept
    {
        return CompStrCat<CompStr(compStr1, compStr2), compStrs...>();
    }

    /**
     * @private
     *
     * @brief Returns it's own function signature which contains a string representation for the given type `T`.
     */
    template <typename T>
    consteval auto &PrettyFunction()
    {
        return __PRETTY_FUNCTION__;
    }

    /**
     * @private
     *
     * @brief Defines position not found.
     */
    inline static constexpr std::size_t npos = static_cast<std::size_t>(-1);

    /**
     * @private
     *
     * @brief Finds the starting position of the substring `needle` in the string `hayStack`.
     *        The search is started from the beginning.
     *        The returned position is the first found one.
     *        Note: Fancy optimizations for finding the substring (e.g. KMP, Boyer-Moore, ...) are likely an overkill, since the strings will typically be of small size.
     */
    template <CompStr hayStack, CompStr needle>
    consteval std::size_t CompStrFind()
    {
        constexpr std::size_t hayStackSize = std::size(hayStack.buffer) - 1;
        constexpr std::size_t needleSize = std::size(needle.buffer) - 1;

        static_assert(hayStackSize >= needleSize);

        for (std::size_t i = 0; i <= hayStackSize - needleSize; i++) {
            std::size_t j;
            for (j = 0; j < needleSize; j++) {
                if (hayStack.buffer[i + j] != needle.buffer[j])
                    break;
            }
            if (j == needleSize)
                return i;
        }

        return npos;
    }

    /**
     * @private
     *
     * @brief Finds the starting position of the substring `needle` in the string `hayStack`.
     *        The search is started from the end.
     *        The returned position is the first found one.
     *        Note: Fancy optimizations for finding the substring (e.g. KMP, Boyer-Moore, ...) are likely an overkill, since the strings will typically be of small size.
     */
    template <CompStr hayStack, CompStr needle>
    consteval std::size_t CompStrRFind()
    {
        constexpr std::size_t hayStackSize = std::size(hayStack.buffer) - 1;
        constexpr std::size_t needleSize = std::size(needle.buffer) - 1;

        static_assert(hayStackSize >= needleSize);

        for (std::size_t i = 0; i <= hayStackSize - needleSize; i++) {
            std::size_t j;
            for (j = 0; j < needleSize; j++) {
                if (hayStack.buffer[(hayStackSize - needleSize - i) + (needleSize - 1 - j)] != needle.buffer[needleSize - 1 - j])
                    break;
            }

            if (j == needleSize)
                return hayStackSize - needleSize - i;
        }

        return npos;
    }

    /**
     * @private
     *
     * @brief Returns a substring literal starting from `begin` and ending at `end`.
     */
    template <CompStr funcName, std::size_t begin, std::size_t offset, std::size_t end>
    consteval decltype(auto) CompStrExtract() noexcept
    {
        static_assert(begin != npos);
        static_assert(end != npos);

        return CompStrToLiteral<CompStr<end - (begin + offset) + 1>(funcName, begin + offset)>();
    }

    /**
     * @private
     *
     * @brief Returns a substring literal by removing a prefix that ends with `prefix` and removing a suffix that starts with `suffix`.
     */
    template <CompStr funcName, CompStr prefix, CompStr suffix>
    consteval decltype(auto) CompStrExtract() noexcept
    {
        return CompStrExtract<
            funcName,
            CompStrFind<funcName, prefix>(),
            std::size(prefix.buffer) - 1,
            CompStrRFind<funcName, suffix>()
        >();
    }

    /**
     * @private
     *
     * @brief Returns the string representation of the given type `T` at compile time.
     *        WARNING: The output is compiler-specific.
     *        There are NO GUARANTEES that the output is the same across different compilers or across versions of the same compiler.
     *
     *        For instance on gcc, `PrettyFunction<int>()` would return "consteval auto& PrettyFunction() [with T = int]".
     *        We want to extract the string representation of the given type from this string literal, which in this case would be "int".
     *        For this we try to identify a short enough substring that contains "int", where the prefix of this substring and the suffix is unique in the entire string literal.
     *        In this case, we get "= int]" as the subtring, because searching for the prefix "= " will get us the position at the beginning of "int" and searching for "]" will give us the position at the end of "int".
     *        The prefix and suffix work for every type `T` using gcc and clang.
     *        However, if a new compiler should be used we might get unexpected results or errors.
     *        In that case use the prefix and suffix that works for the target compiler and make the necessary adjustments in the macros, e.g. `CompStrExtract<__VA_ARGS__, yourPrefix, yourSuffix>()` inside of SYS_SAGE_REGISTER_TYPE_TRAIR_NON_PORTABLE.
     */
    template <typename T, CompStr prefix = "= ", CompStr suffix = "]">
    consteval decltype(auto) CompStrExtract() noexcept
    {
        return CompStrExtract<
            CompStr( PrettyFunction<T>() ),
            prefix,
            suffix
        >();
    }
}
