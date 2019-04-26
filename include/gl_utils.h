#ifndef OGLW_GL_UTILS_H_190205
#define OGLW_GL_UTILS_H_190205

#include <functional>
#include <memory>

#if 1
#define OGLW_CHECK(func, ...)                              \
    [&]() {                                                \
        func(__VA_ARGS__);                                 \
        oglw::CheckOpenGlError(__FILE__, __LINE__, #func); \
    }()
#else
#define OGLW_CHECK(func, ...) func(__VA_ARGS__)
#endif

namespace oglw {

void CheckOpenGlError(const char* file, int line, const char* func);

}  // namespace oglw

#endif /* end of include guard */
