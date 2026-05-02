#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <glm/gtc/type_ptr.hpp>

// Loads, compiles, and links a vertex/fragment shader program.
Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    GLuint vert = compile(GL_VERTEX_SHADER, loadSource(vertexPath));
    GLuint frag = compile(GL_FRAGMENT_SHADER, loadSource(fragmentPath));

    id_ = glCreateProgram();
    glAttachShader(id_, vert);
    glAttachShader(id_, frag);
    glLinkProgram(id_);

    GLint ok = 0;
    glGetProgramiv(id_, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(id_, sizeof(log), nullptr, log);
        glDeleteShader(vert);
        glDeleteShader(frag);
        throw std::runtime_error(std::string("Shader link error:\n") + log);
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
}

// Deletes the linked OpenGL shader program.
Shader::~Shader() {
    glDeleteProgram(id_);
}

// Activates this shader program for subsequent draw calls.
void Shader::use() const { glUseProgram(id_); }

// Sets an integer uniform by name.
void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(id_, name.c_str()), value);
}

// Sets a float uniform by name.
void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(id_, name.c_str()), value);
}

// Sets a vec3 uniform by name.
void Shader::setVec3(const std::string& name, const glm::vec3& v) const {
    glUniform3fv(glGetUniformLocation(id_, name.c_str()), 1, glm::value_ptr(v));
}

// Sets a 3x3 matrix uniform by name.
void Shader::setMat3(const std::string& name, const glm::mat3& m) const {
    glUniformMatrix3fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, glm::value_ptr(m));
}

// Sets a 4x4 matrix uniform by name.
void Shader::setMat4(const std::string& name, const glm::mat4& m) const {
    glUniformMatrix4fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, glm::value_ptr(m));
}

// Reads an entire shader source file into a string.
std::string Shader::loadSource(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Cannot open shader file: " + path);
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Compiles one shader stage and reports compiler errors.
GLuint Shader::compile(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        const char* typeStr = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
        throw std::runtime_error(std::string(typeStr) + " shader compile error:\n" + log);
    }
    return shader;
}
