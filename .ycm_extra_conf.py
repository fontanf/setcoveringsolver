def Settings( **kwargs ):
    return {
            'flags': [
                '-x', 'c++',
                '-Wall', '-Wextra', '-Werror',
                '-I', '.',
                '-I', './bazel-setcoveringsolver/external/json/single_include',
                '-I', './bazel-setcoveringsolver/external/googletest/googletest/include',
                '-I', './bazel-setcoveringsolver/external/boost/',
                '-I', './bazel-setcoveringsolver/external/optimizationtools',
                '-I', '/home/florian/Programmes/gurobi811/linux64/include/',
                ],
            }

