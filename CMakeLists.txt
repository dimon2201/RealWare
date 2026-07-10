cmake_minimum_required(VERSION 3.5)

project(Triton)

if(MSVC)
    add_compile_options("$<$<CONFIG:debug_win64>:/fsanitize=address>") # /EHsc
    add_link_options("$<$<CONFIG:debug_win64>:/fsanitize=address>")
else()
    add_compile_options("$<$<CONFIG:debug_win64>:-fsanitize=address>")
    add_link_options("$<$<CONFIG:debug_win64>:-fsanitize=address>")
endif()

add_subdirectory(engine/submodules/assimp)
add_subdirectory(engine/submodules/freetype)
add_subdirectory(engine/submodules/openal-soft)
add_subdirectory(engine/submodules/sdl)
add_subdirectory(engine/submodules/glbinding)

add_subdirectory(engine)
#add_subdirectory(editor)
add_subdirectory(examples)