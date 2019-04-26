#include "gl_utils.h"

#include <glad/glad.h>

#include <iostream>
#include <sstream>

namespace oglw {

void CheckOpenGlError(const char* file, int line, const char* func) {
    std::stringstream ss;

    // Check error existence
    GLenum err = glGetError();
    if (err == GL_NO_ERROR) {
        return;
    }

    // Create error message
    while (err != GL_NO_ERROR) {
        const char* message = "";
        switch (err) {
            case GL_INVALID_ENUM: message = "Invalid enum"; break;
            case GL_INVALID_VALUE: message = "Invalid value"; break;
            case GL_INVALID_OPERATION: message = "Invalid operation"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                message = "Invalid framebuffer operation";
                break;
            case GL_OUT_OF_MEMORY: message = "Out of memory"; break;
            default: message = "Unknown error";
        }

        ss << "OpenGL Error in file " << file << " @ line " << line << ", "
           << func << "() : " << message << "\n";

        err = glGetError();
    }

    throw std::runtime_error(ss.str());
}

}  // namespace oglw
