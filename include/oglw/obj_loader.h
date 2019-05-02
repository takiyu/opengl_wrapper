#ifndef OGLW_OBJ_LOADER_H_190429
#define OGLW_OBJ_LOADER_H_190429

#include <memory>
#include <string>
#include <map>

#include <oglw/geometry.h>
#include <oglw/types.h>

namespace oglw {

enum class ObjLoaderMode {
    INDEXING,
    INDEXING_VTX_ONLY,
    NO_INDICING,
};

// ================================ Obj Loader =================================
void LoadObj(const std::string& filename,
             std::map<std::string, GeometryPtr>& geoms,
             ObjLoaderMode mode = ObjLoaderMode::INDEXING,
             const Vec3& shift = {0.f, 0.f, 0.f});

}  // namespace oglw

#endif
