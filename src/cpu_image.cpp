#include <oglw/image.h>

// Include STB
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#elif defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wstrict-overflow"
#endif
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#ifdef __clang__
#pragma clang diagnostic pop
#elif __GNUC__
#pragma GCC diagnostic pop
#endif

#include "fast_array.h"

#include <atomic>
#include <thread>
#include <vector>

namespace oglw {

namespace {

// -----------------------------------------------------------------------------
template <typename T>
T CastFromUint8(uint8_t v);

template <>
uint8_t CastFromUint8(uint8_t v) {
    return v;
}

template <>
float CastFromUint8(uint8_t v) {
    return static_cast<float>(v) / 255.f;
}

template <>
Float16 CastFromUint8(uint8_t v) {
    return CastFromUint8<float>(v);
}

// -----------------------------------------------------------------------------
template <typename T>
uint8_t CastToUint8(T v);

template <>
uint8_t CastToUint8(uint8_t v) {
    return v;
}

template <>
uint8_t CastToUint8(float v) {
    return static_cast<uint8_t>(std::min(std::max(v * 255.f, 0.f), 255.f));
}

template <>
uint8_t CastToUint8(Float16 v) {
    return CastFromUint8<float>(v.v);
}

// -----------------------------------------------------------------------------
template <typename T>
void ForeachThread(FastArray<T>& array, size_t w, size_t h, size_t d,
                   std::function<void(size_t x, size_t y, size_t z, T& v)> func,
                   size_t n_worker) {
    std::atomic<size_t> next_y(0);
    std::vector<std::thread> workers(n_worker);
    for (auto&& worker : workers) {
        worker = std::thread([&]() {
            size_t y = 0;
            while ((y = next_y++) < h) {
                auto itr = array.begin();
                std::advance(itr, y * w * d);
                for (size_t x = 0; x < w; x++) {
                    for (size_t z = 0; z < d; z++) {
                        func(x, y, z, *itr++);
                    }
                }
            }
        });
    }
    for (auto&& worker : workers) {
        worker.join();
    }
}

template <typename T>
void ForeachThread(FastArray<T>& array, size_t w, size_t h, size_t d,
                   std::function<void(size_t x, size_t y, T* channel_vs)> func,
                   size_t n_worker) {
    std::atomic<size_t> next_y(0);
    std::vector<std::thread> workers(n_worker);
    for (auto&& worker : workers) {
        worker = std::thread([&]() {
            size_t y = 0;
            while ((y = next_y++) < h) {
                auto itr = array.begin();
                std::advance(itr, y * w * d);
                for (size_t x = 0; x < w; x++, itr += d) {
                    func(x, y, itr);
                }
            }
        });
    }
    for (auto&& worker : workers) {
        worker.join();
    }
}

template <typename T>
void ForeachSimple(
        FastArray<T>& array, size_t w, size_t h, size_t d,
        std::function<void(size_t x, size_t y, size_t z, T& v)> func) {
    auto itr = array.begin();
    for (size_t y = 0; y < h; y++) {
        for (size_t x = 0; x < w; x++) {
            for (size_t z = 0; z < d; z++) {
                func(x, y, z, *itr++);
            }
        }
    }
}

template <typename T>
void ForeachSimple(
        FastArray<T>& array, size_t w, size_t h, size_t d,
        std::function<void(size_t x, size_t y, T* channel_vs)> func) {
    auto itr = array.begin();
    for (size_t y = 0; y < h; y++) {
        for (size_t x = 0; x < w; x++, itr += d) {
            func(x, y, itr);
        }
    }
}

// -----------------------------------------------------------------------------

}  // namespace

// ================================= CPU Image =================================

template <typename T>
class CpuImage<T>::Impl {
public:
    Impl() {}
    Impl(size_t w, size_t h, size_t d) {
        init(w, h, d);
    }

    Impl(const Impl&) = default;
    Impl(Impl&&) = delete;
    Impl& operator=(const Impl&) = default;
    Impl& operator=(Impl&&) = delete;
    ~Impl() = default;

    // -------------------------------------------------------------------------
    void init(size_t w, size_t h, size_t d) {
        m_w = w;
        m_h = h;
        m_d = d;
        m_array.alloc(w * h * d);
    }

    bool empty() const {
        return m_array.empty();
    }

    size_t getWidth() const {
        return m_w;
    }

    size_t getHeight() const {
        return m_h;
    }

    size_t getDepth() const {
        return m_d;
    }

    // -------------------------------------------------------------------------
    void load(const std::string& filename) {
        // Load with STB
        int w_i, h_i, d_i;
        uint8_t* data = stbi_load(filename.c_str(), &w_i, &h_i, &d_i, 0);
        if (!data) {
            throw std::runtime_error("Failed to load: " + filename);
        }
        // Allocate
        const size_t w = static_cast<size_t>(w_i);
        const size_t h = static_cast<size_t>(h_i);
        const size_t d = static_cast<size_t>(d_i);
        init(w, h, d);
        // Cast and set
        foreach (
                [&data, w, h, d](size_t x, size_t y, size_t z, T& v) {
                    // y-flip
                    const size_t idx = (w * (h - y - 1) + x) * d + z;
                    v = CastFromUint8<T>(data[idx]);
                },
                0)
            ;
        // Free
        stbi_image_free(data);
    }

