#ifndef CACHE_HPP
#define CACHE_HPP

#include <sys-sage/Component.hpp>

namespace sys_sage {

    /**
     * @class Cache
     * @brief Represents a data cache in the system (of different levels/purposes).
     *
     * Models hardware cache components (L1, L2, L3, texture, etc.) as part of the unified hardware topology abstraction.
     * Inherits from Component and provides attributes and methods for describing cache hierarchy, size, associativity, and line size.
     * The cache abstraction supports both classical and heterogeneous (e.g., GPU, accelerator) memory hierarchies, and is extensible for new architectures and data sources.
     *
     * @see Component
     */
    class Cache : public Component {
    public:
        /**
         * @brief Cache constructor (no automatic insertion in the Component Tree).
         * @param _id Unique cache ID (default 0)
         * @param _cache_level Cache level (1=L1, 2=L2, ...), default 0. This value is represented as cache_type.
         * @param _cache_size Size of the cache in bytes (default -1)
         * @param _associativity Number of cache associativity ways (default -1)
         * @param _cache_line_size Size of a cache line in bytes (default -1)
         *
         * Sets componentType to sys_sage::ComponentType::Cache.
         */
        Cache(int _id = 0, int  _cache_level = 0, long long _cache_size = -1, int _associativity = -1, int _cache_line_size = -1);
        /**
         * @brief Cache constructor with insertion into the Component Tree as the parent's child (as long as parent is an existing Component).
         * @param parent Parent component
         * @param _id Unique cache ID (default 0)
         * @param _cache_type Name/type of the cache (e.g., "L1", "texture"). Only one of int cache_level or string cache_type should be used.
         * @param _cache_size Size of the cache in bytes (default 0)
         * @param _associativity Number of cache associativity ways (default -1)
         * @param _cache_line_size Size of a cache line in bytes (default -1)
         *
         * Sets componentType to sys_sage::ComponentType::Cache.
         */
        Cache(Component * parent, int _id, const std::string &_cache_type, long long _cache_size = -1, int _associativity = -1, int _cache_line_size = -1);

        /**
         * @brief Cache constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component).
         * @param parent Parent component
         * @param _id Unique cache ID (default 0)
         * @param _cache_level Cache level (1=L1, 2=L2, ...), default 0. This value is represented as cache_type.
         * @param _cache_size Size of the cache in bytes (default -1)
         * @param _associativity Number of cache associativity ways (default -1)
         * @param _cache_line_size Size of a cache line in bytes (default -1)
         *
         * Sets componentType to sys_sage::ComponentType::Cache.
         */
        Cache(Component * parent, int _id = 0, int _cache_level = 0, long long _cache_size = -1, int _associativity = -1, int _cache_line_size = -1);

        /**
         * @brief Get the cache level (e.g., 1 for L1, 2 for L2).
         * @return Cache level as integer.
         *
         * Assumes there's only 1 or no digit in the "cache_type" (e.g., "L1", "texture").
         */
        int GetCacheLevel() const;
        
        /**
         * @brief Set the cache level using an integer value (e.g., 1 for "L1").
         * @param _cache_level Value for cache_type.
         */
        void SetCacheLevel(int _cache_level);
        
        /**
         * @brief Retrieves cache name/type (e.g., "L1", "texture").
         * @return Cache name as string.
         * @see cache_type
         */
        const std::string& GetCacheName() const;
        
        /**
         * @brief Set the cache name/type (e.g., "L1", "texture").
         * @param _name Value for cache_type.
         */
        void SetCacheName(const std::string &_name);

        /**
         * @brief Retrieves size/capacity of the cache.
         * @return Cache size in bytes.
         */
        long long GetCacheSize() const;
        /**
         * @brief Sets size/capacity of the cache.
         * @param _cache_size Size in bytes.
         */
        void SetCacheSize(long long _cache_size);
        /**
         * @brief Get the number of cache associativity ways.
         * @return Number of associativity ways.
         */
        int GetCacheAssociativityWays() const;

        /**
         * @brief Set the number of cache associativity ways.
         * @param _associativity Value for cache_associativity_ways.
         */
        void SetCacheAssociativityWays(int _associativity);
        /**
         * @brief Get the size of a cache line.
         * @return Cache line size in bytes.
         */
        int GetCacheLineSize() const;
        /**
         * @brief Set the size of a cache line.
         * @param _cache_line_size Cache line size in bytes.
         */
        void SetCacheLineSize(int _cache_line_size);
        /**
         * @private
         * @brief Helper function for XML dump generation.
         *
         * Should normally not be called from the outside. Used internally for exporting the topology to XML.
         * @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL)
         * @return Pointer to the created XML subtree node.
         */
        xmlNodePtr _CreateXmlSubtree() override;

        /**
         * @private
         *
         * @brief Initializes a JSON object that represents this component.
         *        Intended for internal use.
         *
         * @param obj The JSON object to be initialized.
         */
        void _ToJson(nlohmann::json &obj) const override;

        /**
         * @private
         *
         * @brief Initializes this component through JSON. Intended for
         *        internal use.
         *
         * @param obj The JSON object containing the data.
         *
         * @return 0 on success, 1 otherwise.
         */
        int _FromJson(const nlohmann::json &obj) override;

    private:
        std::string cache_type;           ///< Cache level or cache type (e.g., "L1", "texture")
        long long cache_size;             ///< Size/capacity of the cache in bytes
        int cache_associativity_ways;     ///< Number of cache associativity ways
        int cache_line_size;              ///< Size of a cache line in bytes

    #ifdef NVIDIA_MIG
    public:
        /**
         * @brief Gets the MIG size of the cache element (NVIDIA-specific).
         * @param uuid Optional UUID string for the MIG instance.
         * @return MIG size in bytes.
         */
        long long GetMIGSize(std::string uuid = "") const;
    #endif
    };
}
#endif //CACHE_HPP