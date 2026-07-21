project(TritonEngine)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)
add_library(TritonEngine STATIC ${SOURCE_FILES})
target_include_directories(
    TritonEngine
    PRIVATE
    ${CMAKE_SOURCE_DIR}/thirdparty/glm
	${CMAKE_SOURCE_DIR}/thirdparty/stb
    ${CMAKE_SOURCE_DIR}/tools/tasset/src/tasset
)
target_link_libraries(
    TritonEngine
	PRIVATE
	assimp
	freetype
	OpenAL
	SDL3-static
    glbinding
    Jolt
    TracyClient
    TritonThirdpartyTinyDDSLoaderLib
    TritonTAssetLib
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