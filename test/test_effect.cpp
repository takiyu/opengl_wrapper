#include "catch2/catch.hpp"

#include "gpu_shader.h"
#include "gl_utils.h"
#include "gl_window.h"

#include <sstream>
#include <stdexcept>

TEST_CASE("Effect test") {
    SECTION("CpuEffect basic") {
        REQUIRE(1 + 3 == 4);
    }

    SECTION("GpuShader basic") {
        oglw::GlWindow win("Title");

        oglw::GpuShader gpu_shader;
        gpu_shader.attach(oglw::GpuShader::VERTEX, "");
        gpu_shader.attach(oglw::GpuShader::FRAGMENT, "");
        gpu_shader.link();

        for (size_t i = 0; i < 50; i++) {
            int width, height;
            OGLW_CHECK(glfwGetFramebufferSize, win.getWindowPtr(), &width, &height);
            OGLW_CHECK(glViewport, 0, 0, width, height);

            gpu_shader.run();

            OGLW_CHECK(glfwSwapBuffers, win.getWindowPtr());
            OGLW_CHECK(glfwPollEvents);
        }
    }
}