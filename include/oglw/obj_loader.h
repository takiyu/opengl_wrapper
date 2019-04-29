#ifndef OGLW_OBJ_LOADER_H_190429
#define OGLW_OBJ_LOADER_H_190429

#include <memory>
#include <string>

#include <oglw/geometry.h>

namespace oglw {

// ================================ Obj Loader =================================
void LoadObj(const std::string& filename, std::vector<std::shared_ptr<Geometry>>& geoms);

}  // namespace oglw

#endif
