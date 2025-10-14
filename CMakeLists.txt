cmake_minimum_required(VERSION 3.15.0)

project(SetCoveringSolver LANGUAGES CXX)

# Build options.
option(SETCOVERINGSOLVER_BUILD_MAIN "Build main" ON)
option(SETCOVERINGSOLVER_BUILD_TEST "Build the unit tests" ON)

option(SETCOVERINGSOLVER_USE_CBC "Use Cbc" OFF)
option(SETCOVERINGSOLVER_USE_HIGHS "Use HiGHS" ON)
option(SETCOVERINGSOLVER_USE_XPRESS "Use FICO Xpress" OFF)

# Avoid FetchContent warning.
cmake_policy(SET CMP0135 NEW)

# Require C++14.
set(CMAKE_CXX_STANDARD 14)

# Enable output of compile commands during generation.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set MSVC_RUNTIME_LIBRARY.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Add sub-directories.
add_subdirectory(extern)
add_subdirectory(src)
if(SETCOVERINGSOLVER_BUILD_TEST)
    add_subdirectory(test)
endif()
