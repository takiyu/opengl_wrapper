#ifndef FAST_ARRAY_H_190209
#define FAST_ARRAY_H_190209

#include <cstdlib>
#include <cstring>
#include <memory>

namespace oglw {

template <typename T>
class FastArray {
public:
    using value_type = T;

    FastArray();
    FastArray(size_t n);
    FastArray(size_t n, const T& v);
    FastArray(const FastArray& other);
    FastArray& operator=(const FastArray& other);
    FastArray(FastArray&& other) noexcept;
    FastArray& operator=(FastArray&& other) noexcept;
    ~FastArray();

    size_t size() const;
    bool empty() const;

    void alloc(size_t n);
    void clear();
    void resize(size_t n);
    void resize(size_t n, const T& v);

    void fill(const T& v);
    const T* data() const;
    T* data();
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