    void save(const std::string& filename) {
        // Cast
        FastArray<uint8_t> u8_img(m_w * m_h * m_d);
        foreach (
                [this, &u8_img](size_t x, size_t y, size_t z, const T& v) {
                    // y-flip
                    const size_t idx = (m_w * (m_h - y - 1) + x) * m_d + z;
                    u8_img.data()[idx] = CastToUint8(v);
                },
                0)
            ;
        // Save with STB
        const int ret = stbi_write_jpg(filename.c_str(), m_w, m_h, m_d,
                                       u8_img.data(), 90);
        if (!ret) {
            throw std::runtime_error("Failed to save: " + filename);
        }
    }

    // -------------------------------------------------------------------------
    T* data() {
        return m_array.data();
    }

    T& at(size_t x, size_t y, size_t z) {
        return m_array[(y * m_w + x) * m_d + z];
    }

    // -------------------------------------------------------------------------
    void foreach (std::function<void(size_t x, size_t y, size_t z, T& v)> func,
                  size_t n_worker) {
        if (n_worker <= 0) {
            n_worker = std::thread::hardware_concurrency();
        }
        if (n_worker == 1) {
            ForeachSimple(m_array, m_w, m_h, m_d, func);
        } else {
            ForeachThread(m_array, m_w, m_h, m_d, func, n_worker);
        }
    }

    void foreach (std::function<void(size_t x, size_t y, T* channel_vs)> func,
                  size_t n_worker) {
        if (n_worker <= 0) {
            n_worker = std::thread::hardware_concurrency();
        }
        if (n_worker == 1) {
            ForeachSimple(m_array, m_w, m_h, m_d, func);
        } else {
            ForeachThread(m_array, m_w, m_h, m_d, func, n_worker);
        }
    }

    // -------------------------------------------------------------------------
private:
    size_t m_w = 0, m_h = 0, m_d = 0;
    FastArray<T> m_array;
};

// -----------------------------------------------------------------------------
// ------------------------------- Pimpl Pattern -------------------------------
// -----------------------------------------------------------------------------
template <typename T>
CpuImage<T>::CpuImage() : m_impl(std::make_unique<Impl>()) {}

template <typename T>
CpuImage<T>::CpuImage(size_t w, size_t h, size_t d)
    : m_impl(std::make_unique<Impl>(w, h, d)) {}

template <typename T>
CpuImage<T>::CpuImage(const CpuImage& lhs)
    : m_impl(std::make_unique<Impl>(*lhs.m_impl)) {}

template <typename T>
CpuImage<T>::CpuImage(CpuImage&&) = default;

template <typename T>
CpuImage<T>& CpuImage<T>::operator=(const CpuImage& lhs) {
    *m_impl = *lhs.m_impl;
    return *this;
}

template <typename T>
CpuImage<T>& CpuImage<T>::operator=(CpuImage&&) = default;

template <typename T>
CpuImage<T>::~CpuImage() = default;

// -----------------------------------------------------------------------------
template <typename T>
GpuImage<T> CpuImage<T>::toGpu() const {
    // Just use GpuImage's implementation
    GpuImage<T> gpu_img;
    gpu_img.fromCpu(*this);
    return std::move(gpu_img);
}

template <typename T>
void CpuImage<T>::fromGpu(const GpuImage<T>& gpu_img) {
    // Just use GpuImage's implementation
    *this = gpu_img.toCpu();
}

// -----------------------------------------------------------------------------
template <typename T>
void CpuImage<T>::init(size_t w, size_t h, size_t d) {
    m_impl->init(w, h, d);
}

template <typename T>
bool CpuImage<T>::empty() const {
    return m_impl->empty();
}

template <typename T>
size_t CpuImage<T>::getWidth() const {
    return m_impl->getWidth();
}

template <typename T>
size_t CpuImage<T>::getHeight() const {
    return m_impl->getHeight();
}

template <typename T>
size_t CpuImage<T>::getDepth() const {
    return m_impl->getDepth();
}

// -----------------------------------------------------------------------------
template <typename T>
void CpuImage<T>::load(const std::string& filename) {
    m_impl->load(filename);
}

template <typename T>
void CpuImage<T>::save(const std::string& filename) const {
    m_impl->save(filename);
}

// -----------------------------------------------------------------------------
template <typename T>
const T* CpuImage<T>::data() const {
    return m_impl->data();
}

template <typename T>
T* CpuImage<T>::data() {
    return m_impl->data();
}

template <typename T>
const T& CpuImage<T>::at(size_t x, size_t y, size_t z) const {
    return m_impl->at(x, y, z);
}

template <typename T>
T& CpuImage<T>::at(size_t x, size_t y, size_t z) {
    return m_impl->at(x, y, z);
}

// -----------------------------------------------------------------------------
template <typename T>
void CpuImage<T>::foreach (
        std::function<void(size_t x, size_t y, size_t z, T& v)> func,
        size_t n_worker) {
    m_impl->foreach (func, n_worker);
}

template <typename T>
void CpuImage<T>::foreach (
        std::function<void(size_t x, size_t y, size_t z, const T& v)> func,
        size_t n_worker) const {
    m_impl->foreach (func, n_worker);
}

template <typename T>
void CpuImage<T>::foreach (
        std::function<void(size_t x, size_t y, T* channel_vs)> func,
        size_t n_worker) {
    m_impl->foreach (func, n_worker);
}

template <typename T>
void CpuImage<T>::foreach (
        std::function<void(size_t x, size_t y, const T* channel_vs)> func,
        size_t n_worker) const {
    m_impl->foreach (func, n_worker);
}

// -----------------------------------------------------------------------------

}  // namespace oglw
