#ifndef OGLW_GPU_SHADER_H_190205
#define OGLW_GPU_SHADER_H_190205

#include <array>
#include <memory>
#include <vector>

#include "image.h"

namespace oglw {

// ================================ GPU Shader =================================
class GpuShader {
public:
    enum ShaderType {
        VERTEX, FRAGMENT
    };

    GpuShader();

    GpuShader(const GpuShader&) = delete;  // non-copyable
    GpuShader(GpuShader&&);
    GpuShader& operator=(const GpuShader&) = delete;  // non-copyable
    GpuShader& operator=(GpuShader&&);
    virtual ~GpuShader();

    void attach(ShaderType type, const std::string& source);
    void link();

    void setUniform(const std::string& name, bool v);
    void setUniform(const std::string& name, int v);
    void setUniform(const std::string& name, unsigned int v);
    void setUniform(const std::string& name, float v);
    void setUniform(const std::string& name, const std::array<float, 2>& v);
    void setUniform(const std::string& name, const std::array<float, 3>& v);
    void setUniform(const std::string& name, const std::array<float, 4>& v);
    void setUniform(const std::string& name, const std::array<float, 9>& v);
    void setUniform(const std::string& name, const std::array<float, 16>& v);

    void run() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

// ------------------------------ Specialization -------------------------------

}  // namespace oglw

#endif /* end of include guard */
