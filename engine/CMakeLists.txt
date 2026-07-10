project(TritonEngine)

file(GLOB_RECURSE SOURCE_FILES thirdparty/lodepng/lodepng.cpp src/*.cpp src/*.hpp)
add_library(TritonEngine ${SOURCE_FILES})
target_include_directories(
    TritonEngine
    PRIVATE
    submodules/glm/glm
	submodules/stb
    submodules/tinyddsloader
)
target_link_libraries(
    TritonEngine
	PRIVATE
	assimp
	freetype
	OpenAL
	SDL3
    glbinding
)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")
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