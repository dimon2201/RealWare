cmake_minimum_required(VERSION 3.25.1)

project(TritonEngine)

file(GLOB_RECURSE SOURCE_FILES thirdparty/lodepng/lodepng.cpp src/*.cpp src/*.hpp)
add_library(TritonEngine ${SOURCE_FILES})
target_include_directories(
    TritonEngine
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
    TritonEngine
    PUBLIC
    thirdparty/compiled_glfw/lib/Debug
    thirdparty/compiled_openal-soft/lib/Debug
    thirdparty/compiled_glew/lib/Debug
    thirdparty/Freetype/lib
    thirdparty/compiled_assimp/lib/Debug
)
target_link_libraries(
    TritonEngine
	glew32
    glfw3
	opengl32
    OpenAL32
    freetype
    assimp-vc143-mtd
)
add_custom_command(
    TARGET
    TritonEngine
    POST_BUILD
    COMMAND
    ${CMAKE_COMMAND}
    -E
    copy
    "$<TARGET_FILE:TritonEngine>"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib"
)