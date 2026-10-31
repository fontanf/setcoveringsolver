#include "setcoveringsolver/instance_builder.hpp"

#include "setcoveringsolver/solution.hpp"
#include "setcoveringsolver/algorithms/greedy.hpp"
#include "setcoveringsolver/algorithms/milp.hpp"
#if ORTOOLS_FOUND
#include "setcoveringsolver/algorithms/cp_sat_ortools.hpp"
#endif
#include "setcoveringsolver/algorithms/local_search_row_weighting.hpp"
#include "setcoveringsolver/algorithms/large_neighborhood_search.hpp"
#include "setcoveringsolver/algorithms/trivial_bound.hpp"
#include "setcoveringsolver/algorithms/clique_cover_bound.hpp"

#include <boost/program_options.hpp>

using namespace setcoveringsolver;

namespace po = boost::program_options;

std::string get_certificate_format(
        const po::variables_map& vm)
{
    std::string certificate_format = vm["certificate-format"].as<std::string>();
    if (!certificate_format.empty())
        return certificate_format;

    // By default, use the certificate format corresponding to the input
    // format.
    return vm["format"].as<std::string>();
}

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

        std::string certificate_format = get_certificate_format(vm);

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
    if (vm.count("vertex-cover-domination"))
        parameters.reduction_parameters.vertex_cover_domination = vm["vertex-cover-domination"].as<bool>();
    if (vm.count("dominated-sets"))
        parameters.reduction_parameters.dominated_sets_removal = vm["dominated-sets"].as<bool>();
    if (vm.count("dominated-elements"))
        parameters.reduction_parameters.dominated_elements_removal = vm["dominated-elements"].as<bool>();
    if (vm.count("linear-programming"))
        parameters.reduction_parameters.linear_programming = vm["linear-programming"].as<bool>();
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
    std::unique_ptr<Solution> initial_solution;
    if (vm.count("initial-solution")) {
        initial_solution = std::make_unique<Solution>(
                instance,
                vm["initial-solution"].as<std::string>(),
                get_certificate_format(vm));
    }

    // Run algorithm.
    std::string algorithm = vm["algorithm"].as<std::string>();
    if (algorithm == "greedy") {
        Parameters parameters;
        read_args(parameters, vm);
        return greedy(instance, initial_solution.get(), parameters);
    } else if (algorithm == "greedy-lin") {
        Parameters parameters;
        read_args(parameters, vm);
        return greedy_lin(instance, initial_solution.get(), parameters);
    } else if (algorithm == "greedy-reverse") {
        Parameters parameters;
        read_args(parameters, vm);
        return greedy_reverse(instance, initial_solution.get(), parameters);
    } else if (algorithm == "greedy-dual") {
        Parameters parameters;
        read_args(parameters, vm);
        return greedy_dual(instance, initial_solution.get(), parameters);
    } else if (algorithm == "greedy-dual-sort") {
        Parameters parameters;
        read_args(parameters, vm);
        return greedy_dual_sort(instance, initial_solution.get(), parameters);
    } else if (algorithm == "greedy-gwmin") {
        Parameters parameters;
        read_args(parameters, vm);
        return greedy_gwmin(instance, initial_solution.get(), parameters);
    } else if (algorithm == "greedy-or-greedy-reverse") {
        Parameters parameters;
        read_args(parameters, vm);
        return greedy_or_greedy_reverse(instance, initial_solution.get(), parameters);
    } else if (algorithm == "milp") {
#ifdef XPRESS_FOUND
        XPRSinit(NULL);
#endif
        MilpParameters parameters;
        read_args(parameters, vm);
        if (vm.count("solver"))
            parameters.solver = vm["solver"].as<mathoptsolverscmake::SolverName>();
        auto output = milp(instance, initial_solution.get(), parameters);
#ifdef XPRESS_FOUND
        XPRSfree();
#endif
        return output;
#if ORTOOLS_FOUND
    } else if (algorithm == "cp-sat-ortools") {
        CpSatOrtoolsParameters parameters;
        read_args(parameters, vm);
        return cp_sat_ortools(instance, initial_solution.get(), parameters);
#endif
    } else if (algorithm == "local-search-row-weighting") {
        LocalSearchRowWeightingParameters parameters;
        read_args(parameters, vm);
        if (vm.count("best-solution-update-frequency")) {
            parameters.best_solution_update_frequency
                = vm["best-solution-update-frequency"].as<Counter>();
        }
        if (vm.count("maximum-number-of-iterations")) {
            parameters.maximum_number_of_iterations
                = vm["maximum-number-of-iterations"].as<Counter>();
        }
        if (vm.count("maximum-number-of-iterations-without-improvement")) {
            parameters.maximum_number_of_iterations_without_improvement
                = vm["maximum-number-of-iterations-without-improvement"].as<Counter>();
        }
        return local_search_row_weighting(instance, initial_solution.get(), generator, parameters);
    } else if (algorithm == "large-neighborhood-search"
            || algorithm == "large-neighborhood-search-2") {
        LargeNeighborhoodSearchParameters parameters;
        read_args(parameters, vm);
        if (vm.count("maximum-number-of-iterations")) {
            parameters.maximum_number_of_iterations
                = vm["maximum-number-of-iterations"].as<Counter>();
        }
        if (vm.count("maximum-number-of-iterations-without-improvement")) {
            parameters.maximum_number_of_iterations_without_improvement
                = vm["maximum-number-of-iterations-without-improvement"].as<Counter>();
        }
        return large_neighborhood_search(instance, initial_solution.get(), parameters);
    } else if (algorithm == "trivial-bound") {
        Parameters parameters;
        read_args(parameters, vm);
        return trivial_bound(instance, initial_solution.get(), parameters);
    } else if (algorithm == "clique-cover-bound") {
        Parameters parameters;
        read_args(parameters, vm);
        return clique_cover_bound(instance, initial_solution.get(), parameters);

    } else {
        throw std::invalid_argument(
                "Unknown algorithm \"" + algorithm + "\".");
    }
}

