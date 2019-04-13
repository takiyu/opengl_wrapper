#include "effect.h"

#include "gl_utils.h"

#include <glad/glad.h>

#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>

namespace oglw {

namespace {

const std::string DEFAULT_VERT_SHADER =
        "#version 430\n"
        "layout (location=0) in vec3 vertex_pos;\n"
        "out vec2 frag_pos;\n"
        "void main() {\n"
        "    gl_Position = vec4(vertex_pos, 1.0);\n"
        "    frag_pos = vertex_pos.xy;\n"
        "}\n";

const std::string DEFAULT_FRAG_SHADER =
        "#version 430\n"
        "in vec2 frag_pos;\n"
        "layout (location=0) out vec4 FragColor;\n"
        "void main() {\n"
        "    FragColor = vec4(frag_pos, 0.0, 0.0);\n"
        "}\n";

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

void AttachShader(GLuint program, GLenum shader_type,
                  const std::string& source) {
    GLuint shader = glCreateShader(shader_type);

    // Compile
    const char* c_code = source.c_str();
    OGLW_CHECK(glShaderSource, shader, 1, &c_code, nullptr);
    OGLW_CHECK(glCompileShader, shader);

    // Error check
    CheckGlslError(shader, GL_COMPILE_STATUS,
                   "----------<< Shader Compile Error >>----------",
                   glGetShaderiv, glGetShaderInfoLog);

    // Attach
    OGLW_CHECK(glAttachShader, program, shader);

    OGLW_CHECK(glDeleteShader, shader);
}

GLuint CreateProgram(const std::string& vert_src, const std::string& frag_src) {
    GLuint program = glCreateProgram();
    // Compile vertex shader
    if (vert_src == "") {
        AttachShader(program, GL_VERTEX_SHADER, DEFAULT_VERT_SHADER);
    } else {
        AttachShader(program, GL_VERTEX_SHADER, vert_src);
    }
    // Compile fragment shader
    if (frag_src == "") {
        AttachShader(program, GL_FRAGMENT_SHADER, DEFAULT_FRAG_SHADER);
    } else {
        AttachShader(program, GL_FRAGMENT_SHADER, frag_src);
    }
    // Link
    OGLW_CHECK(glLinkProgram, program);

    // Error check
    CheckGlslError(program, GL_LINK_STATUS,
                   "----------<< Program Link Error >>----------",
                   glGetProgramiv, glGetProgramInfoLog);

    return program;
}

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

void ObtainUniformLocations(GLuint shader,
                            std::map<std::string, int>& uniform_locs) {
    uniform_locs.clear();

    // Get the number of uniforms
    GLint n_uniforms = 0;
    glGetProgramInterfaceiv(shader, GL_UNIFORM, GL_ACTIVE_RESOURCES,
                            &n_uniforms);

    const GLenum PROPS[] = {GL_NAME_LENGTH, GL_TYPE, GL_LOCATION,
                            GL_BLOCK_INDEX};

    for (GLuint i = 0; i < static_cast<GLuint>(n_uniforms); i++) {
        GLint results[4];
        glGetProgramResourceiv(shader, GL_UNIFORM, i, 4, PROPS, 4, nullptr,
                               results);

        // Skip uniforms in blocks
        if (results[3] != -1) {
            continue;
        }

        // Get uniform name
        GLint name_size = results[0] + 1;
        std::vector<char> name(static_cast<size_t>(name_size));
        glGetProgramResourceName(shader, GL_UNIFORM, i, name_size, nullptr,
                                 name.data());

        // Register the location
        uniform_locs[std::string(name.data())] = results[2];
    }
}

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------

}  // namespace

// ================================= GPU Effect ================================

class GpuEffect::Impl {
public:
    Impl() {}

    Impl(size_t n_inps, size_t n_outs, const std::string& vert_src,
         const std::string& frag_src) {
        init(n_inps, n_outs, vert_src, frag_src);
    }

    Impl(const Impl& lhs) = delete;  // non-copyable

    Impl(Impl&&) = delete;

    Impl& operator=(const Impl& lhs) = delete;  // non-copyable

    Impl& operator=(Impl&&) = delete;

    ~Impl() {
        release();
    }

    // -------------------------------------------------------------------------
    void init(size_t n_inps, size_t n_outs, const std::string& vert_src,
              const std::string& frag_src) {
        // Release forcibly
        release();

        // Set input / output images
        m_n_inps = n_inps;
        m_n_outs = n_outs;

        // Delete previous program
        m_program = CreateProgram(vert_src, frag_src);

        // Create Vertex Buffer Object for positions
        m_vbo = CreateQuadPositionVBO();

        // Vertex Array Object
        m_vao = CreateQuadPositionVAO(m_vbo);
    }

