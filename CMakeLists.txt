cmake_minimum_required(VERSION 3.14)
project(my_bgfx_app)

set(CMAKE_CXX_STANDARD 17)

get_filename_component(REPOSITORY_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} ABSOLUTE)
set(EXTERNAL_LIBRARIES_DIRECTORY ${REPOSITORY_DIRECTORY}/3rd)

add_subdirectory(${EXTERNAL_LIBRARIES_DIRECTORY}/glfw)

include_directories(${EXTERNAL_LIBRARIES_DIRECTORY}/glfw/include)
set(BUILD_UTILS OFF CACHE BOOL "" FORCE)

include_directories(${EXTERNAL_LIBRARIES_DIRECTORY}/glm/)

add_subdirectory(${EXTERNAL_LIBRARIES_DIRECTORY}/eigen)
set(EIGEN3_INCLUDE_DIR ${EXTERNAL_LIBRARIES_DIRECTORY}/eigen)
MESSAGE(STATUS "Using bundled Eigen3 : ${EIGEN_INCLUDE_DIR}")


# Add bgfx.cmake
add_subdirectory(3rd/bgfx.cmake)

# Add executable

include_directories(3rd/bgfx.cmake/bgfx/examples)

include_directories(3rd/LASzip/include)

add_subdirectory(3rd/LASzip)
set(LASZIP_INCLUDE_DIR ${EXTERNAL_LIBRARIES_DIRECTORY})


include_directories(3rd/LASzip/include)
# Link only bgfx (not example-common!)

find_package(X11 REQUIRED)
include_directories(${EXTERNAL_LIBRARIES_DIRECTORY}/glm/)

#find_package(PkgConfig REQUIRED)
#pkg_search_module(GLFW REQUIRED glfw3)
#find_package(Eigen3 3.3 REQUIRED NO_MODULE)

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
target_link_libraries(my_bgfx_app PRIVATE 
#X11::X11 
glfw
Eigen3::Eigen  
laszip3)

# Copy LASzip DLL to the executable output directory after build
# If LASzip is provided as a target named 'laszip3' this will copy the target file.
if(TARGET laszip3)
    add_custom_command(TARGET my_bgfx_app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:laszip3>
            $<TARGET_FILE_DIR:my_bgfx_app>
        COMMENT "Copying LASzip DLL to output directory"
    )
endif()