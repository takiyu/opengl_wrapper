#include "image.h"

#include "fast_array.h"

#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

namespace oglw {

namespace {

template <typename T>
void ForeachThread(FastArray<T>& array, size_t w, size_t h, size_t c,
                   std::function<void(size_t x, size_t y, size_t i, T& v)> func,
                   size_t n_worker) {
    std::atomic<size_t> next_y(0);
    std::vector<std::thread> workers(n_worker);
    for (auto&& worker : workers) {
        worker = std::thread([&]() {
            size_t y = 0;
            while ((y = next_y++) < h) {
                auto itr = array.begin();
                std::advance(itr, y * w * c);
                for (size_t x = 0; x < w; x++) {
                    for (size_t i = 0; i < c; i++) {
                        func(x, y, i, *itr++);
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
void ForeachThread(FastArray<T>& array, size_t w, size_t h, size_t c,
                   std::function<void(size_t x, size_t y, T* channel_vs)> func,
                   size_t n_worker) {
    std::atomic<size_t> next_y(0);
    std::vector<std::thread> workers(n_worker);
    for (auto&& worker : workers) {
        worker = std::thread([&]() {
            size_t y = 0;
            while ((y = next_y++) < h) {
                auto itr = array.begin();
                std::advance(itr, y * w * c);
                for (size_t x = 0; x < w; x++, itr += c) {
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
void ForeachSimple(FastArray<T>& array, size_t w, size_t h, size_t c,
                   std::function<void(size_t x, size_t y, size_t i, T& v)> func,
                   size_t n_worker) {
    auto itr = array.begin();
    for (size_t y = 0; y < w; y++) {
        for (size_t x = 0; x < w; x++) {
            for (size_t i = 0; i < c; i++) {
                func(x, y, i, *itr++);
            }
        }
    }
}

template <typename T>
void ForeachSimple(FastArray<T>& array, size_t w, size_t h, size_t c,
                   std::function<void(size_t x, size_t y, T* channel_vs)> func,
                   size_t n_worker) {
    auto itr = array.begin();
    for (size_t y = 0; y < w; y++) {
        for (size_t x = 0; x < w; x++, itr += c) {
            func(x, y, itr);
        }
    }
}

// template <typename Base, typename T>
// inline bool isinstance(const T *ptr) {
//     return dynamic_cast<const Base*>(ptr) != nullptr;
// }

}  // namespace

// // ================================= Image Base
// ================================ class ImageImpl { public:
//     virtual void init(size_t w, size_t h, size_t c, DatType dat_type) = 0;
// protected:
//     DatType m_dat_type = DatType::UINT8;
// };
//
// // ================================= CPU Image
// ================================= class ImageImplCPU : public ImageImpl {
// public:
//     void init(size_t w, size_t h, size_t c, DatType dat_type);
// private:
//     Any m_array;
// };
//
// void ImageImplCPU::init(size_t w, size_t h, size_t c, DatType dat_type) {
//     m_dat_type = dat_type;
//     // Create array instance
//     if (dat_type == DatType::UINT8) {
//         m_array = FastArray<uint8_t>(w * h * c);
//     } else if (dat_type == DatType::FLOAT32) {
//         m_array = FastArray<float>(w * h * c);
//     } else {
//         std::cerr << "Invalid oglw::DatType" << std::endl;
//     }
// }
//
// // ================================= GPU Image
// ================================= class ImageImplGPU : public ImageImpl {
// public:
//     void init(size_t w, size_t h, size_t c, DatType dat_type);
// private:
//     GLuint m_tex_id;
// };
//
// void ImageImplGPU::init(size_t w, size_t h, size_t c, DatType dat_type) {
//     m_dat_type = dat_type;
//
//     glActiveTexture(GL_TEXTURE0);
//     glGenTextures(1, &m_tex_id);
//     glBindTexture(GL_TEXTURE_2D, m_tex_id);
// #ifdef __APPLE__
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
//     GL_UNSIGNED_BYTE, NULL);
// #else
//     glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, w, h);
// #endif
// //     glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA,
// GL_UNSIGNED_BYTE, data);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
// }
//
// // ============================== Image Interface
// ============================== Image::Image(MemType mem_type) {
//     // Create impl instance
//     if (mem_type == MemType::CPU) {
//         m_impl = std::make_unique<ImageImplCPU>();
//     } else if (mem_type == MemType::GPU) {
//         m_impl = std::make_unique<ImageImplGPU>();
//     } else {
//         std::cerr << "Invalid oglw::MemType" << std::endl;
//     }
// }
//
// void Image::init(size_t w, size_t h, size_t c, DatType dat_type) {
//     m_impl->init(w, h, c, dat_type);
// }
//
// void Image::toCPU() {
// }
//
// void Image::toGPU() {
// }

// ================================= CPU Image =================================

template <typename T>
class CpuImage<T>::Impl {
public:
    Impl() {}
    Impl(size_t w, size_t h, size_t c) {
        init(w, h, c);
    }

    Impl(const Impl&) = default;
    Impl(Impl&&) = delete;
    Impl& operator=(const Impl&) = default;
    Impl& operator=(Impl&&) = delete;
    ~Impl() = default;

    // -------------------------------------------------------------------------
    void init(size_t w, size_t h, size_t c) {
        m_w = w;
        m_h = h;
        m_c = c;
        m_array.alloc(w * h * c);
    }

    size_t getWidth() const {
        return m_w;
    }

    size_t getHeight() const {
        return m_h;
    }

    size_t getChannels() const {
        return m_c;
    }

    // -------------------------------------------------------------------------
    const T& at(size_t x, size_t y, size_t i) const {
        return m_array[(y * m_w + x) * m_c + i];
    }
    T& at(size_t x, size_t y, size_t i) {
        return m_array[(y * m_w + x) * m_c + i];
    }

    // -------------------------------------------------------------------------
    void foreach (std::function<void(size_t x, size_t y, size_t i, T& v)> func,
                  size_t n_worker) {
        if (n_worker <= 0) {
            n_worker = std::thread::hardware_concurrency();
        }
        if (n_worker == 1) {
            ForeachThread(m_array, m_w, m_h, m_c, func, n_worker);
        } else {
            ForeachSimple(m_array, m_w, m_h, m_c, func, n_worker);
        }
    }

    void foreach (std::function<void(size_t x, size_t y, T* channel_vs)> func,
                  size_t n_worker) {
        if (n_worker <= 0) {
            n_worker = std::thread::hardware_concurrency();
        }
        if (n_worker == 1) {
            ForeachThread(m_array, m_w, m_h, m_c, func, n_worker);
        } else {
            ForeachSimple(m_array, m_w, m_h, m_c, func, n_worker);
        }
    }

    // -------------------------------------------------------------------------
private:
    size_t m_w, m_h, m_c;
    FastArray<T> m_array;
};

// -----------------------------------------------------------------------------
// ------------------------------- Pimpl Pattern -------------------------------
// -----------------------------------------------------------------------------
template <typename T>
CpuImage<T>::CpuImage() : m_impl(std::make_unique<Impl>()) {}

template <typename T>
CpuImage<T>::CpuImage(size_t w, size_t h, size_t c)
    : m_impl(std::make_unique<Impl>(w, h, c)) {}

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
void CpuImage<T>::init(size_t w, size_t h, size_t c) {
    m_impl->init(w, h, c);
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
size_t CpuImage<T>::getChannels() const {
    return m_impl->getChannels();
}

// -----------------------------------------------------------------------------
template <typename T>
const T& CpuImage<T>::at(size_t x, size_t y, size_t i) const {
    return m_impl->at(x, y, i);
}

template <typename T>
T& CpuImage<T>::at(size_t x, size_t y, size_t i) {
    return m_impl->at(x, y, i);
}

// -----------------------------------------------------------------------------
template <typename T>
void CpuImage<T>::foreach (
        std::function<void(size_t x, size_t y, size_t i, T& v)> func,
        size_t n_worker) {
    m_impl->foreach (func, n_worker);
}

template <typename T>
void CpuImage<T>::foreach (
        std::function<void(size_t x, size_t y, size_t i, const T& v)> func,
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

// ================================= GPU Image =================================

}  // namespace oglw
