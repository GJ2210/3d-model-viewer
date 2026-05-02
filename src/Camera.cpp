#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

// Stores the initial orbit camera settings so reset can restore them later.
Camera::Camera(float radius, float theta, float phi)
    : radius_(radius), theta_(theta), phi_(phi), target_(0.0f),
      initRadius_(radius), initTheta_(theta), initPhi_(phi), initTarget_(0.0f)
{}

// Converts spherical orbit coordinates into a world-space camera position.
glm::vec3 Camera::getPosition() const {
    // spherical coords
    return target_ + glm::vec3(
        radius_ * std::sin(phi_) * std::sin(theta_),
        radius_ * std::cos(phi_),
        radius_ * std::sin(phi_) * std::cos(theta_)
    );
}

// Builds the view matrix that looks from the camera position to the target.
glm::mat4 Camera::getView() const {
    glm::vec3 pos = getPosition();
    glm::vec3 up = (phi_ < 0.1f || phi_ > 3.04f)
        ? glm::vec3(0.0f, 0.0f, (phi_ < 1.57f ? 1.0f : -1.0f))
        : glm::vec3(0.0f, 1.0f, 0.0f);
    return glm::lookAt(pos, target_, up);
}

// Builds either the perspective or orthographic projection matrix.
glm::mat4 Camera::getProjection(float aspectRatio) const {
    if (projType == ProjectionType::Perspective) {
        return glm::perspective(glm::radians(FOV), aspectRatio, 0.01f, 1000.0f);
    }
    // ortho zoom
    float h = radius_ * std::tan(glm::radians(FOV * 0.5f));
    float w = h * aspectRatio;
    return glm::ortho(-w, w, -h, h, -1000.0f, 1000.0f);
}

// Rotates the camera around the target while keeping phi within safe limits.
void Camera::orbit(float dTheta, float dPhi) {
    theta_ += dTheta;
    phi_ = std::clamp(phi_ + dPhi, PHI_MIN, PHI_MAX);
}

// Moves the camera target sideways/upward based on screen-space drag distance.
void Camera::pan(float dx, float dy) {
    glm::mat4 view = getView();
    glm::vec3 right = glm::vec3(view[0][0], view[1][0], view[2][0]);
    glm::vec3 up    = glm::vec3(view[0][1], view[1][1], view[2][1]);

    const float speed = radius_ * 0.001f;
    target_ -= right * (dx * speed);
    target_ += up * (dy * speed);
}

// Changes camera distance from the target while clamping the zoom range.
void Camera::zoom(float delta) {
    radius_ = std::clamp(radius_ - delta * 0.3f, ZOOM_MIN, ZOOM_MAX);
}

// Restores the camera to its constructor-provided starting position.
void Camera::reset() {
    radius_ = initRadius_;
    theta_ = initTheta_;
    phi_ = initPhi_;
    target_ = initTarget_;
}
