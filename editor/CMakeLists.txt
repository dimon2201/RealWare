cmake_minimum_required(VERSION 3.25)

project(TritonEditor)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)
add_executable(TritonEditor ${SOURCE_FILES})
target_include_directories(
    TritonEditor
    PUBLIC
    ../engine/src
)
target_link_directories(
    TritonEditor
    PUBLIC
    ../engine/lib
)
target_link_libraries(TritonEditor TritonEngine user32 gdi32)
add_custom_command(
    TARGET
    TritonEditor
    POST_BUILD
    COMMAND
    ${CMAKE_COMMAND}
    -E
    copy
    "$<TARGET_FILE:TritonEditor>"
    "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)