int main(int argc, char *argv[])
{
    std::string algorithms =
        "greedy, greedy-lin, greedy-reverse, greedy-dual, greedy-dual-sort, "
        "greedy-gwmin, greedy-or-greedy-reverse, milp, "
#if ORTOOLS_FOUND
        "cp-sat-ortools, "
#endif
        "local-search-row-weighting, large-neighborhood-search, "
        "trivial-bound, clique-cover-bound";
    std::string formats =
        "gecco2020 (or gecco), fulkerson1974 (or sts), "
        "balas1980 (or orlibrary), balas1996, "
        "faster1994 (or faster, wedelin1995, wedelin), "
        "pace2019_vc, pace2025, pace2025_ds, dimacs2010_vc";
    std::string solvers = ""
#ifdef CBC_FOUND
        "cbc, "
#endif
#ifdef HIGHS_FOUND
        "highs, "
#endif
#ifdef XPRESS_FOUND
        "xpress, "
#endif
        ;
    if (!solvers.empty())
        solvers = solvers.substr(0, solvers.size() - 2);
    else
        solvers = "none, this build has no MILP solver";

    // Parse program options
    po::options_description general_options("General options");
    general_options.add_options()
        ("help,h", "print this help message")
        ("algorithm,a", po::value<std::string>()->default_value("large-neighborhood-search"),
            ("set the algorithm; one of: " + algorithms).c_str())
        ("seed,s", po::value<Seed>()->default_value(0), "set the seed of the random number generator")
        ("verbosity-level,v", po::value<int>(), "set the verbosity level: 0 (no output), 1, 2 or 3 (default: 1)")
        ("log,l", po::value<std::string>(), "write the log to a file")
        ("log-to-stderr", "write the log to the standard error")
        ;

    po::options_description io_options("Input and output options");
    io_options.add_options()
        ("input,i", po::value<std::string>()->required(), "set the instance file (required)")
        ("format,f", po::value<std::string>()->required(),
            ("set the instance file format (required); one of: " + formats + "; see docs/formats.md").c_str())
        ("unicost,u", "set the cost of all sets to 1")
        ("certificate,c", po::value<std::string>(), "write the solution to a certificate file")
        ("certificate-format", po::value<std::string>()->default_value(""),
            "set the format of the certificate file and of the initial solution file; same names as the instance file formats (default: the instance file format)")
        ("initial-solution", po::value<std::string>(), "read an initial solution from a file in the certificate format")
        ("output,o", po::value<std::string>(), "write the output (solution, bound, intermediary solutions, parameters) to a JSON file")
        ("only-write-at-the-end,e", "only write the output and certificate files at the end, instead of each time a new best solution is found")
        ;

    po::options_description stop_options("Stopping criteria");
    stop_options.add_options()
        ("time-limit,t", po::value<double>(), "set the time limit in seconds (default: no time limit)")
        ("goal", po::value<Cost>(), "stop as soon as a solution of cost lower than or equal to the goal is found")
        ("maximum-number-of-iterations", po::value<Counter>(), "set the maximum number of iterations of large-neighborhood-search and local-search-row-weighting (default: no limit)")
        ("maximum-number-of-iterations-without-improvement", po::value<Counter>(), "set the maximum number of iterations without improvement of large-neighborhood-search and local-search-row-weighting (default: no limit)")
        ;

    po::options_description reduction_options("Reduction options (0 to disable, 1 to enable; default: 1)");
    reduction_options.add_options()
        ("reduce", po::value<bool>(), "reduce the instance before solving it")
        ("set-folding", po::value<bool>(), "enable the set folding reduction")
        ("twin", po::value<bool>(), "enable the twin reduction")
        ("unconfined-sets", po::value<bool>(), "enable the unconfined sets reduction")
        ("vertex-cover-domination", po::value<bool>(), "enable the vertex cover domination reduction")
        ("dominated-sets", po::value<bool>(), "enable the dominated sets removal")
        ("dominated-elements", po::value<bool>(), "enable the dominated elements removal")
        ("linear-programming", po::value<bool>(), "enable the linear programming reduction")
        ("reduction-time-limit", po::value<double>(), "set the time limit of the reduction in seconds; the reduction also stops at the time limit (default: no time limit)")
        ;

    po::options_description algorithm_options("Algorithm-specific options");
    algorithm_options.add_options()
        ("solver", po::value<mathoptsolverscmake::SolverName>(),
            ("set the solver of milp; one of: " + solvers + " (default: highs)").c_str())
        ("best-solution-update-frequency", po::value<Counter>(), "set the number of iterations between two updates of the best solution of local-search-row-weighting (default: 1)")
        ;

    // Options which are accepted but not shown in the help message.
    po::options_description hidden_options("Hidden options");
    hidden_options.add_options()
        ("enable-new-solution-callback", po::value<bool>(), "enable new solution callback")
        ;

    po::options_description visible_options;
    visible_options
        .add(general_options)
        .add(io_options)
        .add(stop_options)
        .add(reduction_options)
        .add(algorithm_options);
    po::options_description all_options;
    all_options.add(visible_options).add(hidden_options);

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, all_options), vm);
        if (vm.count("help")) {
            std::cout
                << "Usage: setcoveringsolver --input FILE --format FORMAT [options]" << std::endl
                << visible_options << std::endl;
            return 0;
        }
        po::notify(vm);
    } catch (const po::error& e) {
        std::cerr << "error: " << e.what() << std::endl
            << "Run 'setcoveringsolver --help' for the list of options." << std::endl;
        return 1;
    }

    try {
        // Build instance.
        InstanceBuilder instance_builder;
        instance_builder.read(
                vm["input"].as<std::string>(),
                vm["format"].as<std::string>());
        if (vm.count("unicost"))
            instance_builder.set_unicost();
        if (vm.count("goal"))
            instance_builder.set_goal(vm["goal"].as<Cost>());
        const Instance instance = instance_builder.build();

        // Run.
        Output output = run(instance, vm);

        // Write outputs.
        if (vm.count("certificate")) {
            std::string certificate_format = get_certificate_format(vm);
            output.solution.write(
                    vm["certificate"].as<std::string>(),
                    certificate_format);
        }
        if (vm.count("output"))
            output.write_json_output(vm["output"].as<std::string>());
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
