#ifndef OGLW_H_190205

#define OGLW_H_190205

#include <glad/glad.h>
#include <memory>

namespace oglw {

enum class MemType { CPU, GPU };
enum class DatType { UINT8, FLOAT32 };

class Image {
    Image() {}
    virtual ~Image() {}

    virtual void init(size_t w, size_t h, size_t c, DatType dat_type) = 0;
};

class CpuImage : public Image {
    CpuImage();
    virtual ~CpuImage() {}

    virtual void init(size_t w, size_t h, size_t c, DatType dat_type) = 0;
};

class GpuImage : public Image  {
    GpuImage();
    virtual ~GpuImage() {}

    virtual void init(size_t w, size_t h, size_t c, DatType dat_type) = 0;
};

class Effect {
public:
    Effect();

private:
    class Impl;
    std::unique_ptr<ImageImpl> m_impl;
};

}  // namespace oglw

#endif /* end of include guard */
