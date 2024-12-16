#pragma once

#include <functional>

#ifndef _SKMP_DISABLE_BOOST_SERIALIZATION
#include <boost/serialization/access.hpp>
#endif

template <class T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
struct IntegralWrapper
{
    using held_type = T;

#ifndef _SKMP_DISABLE_BOOST_SERIALIZATION
    friend class boost::serialization::access;
#endif

    IntegralWrapper()
        : m_item(T(0))
    {
    }

    IntegralWrapper(const T a_rhs)
        : m_item(a_rhs)
    {
    }

    IntegralWrapper& operator=(const T a_rhs) {
        m_item = a_rhs;
        return *this;
    }
    
    operator T() const {
        return m_item;
    }

    T& operator*() {
        return m_item;
    }

    T get() const {
        return m_item;
    }

protected:

    mutable T m_item;

private:

#ifndef _SKMP_DISABLE_BOOST_SERIALIZATION
    template<class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& m_item;
    }

#endif
};

/*template <class T>
bool operator==(const IntegralWrapper<T>& lhs, const IntegralWrapper<T>& rhs) {
    return lhs.m_item == rhs.m_item;
}

template <class T>
bool operator<(const IntegralWrapper<T>& lhs, const IntegralWrapper<T>& rhs) {
    return lhs.m_item < rhs.m_item;
}

template <class T>
bool operator>(const IntegralWrapper<T>& lhs, const IntegralWrapper<T>& rhs) {
    return lhs.m_item > rhs.m_item;
}

template <class T>
bool operator<=(const IntegralWrapper<T>& lhs, const IntegralWrapper<T>& rhs) {
    return lhs.m_item <= rhs.m_item;
}

template <class T>
bool operator>=(const IntegralWrapper<T>& lhs, const IntegralWrapper<T>& rhs) {
    return lhs.m_item >= rhs.m_item;
}*/
