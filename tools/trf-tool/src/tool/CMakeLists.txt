project(TritonTRFTool)

add_executable(
    TritonTRFTool
    trf_tool.cpp
)

target_include_directories(
    TritonTRFTool
	PRIVATE
    ../trf
    ../../../../engine/submodules/glm/glm
    ../../../../engine/submodules/tinyddsloader
)
target_link_libraries(
    TritonTRFTool
	PRIVATE
    TritonTRFLib
)

file(
    MAKE_DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/../../bin"
)
add_custom_command(
    TARGET
    TritonTRFTool
    POST_BUILD
    COMMAND
    ${CMAKE_COMMAND}
    -E
    copy
    "$<TARGET_FILE:TritonTRFTool>"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../bin"
)