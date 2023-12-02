load("//setcoveringsolver:variables.bzl", "STDCPP",
         "CBC_COPTS", "CBC_DEP",
         "CPLEX_COPTS", "CPLEX_DEP",
         "GUROBI_COPTS", "GUROBI_DEP")

cc_library(
        name = "data",
        hdrs = [
                "instance.hpp",
                "instance_builder.hpp",
                "solution.hpp",
                "algorithm_formatter.hpp",
        ],
        srcs = [
                "instance.cpp",
                "instance_builder.cpp",
                "solution.cpp",
                "algorithm_formatter.cpp",
        ],
        deps = [
                "@optimizationtools//optimizationtools/utils:utils",
                "@optimizationtools//optimizationtools/containers:containers",
        ],
        copts = STDCPP,
        visibility = ["//visibility:public"],
)

cc_binary(
        name = "main",
        srcs = ["main.cpp"],
        deps = [
                "//setcoveringsolver/setcovering/algorithms:greedy",
                "//setcoveringsolver/setcovering/algorithms:milp_cbc",
                "//setcoveringsolver/setcovering/algorithms:milp_gurobi",
                "//setcoveringsolver/setcovering/algorithms:local_search_row_weighting",
                "//setcoveringsolver/setcovering/algorithms:large_neighborhood_search",
                "@boost//:program_options",
        ] + CBC_DEP + GUROBI_DEP,
        copts = CBC_COPTS + GUROBI_COPTS,
        linkopts = select({
                    "@bazel_tools//src/conditions:windows": [],
                    "//conditions:default": ["-lpthread"],
        }),
)

