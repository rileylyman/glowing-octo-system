cmake_minimum_required(VERSION 3.13)
project(opengl VERSION 0.1.0)

set(JSON_BuildTests OFF CACHE INTERNAL "")

set(CMAKE_BUILD_TYPE Debug)

set(BulletLib BulletDynamics BulletCollision)
set(BULLET_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/bullet3/src)

add_subdirectory(external/stb)
add_subdirectory(external/json)
add_subdirectory(external/bullet3)
add_subdirectory(external/glfw)
add_subdirectory(external/glm)
add_subdirectory(external/glad)
add_subdirectory(external/assimp)
add_subdirectory(external/rxi_log)
add_subdirectory(external/imgui-lib)

add_subdirectory(fluidsim)
add_subdirectory(engine)

add_subdirectory(src)
