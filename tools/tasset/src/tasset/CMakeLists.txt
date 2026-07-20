project(TritonTAssetLib)

add_library(
    TritonTAssetLib
    tasset.cpp
    tasset.hpp
    types.hpp
)

target_include_directories(
    TritonTAssetLib
    PRIVATE
    ../../../../engine/submodules/glm/glm
	../../../../engine/submodules/stb
    ../../../../engine/submodules/tinyddsloader
)
target_link_libraries(
    TritonTAssetLib
	PRIVATE
    assimp
)

file(
    MAKE_DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/../../lib"
)
add_custom_command(
    TARGET
    TritonTAssetLib
    POST_BUILD
    COMMAND
    ${CMAKE_COMMAND}
    -E
    copy
    "$<TARGET_FILE:TritonTAssetLib>"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../lib"
)