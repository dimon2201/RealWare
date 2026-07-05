if(NOT WIN32)
    message(FATAL_ERROR "Error: To use MSVC toolchain you need to have Windows OS")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_COMPILER cl)
set(CMAKE_CXX_COMPILER cl)