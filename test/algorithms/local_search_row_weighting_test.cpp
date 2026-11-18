#include "setcoveringsolver/instance_builder.hpp"
#include "setcoveringsolver/algorithms/local_search_row_weighting.hpp"

#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

using namespace setcoveringsolver;
namespace fs = boost::filesystem;

struct LocalSearchRowWeightingTestParams
{
    fs::path instance_path;
    std::string instance_format;
    bool reduce;
    Cost expected_cost;
};

inline std::ostream& operator<<(std::ostream& os, const LocalSearchRowWeightingTestParams& test_params)
{
    os << test_params.instance_path << " " << test_params.reduce;
    return os;
}

class LocalSearchRowWeightingTest: public testing::TestWithParam<LocalSearchRowWeightingTestParams> { };

TEST_P(LocalSearchRowWeightingTest, LocalSearchRowWeighting)
{
    LocalSearchRowWeightingTestParams test_params = GetParam();
    InstanceBuilder instance_builder;
    instance_builder.read(
            test_params.instance_path.string(),
            test_params.instance_format);
    Instance instance = instance_builder.build();

    for (Seed seed = 0; seed < 3; ++seed) {
        std::mt19937_64 generator(seed);
        LocalSearchRowWeightingParameters parameters;
        parameters.verbosity_level = 0;
        parameters.reduction_parameters.reduce = test_params.reduce;
        parameters.maximum_number_of_iterations = 1000;
        LocalSearchRowWeightingOutput output = local_search_row_weighting(
                instance,
                nullptr,
                generator,
                parameters);

        EXPECT_TRUE(output.solution.feasible());
        EXPECT_EQ(output.solution.cost(), test_params.expected_cost);
    }
}

INSTANTIATE_TEST_SUITE_P(
        SetCoveringSolver,
        LocalSearchRowWeightingTest,
        testing::ValuesIn(std::vector<LocalSearchRowWeightingTestParams>{
            {
                // Every set is mandatory, so the solution of the only
                // component, once found, only contains the last added set
                // or is empty when the local search looks for a set to
                // remove.
                fs::path("data") / "tests" / "local_search_row_weighting" / "all_sets_mandatory" / "instance.txt",
                "orlibrary",
                false,
                3,
            }, {
                fs::path("data") / "tests" / "local_search_row_weighting" / "all_sets_mandatory" / "instance.txt",
                "orlibrary",
                true,
                3,
            }, {
                // Every set of the optimal solution is mandatory but one.
                fs::path("data") / "tests" / "local_search_row_weighting" / "all_sets_mandatory_but_one" / "instance.txt",
                "orlibrary",
                false,
                3,
            }, {
                fs::path("data") / "tests" / "local_search_row_weighting" / "all_sets_mandatory_but_one" / "instance.txt",
                "orlibrary",
                true,
                3,
            }, {
                // Two components: one whose optimal solution has a single
                // set, and a 5-cycle.
                fs::path("data") / "tests" / "local_search_row_weighting" / "single_set_component" / "instance.txt",
                "orlibrary",
                false,
                4,
            }, {
                fs::path("data") / "tests" / "local_search_row_weighting" / "single_set_component" / "instance.txt",
                "orlibrary",
                true,
                4,
            }}));
