#include "oglw.h"

#include "fast_array.h"
#include "any.h"

#include <iostream>

namespace oglw {

namespace {

template <typename Base, typename T>
inline bool isinstance(const T *ptr) {
    return dynamic_cast<const Base*>(ptr) != nullptr;
}

}  // namespace anonymous

// ================================= Image Base ================================
class ImageImpl {
public:
    virtual void init(size_t w, size_t h, size_t c, DatType dat_type) = 0;
protected:
    DatType m_dat_type = DatType::UINT8;
};

// ================================= CPU Image =================================
class ImageImplCPU : public ImageImpl {
public:
    void init(size_t w, size_t h, size_t c, DatType dat_type);
private:
    Any m_array;
};

void ImageImplCPU::init(size_t w, size_t h, size_t c, DatType dat_type) {
    m_dat_type = dat_type;
    // Create array instance
    if (dat_type == DatType::UINT8) {
        m_array = FastArray<uint8_t>(w * h * c);
    } else if (dat_type == DatType::FLOAT32) {
        m_array = FastArray<float>(w * h * c);
    } else {
        std::cerr << "Invalid oglw::DatType" << std::endl;
    }
}

// ================================= GPU Image =================================
class ImageImplGPU : public ImageImpl {
public:
    void init(size_t w, size_t h, size_t c, DatType dat_type);
private:
    GLuint m_tex_id;
};

void ImageImplGPU::init(size_t w, size_t h, size_t c, DatType dat_type) {
    m_dat_type = dat_type;

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &m_tex_id);
    glBindTexture(GL_TEXTURE_2D, m_tex_id);
#ifdef __APPLE__
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#else
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, w, h);
#endif
//     glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

// ============================== Image Interface ==============================
Image::Image(MemType mem_type) {
    // Create impl instance
    if (mem_type == MemType::CPU) {
        m_impl = std::make_unique<ImageImplCPU>();
    } else if (mem_type == MemType::GPU) {
        m_impl = std::make_unique<ImageImplGPU>();
    } else {
        std::cerr << "Invalid oglw::MemType" << std::endl;
    }
}

void Image::init(size_t w, size_t h, size_t c, DatType dat_type) {
    m_impl->init(w, h, c, dat_type);
}

void Image::toCPU() {
}

void Image::toGPU() {
}


}  // namespace oglw
