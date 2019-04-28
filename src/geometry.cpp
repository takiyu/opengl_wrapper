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
        case PrimitiveType::LINE: return GL_LINE;
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
    void setArrayBuffer(const std::shared_ptr<GpuBufferBase> array_buf,
                        unsigned int index) {
        if (m_array_bufs.count(index) == 0) {
            m_update_attrib = true;
        }
        m_array_bufs[index] = array_buf;
    }

    void setIndexBuffer(const std::shared_ptr<GpuIndexBuffer> index_buf) {
        m_index_buffer = index_buf;
    }

    // -------------------------------------------------------------------------
    void setShader(const std::shared_ptr<GpuShader> shader) {
        m_shader = shader;
    }

    void draw(PrimitiveType prim_type) {
        // Create VAO
        if (m_vao == 0) {
            OGLW_CHECK(glGenVertexArrays, 1, &m_vao);
            OGLW_CHECK(glBindVertexArray, m_vao);
        }

        // Check vertex array
        if (m_array_bufs.count(0) == 0 ||
            *m_array_bufs[0]->getDataType() != typeid(float)) {
            throw std::runtime_error("No floating vertex array");
        }

        // Update attributes
        if (m_update_attrib) {
            m_update_attrib = false;
            for (auto& v : m_array_bufs) {
                UpdateAttribute(v.first, *v.second,
                                GetGlType(v.second->getDataType()));
            }
        }

        // Use shader
        if (!m_shader) {
            throw std::runtime_error("No shader is set");
        }
        m_shader->use();

        // Bind VAO
        OGLW_CHECK(glBindVertexArray, m_vao);

        // Draw
        const GLenum gl_prim = GetGlPrimitive(prim_type);
        if (m_index_buffer) {
            const size_t size = m_index_buffer->getElemSize() *
                                m_index_buffer->getNumElem();
            OGLW_CHECK(glDrawElements, gl_prim, size, GL_UNSIGNED_INT, nullptr);
        } else {
            const size_t size = m_array_bufs[0]->getElemSize() *
                                m_array_bufs[0]->getNumElem();
            OGLW_CHECK(glDrawArrays, gl_prim, 0, size);
        }

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

    GLuint m_vao = 0;

    bool m_update_attrib = false;

    std::map<unsigned int, std::shared_ptr<GpuBufferBase>> m_array_bufs;
    std::shared_ptr<GpuIndexBuffer> m_index_buffer;
    std::shared_ptr<GpuShader> m_shader;
};

// -----------------------------------------------------------------------------
// ------------------------------- Pimpl Pattern -------------------------------
// -----------------------------------------------------------------------------
Geometry::Geometry() : m_impl(std::make_unique<Impl>()) {}

Geometry::Geometry(Geometry&&) = default;

Geometry& Geometry::operator=(Geometry&&) = default;

Geometry::~Geometry() = default;

// -----------------------------------------------------------------------------
void Geometry::setArrayBuffer(const std::shared_ptr<GpuBufferBase> array_buf,
                              unsigned int index) {
    m_impl->setArrayBuffer(array_buf, index);
}

void Geometry::setIndexBuffer(const std::shared_ptr<GpuIndexBuffer> index_buf) {
    m_impl->setIndexBuffer(index_buf);
}

// -------------------------------------------------------------------------
void Geometry::setShader(const std::shared_ptr<GpuShader> shader) {
    m_impl->setShader(shader);
}

void Geometry::draw(PrimitiveType prim_type) {
    m_impl->draw(prim_type);
}

}  // namespace oglw
