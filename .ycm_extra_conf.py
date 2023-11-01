def Settings( **kwargs ):
    return {
            'flags': [
                '-x', 'c++',
                '-Wall', '-Wextra', '-Werror',
                '-I', '.',

                '-I', './bazel-setcoveringsolver/external/'
                'json/single_include/',

                '-I', './bazel-setcoveringsolver/external/'
                'googletest/googletest/include/',

                '-I', './bazel-setcoveringsolver/external/'
                'boost/',

                # optimizationtools
                '-I', './bazel-setcoveringsolver/external/'
                # '-I', './../'
                'optimizationtools/',

                # CBC
                '-DCBC_FOUND',
                '-I', './bazel-setcoveringsolver/external/cbc_linux/include/coin/',

                # Gurobi
                '-DGUROBI_FOUND',
                '-I', '/home/florian/Programmes/gurobi811/linux64/include/',

                ],
            }
