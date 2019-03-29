#include "fast_array.h"

template <typename T>
FastArray<T>::FastArray() {}

template <typename T>
FastArray<T>::FastArray(size_t n) {
    alloc(n);
}
template <typename T>
FastArray<T>::FastArray(size_t n, const T& v) {
    alloc(n);
    fill(v);
}

template <typename T>
FastArray<T>::FastArray(const FastArray<T>& other) {
    alloc(other.size());
    memcpy(m_data_uc, other.m_data_uc, m_size * sizeof(T));
}

template <typename T>
FastArray<T>& FastArray<T>::operator=(const FastArray<T>& other) {
    alloc(other.size());
    memcpy(m_data_uc, other.m_data_uc, m_size * sizeof(T));
    return *this;
}

template <typename T>
FastArray<T>::FastArray(FastArray<T>&& other) noexcept {
    // Move pointers
    m_data_uc = other.m_data_uc;
    m_data = other.m_data;
    m_size = other.m_size;
    // Clear the other side
    other.m_data_uc = nullptr;
    other.m_data = nullptr;
    other.m_size = 0;
}

template <typename T>
FastArray<T>& FastArray<T>::operator=(FastArray<T>&& other) noexcept {
    clear();
    // Move pointers
    m_data_uc = other.m_data_uc;
    m_data = other.m_data;
    m_size = other.m_size;
    // Clear the other side
    other.m_data_uc = nullptr;
    other.m_data = nullptr;
    other.m_size = 0;
    return *this;
}

template <typename T>
FastArray<T>::~FastArray() {
    clear();
}

template <typename T>
void FastArray<T>::alloc(size_t n) {
    if (m_size == n) {
        return;
    }

    clear();
    if (0 < n) {
        m_data_uc = new unsigned char[n * sizeof(T)];
        m_data = reinterpret_cast<T*>(m_data_uc);
        m_size = n;
    }
}

template <typename T>
void FastArray<T>::clear() {
    if (m_size != 0) {
        delete[] m_data_uc;
        m_data_uc = nullptr;
        m_data = nullptr;
        m_size = 0;
    }
}

template <typename T>
void FastArray<T>::resize(size_t n) {
    if (m_size == n) {
        return;
    }

    if (0 < n) {
        // Create new size array
        FastArray<T> tmp(n);
        // Copy
        memcpy(tmp.m_data_uc, m_data_uc, n * sizeof(T));
        // Overwrite
        *this = std::move(tmp);
    }
}

template <typename T>
void FastArray<T>::resize(size_t n, const T& v) {
    if (m_size == n) {
        return;
    }

    if (0 < n) {
        // Create new size array
        FastArray<T> tmp(n);
        // Copy
        memcpy(tmp.m_data_uc, m_data_uc, n * sizeof(T));
        // Fill the left space
        for (size_t i = m_size; i < n; i++) {
            tmp[i] = v;
        }
        // Overwrite
        *this = std::move(tmp);
    }
}

template <typename T>
void FastArray<T>::fill(const T& v) {
    for (size_t i = 0; i < m_size; i++) {
        m_data[i] = v;
    }
}

template <typename T>
size_t FastArray<T>::size() const {
    return m_size;
}

template <typename T>
bool FastArray<T>::empty() const {
    return m_size == 0;
}

template <typename T>
const T* FastArray<T>::data() const {
    return m_data;
}

template <typename T>
T* FastArray<T>::data() {
    return m_data;
}

template <typename T>
typename FastArray<T>::iterator FastArray<T>::begin() {
    return m_data;
}

template <typename T>
typename FastArray<T>::iterator FastArray<T>::end() {
    return m_data + m_size;
}

template <typename T>
const T& FastArray<T>::operator[](size_t i) const {
    return m_data[i];
}

template <typename T>
T& FastArray<T>::operator[](size_t i) {
    return m_data[i];
}
