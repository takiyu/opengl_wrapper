#include <oglw/gpu_buffer.h>

#include <glad/glad.h>

#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <oglw/gl_utils.h>

namespace oglw {

namespace {

// -----------------------------------------------------------------------------
template <BufferType B>
GLenum GetGlBufferTarget();

template <>
GLenum GetGlBufferTarget<BufferType::ARRAY>() {
    return GL_ARRAY_BUFFER;
}

template <>
GLenum GetGlBufferTarget<BufferType::INDEX>() {
    return GL_ELEMENT_ARRAY_BUFFER;
}

// -----------------------------------------------------------------------------
GLenum GetGlBufferUsage(BufferUsageType type) {
    switch (type) {
        case BufferUsageType::DYNAMIC_DRAW: return GL_DYNAMIC_DRAW;
        case BufferUsageType::STATIC_DRAW: return GL_STATIC_DRAW;
    }
    std::stringstream ss;
    ss << "Invalid buffer usage type: \"" << static_cast<int>(type) << "\"";
    throw std::runtime_error(ss.str());
}

// -----------------------------------------------------------------------------
template <typename T>
bool IsSameSizeBuffer(const T& lhs, const T& rhs) {
    return (lhs.getNumElem() == rhs.getNumElem() &&
            lhs.getElemSize() == rhs.getElemSize() &&
            lhs.getByteSize() == rhs.getByteSize());
}

// -----------------------------------------------------------------------------
inline void CopyBuffer(GLuint src_buf_id, GLuint dst_buf_id, GLsizei size) {
    if (0 < size) {
        OGLW_CHECK(glCopyBufferSubData, src_buf_id, dst_buf_id, 0, 0, size);
    }
}

// -----------------------------------------------------------------------------

}  // namespace

// ================================= GPU Image =================================

template <typename T, BufferType B>
class GpuBuffer<T, B>::Impl {
public:
    Impl() {}
    Impl(size_t n_elem, size_t elem_size, BufferUsageType usg_type) {
        init(n_elem, elem_size, usg_type);
    }

    Impl(const Impl& lhs) {
        init(lhs.m_num_elem, lhs.m_elem_size, lhs.m_usg_type);
        CopyBuffer(lhs.m_buf_id, m_buf_id, getByteSize());
    }

    Impl(Impl&&) = delete;

    Impl& operator=(const Impl& lhs) {
        if (!IsSameSizeBuffer(*this, lhs)) {
            init(lhs.m_num_elem, lhs.m_elem_size, lhs.m_usg_type);
        }
        CopyBuffer(lhs.m_buf_id, m_buf_id, getByteSize());
        return *this;
    }

    Impl& operator=(Impl&&) = delete;

    ~Impl() {
        release();
    }

    // -------------------------------------------------------------------------
    void init(size_t n_elem, size_t elem_size, BufferUsageType usg_type) {
        // Release forcibly
        release();

        m_num_elem = n_elem;
        m_elem_size = elem_size;
        m_usg_type = usg_type;

        // Zero size
        if (m_num_elem == 0 || m_elem_size == 0) {
            return;
        }

        // Create
        OGLW_CHECK(glGenBuffers, 1, &m_buf_id);
        OGLW_CHECK(glBindBuffer, GetGlBufferTarget<B>(), m_buf_id);
        OGLW_CHECK(glBufferData, GetGlBufferTarget<B>(),
                   static_cast<GLsizeiptr>(getByteSize()), nullptr,
                   GetGlBufferUsage(m_usg_type));
    }

    void sendData(const T* array) {
        OGLW_CHECK(glBufferSubData, GetGlBufferTarget<B>(), 0,
                   static_cast<GLsizeiptr>(getByteSize()), array);
    }

    // -------------------------------------------------------------------------
    size_t getNumElem() const {
        return m_num_elem;
    }

    size_t getElemSize() const {
        return m_elem_size;
    }

    size_t getByteSize() const {
        return m_num_elem * m_elem_size * sizeof(T);
    }

    const std::type_info* getDataType() const {
        return &typeid(T);
    }

    BufferType getBufferType() const {
        return B;
    }

    BufferUsageType getBufferUsageType() const {
        return m_usg_type;
    }

    // -------------------------------------------------------------------------
    unsigned int getBufferId() const {
        return m_buf_id;
    }

    // -------------------------------------------------------------------------
private:
    void release() {
        if (0 < m_buf_id) {
            glDeleteBuffers(1, &m_buf_id);
            m_buf_id = 0;
            m_num_elem = 0;
            m_elem_size = 0;
        }
    }

    size_t m_num_elem = 0, m_elem_size = 0;
    BufferUsageType m_usg_type;
    GLuint m_buf_id = 0;
};

// -----------------------------------------------------------------------------
// ------------------------------- Pimpl Pattern -------------------------------
// -----------------------------------------------------------------------------
template <typename T, BufferType B>
GpuBuffer<T, B>::GpuBuffer() : m_impl(std::make_unique<Impl>()) {}

template <typename T, BufferType B>
GpuBuffer<T, B>::GpuBuffer(size_t n_elem, size_t elem_size,
                           BufferUsageType usg_type)
    : m_impl(std::make_unique<Impl>(n_elem, elem_size, usg_type)) {}

template <typename T, BufferType B>
GpuBuffer<T, B>::GpuBuffer(const GpuBuffer& lhs)
    : m_impl(std::make_unique<Impl>(*lhs.m_impl)) {}

template <typename T, BufferType B>
GpuBuffer<T, B>::GpuBuffer(GpuBuffer&&) = default;

template <typename T, BufferType B>
GpuBuffer<T, B>& GpuBuffer<T, B>::operator=(const GpuBuffer& lhs) {
    *m_impl = *lhs.m_impl;
    return *this;
}

template <typename T, BufferType B>
GpuBuffer<T, B>& GpuBuffer<T, B>::operator=(GpuBuffer&&) = default;

template <typename T, BufferType B>
GpuBuffer<T, B>::~GpuBuffer() = default;

// -----------------------------------------------------------------------------
template <typename T, BufferType B>
void GpuBuffer<T, B>::init(size_t n_elem, size_t elem_size,
                           BufferUsageType usg_type) {
    m_impl->init(n_elem, elem_size, usg_type);
}

template <typename T, BufferType B>
void GpuBuffer<T, B>::sendData(const T* array) {
    m_impl->sendData(array);
}

// -----------------------------------------------------------------------------
template <typename T, BufferType B>
size_t GpuBuffer<T, B>::getNumElem() const {
    return m_impl->getNumElem();
}

template <typename T, BufferType B>
size_t GpuBuffer<T, B>::getElemSize() const {
    return m_impl->getElemSize();
}

template <typename T, BufferType B>
size_t GpuBuffer<T, B>::getByteSize() const {
    return m_impl->getByteSize();
}

template <typename T, BufferType B>
const std::type_info* GpuBuffer<T, B>::getDataType() const {
    return m_impl->getDataType();
}

template <typename T, BufferType B>
BufferType GpuBuffer<T, B>::getBufferType() const {
    return m_impl->getBufferType();
}

template <typename T, BufferType B>
BufferUsageType GpuBuffer<T, B>::getBufferUsageType() const {
    return m_impl->getBufferUsageType();
}

// -----------------------------------------------------------------------------
template <typename T, BufferType B>
unsigned int GpuBuffer<T, B>::getBufferId() const {
    return m_impl->getBufferId();
}

// -----------------------------------------------------------------------------

}  // namespace oglw
