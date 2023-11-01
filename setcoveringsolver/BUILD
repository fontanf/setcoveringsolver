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
