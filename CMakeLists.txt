cmake_minimum_required(VERSION 3.10)
project(encoder_profiler)

set(CMAKE_CXX_STANDING 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0)

add_executable(profiler src/main.cpp)

target_include_directories(profiler PRIVATE ${GST_INCLUDE_DIRS})
target_link_libraries(profiler PRIVATE ${GST_LIBRARIES})
target_compile_options(profiler PRIVATE
    ${GST_CFLAGS_OTHER}
    -Wall -Wextra)

