#include "catch2/catch.hpp"

#include <oglw/geometry.h>
#include <oglw/gl_utils.h>
#include <oglw/gpu_buffer.h>
#include <oglw/gpu_shader.h>
#include <oglw/obj_loader.h>

#include "gl_window.h"

#include <sstream>
#include <stdexcept>

TEST_CASE("ObjLoader test") {
    SECTION("GpuShader basic") {
        oglw::GlWindow win("Title");

        std::map<std::string, oglw::GeometryPtr> geoms;
        oglw::LoadObj(
                "/home/takiyu/Projects/work/huawei/hair_geom/hairstyles/"
                "head_model.obj",
                geoms, oglw::ObjLoaderMode::INDEXING_VTX_ONLY, {0.f, -1.5, 0.f});
        REQUIRE(geoms.size() == 1);
        oglw::GeometryPtr geom = geoms.begin()->second;

        const std::string VTX_SHADER =
                "#version 430\n"
                "layout (location=0) in vec3 position;\n"
                "layout (location=1) in vec3 normal;\n"
                "layout (location=2) in vec2 texcoord;\n"
                "out vec3 frag_normal;\n"
                "out vec2 frag_texcoord;\n"
                "void main() {\n"
                "    gl_Position = vec4(position, 1.0);\n"
                "    frag_normal = normal;\n"
                "    frag_texcoord = texcoord;\n"
                "}\n";
        const std::string FRG_SHADER =
                "#version 430\n"
                "in vec3 frag_normal;\n"
                "in vec2 frag_texcoord;\n"
                "layout (location=0) out vec4 FragColor;\n"
                "void main() {\n"
                "    FragColor = vec4(frag_texcoord, 0.0, 0.0);\n"
                "}\n";

        auto gpu_shader = oglw::GpuShader::Create();
        gpu_shader->attach(oglw::ShaderType::VERTEX, VTX_SHADER);
        gpu_shader->attach(oglw::ShaderType::FRAGMENT, FRG_SHADER);
        gpu_shader->link();

        geom->setShader(gpu_shader);

        for (size_t i = 0; i < 50; i++) {
            int width, height;
            OGLW_CHECK(glfwGetFramebufferSize, win.getWindowPtr(), &width,
                       &height);
            OGLW_CHECK(glViewport, 0, 0, width, height);

            OGLW_CHECK(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            OGLW_CHECK(glClearColor, 0.3, 0.3, 1.0, 1.0);

            geom->draw(oglw::PrimitiveType::TRIANGLE);

            OGLW_CHECK(glfwSwapBuffers, win.getWindowPtr());
            OGLW_CHECK(glfwPollEvents);
        }
    }
}
