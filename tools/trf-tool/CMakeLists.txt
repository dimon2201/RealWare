project(TritonTRFTool)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)
add_executable(TritonTRFTool ${SOURCE_FILES})
target_include_directories(
    TritonTRFTool
    PRIVATE
    ../../engine/src
    ../../engine/submodules/glm/glm
	../../engine/submodules/stb
    ../../engine/submodules/tinyddsloader
)
target_link_libraries(
    TritonTRFTool
	PRIVATE
    assimp
	TritonEngine
)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
add_custom_command(
    TARGET
    TritonTRFTool
    POST_BUILD
    COMMAND
    ${CMAKE_COMMAND}
    -E
    copy
    "$<TARGET_FILE:TritonTRFTool>"
    "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)