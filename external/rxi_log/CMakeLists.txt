add_library(
    log
    log/src/log.c
    log/src/log.h
)
target_include_directories(log PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/log/src")