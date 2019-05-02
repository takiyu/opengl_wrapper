#ifndef OGLW_HAIR_LOADER_H_190429
#define OGLW_HAIR_LOADER_H_190429

#include <memory>
#include <string>
#include <map>

#include <oglw/geometry.h>
#include <oglw/types.h>

namespace oglw {

// ================================ Hair Loader ================================
GeometryPtr LoadHair(const std::string& filename,
                     const Vec3& shift = {0.f, 0.f, 0.f});

}  // namespace oglw

#endif

