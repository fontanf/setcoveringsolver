# Enable FetchContent.
include(FetchContent)

# Fetch boost.
set(BOOST_INCLUDE_LIBRARIES thread filesystem system program_options dynamic_bitset)
set(BOOST_ENABLE_CMAKE ON)
include(FetchContent)
FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
    EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(Boost)

# Fetch fontanf/optimizationtools.
FetchContent_Declare(
    optimizationtools
    GIT_REPOSITORY https://github.com/fontanf/optimizationtools.git
    GIT_TAG 65c8e11224aac1cf6e6e33c040b6b895341ad156
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../optimizationtools/"
    EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(optimizationtools)

# Fetch fontanf/coloringsolver.
FetchContent_Declare(
    coloringsolver
    GIT_REPOSITORY https://github.com/fontanf/coloringsolver.git
    GIT_TAG 12b0eb7dd085a9284f254cdbc9e49c9ba0cfffff
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../coloringsolver/"
    EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(coloringsolver)

# Fetch fontanf/mathoptsolverscmake.
if(SETCOVERINGSOLVER_USE_CBC)
    set(MATHOPTSOLVERSCMAKE_USE_CBC ON)
endif()
if(SETCOVERINGSOLVER_USE_GUROBI)
    set(MATHOPTSOLVERSCMAKE_USE_GUROBI ON)
endif()
FetchContent_Declare(
    mathoptsolverscmake
    GIT_REPOSITORY https://github.com/fontanf/mathoptsolverscmake.git
    GIT_TAG 4472814a28a40f4d861ccd757e140835c3a31bd0
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../mathoptsolverscmake/"
    EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(mathoptsolverscmake)
