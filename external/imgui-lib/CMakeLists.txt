add_library(
    imgui
    imgui/imconfig.h
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_internal.h
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h
    imgui/examples/imgui_impl_glfw.h
    imgui/examples/imgui_impl_glfw.cpp
    imgui/examples/imgui_impl_opengl3.h
    imgui/examples/imgui_impl_opengl3.cpp
)

remove_definitions(IMGUI_IMPL_OPENGL_LOADER_GLEW)
add_compile_definitions(IMGUI_IMPL_OPENGL_LOADER_GLAD)

target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui/examples")

target_link_libraries(imgui PRIVATE glad glfw)
