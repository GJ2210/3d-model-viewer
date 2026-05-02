#pragma once

#include <glm/glm.hpp>

enum class ProjectionType { Perspective, Orthographic };

class Camera {
public:
    explicit Camera(float radius = 5.0f, float theta = 0.5f, float phi = 1.1f);

    glm::mat4 getView() const;
    glm::mat4 getProjection(float aspectRatio) const;
    glm::vec3 getPosition() const;

    // camera controls
    void orbit(float dTheta, float dPhi);
    void pan(float dx, float dy);
    void zoom(float delta);
    void reset();

    ProjectionType projType = ProjectionType::Perspective;

private:
    float radius_, theta_, phi_;
    glm::vec3 target_;

    // saved defaults
    float initRadius_, initTheta_, initPhi_;
    glm::vec3 initTarget_;

    static constexpr float FOV = 45.0f;
    static constexpr float PHI_MIN = 0.05f;
    static constexpr float PHI_MAX = 3.09f;
    static constexpr float ZOOM_MIN = 0.5f;
    static constexpr float ZOOM_MAX = 50.0f;
};
