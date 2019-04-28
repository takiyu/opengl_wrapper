#include <oglw/image.h>

#include <glad/glad.h>

#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <oglw/gl_utils.h>
#include <oglw/image_utils.h>

namespace oglw {

namespace {

// -----------------------------------------------------------------------------
template <typename T>
GLenum GetGlInternalFmt(size_t d);

template <>
GLenum GetGlInternalFmt<uint8_t>(size_t d) {
    switch (d) {
        case 1: return GL_R8;
        case 2: return GL_RG8;
        case 3: return GL_RGB8;
        case 4: return GL_RGBA8;
    }
    std::stringstream ss;
    ss << "Invalid depth size for uint8_t: \"" << d << "\" (internal fmt)";
    throw std::runtime_error(ss.str());
}

template <>
GLenum GetGlInternalFmt<Float16>(size_t d) {
    switch (d) {
        case 1: return GL_R16F;
        case 2: return GL_RG16F;
        case 3: return GL_RGB16F;
        case 4: return GL_RGBA16F;
    }
    std::stringstream ss;
    ss << "Invalid depth size for float16: \"" << d << "\" (internal fmt)";
    throw std::runtime_error(ss.str());
}

template <>
GLenum GetGlInternalFmt<float>(size_t d) {
    switch (d) {
        case 1: return GL_R32F;
        case 2: return GL_RG32F;
        case 3: return GL_RGB32F;
        case 4: return GL_RGBA32F;
    }
    std::stringstream ss;
    ss << "Invalid depth size for float32: \"" << d << "\" (internal fmt)";
    throw std::runtime_error(ss.str());
}

// -----------------------------------------------------------------------------
GLenum GetGlFmt(size_t d) {
    switch (d) {
        case 1: return GL_RED;
        case 2: return GL_RG;
        case 3: return GL_RGB;  // TODO: Compare speed with BGR
        case 4: return GL_RGBA;
    }
    std::stringstream ss;
    ss << "Invalid depth size: \"" << d << "\" (fmt)";
    throw std::runtime_error(ss.str());
}

// -----------------------------------------------------------------------------
template <typename T>
inline GLenum GetGlType();

template <>
inline GLenum GetGlType<uint8_t>() {
    return GL_UNSIGNED_BYTE;
}

template <>
inline GLenum GetGlType<Float16>() {
    return GL_FLOAT;  // float16 in GPU, but float32 in CPU
}

template <>
inline GLenum GetGlType<float>() {
    return GL_FLOAT;
}

// -----------------------------------------------------------------------------
GLint GetGlStoreSize(size_t d) {
    switch (d) {
        case 1: return 1;
        case 2: return 2;
        case 3: return 1;
        case 4: return 4;
    }
    std::stringstream ss;
    ss << "Invalid depth size: \"" << d << "\" (store size)";
    throw std::runtime_error(ss.str());
}

// -----------------------------------------------------------------------------
inline void CopyTexture(GLuint src_tex_id, GLuint dst_tex_id, GLsizei src_w,
                        GLsizei src_h, GLsizei src_d, GLint src_x = 0,
                        GLint src_y = 0, GLint src_z = 0, GLint dst_x = 0,
                        GLint dst_y = 0, GLint dst_z = 0) {
    if (0 < src_w && 0 < src_h && 0 < src_d) {
        OGLW_CHECK(glCopyImageSubData, src_tex_id, GL_TEXTURE_2D, 0, src_x,
                   src_y, src_z, dst_tex_id, GL_TEXTURE_2D, 0, dst_x, dst_y,
                   dst_z, src_w, src_h, src_d);
    }
}

// -----------------------------------------------------------------------------

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
    CpuImage<T> toCpu() const {
        // Copy GPU -> CPU
        // Cost of allocation is almost zero because of FastArray.
        if (empty()) {
            return {};
        }

        GLuint fbo_id;
        OGLW_CHECK(glGenFramebuffers, 1, &fbo_id);
        OGLW_CHECK(glBindFramebuffer, GL_FRAMEBUFFER, fbo_id);
        OGLW_CHECK(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                   GL_TEXTURE_2D, m_tex_id, 0);

        CpuImage<T> cpu_img(m_w, m_h, m_d);

        OGLW_CHECK(glPixelStorei, GL_PACK_ALIGNMENT, GetGlStoreSize(m_d));
        OGLW_CHECK(glReadPixels, 0, 0, m_w, m_h, GetGlFmt(m_d), GetGlType<T>(),
                   cpu_img.data());
        OGLW_CHECK(glBindFramebuffer, GL_FRAMEBUFFER, 0);
        OGLW_CHECK(glDeleteFramebuffers, 1, &fbo_id);

        return std::move(cpu_img);
    }

    void fromCpu(const CpuImage<T>& cpu_img) {
        // Copy CPU -> GPU
        if (!IsSameSize(*this, cpu_img)) {
            init(cpu_img.getWidth(), cpu_img.getHeight(), cpu_img.getDepth());
        }
        OGLW_CHECK(glPixelStorei, GL_UNPACK_ALIGNMENT, GetGlStoreSize(m_d));
        OGLW_CHECK(glTexSubImage2D, GL_TEXTURE_2D, 0, 0, 0, m_w, m_h,
                   GetGlFmt(m_d), GetGlType<T>(), cpu_img.data());
    }

    // -------------------------------------------------------------------------
    void init(size_t w, size_t h, size_t d) {
        // Release forcibly
        release();

        m_w = w;
        m_h = h;
        m_d = d;

        // Zero size
        if (w == 0 || h == 0 || d == 0) {
            return;
        }

        // Create
        OGLW_CHECK(glGenTextures, 1, &m_tex_id);
        OGLW_CHECK(glBindTexture, GL_TEXTURE_2D, m_tex_id);
        OGLW_CHECK(glTexStorage2D, GL_TEXTURE_2D, 1, GetGlInternalFmt<T>(d), w,
                   h);
        OGLW_CHECK(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                   GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        // OGLW_CHECK(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
        // GL_LINEAR); OGLW_CHECK(glTexParameteri, GL_TEXTURE_2D,
        // GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        OGLW_CHECK(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        OGLW_CHECK(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
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
    int getTextureId() const {
        return static_cast<int>(m_tex_id);
    }

    // -------------------------------------------------------------------------
private:
    void release() {
        if (!empty()) {
            glDeleteTextures(1, &m_tex_id);
            m_tex_id = 0;
            m_w = 0;
            m_h = 0;
            m_d = 0;
        }
    }

    size_t m_w = 0, m_h = 0, m_d = 0;
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
CpuImage<T> GpuImage<T>::toCpu() const {
    return m_impl->toCpu();
}

template <typename T>
void GpuImage<T>::fromCpu(const CpuImage<T>& cpu_img) {
    m_impl->fromCpu(cpu_img);
}

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
template <typename T>
int GpuImage<T>::getTextureId() const {
    return m_impl->getTextureId();
}

// -----------------------------------------------------------------------------

}  // namespace oglw
