find_package(glfw3 REQUIRED FATAL_ERROR)
find_package(glad REQUIRED FATAL_ERROR)

add_executable(OpenGL
    main.cpp
    shader_s.h
    )

target_link_libraries(OpenGL PRIVATE glfw3 glad rt dl m X11 pthread xcb Xau Xdmcp)
