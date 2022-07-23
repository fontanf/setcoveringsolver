STDCPP = select({
            "@bazel_tools//src/conditions:windows": ['/std:c++latest'],
            "//conditions:default":                 ["-std=c++14"],})

COINOR_COPTS = select({
            "//setcoveringsolver:coinor_build": ["-DCOINOR_FOUND"],
            "//conditions:default":                       []})
CPLEX_COPTS = select({
            "//setcoveringsolver:cplex_build": [
                    "-DCPLEX_FOUND",
                    "-m64",
                    "-DIL_STD"],
            "//conditions:default": []})
GUROBI_COPTS = select({
            "//setcoveringsolver:gurobi_build": ["-DGUROBI_FOUND"],
            "//conditions:default": []})

COINOR_DEP = select({
            "//setcoveringsolver:coinor_build": ["@coinor//:coinor"],
            "//conditions:default": []})
CPLEX_DEP = select({
            "//setcoveringsolver:cplex_build": ["@cplex//:cplex"],
            "//conditions:default": []})
GUROBI_DEP = select({
            "//setcoveringsolver:gurobi_build": ["@gurobi//:gurobi"],
            "//conditions:default": []})

