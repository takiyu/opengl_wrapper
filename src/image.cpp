#include "oglw.h"

#include "fast_array.h"

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
    virtual void init(int w, int h, int c, DatType dat_type) = 0;
protected:
    DatType m_dat_type = DatType::ANY;
};

// ================================= CPU Image =================================
class ImageImplCPU : public ImageImpl {
public:
    void init(int w, int h, int c, DatType dat_type);
private:
//     std::unique_ptr<void> m_array;
};

void ImageImplCPU::init(int w, int h, int c, DatType dat_type) {
    m_dat_type = dat_type;
    // Create array instance
//     if (dat_type == DatType::ANY) {
//     } else if (dat_type == DatType::UINT8) {
//         m_array = std::make_unique<FastArray<uint8_t>>(w * h * c);
//     } else if (dat_type == DatType::FLOAT32) {
//         m_array = std::make_unique<FastArray<float>>(w * h * c);
//     } else {
//         std::cerr << "Invalid oglw::DatType" << std::endl;
//     }
}

// ================================= GPU Image =================================
class ImageImplGPU : public ImageImpl {
public:
    void init(int w, int h, int c, DatType dat_type);
private:
};

void ImageImplGPU::init(int w, int h, int c, DatType dat_type) {
    m_dat_type = dat_type;
}

// ============================== Image Interface ==============================
Image::Image(MemType mem_type) {
    // Create impl instance
    if (mem_type == MemType::ANY) {
        m_impl = nullptr;
    } else if (mem_type == MemType::CPU) {
        m_impl = std::make_unique<ImageImplCPU>();
    } else if (mem_type == MemType::GPU) {
        m_impl = std::make_unique<ImageImplGPU>();
    } else {
        std::cerr << "Invalid oglw::MemType" << std::endl;
    }
}

void Image::init(int w, int h, int c, DatType dat_type) {
    m_impl->init(w, h, c, dat_type);
}

void Image::toCPU() {
}

void Image::toGPU() {
}


}  // namespace oglw
