#include "gl_window.h"

#include <iostream>

namespace oglw {

GlWindow::GlWindow(const std::string& window_title) {
    // Initialize GLFW
    if (!s_inited && !initGlfw()) {
        return;
    }

    // OpenGL context flags (OpenGL 4.3)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    // Create OpenGL window and context
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    m_window = glfwCreateWindow(1200, 900, window_title.c_str(), nullptr,
                                nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        return;
    }
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);  // Enable vsync

    // Initialize GLAD
    if (!s_inited && !initGlad()) {
        return;
    }

    s_inited = true;
}

GlWindow::~GlWindow() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
}

bool GlWindow::initGlfw() {
    // Set error callback
    glfwSetErrorCallback([](int error, const char* desc) {
        std::cerr << "GLFW Error " << error << ": " << desc << std::endl;
    });

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    atexit([]() { glfwTerminate(); });
    return true;
}

bool GlWindow::initGlad() {
    // Initialize GLAD
    if (!gladLoadGL()) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    return true;
}

bool GlWindow::s_inited = false;

}  // namespace oglw
