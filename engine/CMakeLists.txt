cmake_minimum_required(VERSION 3.25.1)

project(RealWareEngine)

set(CMAKE_CXX_STANDARD 11)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

add_library(RealWareEngine thirdparty/lodepng/lodepng.cpp ${SOURCE_FILES})

#set_property(TARGET RealWareEngine PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")

target_include_directories(
    RealWareEngine
    PUBLIC
    thirdparty/compiled_glfw/include
    thirdparty/compiled_openal-soft/include
    thirdparty/compiled_glew/include
    thirdparty/Freetype/include
    thirdparty/compiled_assimp/include
    thirdparty/stb-master
    thirdparty/glm/glm
    thirdparty/lodepng
    thirdparty/compiled_PhysX/blast_include
    thirdparty/compiled_PhysX/flow_include
    thirdparty/compiled_PhysX/physx_include
)
target_link_directories(
    RealWareEngine
    PUBLIC
    thirdparty/compiled_glfw/lib/Debug
    thirdparty/compiled_openal-soft/lib/Debug
    thirdparty/compiled_glew/lib/Debug
    thirdparty/Freetype/lib
    thirdparty/compiled_assimp/lib/Debug
    thirdparty/compiled_PhysX/lib
)

target_link_libraries(
    RealWareEngine
    glfw3
    opengl32
    glew32
    OpenAL32
    freetype
    assimp-vc143-mtd
    PhysX_64
    PhysXCommon_64
    PhysXFoundation_64
    PhysXCooking_64
    PhysXExtensions_static_64
    PhysXCharacterKinematic_static_64
)