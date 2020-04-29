set (CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)

add_library(
    fluidsim STATIC

    src/fluidsim.cpp

    include/fluidsim/fluidsim.h
    )

set(CMAKE_BUILD_TYPE Debug)
target_include_directories(fluidsim PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

IF (WIN32)
    target_link_libraries(fluidsim PRIVATE engine assimp glfw imgui glm stb log glad opengl32)
ELSE()
    target_link_libraries(fluidsim PRIVATE engine assimp glfw imgui glm stb GL log glad rt dl m Xrandr Xi X11 pthread xcb Xau Xdmcp)
ENDIF()
