#include <oglw/geometry.h>

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
GLenum GetGlPrimitive(PrimitiveType type) {
    switch (type) {
        case PrimitiveType::TRIANGLE: return GL_TRIANGLES;
        case PrimitiveType::LINE: return GL_LINES;
        case PrimitiveType::POINT: return GL_POINTS;
    }
    std::stringstream ss;
    ss << "Invalid primitive type: \"" << static_cast<int>(type) << "\"";
    throw std::runtime_error(ss.str());
}

GLenum GetGlType(const std::type_info* type) {
    if (*type == typeid(float)) {
        return GL_FLOAT;
    } else if (*type == typeid(int)) {
        return GL_INT;
    } else if (*type == typeid(unsigned int)) {
        return GL_UNSIGNED_INT;
    }
    std::stringstream ss;
    ss << "Invalid type: \"" << type->name() << "\"";
    throw std::runtime_error(ss.str());
}

// -----------------------------------------------------------------------------
void SetPrimitiveSize(PrimitiveType prim_type, float prim_size) {
    switch (prim_type) {
        case PrimitiveType::TRIANGLE: return;
        case PrimitiveType::LINE: glLineWidth(prim_size); return;
        case PrimitiveType::POINT: glPointSize(prim_size); return;
    }
}

// -----------------------------------------------------------------------------
void UpdateAttribute(unsigned int index, const GpuBufferBase& array_buf,
                     GLenum type) {
    OGLW_CHECK(glEnableVertexAttribArray, index);
    OGLW_CHECK(glBindBuffer, GL_ARRAY_BUFFER, array_buf.getBufferId());
    OGLW_CHECK(glVertexAttribPointer, index, array_buf.getElemSize(), type,
               GL_FALSE, 0, nullptr);
}

// -----------------------------------------------------------------------------

}  // namespace

// ================================ GPU Geometry ===============================

class Geometry::Impl {
public:
    Impl() {}

    Impl(const Impl& lhs) = delete;  // non-copyable

    Impl(Impl&&) = delete;

    Impl& operator=(const Impl& lhs) = delete;  // non-copyable

    Impl& operator=(Impl&&) = delete;

    ~Impl() {
        release();
    }

    // -------------------------------------------------------------------------
    void setArrayBuffer(const GpuBufferBasePtr array_buf, unsigned int index) {
        if (array_buf->getBufferType() != BufferType::ARRAY) {
            throw std::runtime_error("Non array buffer");
        }
        if (m_array_bufs.count(index) == 0) {
            m_update_attrib = true;
        }
        m_array_bufs[index] = array_buf;
    }

    void setIndexBuffer(const GpuIndexBufferPtr index_buf) {
        if (*index_buf->getDataType() != typeid(unsigned int)) {
            throw std::runtime_error("Index buffer type must be uint");
        }
        m_update_indices = true;
        m_index_buffer = index_buf;
    }

    // -------------------------------------------------------------------------
    void setPrimitive(PrimitiveType prim_type, float prim_size) {
        m_prim_type = prim_type;
        m_prim_size = prim_size;
    }

    // -------------------------------------------------------------------------
    void setShader(const GpuShaderPtr shader) {
        m_shader = shader;
    }

    GpuShaderPtr getShader() const {
        return m_shader;
    }

    // -------------------------------------------------------------------------
    void setFrameImage(const GpuImageBasePtr frame_img) {
        if (frame_img != nullptr && frame_img->empty()) {
            throw std::runtime_error("GpuImage for FBO cannot be empty");
        }
        m_update_fragment = true;
        m_frame_img = frame_img;
    }

    GpuImageBasePtr getFrameImage() const {
        return m_frame_img;
    }

    // -------------------------------------------------------------------------
    void draw(bool clear) {
        // Check vertex array
        if (m_array_bufs.count(0) == 0 ||
            *m_array_bufs[0]->getDataType() != typeid(float)) {
            throw std::runtime_error("No floating vertex array");
        }

        // Create VAO
        if (m_vao == 0) {
            OGLW_CHECK(glGenVertexArrays, 1, &m_vao);
        }
        // Bind VAO
        OGLW_CHECK(glBindVertexArray, m_vao);

        // Update attribute binding
        updateAttribute();
        // Update index buffer binding
        updateIndexBuffer();

        // Update frame buffer binding for off-screen rendering
        updateFrameBuffer();

        // Use shader
        if (!m_shader) {
            throw std::runtime_error("No shader is set");
        }
        m_shader->use();

        // Primitive size
        SetPrimitiveSize(m_prim_type, m_prim_size);

        // Draw
        drawPrimitives(clear);

        // Unbind
        OGLW_CHECK(glBindVertexArray, 0);
        OGLW_CHECK(glUseProgram, 0);
    }

    // -------------------------------------------------------------------------
private:
    void release() {
        if (m_vao) {
            OGLW_CHECK(glDeleteVertexArrays, 1, &m_vao);
            m_vao = 0;
        }
        if (m_fbo_id) {
            OGLW_CHECK(glDeleteFramebuffers, 1, &m_fbo_id);
        }
        if (m_depth_buf_id) {
            OGLW_CHECK(glDeleteRenderbuffers, 1, &m_depth_buf_id);
        }
        m_array_bufs.clear();  // All destructors will be called.
        m_index_buffer = nullptr;
        m_shader = nullptr;
    }

