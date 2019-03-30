#define CATCH_CONFIG_RUNNER
#include "catch2/catch.hpp"

#include "init_gl.h"

int main(int argc, char* argv[]) {
    auto window = InitOpenGL("Title");
    if (!window) {
        std::cerr << "Failed to create a OpenGL window" << std::endl;
        return 1;
    }
    const int res = Catch::Session().run(argc, argv);
    return res;
}
