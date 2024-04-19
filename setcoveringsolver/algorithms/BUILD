load("//setcoveringsolver:variables.bzl", "STDCPP",
         "CBC_COPTS", "CBC_DEP",
         "CPLEX_COPTS", "CPLEX_DEP",
         "GUROBI_COPTS", "GUROBI_DEP")

cc_library(
        name = "greedy",
        hdrs = ["greedy.hpp"],
        srcs = ["greedy.cpp"],
        deps = ["//setcoveringsolver:data"],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "milp_cbc",
        hdrs = ["milp_cbc.hpp"],
        srcs = ["milp_cbc.cpp"],
        deps = [
                "//setcoveringsolver:data",
        ] + CBC_DEP,
        copts = CBC_COPTS,
        visibility = ["//visibility:public"],
)

cc_library(
        name = "milp_gurobi",
        hdrs = ["milp_gurobi.hpp"],
        srcs = ["milp_gurobi.cpp"],
        deps = [
                "//setcoveringsolver:data",
        ] + GUROBI_DEP,
        copts = GUROBI_COPTS,
        visibility = ["//visibility:public"],
)

cc_library(
        name = "local_search_row_weighting",
        hdrs = ["local_search_row_weighting.hpp"],
        srcs = ["local_search_row_weighting.cpp"],
        deps = [
                ":greedy",
                "//setcoveringsolver:data",
        ],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "large_neighborhood_search",
        hdrs = ["large_neighborhood_search.hpp"],
        srcs = ["large_neighborhood_search.cpp"],
        deps = [
                ":greedy",
                "//setcoveringsolver:data",
        ],
        visibility = ["//visibility:public"],
)
