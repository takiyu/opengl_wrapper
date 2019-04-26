#ifndef OGLW_EFFECT_H_190205
#define OGLW_EFFECT_H_190205

#include <array>
#include <memory>
#include <vector>

#include "image.h"

namespace oglw {

// ================================ Effect Base ================================
// class Effect {
// public:
//     virtual ~Effect() {}
//
//     virtual void run(const std::vector<const Image*>& inp_imgs,
//                      const std::vector<Image*>& out_imgs) const = 0;
//
// private:
// };

// ================================ CPU Effect =================================
// template <typename T>
// class CpuEffect : public Effect {
// public:
//     using ValueType = T;
//
//     virtual void run(const std::vector<const Image *>& inp_imgs,
//                      const std::vector<Image *>& out_imgs) = 0;
//
// private:
//     class Impl;
//     std::unique_ptr<Impl> m_impl;
// };

// ================================ GPU Effect =================================
class GpuEffect {
public:
    GpuEffect();
    GpuEffect(size_t n_inps, size_t n_outs, const std::string& vert_src,
              const std::string& frag_src);

    GpuEffect(const GpuEffect&) = delete;  // non-copyable
    GpuEffect(GpuEffect&&);
    GpuEffect& operator=(const GpuEffect&) = delete;  // non-copyable
    GpuEffect& operator=(GpuEffect&&);
    virtual ~GpuEffect();

    void init(size_t n_inps, size_t n_outs, const std::string& vert_src,
              const std::string& frag_src);

    void setInpImgs(const std::vector<const Image*>& inp_imgs);
    void setOutImgs(const std::vector<Image*>& out_imgs);

    enum struct UniformType { VERT, FRAG };
    void setUniform(const std::string& name, bool v,
                    UniformType type = UniformType::FRAG);
    void setUniform(const std::string& name, int v,
                    UniformType type = UniformType::FRAG);
    void setUniform(const std::string& name, unsigned int v,
                    UniformType type = UniformType::FRAG);
    void setUniform(const std::string& name, float v,
                    UniformType type = UniformType::FRAG);
    void setUniform(const std::string& name, const std::array<float, 2>& v,
                    UniformType type = UniformType::FRAG);
    void setUniform(const std::string& name, const std::array<float, 3>& v,
                    UniformType type = UniformType::FRAG);
    void setUniform(const std::string& name, const std::array<float, 4>& v,
                    UniformType type = UniformType::FRAG);
    void setUniform(const std::string& name, const std::array<float, 9>& v,
                    UniformType type = UniformType::FRAG);
    void setUniform(const std::string& name, const std::array<float, 16>& v,
                    UniformType type = UniformType::FRAG);

    void run() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

// ------------------------------ Specialization -------------------------------

}  // namespace oglw

#endif /* end of include guard */
