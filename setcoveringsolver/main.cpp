#include "setcoveringsolver/algorithms/algorithms.hpp"
#include "setcoveringsolver/instance_builder.hpp"

#include <boost/program_options.hpp>

using namespace setcoveringsolver;

int main(int argc, char *argv[])
{
    namespace po = boost::program_options;

    // Parse program options

    std::string algorithm = "";
    std::string instance_path = "";
    std::string format = "gecco2020";
    std::string initial_solution_path = "";
    std::string output_path = "";
    std::string certificate_path = "";
    std::string log_path = "";
    int verbosity_level = 0;
    int loglevelmax = 999;
    int seed = 0;
    Cost goal = 0;
    double time_limit = std::numeric_limits<double>::infinity();

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("algorithm,a", po::value<std::string>(&algorithm), "set algorithm")
        ("input,i", po::value<std::string>(&instance_path)->required(), "set input file (required)")
        ("format,f", po::value<std::string>(&format), "set input file format (default: standard)")
        ("unicost,u", "set unicost")
        ("output,o", po::value<std::string>(&output_path), "set JSON output file")
        ("initial-solution,", po::value<std::string>(&initial_solution_path), "")
        ("certificate,c", po::value<std::string>(&certificate_path), "set certificate file")
        ("goal,", po::value<Cost>(&goal), "")
        ("time-limit,t", po::value<double>(&time_limit), "Time limit in seconds")
        ("seed,s", po::value<int>(&seed), "set seed")
        ("verbosity-level,v", po::value<int>(&verbosity_level), "set verbosity level")
        ("log,l", po::value<std::string>(&log_path), "set log file")
        ("loglevelmax", po::value<int>(&loglevelmax), "set log max level")
        ("log2stderr", "write log to stderr")
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

    // Run algorithm

    InstanceBuilder instance_builder;
    instance_builder.read(instance_path, format);
    if (vm.count("unicost"))
        instance_builder.set_unicost();

    Instance instance = instance_builder.build();

    optimizationtools::Info info = optimizationtools::Info()
        .set_verbosity_level(verbosity_level)
        .set_time_limit(time_limit)
        .set_certificate_path(certificate_path)
        .set_json_output_path(output_path)
        .set_only_write_at_the_end(false)
        .set_log_path(log_path)
        .set_log2stderr(vm.count("log2stderr"))
        .set_maximum_log_level(loglevelmax)
        .set_sigint_handler()
        ;

    std::mt19937_64 generator(seed);
    Solution solution(instance, initial_solution_path);

    auto output = run(algorithm, instance, goal, generator, info);

    return 0;
}

