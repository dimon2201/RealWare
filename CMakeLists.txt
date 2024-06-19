cmake_minimum_required(VERSION 3.25.1)

project(RealWare)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "/O0")

add_subdirectory(engine)
#add_subdirectory(samples/Survapunk)
add_subdirectory(samples/Sample01)