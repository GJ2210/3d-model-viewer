/**
 * @file Camera.h
 * @brief Orbit camera supporting perspective and orthographic projections.
 *
 * The camera orbits a target point in spherical coordinates:
 *   - theta : azimuth (rotation around the world Y axis, radians)
 *   - phi   : polar angle from +Y axis (radians, clamped away from poles)
 *   - radius: distance from target (zoom level)
 *
 * Pan translates the target in the camera's local XY plane.
 * Projection type can be toggled at runtime.
 */
#pragma once

#include <glm/glm.hpp>

enum class ProjectionType { Perspective, Orthographic };

class Camera {
public:
    /**
     * @param radius  Initial orbit radius.
     * @param theta   Initial azimuth in radians.
     * @param phi     Initial polar angle in radians (measured from +Y).
     */
    explicit Camera(float radius = 5.0f, float theta = 0.5f, float phi = 1.1f);

    /// View matrix from the current spherical position looking at target.
    glm::mat4 getView() const;

    /// Projection matrix. @p aspectRatio = framebuffer width / height.
    glm::mat4 getProjection(float aspectRatio) const;

    /// World-space camera position (computed from spherical coords + target).
    glm::vec3 getPosition() const;

    // ── Camera manipulation ───────────────────────────────────────────────
    void orbit(float dTheta, float dPhi);   ///< Rotate around target.
    void pan  (float dx,     float dy);     ///< Translate target in camera XY.
    void zoom (float delta);                ///< Adjust radius.
    void reset();                           ///< Restore initial state.

    // ── Public state ──────────────────────────────────────────────────────
    ProjectionType projType = ProjectionType::Perspective;

private:
    float     radius_, theta_, phi_;
    glm::vec3 target_;

    // Saved initial values for reset()
    float     initRadius_, initTheta_, initPhi_;
    glm::vec3 initTarget_;

    static constexpr float FOV      = 45.0f;    // vertical FOV, degrees
    static constexpr float PHI_MIN  = 0.05f;    // prevent gimbal at north pole
    static constexpr float PHI_MAX  = 3.09f;    // ~pi - 0.05, prevent south pole
    static constexpr float ZOOM_MIN = 0.5f;
    static constexpr float ZOOM_MAX = 50.0f;
};
