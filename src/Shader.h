/**
 * @file Shader.h
 * @brief GLSL shader program wrapper.
 *
 * Loads vertex and fragment shader GLSL source from files on disk,
 * compiles and links them into an OpenGL program, and provides
 * typed convenience setters for common uniform types.
 */
#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

class Shader {
public:
    /**
     * Compile and link a program from the given GLSL source files.
     * Throws std::runtime_error on compile/link failure.
     */
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    // Non-copyable – each instance owns a GL program object.
    Shader(const Shader&)            = delete;
    Shader& operator=(const Shader&) = delete;

    /// Bind this shader program for subsequent draw calls.
    void use() const;

    // ── Uniform setters ──────────────────────────────────────────────────
    void setInt  (const std::string& name, int          value) const;
    void setFloat(const std::string& name, float        value) const;
    void setVec3 (const std::string& name, const glm::vec3& v) const;
    void setMat3 (const std::string& name, const glm::mat3& m) const;
    void setMat4 (const std::string& name, const glm::mat4& m) const;

    GLuint id() const { return id_; }

private:
    GLuint id_;

    static std::string loadSource(const std::string& path);
    static GLuint      compile(GLenum type, const std::string& source);
};
