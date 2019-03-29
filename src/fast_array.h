#ifndef FAST_ARRAY_H_190209
#define FAST_ARRAY_H_190209

#include <cstdlib>
#include <cstring>
#include <memory>

namespace oglw {

template <typename T>
class FastArray {
public:
    using ValueType = T;
    using Iterator = T*;

    FastArray();
    FastArray(size_t n);
    FastArray(size_t n, const T& v);
    FastArray(const FastArray& lhs);
    FastArray& operator=(const FastArray& lhs);
    FastArray(FastArray&& lhs) noexcept;
    FastArray& operator=(FastArray&& lhs) noexcept;
    ~FastArray();

    void alloc(size_t n);  // Allocate without copy
    void clear();
    void resize(size_t n); // Resize with copy
    void resize(size_t n, const T& v);  // Resize with copy and fill
    void fill(const T& v);

    size_t size() const;
    bool empty() const;
    const T* data() const;
    T* data();

    Iterator begin();
    Iterator end();

    const T& operator[](size_t i) const;
    T& operator[](size_t i);

private:
    unsigned char* m_data_uc = nullptr;
    T* m_data = nullptr;
    size_t m_size = 0;
};

#include "fast_array_impl.h"

}  // namespace oglw

#endif  // FAST_ARRAY_H_190209
