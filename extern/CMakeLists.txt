# Enable FetchContent.
include(FetchContent)

# Fetch boost.
set(BOOST_INCLUDE_LIBRARIES thread filesystem system program_options)
set(BOOST_ENABLE_CMAKE ON)
include(FetchContent)
FetchContent_Declare(
  Boost
  GIT_REPOSITORY https://github.com/boostorg/boost.git
  GIT_TAG boost-1.84.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Boost)

# Fetch fontanf/optimizationtools.
FetchContent_Declare(
    optimizationtools
    GIT_REPOSITORY https://github.com/fontanf/optimizationtools.git
    GIT_TAG 33a3966ece149d390ec7ce08699669b5267e64aa)
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../optimizationtools/")
FetchContent_MakeAvailable(optimizationtools)

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
    GIT_TAG 56190725f424249a3acd8fce3ff50e08fe675cc6)
    #SOURCE_DIR "${PROJECT_SOURCE_DIR}/../mathoptsolverscmake/")
FetchContent_MakeAvailable(mathoptsolverscmake)
