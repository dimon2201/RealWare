project(TritonExample01)

add_executable(
    TritonExample01
    main.cpp
)
target_include_directories(TritonExample01 PUBLIC
	${CMAKE_SOURCE_DIR}/engine/src
	${CMAKE_SOURCE_DIR}/thirdparty/glm
)
target_link_libraries(
    TritonExample01
    PUBLIC
    TritonEngine
)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
add_custom_command(
    TARGET
    TritonExample01
    POST_BUILD
    COMMAND
    ${CMAKE_COMMAND}
    -E
    copy
    "$<TARGET_FILE:TritonExample01>"
    "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)
add_custom_command(
    TARGET
    TritonExample01
    POST_BUILD
    COMMAND
    ${CMAKE_COMMAND}
    -E
    copy_directory
    "${CMAKE_SOURCE_DIR}/runtime"
    "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)
add_custom_command(
    TARGET
    TritonExample01
    POST_BUILD
    COMMAND
    ${CMAKE_COMMAND}
    -E
    copy_directory
    "${CMAKE_SOURCE_DIR}/runtime"
    "$<TARGET_FILE_DIR:TritonExample01>"
)

set(TRITON_OPENAL_DEBUG_DLL_PATH "${CMAKE_BINARY_DIR}/thirdparty/openal-soft/Debug/OpenAL32d.dll")
if(EXISTS ${TRITON_OPENAL_DEBUG_DLL_PATH})
    add_custom_command(
        TARGET 
        TritonExample01
        POST_BUILD
        COMMAND
        ${CMAKE_COMMAND}
        -E
        copy
        ${TRITON_OPENAL_DEBUG_DLL_PATH}
        "$<TARGET_FILE_DIR:TritonExample01>/OpenAL32d.dll"
    )
endif()
