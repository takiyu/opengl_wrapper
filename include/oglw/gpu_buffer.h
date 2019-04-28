#ifndef OGLW_GPU_BUFFER_H_190428
#define OGLW_GPU_BUFFER_H_190428

#include <memory>

namespace oglw {

enum class BufferTargetType {
    ARRAY,
    INDEX,
};

enum class BufferUsageType {
    DYNAMIC_DRAW,
    STATIC_DRAW,
};

// ================================ GPU Buffer =================================
template <typename T>
class GpuBuffer {
public:
    GpuBuffer();
    GpuBuffer(size_t n_elem, size_t elem_size,
              BufferTargetType tgt_type = BufferTargetType::ARRAY,
              BufferUsageType type = BufferUsageType::DYNAMIC_DRAW);

    GpuBuffer(const GpuBuffer&);
    GpuBuffer(GpuBuffer&&);
    GpuBuffer& operator=(const GpuBuffer&);
    GpuBuffer& operator=(GpuBuffer&&);
    virtual ~GpuBuffer();

    void init(size_t n_elem, size_t elem_size,
              BufferTargetType tgt_type = BufferTargetType::ARRAY,
              BufferUsageType type = BufferUsageType::DYNAMIC_DRAW);
    void sendData(const T* array);

    size_t getNumElem() const;
    size_t getElemSize() const;
    size_t getByteSize() const;
    BufferTargetType getBufferTargetType() const;
    BufferUsageType getBufferUsageType() const;

    int getBufferId() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

// ------------------------------ Specialization -------------------------------
template class GpuBuffer<float>;
template class GpuBuffer<int>;

}  // namespace oglw

#endif
