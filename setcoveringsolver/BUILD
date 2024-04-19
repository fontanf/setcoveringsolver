load("@bazel_skylib//lib:selects.bzl", "selects")

config_setting(
    name = "cbc_build",
    values = {"define": "cbc=true"},
    visibility = ["//visibility:public"],
)

selects.config_setting_group(
    name = "cbc_linux",
    match_all = [":cbc_build", "@bazel_tools//src/conditions:linux"],
)

selects.config_setting_group(
    name = "cbc_windows",
    match_all = [":cbc_build", "@bazel_tools//src/conditions:windows"],
)

config_setting(
    name = "cplex_build",
    values = {"define": "cplex=true"},
    visibility = ["//visibility:public"],
)

config_setting(
    name = "gurobi_build",
    values = {"define": "gurobi=true"},
    visibility = ["//visibility:public"],
)

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
                "reduction.hpp",
                "algorithm.hpp",
                "algorithm_formatter.hpp",
        ],
        srcs = [
                "instance.cpp",
                "instance_builder.cpp",
                "solution.cpp",
                "reduction.cpp",
                "algorithm.cpp",
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
                "//setcoveringsolver/algorithms:greedy",
                "//setcoveringsolver/algorithms:milp_cbc",
                "//setcoveringsolver/algorithms:milp_gurobi",
                "//setcoveringsolver/algorithms:local_search_row_weighting",
                "//setcoveringsolver/algorithms:large_neighborhood_search",
                "@boost//:program_options",
        ] + CBC_DEP + GUROBI_DEP,
        copts = CBC_COPTS + GUROBI_COPTS,
        linkopts = select({
                    "@bazel_tools//src/conditions:windows": [],
                    "//conditions:default": ["-lpthread"],
        }),
)

