#include "catch2/catch.hpp"

#include <oglw/image.h>
#include <oglw/image_utils.h>
#include <oglw/gl_utils.h>

#include "gl_window.h"

#include <iostream>
#include <chrono>
#include <vector>

namespace {

template <typename T>
void SetAll(oglw::CpuImage<T>& img) {
    for (size_t y = 0; y < img.getHeight(); y++) {
        for (size_t x = 0; x < img.getWidth(); x++) {
            for (size_t c = 0; c < img.getDepth(); c++) {
                img.at(x, y, c) = y + x + c;
            }
        }
    }
}

template <typename T>
bool CheckAll(T& img) {
    for (size_t y = 0; y < img.getHeight(); y++) {
        for (size_t x = 0; x < img.getWidth(); x++) {
            for (size_t c = 0; c < img.getDepth(); c++) {
                if (img.at(x, y, c) != y + x + c) {
                    return false;
                }
            }
        }
    }
    return true;
}

template <typename T>
bool CheckAllByPtr(T& img) {
    auto p = img.data();
    for (size_t y = 0; y < img.getHeight(); y++) {
        for (size_t x = 0; x < img.getWidth(); x++) {
            for (size_t c = 0; c < img.getDepth(); c++) {
                if (*p++ != y + x + c) {
                    return false;
                }
            }
        }
    }
    return true;
}
}  // namespace anonymous

// =============================================================================

