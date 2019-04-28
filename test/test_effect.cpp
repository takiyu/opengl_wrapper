#include "catch2/catch.hpp"

#include <oglw/geometry.h>
#include <oglw/gl_utils.h>
#include <oglw/gpu_buffer.h>
#include <oglw/gpu_shader.h>

#include "gl_window.h"

#include <sstream>
#include <stdexcept>

TEST_CASE("Effect test") {
    SECTION("CpuEffect basic") {
        REQUIRE(1 + 3 == 4);
    }

    SECTION("GpuShader basic") {
        oglw::GlWindow win("Title");

        auto vertex_array = std::make_shared<oglw::GpuArrayBuffer<float>>();
        vertex_array->init(3, 3);
        const float VERTICES[9] = {0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f};
        vertex_array->sendData(VERTICES);

        auto color_array = std::make_shared<oglw::GpuArrayBuffer<float>>();
        color_array->init(3, 3);
        const float COLORS[9] = {0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f};
        color_array->sendData(COLORS);

        const std::string VTX_SHADER =
                "#version 430\n"
                "layout (location=0) in vec3 vertex_pos;\n"
                "layout (location=1) in vec3 col;\n"
                "out vec2 frag_pos;\n"
                "out vec3 frag_col;\n"
                "void main() {\n"
                "    gl_Position = vec4(vertex_pos, 1.0);\n"
                "    frag_pos = vertex_pos.xy;\n"
                "    frag_col = col;\n"
                "}\n";
        const std::string FRG_SHADER =
                "#version 430\n"
                "in vec2 frag_pos;\n"
                "in vec3 frag_col;\n"
                "layout (location=0) out vec4 FragColor;\n"
                "void main() {\n"
                "    FragColor = vec4(frag_col, 0.0);\n"
                "}\n";

        auto gpu_shader = std::make_shared<oglw::GpuShader>();
        gpu_shader->attach(oglw::ShaderType::VERTEX, VTX_SHADER);
        gpu_shader->attach(oglw::ShaderType::FRAGMENT, FRG_SHADER);
        gpu_shader->link();

        oglw::Geometry geom;
        geom.setArrayBuffer(vertex_array, 0);
        geom.setArrayBuffer(color_array, 1);
        geom.setShader(gpu_shader);

        for (size_t i = 0; i < 50; i++) {
            int width, height;
            OGLW_CHECK(glfwGetFramebufferSize, win.getWindowPtr(), &width,
                       &height);
            OGLW_CHECK(glViewport, 0, 0, width, height);

            OGLW_CHECK(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            OGLW_CHECK(glClearColor, 0.3, 0.3, 1.0, 1.0);

            geom.draw(oglw::PrimitiveType::TRIANGLE);

            OGLW_CHECK(glfwSwapBuffers, win.getWindowPtr());
            OGLW_CHECK(glfwPollEvents);
        }
    }
}
