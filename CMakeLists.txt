cmake_minimum_required(VERSION 3.25.1)

project(Triton)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(engine)
#add_subdirectory(samples/Editor)
add_subdirectory(samples/Sample01)

if (MSVC)
    add_compile_options(Triton PUBLIC /O2 /EHsc)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(Triton PUBLIC -O2)
endif()

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)