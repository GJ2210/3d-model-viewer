cmake_minimum_required(VERSION 3.16)
project(ModelViewer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# macOS: suppress OpenGL deprecation warnings (deprecated since macOS 10.14)
if(APPLE)
    add_definitions(-DGL_SILENCE_DEPRECATION)
endif()

find_package(OpenGL REQUIRED)
find_package(glfw3  REQUIRED)
find_package(GLEW   REQUIRED)
find_package(glm    REQUIRED)

set(SOURCES
    src/main.cpp
    src/Shader.cpp
    src/Camera.cpp
    src/Mesh.cpp
    src/OBJLoader.cpp
)

add_executable(ModelViewer ${SOURCES})

target_include_directories(ModelViewer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(ModelViewer PRIVATE
    OpenGL::GL
    glfw
    GLEW::GLEW
    glm::glm
)

# Copy shaders to build dir so the executable can find them at runtime
add_custom_command(TARGET ModelViewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:ModelViewer>/shaders
    COMMENT "Copying shaders to build directory"
)
