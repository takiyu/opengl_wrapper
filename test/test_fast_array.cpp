#include "catch2/catch.hpp"

#include "../src/fast_array.h"

#include <iostream>
#include <chrono>
#include <vector>

class Timer {
public:
    Timer() {}

    void start() {
        m_start = std::chrono::system_clock::now();
    }

    void end() {
        m_end = std::chrono::system_clock::now();
    }

    float getElapsedMsec() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(m_end -
                                                                     m_start)
                .count();
    }

private:
    std::chrono::system_clock::time_point m_start, m_end;
};

TEST_CASE("FastArray test") {

    SECTION("Time comparison") {
        const size_t N = 100000000;
        Timer t1, t2;
        {
            // FastArray
            oglw::FastArray<int> a;
            t1.start();
            a.alloc(N);
            t1.end();
            std::cout << "FastArray: " << t1.getElapsedMsec() << std::endl;
        }
        {
            // Vector
            std::vector<int> v;
            t2.start();
            v.resize(N);
            t2.end();
            std::cout << "FastArray: " << t2.getElapsedMsec() << std::endl;
        }
        REQUIRE(t1.getElapsedMsec() < t2.getElapsedMsec());
    }

    SECTION("Copy") {
        oglw::FastArray<int> a(10);
        for (size_t i = 0; i < a.size(); i++) {
            a[i] = i;
        }

        oglw::FastArray<int> b = a;
        a[0] = 100;
        REQUIRE(a[0] == 100);
        REQUIRE(b[0] == 0);
        for (size_t i = 1; i < 10; i++) {
            REQUIRE(a[i] == b[i]);
        }
    }

    SECTION("Move") {
        oglw::FastArray<int> a(10);
        for (size_t i = 0; i < a.size(); i++) {
            a[i] = i;
        }

        oglw::FastArray<int> b = std::move(a);
        REQUIRE(b.size() == 10);
        REQUIRE(a.size() == 0);
    }

    SECTION("Resize") {
        oglw::FastArray<int> a(10);
        for (size_t i = 0; i < a.size(); i++) {
            a[i] = i;
        }

        // Previous values must be remained.
        a.resize(100);
        for (size_t i = 0; i < 10; i++) {
            REQUIRE(a[i] == i);
        }
        // Check the allocation
        for (size_t i = 0; i < a.size(); i++) {
            a[i] = i;
        }
    }

    SECTION("Resize with filling") {
        oglw::FastArray<int> a(10);
        for (size_t i = 0; i < a.size(); i++) {
            a[i] = i;
        }

        // Previous values must be remained.
        a.resize(100, 123);
        for (size_t i = 0; i < 10; i++) {
            REQUIRE(a[i] == i);
        }
        // Check new values
        for (size_t i = 10; i < 100; i++) {
            REQUIRE(a[i] == 123);
        }
    }
}
