#include "catch2/catch.hpp"

#include <oglw/camera.h>
#include <oglw/geometry.h>
#include <oglw/gl_utils.h>
#include <oglw/gpu_buffer.h>
#include <oglw/gpu_shader.h>
#include <oglw/obj_loader.h>

#include "gl_window.h"

#include <sstream>
#include <stdexcept>

static auto g_camera = oglw::Camera::Create();
static double g_prev_xpos = 0.0, g_prev_ypos = 0.0;
static bool g_move = false;
static void MouseMoveCallback(GLFWwindow* win, double xpos, double ypos) {
    (void)win;
    if (g_move) {
        g_camera->rotateOrbit((xpos - g_prev_xpos) * -0.001,
                              (ypos - g_prev_ypos) * -0.001);
    }
    g_prev_xpos = xpos;
    g_prev_ypos = ypos;
}
static void MouseButtonCallback(GLFWwindow* win, int btn, int act, int mods) {
    (void)win;
    (void)act;
    (void)mods;
    if (btn == GLFW_MOUSE_BUTTON_LEFT && act == GLFW_PRESS) {
        g_move = true;
    }
    if (btn == GLFW_MOUSE_BUTTON_LEFT && act == GLFW_RELEASE) {
        g_move = false;
    }
}

TEST_CASE("ObjLoader test") {
    SECTION("GpuShader basic") {
        oglw::GlWindow win("Title");
        glfwSetCursorPosCallback(win.getWindowPtr(), MouseMoveCallback);
        glfwSetMouseButtonCallback(win.getWindowPtr(), MouseButtonCallback);

        std::map<std::string, oglw::GeometryPtr> geoms;
        oglw::LoadObj(
                "/home/takiyu/Projects/work/huawei/hair/hairstyles/"
                "head_model.obj",
                geoms, oglw::ObjLoaderMode::INDEXING_VTX_ONLY,
                {0.f, -1.5, 0.f});
        REQUIRE(geoms.size() == 1);
        oglw::GeometryPtr geom = geoms.begin()->second;

        const std::string VTX_SHADER =
                "#version 430\n"
                "layout (location=0) in vec3 position;\n"
                "layout (location=1) in vec3 normal;\n"
                "layout (location=2) in vec2 texcoord;\n"
                "uniform mat4 proj_mat;\n"
                "uniform mat4 view_mat;\n"
                "out vec3 frag_normal;\n"
                "out vec2 frag_texcoord;\n"
                "void main() {\n"
                "    gl_Position = proj_mat * view_mat * vec4(position, 1.0);\n"
                "    frag_normal = normal;\n"
                "    frag_texcoord = texcoord;\n"
                "}\n";
        const std::string FRG_SHADER =
                "#version 430\n"
                "in vec3 frag_normal;\n"
                "in vec2 frag_texcoord;\n"
                "layout (location=0) out vec4 FragColor;\n"
                "void main() {\n"
                "    FragColor = vec4(frag_normal * 0.5 + 0.5, 0.0);\n"
                "}\n";

        auto gpu_shader = oglw::GpuShader::Create();
        gpu_shader->attach(oglw::ShaderType::VERTEX, VTX_SHADER);
        gpu_shader->attach(oglw::ShaderType::FRAGMENT, FRG_SHADER);
        gpu_shader->link();

        geom->setShader(gpu_shader);

        for (size_t i = 0; i < 20; i++) {
            int width, height;
            OGLW_CHECK(glfwGetFramebufferSize, win.getWindowPtr(), &width,
                       &height);
            OGLW_CHECK(glViewport, 0, 0, width, height);

            g_camera->setScreenSize(width, height);
            const oglw::Mat4& proj_mat = g_camera->getProj();
            const oglw::Mat4& view_mat = g_camera->getView();
            gpu_shader->setUniform("proj_mat", proj_mat);
            gpu_shader->setUniform("view_mat", view_mat);

            OGLW_CHECK(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            OGLW_CHECK(glClearColor, 0.3, 0.3, 1.0, 1.0);

            glEnable(GL_DEPTH_TEST);
            geom->draw();

            OGLW_CHECK(glfwSwapBuffers, win.getWindowPtr());
            OGLW_CHECK(glfwPollEvents);
        }
    }
}
