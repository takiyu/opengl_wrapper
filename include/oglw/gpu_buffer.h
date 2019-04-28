#ifndef OGLW_GPU_BUFFER_H_190428
#define OGLW_GPU_BUFFER_H_190428

#include <memory>

namespace oglw {

enum class BufferType {
    ARRAY,
    INDEX,
};

enum class BufferUsageType {
    DYNAMIC_DRAW,
    STATIC_DRAW,
};

// ============================== GPU Buffer Base ==============================
class GpuBufferBase {
public:
    virtual ~GpuBufferBase() {}
    virtual size_t getNumElem() const = 0;
    virtual size_t getElemSize() const = 0;
    virtual size_t getByteSize() const = 0;
    virtual const std::type_info* getDataType() const = 0;
    virtual BufferType getBufferType() const = 0;
    virtual BufferUsageType getBufferUsageType() const = 0;
    virtual unsigned int getBufferId() const = 0;
};

// ================================ GPU Buffer =================================
template <typename T, BufferType B>
class GpuBuffer : public GpuBufferBase {
public:
    GpuBuffer();
    GpuBuffer(size_t n_elem, size_t elem_size,
              BufferUsageType type = BufferUsageType::DYNAMIC_DRAW);

    GpuBuffer(const GpuBuffer&);
    GpuBuffer(GpuBuffer&&);
    GpuBuffer& operator=(const GpuBuffer&);
    GpuBuffer& operator=(GpuBuffer&&);
    virtual ~GpuBuffer();

    void init(size_t n_elem, size_t elem_size,
              BufferUsageType type = BufferUsageType::DYNAMIC_DRAW);
    void sendData(const T* array);

    virtual size_t getNumElem() const;
    virtual size_t getElemSize() const;
    virtual size_t getByteSize() const;

    virtual const std::type_info* getDataType() const;
    virtual BufferType getBufferType() const;
    virtual BufferUsageType getBufferUsageType() const;

    virtual unsigned int getBufferId() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

// ---------------------------------- Aliases ----------------------------------
template <typename T>
using GpuArrayBuffer = GpuBuffer<T, BufferType::ARRAY>;
using GpuIndexBuffer = GpuBuffer<unsigned int, BufferType::ARRAY>;

// ------------------------------ Specialization -------------------------------
template class GpuBuffer<float, BufferType::ARRAY>;
template class GpuBuffer<int, BufferType::ARRAY>;
template class GpuBuffer<unsigned int, BufferType::ARRAY>;
template class GpuBuffer<unsigned int, BufferType::INDEX>;

}  // namespace oglw

#endif
