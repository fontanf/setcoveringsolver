#include "setcoveringsolver/algorithms/algorithms.hpp"

#include <boost/program_options.hpp>

using namespace setcoveringsolver;
namespace po = boost::program_options;

LocalSearchOptionalParameters read_localsearch_args(const std::vector<char*>& argv)
{
    LocalSearchOptionalParameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("threads,t", po::value<Counter>(&parameters.thread_number), "")
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

LocalSearch2OptionalParameters read_localsearch_2_args(const std::vector<char*>& argv)
{
    LocalSearch2OptionalParameters parameters;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("threads,t", po::value<Counter>(&parameters.thread_number), "")
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
        ("iteration-limit,i", po::value<Counter>(&parameters.iteration_limit), "")
        ("iteration-without-improvment-limit,w", po::value<Counter>(&parameters.iteration_without_improvment_limit), "")
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
        ("iteration-limit,i", po::value<Counter>(&parameters.iteration_limit), "")
        ("iteration-without-improvment-limit,w", po::value<Counter>(&parameters.iteration_without_improvment_limit), "")
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

Output setcoveringsolver::run(std::string algorithm, Instance& instance, std::mt19937_64& generator, Info info)
{
    std::vector<std::string> algorithm_args = po::split_unix(algorithm);
    std::vector<char*> algorithm_argv;
    for (Counter i = 0; i < (Counter)algorithm_args.size(); ++i)
        algorithm_argv.push_back(const_cast<char*>(algorithm_args[i].c_str()));

    if (algorithm.empty() || algorithm_args[0].empty()) {
        std::cerr << "\033[31m" << "ERROR, missing algorithm." << "\033[0m" << std::endl;
        return Output(instance, info);

    } else if (algorithm_args[0] == "greedy") {
        return greedy(instance, info);
    } else if (algorithm_args[0] == "greedy_lin") {
        return greedy_lin(instance, info);
    } else if (algorithm_args[0] == "greedy_dual") {
        return greedy_dual(instance, info);
#if GUROBI_FOUND
    } else if (algorithm_args[0] == "branchandcut_gurobi") {
        BranchAndCutGurobiOptionalParameters parameters;
        parameters.info = info;
        return branchandcut_gurobi(instance, parameters);
#endif
    } else if (algorithm_args[0] == "localsearch") {
        auto parameters = read_localsearch_args(algorithm_argv);
        parameters.info = info;
        return localsearch(instance, generator, parameters);
    } else if (algorithm_args[0] == "localsearch_2") {
        auto parameters = read_localsearch_2_args(algorithm_argv);
        parameters.info = info;
        return localsearch_2(instance, generator, parameters);
    } else if (algorithm_args[0] == "largeneighborhoodsearch") {
        auto parameters = read_largeneighborhoodsearch_args(algorithm_argv);
        parameters.info = info;
        return largeneighborhoodsearch(instance, generator, parameters);
    } else if (algorithm_args[0] == "largeneighborhoodsearch_2") {
        auto parameters = read_largeneighborhoodsearch_2_args(algorithm_argv);
        parameters.info = info;
        return largeneighborhoodsearch_2(instance, parameters);

    } else {
        std::cerr << "\033[31m" << "ERROR, unknown algorithm: '" << algorithm_args[0] << "'\033[0m" << std::endl;
        assert(false);
        return Output(instance, info);
    }
}

