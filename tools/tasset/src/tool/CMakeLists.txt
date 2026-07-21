project(TritonTAssetTool)

add_executable(
    TritonTAssetTool
    tasset_tool.cpp
)

target_include_directories(
    TritonTAssetTool
	PRIVATE
    ../tasset
    ${CMAKE_SOURCE_DIR}/thirdparty/glm
    ${CMAKE_SOURCE_DIR}/thirdparty/tinyddsloader
)
target_link_libraries(
    TritonTAssetTool
	PRIVATE
    TritonTAssetLib
)

file(
    MAKE_DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/../../bin"
)
add_custom_command(
    TARGET
    TritonTAssetTool
    POST_BUILD
    COMMAND
    ${CMAKE_COMMAND}
    -E
    copy
    "$<TARGET_FILE:TritonTAssetTool>"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../bin"
)