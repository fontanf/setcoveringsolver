find_package(Threads REQUIRED)
add_library(SetCoveringSolver_set_covering)
target_sources(SetCoveringSolver_set_covering PRIVATE
    instance.cpp
    instance_builder.cpp
    solution.cpp
    reduction.cpp
    algorithm.cpp
    algorithm_formatter.cpp)
target_include_directories(SetCoveringSolver_set_covering PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(SetCoveringSolver_set_covering PUBLIC
    OptimizationTools::utils
    OptimizationTools::containers
    Threads::Threads)
add_library(SetCoveringSolver::set_covering ALIAS SetCoveringSolver_set_covering)

add_subdirectory(algorithms)

add_executable(SetCoveringSolver_main)
target_sources(SetCoveringSolver_main PRIVATE
    main.cpp)
target_link_libraries(SetCoveringSolver_main PUBLIC
    SetCoveringSolver_greedy
    SetCoveringSolver_local_search_row_weighting
    SetCoveringSolver_large_neighborhood_search
    SetCoveringSolver_trivial_bound
    SetCoveringSolver_clique_cover_bound
    Boost::program_options)
if(SETCOVERINGSOLVER_USE_CBC)
    target_compile_definitions(SetCoveringSolver_main PUBLIC
        CBC_FOUND=1)
    target_link_libraries(SetCoveringSolver_main PUBLIC
        SetCoveringSolver_milp_cbc)
endif()
if(SETCOVERINGSOLVER_USE_GUROBI)
    target_compile_definitions(SetCoveringSolver_main PUBLIC
        GUROBI_FOUND=1)
    target_link_libraries(SetCoveringSolver_main PUBLIC
        SetCoveringSolver_milp_gurobi)
endif()
set_target_properties(SetCoveringSolver_main PROPERTIES OUTPUT_NAME "setcoveringsolver")
install(TARGETS SetCoveringSolver_main)
