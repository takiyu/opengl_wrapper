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
    void setShader(const GpuShaderPtr shader) {
        m_shader = shader;
    }

    void draw(PrimitiveType prim_type, float prim_size) {
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

        // Use shader
        if (!m_shader) {
            throw std::runtime_error("No shader is set");
        }
        m_shader->use();

        // Draw
        drawPrimitives(prim_type, prim_size);

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

    void drawPrimitives(PrimitiveType prim_type, float prim_size) {
        // Primitive size
        SetPrimitiveSize(prim_type, prim_size);

        // Draw
        const GLenum gl_prim = GetGlPrimitive(prim_type);
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
    }

    GLuint m_vao = 0;

    bool m_update_attrib = false;
    bool m_update_indices = false;

    std::map<unsigned int, GpuBufferBasePtr> m_array_bufs;
    GpuIndexBufferPtr m_index_buffer;
    GpuShaderPtr m_shader;
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
void Geometry::setShader(const GpuShaderPtr shader) {
    m_impl->setShader(shader);
}

void Geometry::draw(PrimitiveType prim_type, float prim_size) {
    m_impl->draw(prim_type, prim_size);
}

}  // namespace oglw
