#include "setcoveringsolver/algorithms/algorithms.hpp"

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
    int loglevelmax = 999;
    int seed = 0;
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
        ("time-limit,t", po::value<double>(&time_limit), "Time limit in seconds")
        ("seed,s", po::value<int>(&seed), "set seed")
        ("verbose,v", "set verbosity")
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

    Instance instance(instance_path, format);
    if (vm.count("unicost"))
        instance.set_unicost();
    //Cost cost_sum = 0;
    //Cost cost_sum_2 = 0;
    //for (SetId s = 0; s < instance.number_of_sets(); ++s) {
        //cost_sum += instance.set(s).cost;
        //cost_sum_2 += instance.set(s).cost * instance.set(s).cost;
    //}
    //double mean = (double)cost_sum / instance.number_of_sets();
    //double v    = (double)cost_sum_2 / instance.number_of_sets();
    //double std_dev = std::sqrt(std::abs(mean * mean - v));
    //double coefficient_of_variation = std_dev / mean * 100;
    //std::cout << instance_path
        //<< "," << instance.number_of_elements()
        //<< "," << instance.number_of_sets()
        //<< "," << instance.number_of_arcs()
        //<< "," << coefficient_of_variation
        //<< std::endl;
    //VER(info, "* Components:");
    //for (ComponentId c = 0; c < number_of_components(); ++c)
        //VER(info, " " << c << "/" << component(c).elements.size() << "/" << component(c).sets.size());
    //VER(info, std::endl);

    Info info = Info()
        .set_verbose(vm.count("verbose"))
        .set_time_limit(time_limit)
        .set_certificate_path(certificate_path)
        .set_json_output_path(output_path)
        .set_only_write_at_the_end(false)
        .set_log_path(log_path)
        .set_log2stderr(vm.count("log2stderr"))
        .set_maximum_log_level(loglevelmax)
        ;

    std::mt19937_64 generator(seed);
    Solution solution(instance, initial_solution_path);

    auto output = run(algorithm, instance, generator, info);

    return 0;
}

