#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

class Shader {
public:
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void use() const;

    // uniform setters
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, const glm::vec3& v) const;
    void setMat3(const std::string& name, const glm::mat3& m) const;
    void setMat4(const std::string& name, const glm::mat4& m) const;

    GLuint id() const { return id_; }

private:
    GLuint id_;

    static std::string loadSource(const std::string& path);
    static GLuint compile(GLenum type, const std::string& source);
};
