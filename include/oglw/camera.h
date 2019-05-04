#ifndef OGLW_CAMERA_H_190501
#define OGLW_CAMERA_H_190501

#include <array>
#include <memory>
#include <vector>

#include <oglw/image.h>
#include <oglw/types.h>

namespace oglw {

// ================================== Camera ===================================
class Camera {
public:
    static auto Create() {
        return std::make_shared<Camera>();
    }

    Camera();

    Camera(const Camera&);
    Camera(Camera&&);
    Camera& operator=(const Camera&);
    Camera& operator=(Camera&&);
    virtual ~Camera();

    void setScreenSize(int width, int height);
    void setFov(float fov);
    void setNearFar(float near, float far);

    void setEye(const Vec3& eye);
    void setCenter(const Vec3& center);

    void rotateOrbit(float dtheta, float dphi);

    Mat4 getProj();
    Mat4 getView();
    Vec3 getEye();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace oglw

#endif /* end of include guard */

