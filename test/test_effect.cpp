#include "catch2/catch.hpp"

#include "effect.h"
#include "gl_utils.h"
#include "gl_window.h"

#include <sstream>
#include <stdexcept>

TEST_CASE("Effect test") {
    SECTION("CpuEffect basic") {
        REQUIRE(1 + 3 == 4);
    }

    SECTION("GpuEffect basic") {
        oglw::GlWindow win("Title");

        oglw::GpuEffect gpu_effect;
        gpu_effect.init(0, 0, "", "");

        for (size_t i = 0; i < 50; i++) {
            int width, height;
            OGLW_CHECK(glfwGetFramebufferSize, win.getWindowPtr(), &width, &height);
            OGLW_CHECK(glViewport, 0, 0, width, height);

            gpu_effect.run();

            OGLW_CHECK(glfwSwapBuffers, win.getWindowPtr());
            OGLW_CHECK(glfwPollEvents);
        }
    }
}
