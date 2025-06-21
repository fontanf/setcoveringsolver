#include "setcoveringsolver/instance_builder.hpp"

#include "setcoveringsolver/solution.hpp"
#include "setcoveringsolver/algorithms/greedy.hpp"
#include "setcoveringsolver/algorithms/milp_cbc.hpp"
#if GUROBI_FOUND
#include "setcoveringsolver/algorithms/milp_gurobi.hpp"
#endif
#include "setcoveringsolver/algorithms/local_search_row_weighting.hpp"
#include "setcoveringsolver/algorithms/large_neighborhood_search.hpp"
#include "setcoveringsolver/algorithms/trivial_bound.hpp"
#include "setcoveringsolver/algorithms/clique_cover_bound.hpp"

#include <boost/program_options.hpp>

using namespace setcoveringsolver;

namespace po = boost::program_options;

void read_args(
        Parameters& parameters,
        const po::variables_map& vm)
{
    parameters.timer.set_sigint_handler();
    parameters.messages_to_stdout = true;
    if (vm.count("time-limit"))
        parameters.timer.set_time_limit(vm["time-limit"].as<double>());
    if (vm.count("verbosity-level"))
        parameters.verbosity_level = vm["verbosity-level"].as<int>();
    if (vm.count("log"))
        parameters.log_path = vm["log"].as<std::string>();
    parameters.log_to_stderr = vm.count("log-to-stderr");
    bool only_write_at_the_end = vm.count("only-write-at-the-end");
    if (!only_write_at_the_end) {

        std::string certificate_path;
        if (vm.count("certificate"))
            certificate_path = vm["certificate"].as<std::string>();

        std::string certificate_format;
        if (vm.count("certificate-format"))
            certificate_format = vm["certificate-format"].as<std::string>();

        std::string json_output_path;
        if (vm.count("output"))
            json_output_path = vm["output"].as<std::string>();

        parameters.new_solution_callback = [
            json_output_path,
            certificate_path,
            certificate_format](
                    const Output& output,
                    const std::string&)
        {
            if (!json_output_path.empty())
                output.write_json_output(json_output_path);
            if (!certificate_path.empty())
                output.solution.write(certificate_path, certificate_format);
        };
    }
    if (vm.count("reduce"))
        parameters.reduction_parameters.reduce = vm["reduce"].as<bool>();
    if (vm.count("set-folding"))
        parameters.reduction_parameters.set_folding = vm["set-folding"].as<bool>();
    if (vm.count("twin"))
        parameters.reduction_parameters.twin = vm["twin"].as<bool>();
    if (vm.count("unconfined-sets"))
        parameters.reduction_parameters.unconfined_sets = vm["unconfined-sets"].as<bool>();
    if (vm.count("dominated-sets-removal"))
        parameters.reduction_parameters.dominated_sets_removal = vm["dominated-sets-removal"].as<bool>();
    if (vm.count("dominated-elements-removal"))
        parameters.reduction_parameters.dominated_elements_removal = vm["dominated-elements-removal"].as<bool>();
    if (vm.count("reduction-time-limit"))
        parameters.reduction_parameters.timer.set_time_limit(vm["reduction-time-limit"].as<double>());
    if (vm.count("enable-new-solution-callback"))
        parameters.enable_new_solution_callback = vm["enable-new-solution-callback"].as<bool>();
}

