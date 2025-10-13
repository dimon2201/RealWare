cmake_minimum_required(VERSION 3.25.1)

project(RealWareSample01)

set(CMAKE_CXX_STANDARD 17)

link_libraries(RealWareEngine)

add_executable(
    RealWareSample01
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

add_custom_command(
    TARGET
    RealWareSample01
    POST_BUILD
    COMMAND
    ${CMAKE_COMMAND}
    -E
    copy_directory
    "${CMAKE_SOURCE_DIR}/exe_files"
    "${CMAKE_CURRENT_BINARY_DIR}/Debug"
)