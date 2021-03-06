set (CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)

add_library(
    engine STATIC

    src/debug.cpp
    src/window.cpp
    src/texture.cpp
    src/shader.cpp
    src/skybox.cpp
    src/vertex.cpp
    src/model.cpp
    src/camera.cpp
    src/common.cpp
    src/imgui-instance.cpp
    src/framebuffer.cpp
    src/scene.cpp
    src/light.cpp
    src/kernel.cpp
    src/fsrender.cpp
    src/physics.cpp

    include/engine/debug.h
    include/engine/window.h
    include/engine/texture.h
    include/engine/shader.h
    include/engine/skybox.h
    include/engine/vertex.h
    include/engine/model.h
    include/engine/camera.h
    include/engine/common.h
    include/engine/light.h
    include/engine/imgui-instance.h
    include/engine/framebuffer.h
    include/engine/scene.h
    include/engine/kernel.h
    include/engine/fsrender.h
    include/engine/physics.h
)

set(CMAKE_BUILD_TYPE Debug)
set(COMPILE_FLAGS "-g -DLOG_USE_COLOR ${COMPILE_FLAGS}")

target_include_directories(engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(engine PRIVATE ${BULLET_INCLUDE_DIR})

message(STATUS "(engine) BULLET_INCLUDE_DIR = ${BULLET_INCLUDE_DIR}")
message(STATUS "(engine) BulletLib  = ${BulletLib}")

IF (WIN32)
    target_link_libraries(engine PRIVATE fluidsim nlohmann_json::nlohmann_json ${BulletLib} assimp glfw imgui glm stb log glad opengl32)
ELSE()
    target_link_libraries(engine PRIVATE fluidsim nlohmann_json::nlohmann_json ${BulletLib} assimp glfw imgui glm stb GL log glad rt dl m Xrandr Xi X11 pthread xcb Xau Xdmcp)
ENDIF()
