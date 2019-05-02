#ifndef OGLW_VEC_TYPES_H_190205
#define OGLW_VEC_TYPES_H_190205

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#elif defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wstrict-overflow"
#endif

#include <Eigen/Core>
#include <Eigen/Geometry>

#ifdef __clang__
#pragma clang diagnostic pop
#elif __GNUC__
#pragma GCC diagnostic pop
#endif

namespace oglw {

using Vec4 = Eigen::Vector4f;
using Vec3 = Eigen::Vector3f;
using Vec2 = Eigen::Vector2f;
using Mat4 = Eigen::Matrix4f;
using Mat3 = Eigen::Matrix3f;
using Mat2 = Eigen::Matrix2f;

static constexpr float PI = static_cast<float>(EIGEN_PI);
static constexpr float HALF_PI = static_cast<float>(EIGEN_PI * 0.5);

}  // namespace oglw

#endif /* end of include guard */
