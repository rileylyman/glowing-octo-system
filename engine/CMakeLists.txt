set (CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)

add_library(
    engine STATIC

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
)

set(CMAKE_BUILD_TYPE Debug)
set(COMPILE_FLAGS "-g -DLOG_USE_COLOR ${COMPILE_FLAGS}")

target_include_directories(engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

IF (WIN32)
    target_link_libraries(engine PRIVATE fluidsim assimp glfw imgui glm stb log glad opengl32)
ELSE()
    target_link_libraries(engine PRIVATE fluidsim assimp glfw imgui glm stb GL log glad rt dl m Xrandr Xi X11 pthread xcb Xau Xdmcp)
ENDIF()
