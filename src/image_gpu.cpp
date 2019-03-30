#include "image.h"

#include <glad/glad.h>

#include <iostream>
#include <cassert>
#include <stdexcept>

#include "image_utils.h"

namespace oglw {

namespace {

template <typename T>
GLenum GetInternalFmt(size_t d);

template <>
GLenum GetInternalFmt<uint8_t>(size_t d) {
    switch (d) {
        case 1: return GL_R8;
        case 2: return GL_RG8;
        case 3: return GL_RGB8;
        case 4: return GL_RGBA8;
    }
    throw std::runtime_error("Invalid depth size for uint8_t");
}

template <>
GLenum GetInternalFmt<Float16>(size_t d) {
    switch (d) {
        case 1: return GL_R16F;
        case 2: return GL_RG16F;
        case 3: return GL_RGB16F;
        case 4: return GL_RGBA16F;
    }
    throw std::runtime_error("Invalid depth size for Float16");
}

template <>
GLenum GetInternalFmt<float>(size_t d) {
    switch (d) {
        case 1: return GL_R32F;
        case 2: return GL_RG32F;
        case 3: return GL_RGB32F;
        case 4: return GL_RGBA32F;
    }
    throw std::runtime_error("Invalid depth size for float");
}

inline void CopyTexture(GLuint src_tex_id, GLuint dst_tex_id, GLsizei src_w,
                        GLsizei src_h, GLsizei src_d, GLint src_x = 0,
                        GLint src_y = 0, GLint src_z = 0, GLint dst_x = 0,
                        GLint dst_y = 0, GLint dst_z = 0) {
    glCopyImageSubData(src_tex_id, GL_TEXTURE_2D, 0, src_x, src_y, src_z,
                       dst_tex_id, GL_TEXTURE_2D, 0, dst_x, dst_y, dst_z, src_w,
                       src_h, src_d);
}

}  // namespace

// ================================= GPU Image =================================

template <typename T>
class GpuImage<T>::Impl {
public:
    Impl() {}
    Impl(size_t w, size_t h, size_t d) {
        init(w, h, d);
    }

    Impl(const Impl& lhs) {
        init(lhs.m_w, lhs.m_h, lhs.m_d);
        CopyTexture(lhs.m_tex_id, m_tex_id, lhs.m_w, lhs.m_h, lhs.m_d);
    }

    Impl(Impl&&) = delete;

    Impl& operator=(const Impl& lhs) {
        if (!IsSameSize(*this, lhs)) {
            release();
            init(lhs.m_w, lhs.m_h, lhs.m_d);
        }
        CopyTexture(lhs.m_tex_id, m_tex_id, lhs.m_w, lhs.m_h, lhs.m_d);
        return *this;
    }

    Impl& operator=(Impl&&) = delete;

    ~Impl() {
        release();
    }

    // -------------------------------------------------------------------------
    void init(size_t w, size_t h, size_t d) {
        // Release forcibly
        release();

        // Zero size
        if (w == 0 || h == 0 || d == 0) {
            return;
        }

        // Create
        glGenTextures(1, &m_tex_id);
        glBindTexture(GL_TEXTURE_2D, m_tex_id);
        glTexStorage2D(GL_TEXTURE_2D, 1, GetInternalFmt<T>(d), w, h);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    bool empty() const {
        return (m_tex_id == 0);
    }

    size_t getWidth() const {
        return m_w;
    }

    size_t getHeight() const {
        return m_h;
    }

    size_t getDepth() const {
        return m_d;
    }

    // -------------------------------------------------------------------------
private:
    void release() {
        if (!empty()) {
            glDeleteTextures(1, &m_tex_id);
        }
    }

    size_t m_w, m_h, m_d;
    GLuint m_tex_id = 0;
};

// -----------------------------------------------------------------------------
// ------------------------------- Pimpl Pattern -------------------------------
// -----------------------------------------------------------------------------
template <typename T>
GpuImage<T>::GpuImage() : m_impl(std::make_unique<Impl>()) {}

template <typename T>
GpuImage<T>::GpuImage(size_t w, size_t h, size_t d)
    : m_impl(std::make_unique<Impl>(w, h, d)) {}

template <typename T>
GpuImage<T>::GpuImage(const GpuImage& lhs)
    : m_impl(std::make_unique<Impl>(*lhs.m_impl)) {}

template <typename T>
GpuImage<T>::GpuImage(GpuImage&&) = default;

template <typename T>
GpuImage<T>& GpuImage<T>::operator=(const GpuImage& lhs) {
    *m_impl = *lhs.m_impl;
    return *this;
}

template <typename T>
GpuImage<T>& GpuImage<T>::operator=(GpuImage&&) = default;

template <typename T>
GpuImage<T>::~GpuImage() = default;

// -----------------------------------------------------------------------------
template <typename T>
void GpuImage<T>::init(size_t w, size_t h, size_t d) {
    m_impl->init(w, h, d);
}

template <typename T>
bool GpuImage<T>::empty() const {
    return m_impl->empty();
}

template <typename T>
size_t GpuImage<T>::getWidth() const {
    return m_impl->getWidth();
}

template <typename T>
size_t GpuImage<T>::getHeight() const {
    return m_impl->getHeight();
}

template <typename T>
size_t GpuImage<T>::getDepth() const {
    return m_impl->getDepth();
}
// -----------------------------------------------------------------------------

}  // namespace oglw
