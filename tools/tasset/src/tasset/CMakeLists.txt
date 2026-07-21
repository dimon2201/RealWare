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
    ${CMAKE_SOURCE_DIR}/thirdparty/glm
	${CMAKE_SOURCE_DIR}/thirdparty/stb
)
target_link_libraries(
    TritonTAssetLib
	PRIVATE
    assimp
    TritonThirdpartyTinyDDSLoaderLib
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