set (CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)

add_executable(Guppy
    main.cpp
    )

set(COMPILE_FLAGS "-g -DLOG_USE_COLOR ${COMPILE_FLAGS}")
set(CMAKE_BUILD_TYPE Debug)

IF (WIN32)
    target_link_libraries(Guppy PRIVATE engine reactphysics3d fluidsim assimp glfw imgui glm stb log glad opengl32)
ELSE()
    target_link_libraries(Guppy PRIVATE engine reactphysics3d fluidsim assimp glfw imgui glm stb GL log glad rt dl m Xrandr Xi X11 pthread xcb Xau Xdmcp)
ENDIF()
