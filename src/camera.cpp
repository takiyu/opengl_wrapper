#include <oglw/camera.h>

#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace oglw {

namespace {

Vec3 RotateOrbit(const Vec3& eye, const Vec3& center, float dtheta, float dphi) {
    // TODO: Consider up vector
    Vec3 dir = center - eye;
    const float dir_norm = dir.norm();
    dir *= 1.0 / dir_norm;  // normalize

    float theta = std::atan2(dir[0], dir[2]);
    float phi = std::asin(dir[1]);
    theta += dtheta;
    phi += dphi;
    // Check phi range
    if (HALF_PI < phi) {
        phi = HALF_PI;
    } else if (phi < -HALF_PI) {
        phi = -HALF_PI;
    }

    dir[0] = std::cos(phi) * std::sin(theta);
    dir[1] = std::sin(phi);
    dir[2] = std::cos(phi) * std::cos(theta);

    dir *= dir_norm;  // reverse normalize
    return center - dir;
}

Mat4 ProjectPerspective(float fov, int width, int height, float near,
                        float far) {
    const float rad = fov / 180.0f * PI;
    const float fov_y_tan_half = std::tan(rad / 2.f);
    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    Mat4 proj_mat = Mat4::Zero();
    proj_mat(0, 0) = 1.f / (aspect * fov_y_tan_half);
    proj_mat(1, 1) = 1.f / (fov_y_tan_half);
    proj_mat(2, 2) = -(far + near) / (far - near);
    proj_mat(3, 2) = -1.f;
    proj_mat(2, 3) = -(2.f * far * near) / (far - near);
    return std::move(proj_mat);
}

Mat4 LookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
    Mat4 view_mat;

    const Vec3& z_axis = (center - eye).normalized();
    const Vec3& x_axis = z_axis.cross(up).normalized();
    const Vec3& y_axis = x_axis.cross(z_axis).normalized();

    view_mat(0, 0) = x_axis[0];
    view_mat(0, 1) = x_axis[1];
    view_mat(0, 2) = x_axis[2];
    view_mat(1, 0) = y_axis[0];
    view_mat(1, 1) = y_axis[1];
    view_mat(1, 2) = y_axis[2];
    view_mat(2, 0) = -z_axis[0];
    view_mat(2, 1) = -z_axis[1];
    view_mat(2, 2) = -z_axis[2];

    view_mat(0, 3) = -(x_axis.dot(eye));
    view_mat(1, 3) = -(y_axis.dot(eye));
    view_mat(2, 3) = (z_axis.dot(eye));

    view_mat(3, 0) = 0.f;
    view_mat(3, 1) = 0.f;
    view_mat(3, 2) = 0.f;
    view_mat(3, 3) = 1.f;

    return view_mat;
}

}  // namespace

// ================================== Camera ===================================

class Camera::Impl {
public:
    Impl() {}

    Impl(const Impl& lhs) = default;

    Impl(Impl&&) = delete;

    Impl& operator=(const Impl& lhs) = default;

    Impl& operator=(Impl&&) = delete;

    ~Impl() {}

    // -------------------------------------------------------------------------
    void setScreenSize(int width, int height) {
        m_width = width;
        m_height = height;
    }

    void setFov(float fov) {
        m_fov = fov;
    }

    void setNearFar(float near, float far) {
        if (near <= 0.f) {
            std::stringstream ss;
            ss << near;
            throw std::runtime_error("Invalid near value: " + ss.str());
        }
        m_near = near;
        m_far = far;
    }

    // -------------------------------------------------------------------------
    void setEye(const Vec3& eye) {
        m_eye = eye;
    }

    void setCenter(const Vec3& center) {
        m_center = center;
    }

    // -------------------------------------------------------------------------
    void rotateOrbit(float dtheta, float dphi) {
        m_eye = RotateOrbit(m_eye, m_center, dtheta, dphi);
    }

    // -------------------------------------------------------------------------
    Mat4 getProj() {
        return ProjectPerspective(m_fov, m_width, m_height, m_near, m_far);
    }

    Mat4 getView() {
        return LookAt(m_eye, m_center, m_up);
    }

    Vec3 getEye() {
        return m_eye;
    }

    // -------------------------------------------------------------------------
private:
    Vec3 m_eye = {0.f, 0.f, 1.f};
    Vec3 m_center = {0.f, 0.f, 0.f};
    Vec3 m_up = {0.f, 1.f, 0.f};
    int m_width = 500, m_height = 500;
    float m_fov = 45.f;
    float m_near = 0.0001, m_far = 10000.f;
};

// -----------------------------------------------------------------------------
// ------------------------------- Pimpl Pattern -------------------------------
// -----------------------------------------------------------------------------
Camera::Camera() : m_impl(std::make_unique<Impl>()) {}

Camera::Camera(const Camera& lhs)
    : m_impl(std::make_unique<Impl>(*lhs.m_impl)) {}

Camera::Camera(Camera&&) = default;

Camera& Camera::operator=(const Camera& lhs) {
    *m_impl = *lhs.m_impl;
    return *this;
}

Camera& Camera::operator=(Camera&&) = default;

Camera::~Camera() = default;

// -----------------------------------------------------------------------------
void Camera::setScreenSize(int width, int height) {
    m_impl->setScreenSize(width, height);
}

void Camera::setFov(float fov) {
    m_impl->setFov(fov);
}

void Camera::setNearFar(float near, float far) {
    m_impl->setNearFar(near, far);
}

// -----------------------------------------------------------------------------
void Camera::setEye(const Vec3& eye) {
    m_impl->setEye(eye);
}

void Camera::setCenter(const Vec3& center) {
    m_impl->setCenter(center);
}

// -----------------------------------------------------------------------------
void Camera::rotateOrbit(float dtheta, float dphi) {
    m_impl->rotateOrbit(dtheta, dphi);
}

// -----------------------------------------------------------------------------
Mat4 Camera::getProj() {
    return m_impl->getProj();
}

Mat4 Camera::getView() {
    return m_impl->getView();
}

Vec3 Camera::getEye() {
    return m_impl->getEye();
}

// -----------------------------------------------------------------------------

}  // namespace oglw
