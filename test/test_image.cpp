#include "catch2/catch.hpp"

#include "../include/image.h"

#include <iostream>
#include <chrono>
#include <vector>

TEST_CASE("Image test") {

    SECTION("CpuImage Basic") {
        oglw::CpuImage<uint8_t> img(10, 20, 3);
        REQUIRE(img.getWidth() == 10);
        REQUIRE(img.getHeight() == 20);
        REQUIRE(img.getChannels() == 3);

        // Set
        for (size_t y = 0; y < 20; y++) {
            for (size_t x = 0; x < 10; x++) {
                for (size_t i = 0; i < 3; i++) {
                    img.at(x, y, i) = y + x + i;
                }
            }
        }

        // Get and check
        for (size_t y = 0; y < 20; y++) {
            for (size_t x = 0; x < 10; x++) {
                for (size_t i = 0; i < 3; i++) {
                    REQUIRE(img.at(x, y, i) == y + x + i);
                }
            }
        }
    }

    SECTION("CpuImage foreach parallel") {
        // Non-const, Non-const
        oglw::CpuImage<uint8_t> img(10, 20, 3);
        img.foreach([](size_t x, size_t y, size_t i, uint8_t& v) {
            v = x + y + i;
        });
        // Non-const, Const
        img.foreach([](size_t x, size_t y, size_t i, const uint8_t& v) {
            REQUIRE(v == x + y + i);
        });
        // Const, Const
        const auto img2 = std::move(img);
        img2.foreach([](size_t x, size_t y, size_t i, const uint8_t& v) {
            REQUIRE(v == x + y + i);
        });
    }

    SECTION("CpuImage foreach simple") {
        // Non-const, Non-const
        oglw::CpuImage<uint8_t> img(10, 20, 3);
        img.foreach([](size_t x, size_t y, size_t i, uint8_t& v) {
            v = x + y + i;
        }, 1);
        // Non-const, Const
        img.foreach([](size_t x, size_t y, size_t i, const uint8_t& v) {
            REQUIRE(v == x + y + i);
        }, 1);
        // Const, Const
        const auto img2 = std::move(img);
        img2.foreach([](size_t x, size_t y, size_t i, const uint8_t& v) {
            REQUIRE(v == x + y + i);
        }, 1);
    }

    SECTION("CpuImage foreach parallel pixel") {
        // Non-const, Non-const
        oglw::CpuImage<uint8_t> img(10, 20, 3);
        img.foreach([](size_t x, size_t y, uint8_t* vs) {
            for (size_t i = 0; i < 3; i++) {
                vs[i] = x + y + i;
            }
        });
        // Non-const, Const
        img.foreach([](size_t x, size_t y, const uint8_t* vs) {
            for (size_t i = 0; i < 3; i++) {
                REQUIRE(vs[i] == x + y + i);
            }
        });
        // Const, Const
        const auto img2 = std::move(img);
        img2.foreach([](size_t x, size_t y, const uint8_t* vs) {
            for (size_t i = 0; i < 3; i++) {
                REQUIRE(vs[i] == x + y + i);
            }
        });
    }

    SECTION("CpuImage foreach simple pixel") {
        // Non-const, Non-const
        oglw::CpuImage<uint8_t> img(10, 20, 3);
        img.foreach([](size_t x, size_t y, uint8_t* vs) {
            for (size_t i = 0; i < 3; i++) {
                vs[i] = x + y + i;
            }
        }, 1);
        // Non-const, Const
        img.foreach([](size_t x, size_t y, const uint8_t* vs) {
            for (size_t i = 0; i < 3; i++) {
                REQUIRE(vs[i] == x + y + i);
            }
        }, 1);
        // Const, Const
        const auto img2 = std::move(img);
        img2.foreach([](size_t x, size_t y, const uint8_t* vs) {
            for (size_t i = 0; i < 3; i++) {
                REQUIRE(vs[i] == x + y + i);
            }
        }, 1);
    }
}

