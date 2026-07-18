project(TritonTRFLib)

add_library(
    TritonTRFLib
    TRF.cpp
    TRF.hpp
    types.hpp
)

target_include_directories(
    TritonTRFLib
    PRIVATE
    ../../../../engine/submodules/glm/glm
	../../../../engine/submodules/stb
    ../../../../engine/submodules/tinyddsloader
)
target_link_libraries(
    TritonTRFLib
	PRIVATE
    assimp
)

file(
    MAKE_DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/../../lib"
)
add_custom_command(
    TARGET
    TritonTRFLib
    POST_BUILD
    COMMAND
    ${CMAKE_COMMAND}
    -E
    copy
    "$<TARGET_FILE:TritonTRFLib>"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../lib"
)