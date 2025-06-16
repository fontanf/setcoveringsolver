# Enable FetchContent.
include(FetchContent)

# Fetch boost.
set(BOOST_INCLUDE_LIBRARIES
    thread
    filesystem
    system
    program_options
    dynamic_bitset
    serialization
    multiprecision)
set(BOOST_ENABLE_CMAKE ON)
FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz)
FetchContent_MakeAvailable(Boost)
install(TARGETS boost_program_options)

# OR-Tools
if(SETCOVERINGSOLVER_USE_ORTOOLS)
    set(BUILD_ZLIB ON)
    set(BUILD_absl ON)
    set(BUILD_Protobuf ON)
    set(BUILD_benchmark ON)
    set(BUILD_fuzztest ON)
    set(BUILD_googletest ON)
    set(BUILD_Eigen3 ON)
    set(BUILD_HIGHS ON)
    set(USE_COINOR OFF)
    set(USE_SCIP OFF)
    FetchContent_Declare(
        or-tools
        URL https://github.com/google/or-tools/releases/download/v9.13/or-tools-9.13.tar.gz)
    FetchContent_MakeAvailable(or-tools)
endif()

# Fetch fontanf/optimizationtools.
FetchContent_Declare(
    optimizationtools
    GIT_REPOSITORY https://github.com/fontanf/optimizationtools.git
    GIT_TAG 82f08aa92b1f06be3624674e34b7b6f96c06deb5)
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../optimizationtools/")
FetchContent_MakeAvailable(optimizationtools)

# Fetch fontanf/coloringsolver.
set(COLUMNGENERATIONSOLVER_USE_CBC ${SETCOVERINGSOLVER_USE_CBC})
set(COLUMNGENERATIONSOLVER_USE_CLP ${SETCOVERINGSOLVER_USE_CBC})
set(MATHOPTSOLVERSCMAKE_USE_CBC ${SETCOVERINGSOLVER_USE_CBC})
set(MATHOPTSOLVERSCMAKE_USE_CLP ${SETCOVERINGSOLVER_USE_CBC})
FetchContent_Declare(
    coloringsolver
    GIT_REPOSITORY https://github.com/fontanf/coloringsolver.git
    GIT_TAG 6226a94617aa2c3495b5ad5aaff3cf1c18827f10)
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../coloringsolver/")
FetchContent_MakeAvailable(coloringsolver)

# Fetch fontanf/mathoptsolverscmake.
set(MATHOPTSOLVERSCMAKE_USE_CBC ${SETCOVERINGSOLVER_USE_CBC})
set(MATHOPTSOLVERSCMAKE_USE_GUROBI ${SETCOVERINGSOLVER_USE_GUROBI})
FetchContent_Declare(
    mathoptsolverscmake
    GIT_REPOSITORY https://github.com/fontanf/mathoptsolverscmake.git
    GIT_TAG 4472814a28a40f4d861ccd757e140835c3a31bd0)
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../mathoptsolverscmake/")
FetchContent_MakeAvailable(mathoptsolverscmake)
