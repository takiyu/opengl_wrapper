#include "catch2/catch.hpp"

#include "init_gl.h"

TEST_CASE("Core test") {
    SECTION("Build basic") {
        REQUIRE(InitOpenGL("Title") != nullptr);
        REQUIRE(1 + 3 == 4);
    }

    SECTION("Build basic2") {
        REQUIRE(InitOpenGL("Title2") != nullptr);
        REQUIRE(1 + 3 == 4);
    }
}
