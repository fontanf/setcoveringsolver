#include "setcoveringsolver/setcovering/algorithms/algorithms.hpp"

#include <boost/program_options.hpp>

using namespace setcoveringsolver::setcovering;
namespace po = boost::program_options;

LocalSearchRowWeighting1OptionalParameters read_local_search_row_weighting_1_args(const std::vector<char*>& argv)
{
    LocalSearchRowWeighting1OptionalParameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("iterations,i", po::value<Counter>(&parameters.maximum_number_of_iterations), "")
        ("iterations-without-improvement,w", po::value<Counter>(&parameters.maximum_number_of_iterations_without_improvement), "")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line((Counter)argv.size(), argv.data(), desc), vm);
    try {
        po::notify(vm);
    } catch (const po::required_option& e) {
        std::cout << desc << std::endl;;
        throw "";
    }
    return parameters;
}

LocalSearchRowWeighting2OptionalParameters read_local_search_row_weighting_2_args(const std::vector<char*>& argv)
{
    LocalSearchRowWeighting2OptionalParameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("n1,", po::value<int>(&parameters.neighborhood_1), "")
        ("n2,", po::value<int>(&parameters.neighborhood_2), "")
        ("wu,", po::value<int>(&parameters.weights_update_strategy), "")
        ("iterations,i", po::value<Counter>(&parameters.maximum_number_of_iterations), "")
        ("iterations-without-improvement,w", po::value<Counter>(&parameters.maximum_number_of_iterations_without_improvement), "")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line((Counter)argv.size(), argv.data(), desc), vm);
    try {
        po::notify(vm);
    } catch (const po::required_option& e) {
        std::cout << desc << std::endl;;
        throw "";
    }
    return parameters;
}

LargeNeighborhoodSearchOptionalParameters read_large_neighborhood_search_args(const std::vector<char*>& argv)
{
    LargeNeighborhoodSearchOptionalParameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("iterations,i", po::value<Counter>(&parameters.maximum_number_of_iterations), "")
        ("iterations-without-improvement,w", po::value<Counter>(&parameters.maximum_number_of_iterations_without_improvement), "")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line((Counter)argv.size(), argv.data(), desc), vm);
    try {
        po::notify(vm);
    } catch (const po::required_option& e) {
        std::cout << desc << std::endl;;
        throw "";
    }
    return parameters;
}

Output setcoveringsolver::setcovering::run(
        std::string algorithm,
        Instance& instance,
        Cost goal,
        std::mt19937_64& generator,
        optimizationtools::Info info)
{
    std::vector<std::string> algorithm_args = po::split_unix(algorithm);
    std::vector<char*> algorithm_argv;
    for (Counter i = 0; i < (Counter)algorithm_args.size(); ++i)
        algorithm_argv.push_back(const_cast<char*>(algorithm_args[i].c_str()));

    if (algorithm.empty() || algorithm_args[0].empty()) {
        throw std::invalid_argument("Missing algorithm.");

    } else if (algorithm_args[0] == "greedy") {
        GreedyOptionalParameters parameters;
        parameters.info = info;
        return greedy(instance, parameters);
    } else if (algorithm_args[0] == "greedy-lin") {
        GreedyOptionalParameters parameters;
        parameters.info = info;
        return greedy_lin(instance, parameters);
    } else if (algorithm_args[0] == "greedy-dual") {
        GreedyOptionalParameters parameters;
        parameters.info = info;
        return greedy_dual(instance, parameters);
#if CBC_FOUND
    } else if (algorithm_args[0] == "milp-cbc") {
        MilpCbcOptionalParameters parameters;
        parameters.info = info;
        return milp_cbc(instance, parameters);
#endif
#if GUROBI_FOUND
    } else if (algorithm_args[0] == "milp-gurobi") {
        MilpGurobiOptionalParameters parameters;
        parameters.info = info;
        return milp_gurobi(instance, parameters);
#endif
    } else if (algorithm_args[0] == "local-search-row-weighting-1") {
        auto parameters = read_local_search_row_weighting_1_args(algorithm_argv);
        parameters.info = info;
        return local_search_row_weighting_1(instance, generator, parameters);
    } else if (algorithm_args[0] == "local-search-row-weighting-2") {
        auto parameters = read_local_search_row_weighting_2_args(algorithm_argv);
        parameters.info = info;
        return local_search_row_weighting_2(instance, generator, parameters);
    } else if (algorithm_args[0] == "large-neighborhood-search"
            || algorithm_args[0] == "large-neighborhood-search-2") {
        auto parameters = read_large_neighborhood_search_args(algorithm_argv);
        parameters.goal = goal;
        parameters.info = info;
        return large_neighborhood_search(instance, parameters);

    } else {
        throw std::invalid_argument(
                "Unknown algorithm \"" + algorithm_args[0] + "\".");
    }
}

