cmake_minimum_required(VERSION 3.25.1)

project(TritonExample01)

add_executable(
    TritonExample01
    main.cpp
)

#set(GLEW_DLL_SRC_FILE_PATH ../../engine/thirdparty/glew-2.2.0/lib/glew32.dll)
#set(GLEW_DLL_DST_FILE_PATH ../../cmake-build-debug/samples/HighPressure/glew32.dll)
#file(COPY_FILE ${GLEW_DLL_SRC_FILE_PATH} ${GLEW_DLL_DST_FILE_PATH})
#
#set(FREETYPE_DLL_SRC_FILE_PATH ../../engine/thirdparty/Freetype/bin/freetype.dll)
#set(FREETYPE_DLL_DST_FILE_PATH ../../cmake-build-debug/samples/HighPressure/freetype.dll)
#file(COPY_FILE ${FREETYPE_DLL_SRC_FILE_PATH} ${FREETYPE_DLL_DST_FILE_PATH})

#file(GLOB EXE_FILES ${CMAKE_SOURCE_DIR}/exe_files/*)
#file(COPY ${EXE_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)

target_include_directories(TritonExample01 PUBLIC ${CMAKE_SOURCE_DIR}/engine/src)

target_link_libraries(TritonExample01 PUBLIC TritonEngine)

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

#target_compile_options(TritonSample01 PRIVATE
#    /fsanitize=address
#)
#target_link_options(TritonSample01 PRIVATE
#    /fsanitize=address
#)