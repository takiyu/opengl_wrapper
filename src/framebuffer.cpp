#include <oglw/framebuffer.h>

#include <oglw/gl_utils.h>

#include <glad/glad.h>

#include <cassert>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>

namespace oglw {

namespace {

// -----------------------------------------------------------------------------

}  // namespace

// ================================ Frame Buffer ===============================

class FrameBuffer::Impl {
public:
    Impl() {}

    Impl(size_t w, size_t h) {
        init(w, h);
    }

    Impl(const Impl& lhs) = delete;  // non-copyable

    Impl(Impl&&) = delete;

    Impl& operator=(const Impl& lhs) = delete;  // non-copyable

    Impl& operator=(Impl&&) = delete;

    ~Impl() {
        release();
    }

    // -------------------------------------------------------------------------
    void init(size_t w, size_t h) {
        // Release forcible
        release();

        // Create FBO
        OGLW_CHECK(glGenFramebuffers, 1, &m_fbo_id);

        // Bind FBO
        OGLW_CHECK(glBindFramebuffer, GL_FRAMEBUFFER, m_fbo_id);

        // Create color buffer (texture)
        m_frame_img = GpuImage<uint8_t>::Create(w, h, 3);

        // Bind color buffer to FBO (texture)
        const unsigned int tex_id =
                static_cast<unsigned int>(m_frame_img->getTextureId());
        OGLW_CHECK(glFramebufferTexture2D, GL_FRAMEBUFFER,
                   GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0);


        // Create and bind depth buffer to FBO (render buffer)
        OGLW_CHECK(glGenRenderbuffers, 1, &m_depth_buf_id);
        OGLW_CHECK(glBindRenderbuffer, GL_RENDERBUFFER, m_depth_buf_id);
        OGLW_CHECK(glRenderbufferStorage, GL_RENDERBUFFER,
                   GL_DEPTH_COMPONENT, w, h);
        OGLW_CHECK(glFramebufferRenderbuffer, GL_FRAMEBUFFER,
                   GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                   m_depth_buf_id);

        // Set drawing target
        GLenum drawBufs[] = {GL_COLOR_ATTACHMENT0};
        OGLW_CHECK(glDrawBuffers, 1, drawBufs);

        // Unbind FBO
        OGLW_CHECK(glBindFramebuffer, GL_FRAMEBUFFER, 0);
    }

    GpuImagePtr<uint8_t> getImage() {
        return m_frame_img;
    }

    // -------------------------------------------------------------------------
    void bind() {
        OGLW_CHECK(glBindFramebuffer, GL_FRAMEBUFFER, m_fbo_id);
        OGLW_CHECK(glViewport, 0, 0, m_frame_img->getWidth(),
                   m_frame_img->getHeight());
    }

    static void Unbind() {
        OGLW_CHECK(glBindFramebuffer, GL_FRAMEBUFFER, 0);
    }

    // -------------------------------------------------------------------------
private:
    void release() {
        if (m_fbo_id) {
            OGLW_CHECK(glDeleteFramebuffers, 1, &m_fbo_id);
        }
        if (m_depth_buf_id) {
            OGLW_CHECK(glDeleteRenderbuffers, 1, &m_depth_buf_id);
        }
    }

    GpuImagePtr<uint8_t> m_frame_img;
    GLuint m_fbo_id = 0;
    GLuint m_depth_buf_id = 0;
};

// -----------------------------------------------------------------------------
// ------------------------------- Pimpl Pattern -------------------------------
// -----------------------------------------------------------------------------
FrameBuffer::FrameBuffer() : m_impl(std::make_unique<Impl>()) {}

FrameBuffer::FrameBuffer(size_t w, size_t h) : m_impl(std::make_unique<Impl>(w, h)) {}

FrameBuffer::FrameBuffer(FrameBuffer&&) = default;

FrameBuffer& FrameBuffer::operator=(FrameBuffer&&) = default;

FrameBuffer::~FrameBuffer() = default;

// -----------------------------------------------------------------------------
void FrameBuffer::init(size_t w, size_t h) {
    m_impl->init(w, h);
}

GpuImagePtr<uint8_t> FrameBuffer::getImage() {
    return m_impl->getImage();
}

// -------------------------------------------------------------------------
void FrameBuffer::bind() {
    m_impl->bind();
}

void FrameBuffer::Unbind() {
    FrameBuffer::Impl::Unbind();
}

// -------------------------------------------------------------------------

}  // namespace oglw

