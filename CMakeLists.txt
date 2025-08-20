cmake_minimum_required(VERSION 3.14)
project(my_bgfx_app)

set(CMAKE_CXX_STANDARD 17)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)

# Add bgfx.cmake
add_subdirectory(3rd/bgfx.cmake)

# Add executable

include_directories(3rd/bgfx.cmake/bgfx/examples)

add_subdirectory(3rd/LASzip)
include_directories(3rd/bgfx.cmake/LASzip/include/)
# Link only bgfx (not example-common!)

find_package(X11 REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
# Set include path for shaderc to find bgfx_shader.sh
set(BGFX_SHADER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rd/bgfx.cmake/bgfx/src)

# Shader compilation (vertex + fragment)
bgfx_compile_shaders(
        TYPE VERTEX
        SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vs_triangle.sc
        VARYING_DEF ${CMAKE_CURRENT_SOURCE_DIR}/shaders/varying.def.sc
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders
        AS_HEADERS
        INCLUDE_DIRS ${BGFX_SHADER_INCLUDE_DIR}
)

bgfx_compile_shaders(
        TYPE FRAGMENT
        SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/fs_triangle.sc
        VARYING_DEF ${CMAKE_CURRENT_SOURCE_DIR}/shaders/varying.def.sc
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders
        AS_HEADERS
        INCLUDE_DIRS ${BGFX_SHADER_INCLUDE_DIR}
)

# Pass output path to the app
add_executable(my_bgfx_app main.cpp color_las_loader.cpp shaders/fs_triangle.sc shaders/vs_triangle.sc)
target_link_libraries(my_bgfx_app PRIVATE bgfx bx bimg  example-common)
# Include path for helper header
target_include_directories(my_bgfx_app PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(my_bgfx_app PRIVATE X11::X11 ${GLFW_LIBRARIES} Eigen3::Eigen  laszip)