    // -------------------------------------------------------------------------
    void setInpImgs(const std::vector<const Image*>& inp_imgs) {}

    void setOutImgs(const std::vector<Image*>& out_imgs) {}

    // -------------------------------------------------------------------------
    void setUniform(const std::string& name, bool v, UniformType type) {
        //         glUniform3f(loc, x, y, z);
    }
    void setUniform(const std::string& name, int v, UniformType type) {}
    void setUniform(const std::string& name, unsigned int v, UniformType type) {
    }
    void setUniform(const std::string& name, float v, UniformType type) {}
    void setUniform(const std::string& name, const std::array<float, 2>& v,
                    UniformType type) {}
    void setUniform(const std::string& name, const std::array<float, 3>& v,
                    UniformType type) {}
    void setUniform(const std::string& name, const std::array<float, 4>& v,
                    UniformType type) {}
    void setUniform(const std::string& name, const std::array<float, 9>& v,
                    UniformType type) {}
    void setUniform(const std::string& name, const std::array<float, 16>& v,
                    UniformType type) {}

    // -------------------------------------------------------------------------
    void run() const {
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

    GLuint m_program = 0;
    GLuint m_vbo = 0;
    GLuint m_vao = 0;
    size_t m_n_inps = 0, m_n_outs = 0;
    std::map<std::string, int> m_uniform_locs_vert;           // name -> location
    std::vector<std::function<void()>> m_uniform_funcs_vert;  // location -> setter
};

// -----------------------------------------------------------------------------
// ------------------------------- Pimpl Pattern -------------------------------
// -----------------------------------------------------------------------------
GpuEffect::GpuEffect() : m_impl(std::make_unique<Impl>()) {}

GpuEffect::GpuEffect(size_t n_inps, size_t n_outs, const std::string& vert_src,
                     const std::string& frag_src)
    : m_impl(std::make_unique<Impl>(n_inps, n_outs, vert_src, frag_src)) {}

GpuEffect::GpuEffect(GpuEffect&&) = default;

GpuEffect& GpuEffect::operator=(GpuEffect&&) = default;

GpuEffect::~GpuEffect() = default;

// -----------------------------------------------------------------------------
void GpuEffect::init(size_t n_inps, size_t n_outs, const std::string& vert_src,
                     const std::string& frag_src) {
    m_impl->init(n_inps, n_outs, vert_src, frag_src);
}

// -------------------------------------------------------------------------
void GpuEffect::setInpImgs(const std::vector<const Image*>& inp_imgs) {
    m_impl->setInpImgs(inp_imgs);
}

void GpuEffect::setOutImgs(const std::vector<Image*>& out_imgs) {
    m_impl->setOutImgs(out_imgs);
}

// -------------------------------------------------------------------------
void GpuEffect::setUniform(const std::string& name, bool v,
                           GpuEffect::UniformType type) {
    m_impl->setUniform(name, v, type);
}

void GpuEffect::setUniform(const std::string& name, int v,
                           GpuEffect::UniformType type) {
    m_impl->setUniform(name, v, type);
}

void GpuEffect::setUniform(const std::string& name, unsigned int v,
                           GpuEffect::UniformType type) {
    m_impl->setUniform(name, v, type);
}

void GpuEffect::setUniform(const std::string& name, float v,
                           GpuEffect::UniformType type) {
    m_impl->setUniform(name, v, type);
}

void GpuEffect::setUniform(const std::string& name,
                           const std::array<float, 2>& v,
                           GpuEffect::UniformType type) {
    m_impl->setUniform(name, v, type);
}

void GpuEffect::setUniform(const std::string& name,
                           const std::array<float, 3>& v,
                           GpuEffect::UniformType type) {
    m_impl->setUniform(name, v, type);
}

void GpuEffect::setUniform(const std::string& name,
                           const std::array<float, 4>& v,
                           GpuEffect::UniformType type) {
    m_impl->setUniform(name, v, type);
}

void GpuEffect::setUniform(const std::string& name,
                           const std::array<float, 9>& v,
                           GpuEffect::UniformType type) {
    m_impl->setUniform(name, v, type);
}

void GpuEffect::setUniform(const std::string& name,
                           const std::array<float, 16>& v,
                           GpuEffect::UniformType type) {
    m_impl->setUniform(name, v, type);
}

// -------------------------------------------------------------------------
void GpuEffect::run() const {
    m_impl->run();
}

}  // namespace oglw
