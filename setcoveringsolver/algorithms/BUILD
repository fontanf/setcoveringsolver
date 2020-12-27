load("//setcoveringsolver:variables.bzl", "STDCPP",
         "CPLEX_COPTS", "CPLEX_DEP",
         "GUROBI_COPTS", "GUROBI_DEP")

cc_library(
        name = "algorithms",
        hdrs = [
                "greedy.hpp",
                "branchandcut_gurobi.hpp",
                "localsearch.hpp",
                "largeneighborhoodsearch.hpp",
                "algorithms.hpp",
        ],
        srcs = [
                "greedy.cpp",
                "branchandcut_gurobi.cpp",
                "localsearch.cpp",
                "largeneighborhoodsearch.cpp",
                "algorithms.cpp",
        ],
        deps = [
                "//setcoveringsolver:setcoveringsolver",
                "@optimizationtools//optimizationtools:utils",
                "@boost//:program_options",
        ] + CPLEX_DEP + GUROBI_DEP,
        copts = CPLEX_COPTS + GUROBI_COPTS,
        visibility = ["//visibility:public"],
)

