#ifndef OGLW_GEOMETRY_H_190427
#define OGLW_GEOMETRY_H_190427

#include <memory>

#include <oglw/gpu_buffer.h>
#include <oglw/gpu_shader.h>

namespace oglw {

enum class PrimitiveType {
    TRIANGLE,
    LINE,
    POINT,
};

// =============================== GPU Geometry ================================
class Geometry {
public:
    static auto Create() {
        return std::make_shared<Geometry>();
    }

    Geometry();
    Geometry(const Geometry&) = delete;  // non-copyable (TODO: Copy)
    Geometry(Geometry&&);
    Geometry& operator=(const Geometry&) = delete;  // non-copyable
    Geometry& operator=(Geometry&&);
    virtual ~Geometry();

    void setArrayBuffer(const GpuBufferBasePtr array_buf, unsigned int index);
    void setIndexBuffer(const GpuIndexBufferPtr index_buf);

    void setPrimitive(PrimitiveType prim_type, float prim_size = 1.f);

    void setShader(const GpuShaderPtr shader);
    void draw();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

// ------------------------------ Pointer Aliases ------------------------------
using GeometryPtr = std::shared_ptr<Geometry>;

}  // namespace oglw

#endif
