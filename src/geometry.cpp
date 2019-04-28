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

const std::map<Geometry::ShaderType, GLenum> SHADER_TYPE_MAP = {
    {Geometry::VERTEX, GL_VERTEX_SHADER},
    {Geometry::FRAGMENT, GL_FRAGMENT_SHADER},
};

const std::map<Geometry::ShaderType, std::string> DEFAULT_SHADER = {
    {
        Geometry::VERTEX,
            "#version 430\n"
            "layout (location=0) in vec3 vertex_pos;\n"
            "out vec2 frag_pos;\n"
            "void main() {\n"
            "    gl_Position = vec4(vertex_pos, 1.0);\n"
            "    frag_pos = vertex_pos.xy;\n"
            "}\n"
    }, {
        Geometry::FRAGMENT,
            "#version 430\n"
            "in vec2 frag_pos;\n"
            "layout (location=0) out vec4 FragColor;\n"
            "void main() {\n"
            "    FragColor = vec4(frag_pos, 0.0, 0.0);\n"
            "}\n"
    },
};

// -----------------------------------------------------------------------------

// Check compile or link error
template <typename IGetter, typename LogGetter>
void CheckGlslError(GLuint handler, GLenum type, const std::string& tag,
                    IGetter i_getter, LogGetter log_getter) {
    // Check error
    int result;
    OGLW_CHECK(i_getter, handler, type, &result);
    if (result == GL_FALSE) {
        // Get message length
        int len = 0;
        OGLW_CHECK(i_getter, handler, GL_INFO_LOG_LENGTH, &len);
        if (0 < len) {
            // Get message
            std::vector<char> c_log(static_cast<size_t>(len));
            int written = 0;
            OGLW_CHECK(log_getter, handler, len, &written, c_log.data());
            throw std::runtime_error(tag + "\n" + c_log.data());
        } else {
            throw std::runtime_error(tag + "\n(no message)");
        }
    }
}

void AttachShader(GLuint program, Geometry::ShaderType type,
                  const std::string& source) {
    // Create shader
    const GLenum gl_shader_type = SHADER_TYPE_MAP.at(type);
    GLuint shader = glCreateShader(gl_shader_type);

    // Switch for default source
    const char* c_code =
        source.empty() ? DEFAULT_SHADER.at(type).c_str()
                       : source.c_str();

    // Compile
    OGLW_CHECK(glShaderSource, shader, 1, &c_code, nullptr);
    OGLW_CHECK(glCompileShader, shader);

    // Error check
    CheckGlslError(shader, GL_COMPILE_STATUS,
                   "----------<< Shader Compile Error >>----------",
                   glGetShaderiv, glGetShaderInfoLog);

    // Attach
    OGLW_CHECK(glAttachShader, program, shader);

    // Delete
    OGLW_CHECK(glDeleteShader, shader);
}

void LinkProgram(GLuint program) {
    // Link
    OGLW_CHECK(glLinkProgram, program);

    // Error check
    CheckGlslError(program, GL_LINK_STATUS,
                   "----------<< Program Link Error >>----------",
                   glGetProgramiv, glGetProgramInfoLog);
}

// -----------------------------------------------------------------------------

GLuint CreateQuadPositionVBO() {
    GLuint vbo_position;
    OGLW_CHECK(glGenBuffers, 1, &vbo_position);

    const float POS_DATA[] = {
            -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f,  1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f, 1.0f, 1.0f,  0.0f, -1.0f, 1.0f, 0.0f,
    };

    OGLW_CHECK(glBindBuffer, GL_ARRAY_BUFFER, vbo_position);
    OGLW_CHECK(glBufferData, GL_ARRAY_BUFFER, 9 * sizeof(float) * 2, POS_DATA,
               GL_STATIC_DRAW);

    return vbo_position;
}

GLuint CreateQuadPositionVAO(const GLuint vbo) {
    GLuint vao;
    OGLW_CHECK(glGenVertexArrays, 1, &vao);
    OGLW_CHECK(glBindVertexArray, vao);
    OGLW_CHECK(glEnableVertexAttribArray, 0);
    OGLW_CHECK(glBindBuffer, GL_ARRAY_BUFFER, vbo);
    OGLW_CHECK(glVertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    OGLW_CHECK(glBindVertexArray, 0);
    return vao;
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
    void setVertices(const float *vtx_array, size_t n) {
        m_impl->setVertices(vtx_array, n);
    }

    void setIndices(const float *idx_array, size_t n) {
        m_impl->setIndices(idx_array, n);
    }

    // -------------------------------------------------------------------------
    void setShader(const GpuShader& shader) {
        m_impl->setShader(shader);
    }

    void draw(PrimitiveType prim_type) const {
        // Create Vertex Buffer Object for positions
        m_vbo = CreateQuadPositionVBO();

        // Vertex Array Object
        m_vao = CreateQuadPositionVAO(m_vbo);

        OGLW_CHECK(glClear, GL_COLOR_BUFFER_BIT);
        OGLW_CHECK(glClearColor, 0.3, 0.3, 1.0, 1.0);

        OGLW_CHECK(glUseProgram, m_program);

        OGLW_CHECK(glBindVertexArray, m_vao);
        OGLW_CHECK(glDrawArrays, GL_TRIANGLES, 0, 6);
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
        if (m_vbo) {
            OGLW_CHECK(glDeleteBuffers, 1, &m_vbo);
            m_vbo = 0;
        }
        if (m_program) {
            OGLW_CHECK(glDeleteProgram, m_program);
            m_program = 0;
        }
    }

    GLuint m_vbo = 0;
    GLuint m_vao = 0;

    std::map<std::string, GLint> m_uniform_locs;  // name -> location
};

// -----------------------------------------------------------------------------
// ------------------------------- Pimpl Pattern -------------------------------
// -----------------------------------------------------------------------------
Geometry::Geometry() : m_impl(std::make_unique<Impl>()) {}

Geometry::Geometry(Geometry&&) = default;

Geometry& Geometry::operator=(Geometry&&) = default;

Geometry::~Geometry() = default;

// -----------------------------------------------------------------------------
void Geometry::setVertices(const float *vtx_array, size_t n) {
    m_impl->setVertices(vtx_array, n);
}

void Geometry::setIndices(const float *idx_array, size_t n) {
    m_impl->setIndices(idx_array, n);
}

// -------------------------------------------------------------------------
void Geometry::setShader(const GpuShader& shader) {
    m_impl->setShader(shader);
}

void Geometry::draw(PrimitiveType prim_type) const {
    m_impl->draw(prim_type);
}

}  // namespace oglw

