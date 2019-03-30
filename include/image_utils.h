#ifndef IMAGE_UTILS_H_190330
#define IMAGE_UTILS_H_190330

namespace oglw {

// ================================ Image Utils ================================

template <typename T>
bool IsSameSize(const T& lhs, const T& rhs) {
    return lhs.getWidth() == rhs.getWidth() &&
           lhs.getHeight() == rhs.getHeight() &&
           lhs.getDepth() == rhs.getDepth();
}

}  // namespace oglw

#endif /* end of include guard */