    void updateAttribute() {
        if (m_update_attrib) {
            m_update_attrib = false;
            for (auto& v : m_array_bufs) {
                UpdateAttribute(v.first, *v.second,
                                GetGlType(v.second->getDataType()));
            }
        }
    }

    void updateIndexBuffer() {
        if (m_index_buffer && m_update_indices) {
            m_update_indices = false;
            OGLW_CHECK(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER,
                       m_index_buffer->getBufferId());
        }
    }

    void updateFrameBuffer() {
        if (m_frame_img && m_update_fragment) {
            m_update_fragment = false;

            // Create FBO
            if (m_fbo_id == 0) {
                OGLW_CHECK(glGenFramebuffers, 1, &m_fbo_id);
            }

            // Bind FBO
            OGLW_CHECK(glBindFramebuffer, GL_FRAMEBUFFER, m_fbo_id);

            // Create and bind depth buffer to FBO (render buffer)
            if (m_depth_buf_id == 0) {
                OGLW_CHECK(glGenRenderbuffers, 1, &m_depth_buf_id);
                OGLW_CHECK(glBindRenderbuffer, GL_RENDERBUFFER, m_depth_buf_id);
                OGLW_CHECK(glRenderbufferStorage, GL_RENDERBUFFER,
                           GL_DEPTH_COMPONENT, m_frame_img->getWidth(),
                           m_frame_img->getHeight());
                OGLW_CHECK(glFramebufferRenderbuffer, GL_FRAMEBUFFER,
                           GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                           m_depth_buf_id);
            }

            // Bind color buffer to FBO (texture)
            const unsigned int tex_id =
                    static_cast<unsigned int>(m_frame_img->getTextureId());
            OGLW_CHECK(glFramebufferTexture2D, GL_FRAMEBUFFER,
                       GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0);

            // Set drawing target
            GLenum drawBufs[] = {GL_COLOR_ATTACHMENT0};
            OGLW_CHECK(glDrawBuffers, 1, drawBufs);

            // Unbind FBO
            OGLW_CHECK(glBindFramebuffer, GL_FRAMEBUFFER, 0);
        }
    }

    void drawPrimitives(bool clear) {
        // Off-screen rendering
        if (m_frame_img) {
            glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_id);
            OGLW_CHECK(glViewport, 0, 0, m_frame_img->getWidth(),
                       m_frame_img->getHeight());
        }

        // Clear
        if (clear) {
            OGLW_CHECK(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        // Draw
        const GLenum gl_prim = GetGlPrimitive(m_prim_type);
        if (m_index_buffer) {
            // Index drawing
            const size_t size = m_index_buffer->getElemSize() *
                                m_index_buffer->getNumElem();
            OGLW_CHECK(glDrawElements, gl_prim, size, GL_UNSIGNED_INT, nullptr);
        } else {
            // Basic drawing
            const size_t size = m_array_bufs[0]->getNumElem();
            OGLW_CHECK(glDrawArrays, gl_prim, 0, size);
        }

        // End of off-screen rendering
        if (m_frame_img) {
            OGLW_CHECK(glBindFramebuffer, GL_FRAMEBUFFER, 0);
        }
    }

    GLuint m_vao = 0;

    bool m_update_attrib = false;
    bool m_update_indices = false;
    bool m_update_fragment = false;

    std::map<unsigned int, GpuBufferBasePtr> m_array_bufs;
    GpuIndexBufferPtr m_index_buffer;
    GpuShaderPtr m_shader;

    GpuImageBasePtr m_frame_img;
    GLuint m_fbo_id = 0;
    GLuint m_depth_buf_id = 0;

    PrimitiveType m_prim_type = PrimitiveType::TRIANGLE;
    float m_prim_size = 1.f;
};

// -----------------------------------------------------------------------------
// ------------------------------- Pimpl Pattern -------------------------------
// -----------------------------------------------------------------------------
Geometry::Geometry() : m_impl(std::make_unique<Impl>()) {}

Geometry::Geometry(Geometry&&) = default;

Geometry& Geometry::operator=(Geometry&&) = default;

Geometry::~Geometry() = default;

// -----------------------------------------------------------------------------
void Geometry::setArrayBuffer(const GpuBufferBasePtr array_buf,
                              unsigned int index) {
    m_impl->setArrayBuffer(array_buf, index);
}

void Geometry::setIndexBuffer(const GpuIndexBufferPtr index_buf) {
    m_impl->setIndexBuffer(index_buf);
}

// -------------------------------------------------------------------------
void Geometry::setPrimitive(PrimitiveType prim_type, float prim_size) {
    m_impl->setPrimitive(prim_type, prim_size);
}

// -------------------------------------------------------------------------
void Geometry::setShader(const GpuShaderPtr shader) {
    m_impl->setShader(shader);
}

GpuShaderPtr Geometry::getShader() const {
    return m_impl->getShader();
}

// -------------------------------------------------------------------------
void Geometry::setFrameImage(const GpuImageBasePtr frame_img) {
    m_impl->setFrameImage(frame_img);
}

GpuImageBasePtr Geometry::getFrameImage() const {
    return m_impl->getFrameImage();
}

// -------------------------------------------------------------------------
void Geometry::draw(bool clear) {
    m_impl->draw(clear);
}

}  // namespace oglw
