namespace sys_sage {
    template <typename T>
    std::decay_t<T> *Component::SetAttribute(const std::string &key, T &&value)
    {
        auto &iAttribute = (attributes[key] = std::make_unique<Attribute<std::decay_t<T>>>(std::forward<T>(value)));
        auto attribute = dynamic_cast<Attribute<std::decay_t<T>> *>( iAttribute.get() );
        return &(**attribute);
    }

    template <typename T>
    T *Component::GetAttribute(const std::string &key)
    {
        // This is a hack to force template initialization.
        // DO NOT REMOVE IT
        (void)TypeTrait<T>::serializable;

        auto it = attributes.find(key);
        if (it == attributes.end())
            return nullptr;

        auto attribute = dynamic_cast<Attribute<std::decay_t<T>> *>( it->second.get() );
        if (attribute == nullptr)
            return nullptr;

        return &(**attribute);
    }

    template <typename T>
    const T *Component::GetAttribute(const std::string &key) const
    {
        // This is a hack to force template initialization.
        // DO NOT REMOVE IT
        (void)TypeTrait<T>::serializable;

        auto it = attributes.find(key);
        if (it == attributes.end())
            return nullptr;

        auto attribute = dynamic_cast<Attribute<std::decay_t<T>> *>( it->second.get() );
        if (attribute == nullptr)
            return nullptr;

        return &(**attribute);
    }


    template <typename T>
    T *Component::GetAttribute(Component::attribIterator it)
    {
        // This is a hack to force template initialization.
        // DO NOT REMOVE IT
        (void)TypeTrait<T>::serializable;

        if (it == attributes.end())
            return nullptr;

        auto attribute = dynamic_cast<Attribute<std::decay_t<T>> *>( it->second.get() );
        if (attribute == nullptr)
            return nullptr;

        return &(**attribute);
    }

    template <typename T>
    const T *Component::GetAttribute(Component::constAttribIterator it) const
    {
        // This is a hack to force template initialization.
        // DO NOT REMOVE IT
        (void)TypeTrait<T>::serializable;

        if (it == attributes.end())
            return nullptr;

        auto attribute = dynamic_cast<Attribute<std::decay_t<T>> *>( it->second.get() );
        if (attribute == nullptr)
            return nullptr;

        return &(**attribute);
    }

    template <typename T>
    std::decay_t<T> *Component::UpdateAttribute(const std::string &key, T &&value)
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
    std::decay_t<T> *Component::UpdateAttribute(Component::attribIterator it, T &&value)
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
