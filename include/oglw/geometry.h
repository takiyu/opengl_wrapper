#ifndef OGLW_GEOMETRY_H_190427
#define OGLW_GEOMETRY_H_190427

#include <memory>

#include <oglw/gpu_shader.h>

namespace oglw {

enum class PrimitiveType {
    TRIANGLE, LINE,
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

    void setVertices(const float *vtx_array, size_t n);
    void setIndices(const float *idx_array, size_t n);

    void setShader(const GpuShader& shader);
    void draw(PrimitiveType prim_type) const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace oglw

#endif
