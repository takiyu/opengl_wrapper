#include "oglw.h"

namespace oglw {

class Image::Impl {
};

Image::Image() : m_impl(new Image::Impl) {}

}  // namespace oglw
