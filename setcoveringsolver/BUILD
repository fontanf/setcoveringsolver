cc_library(
        name = "setcoveringsolver",
        hdrs = [
                "instance.hpp",
                "solution.hpp"
        ],
        srcs = [
                "instance.cpp",
                "solution.cpp",
        ],
        deps = [
                "@optimizationtools//optimizationtools:info",
                "@optimizationtools//optimizationtools:indexed_collections",
        ],
        visibility = ["//visibility:public"],
)

cc_binary(
        name = "main",
        srcs = ["main.cpp"],
        deps = [
                "//setcoveringsolver/algorithms:algorithms",
                "@boost//:program_options",
        ],
        linkopts = select({
                    "@bazel_tools//src/conditions:windows": [],
                    "//conditions:default":                 ["-lpthread"],
        }),
)

