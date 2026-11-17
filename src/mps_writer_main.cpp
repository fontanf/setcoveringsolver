#include "setcoveringsolver/instance_builder.hpp"
#include "setcoveringsolver/algorithms/milp.hpp"

#include <boost/program_options.hpp>

#include <iostream>
#include <string>

using namespace setcoveringsolver;
namespace po = boost::program_options;

int main(int argc, char *argv[])
{
    // Parse program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Produce help message")
        ("input,i", po::value<std::string>()->required(), "set input path")
        ("format,f", po::value<std::string>()->required(), "set input format")
        ("output,o", po::value<std::string>()->required(), "set output path")
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
    Instance instance = instance_builder.build();

    write_mps(
            instance,
            vm["output"].as<std::string>());

    return 0;
}
