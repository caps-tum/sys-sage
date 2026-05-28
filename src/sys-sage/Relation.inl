namespace sys_sage {
    template <typename T>
    std::decay_t<T> *Relation::SetAttribute(const std::string &key, T &&value)
    {
        auto &iAttribute = (attributes[key] = std::make_unique<Attribute<std::decay_t<T>>>(std::forward<T>(value)));
        auto attribute = dynamic_cast<Attribute<std::decay_t<T>> *>( iAttribute.get() );
        return &(**attribute);
    }

    template <typename T>
    T *Relation::GetAttribute(const std::string &key)
    {
        auto it = attributes.find(key);
        if (it == attributes.end())
            return nullptr;

        auto attribute = dynamic_cast<Attribute<std::decay_t<T>> *>( it->second.get() );
        if (attribute == nullptr)
            return nullptr;

        return &(**attribute);
    }

    template <typename T>
    const T *Relation::GetAttribute(const std::string &key) const
    {
        auto it = attributes.find(key);
        if (it == attributes.end())
            return nullptr;

        auto attribute = dynamic_cast<Attribute<std::decay_t<T>> *>( it->second.get() );
        if (attribute == nullptr)
            return nullptr;

        return &(**attribute);
    }


    template <typename T>
    T *Relation::GetAttribute(Relation::attribIterator it)
    {
        if (it == attributes.end())
            return nullptr;

        auto attribute = dynamic_cast<Attribute<std::decay_t<T>> *>( it->second.get() );
        if (attribute == nullptr)
            return nullptr;

        return &(**attribute);
    }

    template <typename T>
    const T *Relation::GetAttribute(Relation::constAttribIterator it) const
    {
        if (it == attributes.end())
            return nullptr;

        auto attribute = dynamic_cast<Attribute<std::decay_t<T>> *>( it->second.get() );
        if (attribute == nullptr)
            return nullptr;

        return &(**attribute);
    }

    template <typename T>
    std::decay_t<T> *Relation::UpdateAttribute(const std::string &key, T &&value)
    {
        auto it = attributes.find(key);
        if (it == attributes.end())
            return SetAttribute(key, std::forward<T>(value));

        auto attribute = dynamic_cast<Attribute<std::decay_t<T>> *>( it->second.get() );
        if (attribute == nullptr)
            return nullptr;

        **attribute = std::forward<T>(value);

        return &(**attribute);
    }

    template <typename T>
    std::decay_t<T> *Relation::UpdateAttribute(Relation::attribIterator it, T &&value)
    {
        if (it == attributes.end())
            return nullptr;

        auto attribute = dynamic_cast<Attribute<std::decay_t<T>> *>( it->second.get() );
        if (attribute == nullptr)
            return nullptr;

        **attribute = std::forward<T>(value);

        return &(**attribute);
    }
}