TEST_CASE("Image test") {

    SECTION("Image Instances") {
        oglw::GlWindow win("Title");
        oglw::CpuImage<uint8_t> cpu_img_8u;
        oglw::CpuImage<oglw::Float16> cpu_img_16f;
        oglw::CpuImage<float> cpu_img_32f;
        oglw::GpuImage<uint8_t> gpu_img_8u;
        oglw::GpuImage<oglw::Float16> gpu_img_16f;
        oglw::GpuImage<float> gpu_img_32f;
    }

    SECTION("CpuImage Basic uint8_t") {
        oglw::CpuImage<uint8_t> img(10, 20, 3);
        REQUIRE(!img.empty());
        REQUIRE(img.getWidth() == 10);
        REQUIRE(img.getHeight() == 20);
        REQUIRE(img.getDepth() == 3);

        // Set
        SetAll(img);

        // Get
        REQUIRE(CheckAll(img));

        // Pointer access
        REQUIRE(CheckAllByPtr(img));
    }

    SECTION("CpuImage Basic float16") {
        oglw::CpuImage<oglw::Float16> img(10, 20, 3);
        // Set
        SetAll(img);
        // Get
        REQUIRE(CheckAll(img));
    }

    SECTION("CpuImage Basic float") {
        oglw::CpuImage<float> img(10, 20, 3);
        // Set
        SetAll(img);
        // Get
        REQUIRE(CheckAll(img));
    }

    SECTION("CpuImage Move-constructed") {
        oglw::CpuImage<uint8_t> img1(10, 20, 3);
        SetAll(img1);

        // Move
        const auto img2 = std::move(img1);

        // Get (const)
        REQUIRE(CheckAll(img2));

        // Pointer access (const)
        REQUIRE(CheckAllByPtr(img2));
    }

    SECTION("CpuImage Move-operator") {
        oglw::CpuImage<uint8_t> img1(10, 20, 3);
        SetAll(img1);

        // Move
        oglw::CpuImage<uint8_t> img2;
        img2 = std::move(img1);

        // Get (const)
        REQUIRE(CheckAll(img2));

        // Pointer access (const)
        REQUIRE(CheckAllByPtr(img2));
    }

    SECTION("CpuImage Copy-constructed") {
        oglw::CpuImage<uint8_t> img1(10, 20, 3);
        SetAll(img1);

        // Copy
        const auto img2 = img1;
        img1.at(3, 2, 1) = 45;

        // Get (const)
        REQUIRE(!CheckAll(img1));
        REQUIRE(img1.at(3, 2, 1) == 45);
        REQUIRE(CheckAll(img2));
    }

    SECTION("CpuImage Copy-operator") {
        oglw::CpuImage<uint8_t> img1(10, 20, 3);
        SetAll(img1);

        // Copy
        oglw::CpuImage<uint8_t> img2;
        img2 = img1;
        img1.at(3, 2, 1) = 45;

        // Get (const)
        REQUIRE(!CheckAll(img1));
        REQUIRE(img1.at(3, 2, 1) == 45);
        REQUIRE(CheckAll(img2));
    }

    SECTION("CpuImage foreach parallel") {
        // Non-const, Non-const
        oglw::CpuImage<uint8_t> img(10, 20, 3);
        img.foreach([](size_t x, size_t y, size_t c, uint8_t& v) {
            v = x + y + c;
        });
        // Non-const, Const
        img.foreach([](size_t x, size_t y, size_t c, const uint8_t& v) {
            REQUIRE(v == x + y + c);
        });
        // Const, Const
        const auto img2 = std::move(img);
        img2.foreach([](size_t x, size_t y, size_t c, const uint8_t& v) {
            REQUIRE(v == x + y + c);
        });
    }

    SECTION("CpuImage foreach simple") {
        // Non-const, Non-const
        oglw::CpuImage<uint8_t> img(10, 20, 3);
        img.foreach([](size_t x, size_t y, size_t c, uint8_t& v) {
            v = x + y + c;
        }, 1);
        // Non-const, Const
        img.foreach([](size_t x, size_t y, size_t c, const uint8_t& v) {
            REQUIRE(v == x + y + c);
        }, 1);
        // Const, Const
        const auto img2 = std::move(img);
        img2.foreach([](size_t x, size_t y, size_t c, const uint8_t& v) {
            REQUIRE(v == x + y + c);
        }, 1);
    }

    SECTION("CpuImage foreach parallel pixel") {
        // Non-const, Non-const
        oglw::CpuImage<uint8_t> img(10, 20, 3);
        img.foreach([](size_t x, size_t y, uint8_t* vs) {
            for (size_t c = 0; c < 3; c++) {
                vs[c] = x + y + c;
            }
        });
        // Non-const, Const
        img.foreach([](size_t x, size_t y, const uint8_t* vs) {
            for (size_t c = 0; c < 3; c++) {
                REQUIRE(vs[c] == x + y + c);
            }
        });
        // Const, Const
        const auto img2 = std::move(img);
        img2.foreach([](size_t x, size_t y, const uint8_t* vs) {
            for (size_t c = 0; c < 3; c++) {
                REQUIRE(vs[c] == x + y + c);
            }
        });
    }

    SECTION("CpuImage foreach simple pixel") {
        // Non-const, Non-const
        oglw::CpuImage<uint8_t> img(10, 20, 3);
        img.foreach([](size_t x, size_t y, uint8_t* vs) {
            for (size_t c = 0; c < 3; c++) {
                vs[c] = x + y + c;
            }
        }, 1);
        // Non-const, Const
        img.foreach([](size_t x, size_t y, const uint8_t* vs) {
            for (size_t c = 0; c < 3; c++) {
                REQUIRE(vs[c] == x + y + c);
            }
        }, 1);
        // Const, Const
        const auto img2 = std::move(img);
        img2.foreach([](size_t x, size_t y, const uint8_t* vs) {
            for (size_t c = 0; c < 3; c++) {
                REQUIRE(vs[c] == x + y + c);
            }
        }, 1);
    }

// =============================================================================
    SECTION("GpuImage Basic 1ch") {
        oglw::GlWindow win("Title");
        oglw::CpuImage<uint8_t> cpu_img1(10, 20, 1);
        SetAll(cpu_img1);
        oglw::GpuImage<uint8_t> gpu_img = cpu_img1.toGpu();
        oglw::CpuImage<uint8_t> cpu_img2 = gpu_img.toCpu();
        REQUIRE(CheckAll(cpu_img2));
    }

    SECTION("GpuImage Basic 2ch") {
        oglw::GlWindow win("Title");
        oglw::CpuImage<uint8_t> cpu_img1(10, 20, 2);
        SetAll(cpu_img1);
        oglw::GpuImage<uint8_t> gpu_img = cpu_img1.toGpu();
        oglw::CpuImage<uint8_t> cpu_img2 = gpu_img.toCpu();
        REQUIRE(CheckAll(cpu_img2));
    }

    SECTION("GpuImage Basic 3ch") {
        oglw::GlWindow win("Title");
        oglw::CpuImage<uint8_t> cpu_img1(10, 20, 3);
        SetAll(cpu_img1);
        oglw::GpuImage<uint8_t> gpu_img = cpu_img1.toGpu();
        oglw::CpuImage<uint8_t> cpu_img2 = gpu_img.toCpu();
        REQUIRE(CheckAll(cpu_img2));
    }

    SECTION("GpuImage Basic 4ch") {
        oglw::GlWindow win("Title");
        oglw::CpuImage<uint8_t> cpu_img1(10, 20, 4);
        SetAll(cpu_img1);
        oglw::GpuImage<uint8_t> gpu_img = cpu_img1.toGpu();
        oglw::CpuImage<uint8_t> cpu_img2 = gpu_img.toCpu();
        REQUIRE(CheckAll(cpu_img2));
    }

    SECTION("GpuImage float16") {
        oglw::GlWindow win("Title");
        oglw::CpuImage<oglw::Float16> cpu_img1(10, 20, 4);
        SetAll(cpu_img1);
        oglw::GpuImage<oglw::Float16> gpu_img = cpu_img1.toGpu();
        oglw::CpuImage<oglw::Float16> cpu_img2 = gpu_img.toCpu();
        REQUIRE(CheckAll(cpu_img2));
    }

    SECTION("GpuImage float32") {
        oglw::GlWindow win("Title");
        oglw::CpuImage<float> cpu_img1(10, 20, 4);
        SetAll(cpu_img1);
        oglw::GpuImage<float> gpu_img = cpu_img1.toGpu();
        oglw::CpuImage<float> cpu_img2 = gpu_img.toCpu();
        REQUIRE(CheckAll(cpu_img2));
    }
}

