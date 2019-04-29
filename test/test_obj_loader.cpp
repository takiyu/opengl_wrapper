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

        std::vector<oglw::GeometryPtr> geoms;
        oglw::LoadObj(
                "/home/takiyu/Projects/work/huawei/hair_geom/hairstyles/"
                "head_model.obj",
                geoms);
        REQUIRE(geoms.size() == 1);
        auto geom = geoms[0];

        auto gpu_shader = oglw::GpuShader::Create();
        gpu_shader->attach(oglw::ShaderType::VERTEX, "");
        gpu_shader->attach(oglw::ShaderType::FRAGMENT, "");
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
