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
    Geometry();

    Geometry(const Geometry&) = delete;  // non-copyable (TODO: Copy)
    Geometry(Geometry&&);
    Geometry& operator=(const Geometry&) = delete;  // non-copyable
    Geometry& operator=(Geometry&&);
    virtual ~Geometry();

    void setArrayBuffer(const std::shared_ptr<GpuBufferBase> array_buf,
                        unsigned int index);
    void setIndexBuffer(const std::shared_ptr<GpuIndexBuffer> index_buf);

    void setShader(const std::shared_ptr<GpuShader> shader);
    void draw(PrimitiveType prim_type, float prim_size = 1.f);

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace oglw

#endif
