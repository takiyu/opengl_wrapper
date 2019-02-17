#include "catch2/catch.hpp"

#include "../src/any.h"

TEST_CASE("Any test") {
    SECTION("Initialize with constructor") {
        oglw::Any v = 1;
        REQUIRE(v.type() == typeid(int));
        REQUIRE(v.cast<int>() == 1);
    }

    SECTION("Initialize with operator") {
        oglw::Any v;
        REQUIRE(!v.hasValue());
        v = 2;
        REQUIRE(v.type() == typeid(int));
        REQUIRE(v.cast<int>() == 2);
        REQUIRE(v.hasValue());
    }

    SECTION("Emplace and Reset") {
        oglw::Any v;
        REQUIRE(!v.hasValue());
        v.emplace<int>(3);
        REQUIRE(v.type() == typeid(int));
        REQUIRE(v.cast<int>() == 3);
        REQUIRE(v.hasValue());
        v.reset();
        REQUIRE(!v.hasValue());
    }

    SECTION("Copy") {
        oglw::Any v1 = 4;
        oglw::Any v2 = 5.f;
        REQUIRE(v1.type() == typeid(int));
        REQUIRE(v2.type() == typeid(float));
        v1 = v2;
        REQUIRE(v1.type() == typeid(float));
        REQUIRE(v2.type() == typeid(float));
        REQUIRE(v1.cast<float>() == v2.cast<float>());
        v2 = 6;
        REQUIRE(v1.cast<float>() == Approx(5.f));
        REQUIRE(v2.cast<int>() == 6);
    }

    SECTION("Copy const") {
        oglw::Any v1 = 4;
        const oglw::Any v2 = 5.f;
        REQUIRE(v1.type() == typeid(int));
        REQUIRE(v2.type() == typeid(float));
        v1 = v2;
        REQUIRE(v1.type() == typeid(float));
        REQUIRE(v2.type() == typeid(float));
        REQUIRE(v1.cast<float>() == v2.cast<float>());
    }

    SECTION("Move") {
        oglw::Any v1 = 4;
        oglw::Any v2 = 5.f;
        REQUIRE(v1.type() == typeid(int));
        REQUIRE(v2.type() == typeid(float));
        v1 = std::move(v2);
        REQUIRE(v1.type() == typeid(float));
        REQUIRE(!v2.hasValue());
        REQUIRE(v1.cast<float>() == Approx(5.f));
        v2 = 6;
        REQUIRE(v1.cast<float>() == Approx(5.f));
        REQUIRE(v2.cast<int>() == 6);
    }

    SECTION("Bad cast") {
        oglw::Any v = 10;
        try {
            v.cast<float>();
            REQUIRE(false);
        } catch (oglw::BadAnyCast &e) {
            REQUIRE(*e.correct_type == typeid(int));
            REQUIRE(*e.tried_type == typeid(float));
        }
    }
}
