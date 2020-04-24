set (CMAKE_CXX_STANDARD 17)

add_library(
    fluidsim

    src/fluidsim.cpp

    include/fluidsim/fluidsim.h
    )

set(CMAKE_BUILD_TYPE Debug)
target_include_directories(fluidsim PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(fluidsim PRIVATE glm)
