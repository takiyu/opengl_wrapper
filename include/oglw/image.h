#ifndef OGLW_IMAGE_H_190205
#define OGLW_IMAGE_H_190205

#include <functional>
#include <memory>

#include <oglw/float16.h>

namespace oglw {

template <typename T>
class CpuImage;
template <typename T>
class GpuImage;

// ================================= Image Base ================================
class ImageBase {
public:
    virtual ~ImageBase() {}

    virtual void init(size_t w, size_t h, size_t d) = 0;
    virtual bool empty() const = 0;
    virtual size_t getWidth() const = 0;
    virtual size_t getHeight() const = 0;
    virtual size_t getDepth() const = 0;
};

class CpuImageBase : public ImageBase {
public:
    virtual ~CpuImageBase() {}

    virtual void init(size_t w, size_t h, size_t d) = 0;
    virtual bool empty() const = 0;
    virtual size_t getWidth() const = 0;
    virtual size_t getHeight() const = 0;
    virtual size_t getDepth() const = 0;

    virtual void load(const std::string& filename) = 0;
    virtual void save(const std::string& filename) const = 0;
};

class GpuImageBase : public ImageBase {
public:
    virtual ~GpuImageBase() {}

    virtual void init(size_t w, size_t h, size_t d) = 0;
    virtual bool empty() const = 0;
    virtual size_t getWidth() const = 0;
    virtual size_t getHeight() const = 0;
    virtual size_t getDepth() const = 0;

    virtual int getTextureId() const = 0;
};

// ================================= CPU Image =================================
template <typename T>
class CpuImage : public CpuImageBase {
public:
    using ValueType = T;

    template <typename... Args>
    static auto Create(Args... args) {
        return std::make_shared<CpuImage>(args...);
    }

    CpuImage();
    CpuImage(size_t w, size_t h, size_t d);

    CpuImage(const CpuImage&);
    CpuImage(CpuImage&&);
    CpuImage& operator=(const CpuImage&);
    CpuImage& operator=(CpuImage&&);
    virtual ~CpuImage();

    GpuImage<T> toGpu() const;
    void fromGpu(const GpuImage<T>& gpu_img);

    virtual void init(size_t w, size_t h, size_t d) override;
    virtual bool empty() const override;
    virtual size_t getWidth() const override;
    virtual size_t getHeight() const override;
    virtual size_t getDepth() const override;

    virtual void load(const std::string& filename) override;
    virtual void save(const std::string& filename) const override;

    const T* data() const;
    T* data();
    const T& at(size_t x, size_t y, size_t z) const;
    T& at(size_t x, size_t y, size_t z);

    void foreach (std::function<void(size_t x, size_t y, size_t z, T& v)>,
                  size_t n_worker = 0);
    void foreach (std::function<void(size_t x, size_t y, size_t z, const T& v)>,
                  size_t n_worker = 0) const;
    void foreach (std::function<void(size_t x, size_t y, T* channel_vs)>,
                  size_t n_worker = 0);
    void foreach (std::function<void(size_t x, size_t y, const T* channel_vs)>,
                  size_t n_worker = 0) const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

// ================================= GPU Image =================================
template <typename T>
class GpuImage : public GpuImageBase {
public:
    using ValueType = T;

    template <typename... Args>
    static auto Create(Args... args) {
        return std::make_shared<GpuImage>(args...);
    }

    GpuImage();
    GpuImage(size_t w, size_t h, size_t d);

    GpuImage(const GpuImage&);
    GpuImage(GpuImage&&);
    GpuImage& operator=(const GpuImage&);
    GpuImage& operator=(GpuImage&&);
    virtual ~GpuImage();

    CpuImage<T> toCpu() const;
    void fromCpu(const CpuImage<T>& cpu_img);

    virtual void init(size_t w, size_t h, size_t d) override;
    virtual bool empty() const override;
    virtual size_t getWidth() const override;
    virtual size_t getHeight() const override;
    virtual size_t getDepth() const override;

    virtual int getTextureId() const override;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

// ------------------------------ Pointer Aliases ------------------------------
using ImageBasePtr = std::shared_ptr<ImageBase>;
using CpuImageBasePtr = std::shared_ptr<CpuImageBase>;
using GpuImageBasePtr = std::shared_ptr<GpuImageBase>;
template <typename T>
using GpuImagePtr = std::shared_ptr<GpuImage<T>>;
template <typename T>
using CpuImagePtr = std::shared_ptr<CpuImage<T>>;

// ------------------------------ Specialization -------------------------------
template class CpuImage<uint8_t>;
template class CpuImage<float>;
template class CpuImage<Float16>;
template class GpuImage<uint8_t>;
template class GpuImage<float>;
template class GpuImage<Float16>;

}  // namespace oglw

#endif /* end of include guard */
