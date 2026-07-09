cmake_minimum_required(VERSION 3.5)

project(Triton)

add_subdirectory(engine/submodules/assimp)
add_subdirectory(engine/submodules/freetype)
add_subdirectory(engine/submodules/openal-soft)
add_subdirectory(engine/submodules/sdl)

#add_subdirectory(engine)
#add_subdirectory(editor)
add_subdirectory(examples)