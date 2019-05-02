#ifndef OGLW_GL_WINDOW_H_190407
#define OGLW_GL_WINDOW_H_190407

#include <glad/glad.h>  // must be before GLFW

#include <GLFW/glfw3.h>

#include <string>

namespace oglw {

class GlWindow {
public:
    GlWindow(const std::string& window_title = "");
    ~GlWindow();

    GLFWwindow* getWindowPtr() {
        return m_window;
    }

private:
    static bool s_inited;

    GLFWwindow* m_window = nullptr;

    bool initGlfw();
    bool initGlad();
};

}  // namespace oglw

#endif /* end of include guard */
