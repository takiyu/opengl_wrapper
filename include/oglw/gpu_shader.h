#ifndef OGLW_GPU_SHADER_H_190205
#define OGLW_GPU_SHADER_H_190205

#include <array>
#include <memory>
#include <vector>

#include <oglw/image.h>

namespace oglw {

enum class ShaderType {
    VERTEX,
    FRAGMENT,
};

// ================================ GPU Shader =================================
class GpuShader {
public:
    static auto Create() {
        return std::make_shared<GpuShader>();
    }

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

    template <typename T>
    void setUniform(const std::string& name, const GpuImage<T>& gpu_img) {
        setUniform(name, gpu_img.getTextureId());
    }

    void use() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

// ------------------------------ Pointer Aliases ------------------------------
using GpuShaderPtr = std::shared_ptr<GpuShader>;

}  // namespace oglw

#endif /* end of include guard */
