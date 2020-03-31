find_package(glfw3 REQUIRED FATAL_ERROR)

add_executable(OpenGL
    main.cpp
    window.h
    window.cpp
    texture.h
    texture.cpp
    shader.h
    shader.cpp
    vertex.h
    vertex.cpp
    model.h
    model.cpp
    camera.h
    camera.cpp
    common.h
    common.cpp
    light.h
    light.cpp
    )

set(COMPILE_FLAGS "-g -DLOG_USE_COLOR ${COMPILE_FLAGS}")
set(CMAKE_BUILD_TYPE Debug)

target_link_libraries(OpenGL PRIVATE assimp glfw3 stb GL log glad rt dl m Xrandr Xi X11 pthread xcb Xau Xdmcp)
