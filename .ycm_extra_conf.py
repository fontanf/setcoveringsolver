def Settings( **kwargs ):
    return {
            'flags': [
                '-x', 'c++',
                '-Wall', '-Wextra', '-Werror',
                '-DCOINOR_FOUND',
                '-DGUROBI_FOUND',
                '-I', '.',
                '-I', '/home/florian/Programmes/coinbrew/dist/include/',
                '-I', './bazel-setcoveringsolver/external/json/single_include',
                '-I', './bazel-setcoveringsolver/external/googletest/googletest/include',
                '-I', './bazel-setcoveringsolver/external/boost/',
                '-I', './bazel-setcoveringsolver/external/optimizationtools',
                '-I', '/home/florian/Programmes/gurobi811/linux64/include/',
                ],
            }

