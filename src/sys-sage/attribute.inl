namespace sys_sage {
    template <typename T>
    template <typename U>
    Attribute<T>::Attribute(U &&_value) : value(std::forward<U>(_value)) {}
    
    template <typename T>
    T &Attribute<T>::operator*()
    {
        return value;
    }
    
    template <typename T>
    const T &Attribute<T>::operator*() const
    {
        return value;
    }
    
    template <typename T>
    T *Attribute<T>::operator->()
    {
        return &value;
    }
    
    template <typename T>
    const T *Attribute<T>::operator->() const
    {
        return &value;
    }

    template <typename T>
    void Attribute<T>::Serialize(nlohmann::json &obj) const
    {
        if constexpr (TypeTrait<T>::serializable) {
            if constexpr (TypeTrait<T>::deserializable) {
                obj = {
                    { "_sys_sage_type", TypeTrait<T>::id },
                    { "_sys_sage_value", value }
                };
            } else {
                obj = value;
            }
        } else {
            obj = nlohmann::json();
        }
    }
}
