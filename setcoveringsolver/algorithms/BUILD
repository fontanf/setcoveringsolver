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
                "@gurobi//:gurobi",
        ],
        visibility = ["//visibility:public"],
)

