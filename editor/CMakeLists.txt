cmake_minimum_required(VERSION 3.25)

project(TritonEditor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)
add_library(TritonEditor ${SOURCE_FILES})
target_include_directories(
    TritonEditor
    PUBLIC
    thirdparty/compiled_glfw/include
    thirdparty/compiled_openal-soft/include
    thirdparty/compiled_glew/include
    thirdparty/Freetype/include
    thirdparty/compiled_assimp/include
    thirdparty/stb-master
    thirdparty/glm/glm
    thirdparty/lodepng
    thirdparty/tinyddsloader
)
target_link_directories(
    TritonEditor
    PUBLIC
    thirdparty/compiled_glfw/lib/Debug
    thirdparty/compiled_openal-soft/lib/Debug
    thirdparty/compiled_glew/lib/Debug
    thirdparty/Freetype/lib
    thirdparty/compiled_assimp/lib/Debug
)
target_link_libraries(TritonEditor TritonEngine)