cmake_minimum_required(VERSION 3.5)

project(Triton)

option(TRITON_ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(TRITON_ENABLE_ASAN)
    if(MSVC)
        add_compile_options(/fsanitize=address)
    endif()
endif()

option(TRITON_STATIC_RUNTIME "Enable static runtime library linking" ON)
if(TRITON_STATIC_RUNTIME)
    if(MSVC)
        add_compile_options(
            "$<$<CONFIG:Debug>:/MTd>"
            "$<$<NOT:$<CONFIG:Debug>>:/MT>"
        )
    endif()
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
#set(LIBTYPE STATIC CACHE STRING "" FORCE)
set(ALSOFT_ENABLE_MODULES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(JPH_USE_VK OFF CACHE BOOL "" FORCE)
set(TRACY_ENABLE ON CACHE BOOL "" FORCE)
set(CMAKE_VERBOSE_MAKEFILE ON)

add_subdirectory(thirdparty/assimp)

if(MSVC AND TARGET assimp)
    set_property(TARGET assimp PROPERTY COMPILE_WARNING_AS_ERROR OFF)
    target_compile_options(assimp PRIVATE /WX-)
endif()

add_subdirectory(thirdparty/freetype)
add_subdirectory(thirdparty/openal-soft)
add_subdirectory(thirdparty/sdl)
add_subdirectory(thirdparty/glbinding)
add_subdirectory(thirdparty/jolt-physics/Build)
add_subdirectory(thirdparty/tracy)
add_subdirectory(thirdparty/stb_lib)
add_subdirectory(thirdparty/tinyddsloader_lib)

add_subdirectory(tools)
add_subdirectory(engine)
#add_subdirectory(editor)
add_subdirectory(examples)