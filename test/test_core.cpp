#include "catch2/catch.hpp"

#include <glad/glad.h>  // must be before GLFW

#include <GLFW/glfw3.h>

#include <iostream>

static GLFWwindow* InitOpenGL(const std::string& window_title) {
    // Set error callback
    glfwSetErrorCallback([](int error, const char* description) {
        std::cerr << "GLFW Error " << error << ": " << description << std::endl;
    });

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return nullptr;
    }
    atexit([]() { glfwTerminate(); });

#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#else
    // OpenGL context flags (OpenGL 4.3)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create OpenGL window and context
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    GLFWwindow* window =
            glfwCreateWindow(1200, 900, window_title.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable vsync

    // Initialize GLAD
    if (!gladLoadGL()) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }

    return window;
}

TEST_CASE("Core test") {
    SECTION("Build basic") {
        REQUIRE(InitOpenGL("Title") != nullptr);
        REQUIRE(1 + 3 == 4);
    }

    SECTION("Build basic2") {
        REQUIRE(InitOpenGL("Title2") != nullptr);
        REQUIRE(1 + 3 == 4);
    }
}
