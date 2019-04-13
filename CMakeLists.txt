cmake_minimum_required(VERSION 3.8.2)

message(STATUS "Build OpenGL Wrapper")

# ------------------------------------------------------------------------------
# ----------------------------------- Options ----------------------------------
# ------------------------------------------------------------------------------
set(OGLW_BUILD_TESTS ON CACHE BOOL "Build tests")
message(STATUS "Build tests: ${OGLW_BUILD_TESTS}")

# ------------------------------------------------------------------------------
# ----------------------------------- Common -----------------------------------
# ------------------------------------------------------------------------------
project(oglw CXX C)

set(CMAKE_CXX_STANDARD 14)  # C++ 14

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(LINK_TYPE STATIC)
else()
    set(LINK_TYPE SHARED)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# Print make commands for debug
# set(CMAKE_VERBOSE_MAKEFILE 1)

# Set default build type
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Output `compile_commands.json`
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# cmake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sanitizers)
find_package(Sanitizers) # Address sanitizer (-DSANITIZE_ADDRESS=ON)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Warning options
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(warning_options "-Wall -Wextra -Wconversion")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set(warning_options "-Wall -Wextra -Wcast-align -Wcast-qual \
                         -Wctor-dtor-privacy -Wdisabled-optimization \
                         -Wformat=2 -Winit-self \
                         -Wmissing-declarations -Wmissing-include-dirs \
                         -Wold-style-cast -Woverloaded-virtual \
                         -Wredundant-decls -Wshadow -Wsign-conversion \
                         -Wsign-promo  -Wno-old-style-cast\
                         -Wstrict-overflow=5 -Wundef -Wno-unknown-pragmas")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(warning_options "--pedantic -Wall -Wextra -Wcast-align -Wcast-qual \
                         -Wctor-dtor-privacy -Wdisabled-optimization \
                         -Wformat=2 -Winit-self -Wlogical-op \
                         -Wmissing-declarations -Wmissing-include-dirs \
                         -Wnoexcept -Wold-style-cast -Woverloaded-virtual \
                         -Wredundant-decls -Wshadow -Wsign-conversion \
                         -Wsign-promo -Wstrict-null-sentinel \
                         -Wstrict-overflow=5 -Wundef -Wno-unknown-pragmas")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warning_options "/W4")
else()
    message(WARNING "Unsupported compiler for warning options")
    message("CMAKE_CXX_COMPILER_ID is ${CMAKE_CXX_COMPILER_ID}")
endif()

# Utility function to setup a target (include, link, warning, sanitizer)
function(setup_target target includes libs)
    target_include_directories(${target} PUBLIC ${includes})
    target_link_libraries(${target} ${libs})
    set_target_properties(${target} PROPERTIES COMPILE_FLAGS ${warning_options})
    add_sanitizers(${target})
endfunction(setup_target)

# Utility function to setup a target simply (include, link)
function(setup_target_simple target includes libs)
    target_include_directories(${target} PUBLIC ${includes})
    target_link_libraries(${target} ${libs})
endfunction(setup_target_simple)

# ------------------------------------------------------------------------------
# --------------------------- Third Party Libraries ----------------------------
# ------------------------------------------------------------------------------

# OpenGL
find_package(OpenGL REQUIRED)
list(APPEND OGLW_INCLUDE ${OPENGL_INCLUDE_DIR})
list(APPEND OGLW_LIBRARY ${OPENGL_LIBRARY})

# GLAD
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad)
list(APPEND OGLW_INCLUDE ${GLAD_INCLUDE_DIRS})
list(APPEND OGLW_LIBRARY ${GLAD_LIBRARIES})

# ------------------------------------------------------------------------------
# ----------------------------- Internal Libraries -----------------------------
# ------------------------------------------------------------------------------

# oglw.so
add_library(oglw ${LINK_TYPE}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/image_cpu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/image_gpu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/effect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gl_utils.cpp
)

list(APPEND OGLW_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
setup_target(oglw "${OGLW_INCLUDE}" "${OGLW_LIBRARY}")
list(APPEND OGLW_LIBRARY oglw)

# ----------------------------------- tests ------------------------------------
# For tests
if (OGLW_BUILD_TESTS)
    # GLFW3
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
    set(GLFW_INSTALL OFF CACHE BOOL "")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/glfw)
    list(APPEND OGLW_TEST_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glfw/include)
    list(APPEND OGLW_TEST_LIBRARY glfw)

    # Catch2
    list(APPEND OGLW_TEST_INCLUDE
                ${CMAKE_CURRENT_SOURCE_DIR}/third_party/Catch2/single_include)

    # Executable file
    add_executable(oglw_test
        ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gl_window.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/test_image.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/test_fast_array.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/test_effect.cpp
    )
    setup_target(oglw_test "${OGLW_INCLUDE};${OGLW_TEST_INCLUDE}"
                 "${OGLW_LIBRARY};${OGLW_TEST_LIBRARY}")

endif()

# ------------------------------------------------------------------------------
# ----------------------------- Pass to the parent -----------------------------
# ------------------------------------------------------------------------------
if(HAS_PARENT)
    set(OGLW_INCLUDE ${OGLW_INCLUDE} PARENT_SCOPE)
    set(OGLW_LIBRARY ${OGLW_LIBRARY} PARENT_SCOPE)
endif()
