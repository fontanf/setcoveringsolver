add_library(SetCoveringSolver_greedy)
target_sources(SetCoveringSolver_greedy PRIVATE
    greedy.cpp)
target_include_directories(SetCoveringSolver_greedy PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(SetCoveringSolver_greedy PUBLIC
    SetCoveringSolver_set_covering)
add_library(SetCoveringSolver::greedy ALIAS SetCoveringSolver_greedy)

if(SETCOVERINGSOLVER_USE_CBC)
    add_library(SetCoveringSolver_milp_cbc)
    target_sources(SetCoveringSolver_milp_cbc PRIVATE
        milp_cbc.cpp)
    target_include_directories(SetCoveringSolver_milp_cbc PUBLIC
        ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(SetCoveringSolver_milp_cbc PUBLIC
        SetCoveringSolver_set_covering
        MathOptSolversCMake::cbc)
    add_library(SetCoveringSolver::milp_cbc ALIAS SetCoveringSolver_milp_cbc)
endif()

if(SETCOVERINGSOLVER_USE_GUROBI)
    add_library(SetCoveringSolver_milp_gurobi)
    target_sources(SetCoveringSolver_milp_gurobi PRIVATE
        milp_gurobi.cpp)
    target_include_directories(SetCoveringSolver_milp_gurobi PUBLIC
        ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(SetCoveringSolver_milp_gurobi PUBLIC
        SetCoveringSolver_set_covering
        MathOptSolversCMake::gurobi)
    add_library(SetCoveringSolver::milp_gurobi ALIAS SetCoveringSolver_milp_gurobi)
endif()

add_library(SetCoveringSolver_local_search_row_weighting)
target_sources(SetCoveringSolver_local_search_row_weighting PRIVATE
    local_search_row_weighting.cpp)
target_include_directories(SetCoveringSolver_local_search_row_weighting PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(SetCoveringSolver_local_search_row_weighting PUBLIC
    SetCoveringSolver_set_covering)
add_library(SetCoveringSolver::local_search_row_weighting ALIAS SetCoveringSolver_local_search_row_weighting)

add_library(SetCoveringSolver_large_neighborhood_search)
target_sources(SetCoveringSolver_large_neighborhood_search PRIVATE
    large_neighborhood_search.cpp)
target_include_directories(SetCoveringSolver_large_neighborhood_search PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(SetCoveringSolver_large_neighborhood_search PUBLIC
    SetCoveringSolver_set_covering)
add_library(SetCoveringSolver::large_neighborhood_search ALIAS SetCoveringSolver_large_neighborhood_search)
