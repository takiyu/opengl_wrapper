#include <oglw/gpu_buffer.h>

#include <glad/glad.h>

#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace oglw {

namespace {

// -----------------------------------------------------------------------------
GLenum GetGlBufferTarget(BufferTargetType type) {
    switch (type) {
        case BufferTargetType::ARRAY: return GL_ARRAY_BUFFER;
        case BufferTargetType::INDEX: return GL_ELEMENT_ARRAY_BUFFER;
    }
    std::stringstream ss;
    ss << "Invalid buffer target type: \"" << static_cast<int>(type) << "\"";
    throw std::runtime_error(ss.str());
}

GLenum GetGlBufferUsage(BufferUsageType type) {
    switch (type) {
        case BufferUsageType::DYNAMIC_DRAW: return GL_DYNAMIC_DRAW;
        case BufferUsageType::STATIC_DRAW: return GL_STATIC_DRAW;
    }
    std::stringstream ss;
    ss << "Invalid buffer usage type: \"" << static_cast<int>(type) << "\"";
    throw std::runtime_error(ss.str());
}

template <typename T>
bool IsSameSizeBuffer(const T& lhs, const T& rhs) {
    return (lhs.getNumElem() == rhs.getNumElem() &&
            lhs.getElemSize() == rhs.getElemSize() &&
            lhs.getByteSize() == rhs.getByteSize() &&
            lhs.getBufferTargetType() == rhs.getBufferTargetType());
}

// -----------------------------------------------------------------------------
inline void CopyBuffer(GLuint src_buf_id, GLuint dst_buf_id, GLsizei size) {
    if (0 < size) {
        glCopyBufferSubData(src_buf_id, dst_buf_id, 0, 0, size);
    }
}

// -----------------------------------------------------------------------------

}  // namespace

// ================================= GPU Image =================================

template <typename T>
class GpuBuffer<T>::Impl {
public:
    Impl() {}
    Impl(size_t n_elem, size_t elem_size, BufferTargetType tgt_type,
         BufferUsageType usg_type) {
        init(n_elem, elem_size, tgt_type, usg_type);
    }

    Impl(const Impl& lhs) {
        init(lhs.m_num_elem, lhs.m_elem_size, lhs.m_tgt_type, lhs.m_usg_type);
        CopyBuffer(lhs.m_buf_id, m_buf_id, getByteSize());
    }

    Impl(Impl&&) = delete;

    Impl& operator=(const Impl& lhs) {
        if (!IsSameSizeBuffer(*this, lhs)) {
            init(lhs.m_num_elem, lhs.m_elem_size, lhs.m_tgt_type,
                 lhs.m_usg_type);
        }
        CopyBuffer(lhs.m_buf_id, m_buf_id, getByteSize());
        return *this;
    }

    Impl& operator=(Impl&&) = delete;

    ~Impl() {
        release();
    }

    // -------------------------------------------------------------------------
    void init(size_t n_elem, size_t elem_size, BufferTargetType tgt_type,
              BufferUsageType usg_type) {
        // Release forcibly
        release();

        m_num_elem = n_elem;
        m_elem_size = elem_size;
        m_tgt_type = tgt_type;
        m_usg_type = usg_type;

        // Zero size
        if (m_num_elem == 0 || m_elem_size == 0) {
            return;
        }

        // Create
        glGenBuffers(1, &m_buf_id);
        glBindBuffer(GetGlBufferTarget(m_tgt_type), m_buf_id);
        glBufferData(GetGlBufferTarget(m_tgt_type),
                     static_cast<GLsizeiptr>(getByteSize()), nullptr,
                     GetGlBufferUsage(m_usg_type));
    }

    void sendData(const T* array) {
        glBufferSubData(GetGlBufferTarget(m_tgt_type), 0,
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

    BufferTargetType getBufferTargetType() const {
        return m_tgt_type;
    }

    BufferUsageType getBufferUsageType() const {
        return m_usg_type;
    }

    // -------------------------------------------------------------------------
    int getBufferId() const {
        return static_cast<int>(m_buf_id);
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
    BufferTargetType m_tgt_type;
    BufferUsageType m_usg_type;
    GLuint m_buf_id = 0;
};

// -----------------------------------------------------------------------------
// ------------------------------- Pimpl Pattern -------------------------------
// -----------------------------------------------------------------------------
template <typename T>
GpuBuffer<T>::GpuBuffer() : m_impl(std::make_unique<Impl>()) {}

template <typename T>
GpuBuffer<T>::GpuBuffer(size_t n_elem, size_t elem_size,
                        BufferTargetType tgt_type, BufferUsageType usg_type)
    : m_impl(std::make_unique<Impl>(n_elem, elem_size, tgt_type, usg_type)) {}

template <typename T>
GpuBuffer<T>::GpuBuffer(const GpuBuffer& lhs)
    : m_impl(std::make_unique<Impl>(*lhs.m_impl)) {}

template <typename T>
GpuBuffer<T>::GpuBuffer(GpuBuffer&&) = default;

template <typename T>
GpuBuffer<T>& GpuBuffer<T>::operator=(const GpuBuffer& lhs) {
    *m_impl = *lhs.m_impl;
    return *this;
}

template <typename T>
GpuBuffer<T>& GpuBuffer<T>::operator=(GpuBuffer&&) = default;

template <typename T>
GpuBuffer<T>::~GpuBuffer() = default;

// -----------------------------------------------------------------------------
template <typename T>
void GpuBuffer<T>::init(size_t n_elem, size_t elem_size,
                        BufferTargetType tgt_type, BufferUsageType usg_type) {
    m_impl->init(n_elem, elem_size, tgt_type, usg_type);
}

template <typename T>
void GpuBuffer<T>::sendData(const T* array) {
    m_impl->sendData(array);
}

// -----------------------------------------------------------------------------
template <typename T>
size_t GpuBuffer<T>::getNumElem() const {
    return m_impl->getNumElem();
}

template <typename T>
size_t GpuBuffer<T>::getElemSize() const {
    return m_impl->getElemSize();
}

template <typename T>
size_t GpuBuffer<T>::getByteSize() const {
    return m_impl->getByteSize();
}

template <typename T>
BufferTargetType GpuBuffer<T>::getBufferTargetType() const {
    return m_impl->getBufferTargetType();
}

template <typename T>
BufferUsageType GpuBuffer<T>::getBufferUsageType() const {
    return m_impl->getBufferUsageType();
}

// -----------------------------------------------------------------------------
template <typename T>
int GpuBuffer<T>::getBufferId() const {
    return m_impl->getBufferId();
}

// -----------------------------------------------------------------------------

}  // namespace oglw
