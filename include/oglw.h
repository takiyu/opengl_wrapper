#ifndef OGLW_H_190205

#define OGLW_H_190205

#include <glad/glad.h>
#include <memory>

namespace oglw {

enum class MemType { ANY, CPU, GPU };
enum class DatType { ANY, UINT8, FLOAT32 };

class ImageImpl;

class Image {
public:
    Image(MemType mem_type = MemType::ANY);
    void init(size_t w, size_t h, size_t c, DatType dat_type = DatType::UINT8);

    void toCPU();
    void toGPU();

private:
    std::unique_ptr<ImageImpl> m_impl;
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
