cmake_minimum_required(VERSION 3.15.0)

project(SetCoveringSolver LANGUAGES CXX)

option(SETCOVERINGSOLVER_USE_CBC "Use Cbc" OFF)
option(SETCOVERINGSOLVER_USE_GUROBI "Use Gurobi" OFF)
option(SETCOVERINGSOLVER_USE_ORTOOLS "Use OR-Tools" ON)

# Avoid FetchContent warning.
cmake_policy(SET CMP0135 NEW)

# Require C++17.
set(CMAKE_CXX_STANDARD 17)

# Enable output of compile commands during generation.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set MSVC_RUNTIME_LIBRARY.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Update RPATH.
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")

# Add sub-directories.
add_subdirectory(extern)
add_subdirectory(src)
add_subdirectory(test)
