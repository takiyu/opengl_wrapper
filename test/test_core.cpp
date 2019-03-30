#include "catch2/catch.hpp"

TEST_CASE("Core test") {
    SECTION("Build basic") {
        REQUIRE(1 + 3 == 4);
    }

    SECTION("Build basic2") {
        REQUIRE(1 + 3 == 4);
    }
}