Output run(
        const Instance& instance,
        const po::variables_map& vm)
{
    std::mt19937_64 generator(vm["seed"].as<Seed>());
    Solution solution = (vm.count("initial-solution"))?
        Solution(instance, vm["initial-solution"].as<std::string>()):
        Solution(instance);

    // Run algorithm.
    std::string algorithm = vm["algorithm"].as<std::string>();
    if (algorithm == "greedy") {
        Parameters parameters;
        read_args(parameters, vm);
        return greedy(instance, parameters);
    } else if (algorithm == "greedy-lin") {
        Parameters parameters;
        read_args(parameters, vm);
        return greedy_lin(instance, parameters);
    } else if (algorithm == "greedy-reverse") {
        Parameters parameters;
        read_args(parameters, vm);
        return greedy_reverse(instance, parameters);
    } else if (algorithm == "greedy-dual") {
        Parameters parameters;
        read_args(parameters, vm);
        return greedy_dual(instance, parameters);
    } else if (algorithm == "greedy-or-greedy-reverse") {
        Parameters parameters;
        read_args(parameters, vm);
        return greedy_or_greedy_reverse(instance, parameters);
#if CBC_FOUND
    } else if (algorithm == "milp-cbc") {
        MilpCbcParameters parameters;
        read_args(parameters, vm);
        return milp_cbc(instance, parameters);
#endif
#if GUROBI_FOUND
    } else if (algorithm == "milp-gurobi") {
        MilpGurobiParameters parameters;
        read_args(parameters, vm);
        return milp_gurobi(instance, parameters);
#endif
    } else if (algorithm == "local-search-row-weighting-1") {
        LocalSearchRowWeighting1Parameters parameters;
        read_args(parameters, vm);
        if (vm.count("maximum-number-of-iterations")) {
            parameters.maximum_number_of_iterations
                = vm["maximum-number-of-iterations"].as<int>();
        }
        if (vm.count("maximum-number-of-iterations-without-improvement")) {
            parameters.maximum_number_of_iterations_without_improvement
                = vm["maximum-number-of-iterations-without-improvement"].as<int>();
        }
        return local_search_row_weighting_1(instance, generator, nullptr, parameters);
    } else if (algorithm == "local-search-row-weighting-2") {
        LocalSearchRowWeighting2Parameters parameters;
        read_args(parameters, vm);
        if (vm.count("maximum-number-of-iterations")) {
            parameters.maximum_number_of_iterations
                = vm["maximum-number-of-iterations"].as<int>();
        }
        if (vm.count("maximum-number-of-iterations-without-improvement")) {
            parameters.maximum_number_of_iterations_without_improvement
                = vm["maximum-number-of-iterations-without-improvement"].as<int>();
        }
        return local_search_row_weighting_2(instance, generator, nullptr, parameters);
    } else if (algorithm == "large-neighborhood-search"
            || algorithm == "large-neighborhood-search-2") {
        LargeNeighborhoodSearchParameters parameters;
        read_args(parameters, vm);
        if (vm.count("maximum-number-of-iterations")) {
            parameters.maximum_number_of_iterations
                = vm["maximum-number-of-iterations"].as<int>();
        }
        if (vm.count("maximum-number-of-iterations-without-improvement")) {
            parameters.maximum_number_of_iterations_without_improvement
                = vm["maximum-number-of-iterations-without-improvement"].as<int>();
        }
        if (vm.count("goal"))
            parameters.goal = vm["goal"].as<Cost>();
        return large_neighborhood_search(instance, parameters);
    } else if (algorithm == "trivial-bound") {
        Parameters parameters;
        read_args(parameters, vm);
        return trivial_bound(instance, parameters);
    } else if (algorithm == "clique-cover-bound") {
        Parameters parameters;
        read_args(parameters, vm);
        return clique_cover_bound(instance, parameters);

    } else {
        throw std::invalid_argument(
                "Unknown algorithm \"" + algorithm + "\".");
    }
}

int main(int argc, char *argv[])
{
    // Parse program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("algorithm,a", po::value<std::string>()->default_value("large-neighborhood-search"), "set algorithm")
        ("input,i", po::value<std::string>()->required(), "set input file (required)")
        ("format,f", po::value<std::string>()->default_value(""), "set input file format (default: standard)")
        ("certificate-format,", po::value<std::string>()->default_value(""), "set certificate file format (default: standard)")
        ("unicost,u", "set unicost")
        ("output,o", po::value<std::string>(), "set JSON output file")
        ("initial-solution,", po::value<std::string>(), "")
        ("certificate,c", po::value<std::string>(), "set certificate file")
        ("goal,", po::value<Cost>(), "")
        ("seed,s", po::value<Seed>()->default_value(0), "set seed")
        ("time-limit,t", po::value<double>(), "set time limit in seconds")
        ("verbosity-level,v", po::value<int>(), "set verbosity level")
        ("only-write-at-the-end,e", "only write output and certificate files at the end")
        ("log,l", po::value<std::string>(), "set log file")
        ("log-to-stderr", "write log to stderr")

        ("reduce,", po::value<bool>(), "enable reduction")
        ("set-folding,", po::value<bool>(), "enable set folding reduction")
        ("twin,", po::value<bool>(), "enable twin reduction")
        ("unconfined-sets,", po::value<bool>(), "enable unconfined sets reduction")
        ("dominated-sets-removal,", po::value<bool>(), "enable dominated sets removal")
        ("dominated-elements-removal,", po::value<bool>(), "enable dominated elements removal")
        ("reduction-time-limit,", po::value<double>(), "set reduction time limit in seconds")
        ("enable-new-solution-callback,", po::value<bool>(), "enable new solution callback")
        ("maximum-number-of-iterations,", po::value<int>(), "set the maximum number of iterations")
        ("maximum-number-of-iterations-without-improvement,", po::value<int>(), "set the maximum number of iterations without improvement")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        std::cout << desc << std::endl;;
        return 1;
    }
    try {
        po::notify(vm);
    } catch (const po::required_option& e) {
        std::cout << desc << std::endl;;
        return 1;
    }

    // Build instance.
    InstanceBuilder instance_builder;
    instance_builder.read(
            vm["input"].as<std::string>(),
            vm["format"].as<std::string>());
    if (vm.count("unicost"))
        instance_builder.set_unicost();
    const Instance instance = instance_builder.build();

    // Run.
    Output output = run(instance, vm);

    // Write outputs.
    if (vm.count("certificate")) {
        std::string certificate_format = "";
        if (vm.count("certificate-format"))
            certificate_format = vm["certificate-format"].as<std::string>();
        output.solution.write(
                vm["certificate"].as<std::string>(),
                certificate_format);
    }
    if (vm.count("output"))
        output.write_json_output(vm["output"].as<std::string>());

    return 0;
}
