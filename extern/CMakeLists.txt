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
    GIT_TAG e086ec4cfcc9a885cd63f579a99ea5e8f4737005
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../optimizationtools/"
    EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(optimizationtools)

# Fetch fontanf/mathoptsolverscmake.
set(MATHOPTSOLVERSCMAKE_USE_CLP ON)
set(MATHOPTSOLVERSCMAKE_USE_CBC ON)
if(SETCOVERINGSOLVER_USE_HIGHS)
    set(MATHOPTSOLVERSCMAKE_USE_HIGHS ON)
endif()
if(SETCOVERINGSOLVER_USE_XPRESS)
    set(MATHOPTSOLVERSCMAKE_USE_XPRESS ON)
endif()
FetchContent_Declare(
    mathoptsolverscmake
    GIT_REPOSITORY https://github.com/fontanf/mathoptsolverscmake.git
    GIT_TAG 6721116d85708864bda081e546e444afc158039a
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../mathoptsolverscmake/"
    EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(mathoptsolverscmake)

# Fetch fontanf/coloringsolver.
FetchContent_Declare(
    coloringsolver
    GIT_REPOSITORY https://github.com/fontanf/coloringsolver.git
    GIT_TAG 12b0eb7dd085a9284f254cdbc9e49c9ba0cfffff
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../coloringsolver/"
    EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(coloringsolver)
