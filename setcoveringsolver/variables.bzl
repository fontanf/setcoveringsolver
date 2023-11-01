STDCPP = select({
            "@bazel_tools//src/conditions:windows": ['/std:c++latest'],
            "//conditions:default":                 ["-std=c++14"],})

CBC_COPTS = select({
            "//setcoveringsolver:cbc_build": ["-DCBC_FOUND"],
            "//conditions:default": []})
CBC_DEP = select({
            "//setcoveringsolver:cbc_windows": ["@cbc_windows//:cbc"],
            "//conditions:default": []
        }) + select({
            "//setcoveringsolver:cbc_linux": ["@cbc_linux//:cbc"],
            "//conditions:default": []})

CPLEX_COPTS = select({
            "//setcoveringsolver:cplex_build": [
                    "-DCPLEX_FOUND",
                    "-m64",
                    "-DIL_STD"],
            "//conditions:default": []})
CPLEX_DEP = select({
            "//setcoveringsolver:cplex_build": ["@cplex//:cplex"],
            "//conditions:default": []})

GUROBI_COPTS = select({
            "//setcoveringsolver:gurobi_build": ["-DGUROBI_FOUND"],
            "//conditions:default": []})
GUROBI_DEP = select({
            "//setcoveringsolver:gurobi_build": ["@gurobi//:gurobi"],
            "//conditions:default": []})

