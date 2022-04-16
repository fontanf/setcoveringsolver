#include "setcoveringsolver/algorithms/algorithms.hpp"

#include <boost/program_options.hpp>

using namespace setcoveringsolver;
namespace po = boost::program_options;

LocalSearchRowWeightingOptionalParameters read_localsearch_rowweighting_args(const std::vector<char*>& argv)
{
    LocalSearchRowWeightingOptionalParameters parameters;
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

LocalSearchRowWeighting2OptionalParameters read_localsearch_rowweighting_2_args(const std::vector<char*>& argv)
{
    LocalSearchRowWeighting2OptionalParameters parameters;
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

LargeNeighborhoodSearchOptionalParameters read_largeneighborhoodsearch_args(const std::vector<char*>& argv)
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

LargeNeighborhoodSearch2OptionalParameters read_largeneighborhoodsearch_2_args(const std::vector<char*>& argv)
{
    LargeNeighborhoodSearch2OptionalParameters parameters;
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

Output setcoveringsolver::run(
        std::string algorithm,
        Instance& instance,
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
        return greedy(instance, info);
    } else if (algorithm_args[0] == "greedy_lin") {
        return greedy_lin(instance, info);
    } else if (algorithm_args[0] == "greedy_dual") {
        return greedy_dual(instance, info);
#if GUROBI_FOUND
    } else if (algorithm_args[0] == "milp_gurobi") {
        MilpGurobiOptionalParameters parameters;
        parameters.info = info;
        return milp_gurobi(instance, parameters);
#endif
    } else if (algorithm_args[0] == "localsearch_rowweighting") {
        auto parameters = read_localsearch_rowweighting_args(algorithm_argv);
        parameters.info = info;
        return localsearch_rowweighting(instance, generator, parameters);
    } else if (algorithm_args[0] == "localsearch_rowweighting_2") {
        auto parameters = read_localsearch_rowweighting_2_args(algorithm_argv);
        parameters.info = info;
        return localsearch_rowweighting_2(instance, generator, parameters);
    } else if (algorithm_args[0] == "largeneighborhoodsearch") {
        auto parameters = read_largeneighborhoodsearch_args(algorithm_argv);
        parameters.info = info;
        return largeneighborhoodsearch(instance, generator, parameters);
    } else if (algorithm_args[0] == "largeneighborhoodsearch_2") {
        auto parameters = read_largeneighborhoodsearch_2_args(algorithm_argv);
        parameters.info = info;
        return largeneighborhoodsearch_2(instance, parameters);

    } else {
        throw std::invalid_argument(
                "Unknown algorithm \"" + algorithm_args[0] + "\".");
    }
}

