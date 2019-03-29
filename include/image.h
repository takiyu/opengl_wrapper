#ifndef OGLW_H_190205
#define OGLW_H_190205

#include <glad/glad.h>
#include <memory>
#include <functional>

namespace oglw {

// ================================= Image Base ================================
class Image {
public:
    virtual ~Image() {}

    virtual void init(size_t w, size_t h, size_t c) = 0;
    virtual size_t getWidth() const = 0;
    virtual size_t getHeight() const = 0;
    virtual size_t getChannels() const = 0;

private:
};

// ================================= CPU Image =================================
template <typename T>
class CpuImage : public Image {
public:
    using ValueType = T;

    CpuImage();
    CpuImage(size_t w, size_t h, size_t c);

    CpuImage(const CpuImage&);
    CpuImage(CpuImage&&);
    CpuImage& operator=(const CpuImage&);
    CpuImage& operator=(CpuImage&&);
    virtual ~CpuImage();

    virtual void init(size_t w, size_t h, size_t c) override;
    virtual size_t getWidth() const override;
    virtual size_t getHeight() const override;
    virtual size_t getChannels() const override;

    const T& at(size_t x, size_t y, size_t i) const;
    T& at(size_t x, size_t y, size_t i);

    void foreach(std::function<void(size_t x, size_t y, size_t i, T& v)>,
                 size_t n_worker = 0);
    void foreach(std::function<void(size_t x, size_t y, size_t i, const T& v)>,
                 size_t n_worker = 0) const;
    void foreach(std::function<void(size_t x, size_t y, T* channel_vs)>,
                 size_t n_worker = 0);
    void foreach(std::function<void(size_t x, size_t y, const T* channel_vs)>,
                 size_t n_worker = 0) const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

// ------------------------------ Specialization -------------------------------
template class CpuImage<uint8_t>;
template class CpuImage<float>;

// ================================= GPU Image =================================
// template <typename T>
// class GpuImage : public Image {
// public:
//     GpuImage();
//     virtual ~GpuImage() {}
//
//     virtual void init(size_t w, size_t h, size_t c);
// private:
// };

}  // namespace oglw

#endif /* end of include guard */
