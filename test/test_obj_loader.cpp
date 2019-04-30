#include "catch2/catch.hpp"

#include <oglw/geometry.h>
#include <oglw/gl_utils.h>
#include <oglw/gpu_buffer.h>
#include <oglw/gpu_shader.h>
#include <oglw/obj_loader.h>

#include "gl_window.h"

#include <sstream>
#include <stdexcept>

static void Perspective(float fov_y, float aspect, float z_near, float z_far,
                 std::array<float, 16>& proj_mat) {
    assert(aspect > 0);
    assert(zFar > zNear);

    float radf = fov_y / 180.0f * 3.1415962f;
    float fov_y_tan_half = std::tan(radf / 2.f);
    proj_mat.fill(0.f);
    proj_mat[0 + 4 * 0] = 1.f / (aspect * fov_y_tan_half);
    proj_mat[1 + 4 * 1] = 1.f / (fov_y_tan_half);
    proj_mat[2 + 4 * 2] = -(z_far + z_near) / (z_far - z_near);
    proj_mat[3 + 4 * 2] = -1.f;
    proj_mat[2 + 4 * 3] = -(2.f * z_far * z_near) / (z_far - z_near);
}

TEST_CASE("ObjLoader test") {
    SECTION("GpuShader basic") {
        oglw::GlWindow win("Title");

        std::map<std::string, oglw::GeometryPtr> geoms;
        oglw::LoadObj(
                "/home/takiyu/Projects/work/huawei/hair_geom/hairstyles/"
                "head_model.obj",
                geoms, oglw::ObjLoaderMode::INDEXING_VTX_ONLY,
                {0.f, -1.5, -1.1f});
        REQUIRE(geoms.size() == 1);
        oglw::GeometryPtr geom = geoms.begin()->second;

        const std::string VTX_SHADER =
                "#version 430\n"
                "layout (location=0) in vec3 position;\n"
                "layout (location=1) in vec3 normal;\n"
                "layout (location=2) in vec2 texcoord;\n"
                "uniform mat4 proj_mat;\n"
                "out vec3 frag_normal;\n"
                "out vec2 frag_texcoord;\n"
                "void main() {\n"
                "    gl_Position = proj_mat * vec4(position, 1.0);\n"
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

        for (size_t i = 0; i < 50; i++) {
            int width, height;
            OGLW_CHECK(glfwGetFramebufferSize, win.getWindowPtr(), &width,
                       &height);
            OGLW_CHECK(glViewport, 0, 0, width, height);

            std::array<float, 16> proj_mat;
            const float aspect = static_cast<float>(width) / height;
            Perspective(45.f, aspect, 0.0001f, 100000.f, proj_mat);
            gpu_shader->setUniform("proj_mat", proj_mat);

            OGLW_CHECK(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            OGLW_CHECK(glClearColor, 0.3, 0.3, 1.0, 1.0);

            glEnable(GL_DEPTH_TEST);
            geom->draw(oglw::PrimitiveType::TRIANGLE);

            OGLW_CHECK(glfwSwapBuffers, win.getWindowPtr());
            OGLW_CHECK(glfwPollEvents);
        }
    }
}
