cc_library(
        name = "algorithms",
        hdrs = [
                "greedy.hpp",
                "branchandcut.hpp",
                "localsearch.hpp",
                "largeneighborhoodsearch.hpp",
                "algorithms.hpp",
        ],
        srcs = [
                "greedy.cpp",
                "branchandcut.cpp",
                "localsearch.cpp",
                "largeneighborhoodsearch.cpp",
                "algorithms.cpp",
        ],
        deps = [
                "//setcoveringsolver:setcoveringsolver",
                "@gurobi//:gurobi",
        ],
        visibility = ["//visibility:public"],
)

