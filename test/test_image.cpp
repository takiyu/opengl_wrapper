#include "catch2/catch.hpp"

#include "../include/image.h"
#include "../include/image_utils.h"

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
    for (size_t y = 0; y < 20; y++) {
        for (size_t x = 0; x < 10; x++) {
            for (size_t c = 0; c < 3; c++) {
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

    SECTION("CpuImage Basic") {
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
    SECTION("GpuImage Basic") {
    }
}
