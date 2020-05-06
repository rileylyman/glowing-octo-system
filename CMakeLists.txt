cmake_minimum_required(VERSION 3.13)
project(opengl VERSION 0.1.0)

set(JSON_BuildTests OFF CACHE INTERNAL "")

set(RP3D_LOGS_ENABLED ON)
set(RP3D_PROFILING_ENABLED OFF)

set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(external/stb)
add_subdirectory(external/json)
add_subdirectory(external/reactphysics3d)
add_subdirectory(external/glfw)
add_subdirectory(external/glm)
add_subdirectory(external/glad)
add_subdirectory(external/assimp)
add_subdirectory(external/rxi_log)
add_subdirectory(external/imgui-lib)

add_subdirectory(fluidsim)
add_subdirectory(engine)

add_subdirectory(src)
