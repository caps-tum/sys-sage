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
}
