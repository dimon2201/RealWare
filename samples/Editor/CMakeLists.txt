cmake_minimum_required(VERSION 3.25.1)
project(RealWareEditor)

set(CMAKE_CXX_STANDARD 17)

link_libraries(RealWareEngine)

add_executable(
    RealWareEditor
    map_plugin_loader/map_plugin_loader.cpp
    winapi/window.cpp
    winapi/listview.cpp
    winapi/groupbox.cpp
    winapi/textbox.cpp
    winapi/label.cpp
    winapi/button.cpp
    winapi/checkbox.cpp
    ray_hit.cpp
    main.cpp)

#set(GLEW_DLL_SRC_FILE_PATH ../../engine/thirdparty/glew-2.2.0/lib/glew32.dll)
#set(GLEW_DLL_DST_FILE_PATH ../../cmake-build-debug/samples/HighPressure/glew32.dll)
#file(COPY_FILE ${GLEW_DLL_SRC_FILE_PATH} ${GLEW_DLL_DST_FILE_PATH})
#
#set(FREETYPE_DLL_SRC_FILE_PATH ../../engine/thirdparty/Freetype/bin/freetype.dll)
#set(FREETYPE_DLL_DST_FILE_PATH ../../cmake-build-debug/samples/HighPressure/freetype.dll)
#file(COPY_FILE ${FREETYPE_DLL_SRC_FILE_PATH} ${FREETYPE_DLL_DST_FILE_PATH})