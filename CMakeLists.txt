cmake_minimum_required(VERSION 3.13)
project(opengl VERSION 0.1.0)

add_subdirectory(external/stb)
add_subdirectory(external/glfw)
add_subdirectory(external/glm)
add_subdirectory(external/glad)
add_subdirectory(external/assimp)
add_subdirectory(external/rxi_log)
add_subdirectory(external/imgui-lib)

add_subdirectory(src)
