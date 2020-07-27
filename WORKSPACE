load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

git_repository(
    name = "googletest",
    remote = "https://github.com/google/googletest.git",
    commit = "703bd9caab50b139428cea1aaff9974ebee5742e",
    shallow_since = "1570114335 -0400",
)

git_repository(
    name = "com_github_nelhage_rules_boost",
    commit = "9f9fb8b2f0213989247c9d5c0e814a8451d18d7f",
    remote = "https://github.com/nelhage/rules_boost",
    shallow_since = "1570056263 -0700",
)
load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")
boost_deps()

http_archive(
    name = "json",
    build_file_content = """
cc_library(
        name = "json",
        hdrs = ["single_include/nlohmann/json.hpp"],
        visibility = ["//visibility:public"],
        strip_include_prefix = "single_include/"
)
""",
    urls = ["https://github.com/nlohmann/json/releases/download/v3.7.3/include.zip"],
    sha256 = "87b5884741427220d3a33df1363ae0e8b898099fbc59f1c451113f6732891014",
)

git_repository(
    name = "optimizationtools",
    remote = "https://github.com/fontanf/optimizationtools.git",
    commit = "fa14e34aad2321864d1e7284aa6b6c0a10da8252",
    shallow_since = "1585433429 +0100",
)

local_repository(
    name = "optimizationtools_",
    path = "/home/florian/Dev/optimizationtools/",
)

new_local_repository(
    name = "gurobi",
    path = "/home/florian/Programmes/gurobi811/linux64/",
    build_file_content = """
cc_library(
    name = "gurobi",
    hdrs = [
            "include/gurobi_c.h",
            "include/gurobi_c++.h",
    ],
    strip_include_prefix = "include/",
    srcs = [
            "lib/libgurobi_c++.a",
            "lib/libgurobi81.so",
    ],
    copts = [
            "-m64",
            "-DIL_STD",
    ],
    visibility = ["//visibility:public"],
)
""",
)

