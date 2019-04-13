#ifndef OGLW_IMAGE_UTILS_H_190330
#define OGLW_IMAGE_UTILS_H_190330

namespace oglw {

// ================================ Image Utils ================================

template <typename T1, typename T2>
bool IsSameSize(const T1& lhs, const T2& rhs) {
    return lhs.getWidth() == rhs.getWidth() &&
           lhs.getHeight() == rhs.getHeight() &&
           lhs.getDepth() == rhs.getDepth();
}

}  // namespace oglw

#endif /* end of include guard */
