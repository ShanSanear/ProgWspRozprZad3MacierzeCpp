#include "../include/cxxopts.hpp"

void show_help_and_exit(const cxxopts::Options &options, int exit_code) {
    std::cout << options.help({""}) << std::endl;
    exit(exit_code);
}

auto parse_arguments(int argc, char **argv) {
    try {
        cxxopts::Options options(argv[0]);
        options
                .positional_help("[path_to_matrix_a path_to_matrix_b path_to_output]")
                .show_positional_help();
        options.add_options()
                ("matrix_a",
                 "First matrix to be multipled", cxxopts::value<std::string>())
                ("matrix_b",
                 "Second matrix to be multipled", cxxopts::value<std::string>())
                ("output",
                 "path to output file", cxxopts::value<std::string>())
                ("positional",
                 "Excessive positional arguments", cxxopts::value<std::string>())
                ("help", "Print help");
        options.parse_positional({"matrix_a", "matrix_b", "output", "positional"});
        auto result = options.parse(argc, argv);
        auto arguments = result.arguments();
        int num_of_arguments = arguments.size();
        if (result.count("help") | (num_of_arguments == 0)) {
            show_help_and_exit(options, 0);
        }
        if (result.count("matrix_a") != 1 || result.count("matrix_b") != 1 || result.count("output") != 1) {
            printf("Didnt provide one of the matrixes or more than one matrix parameter provided\n");
            show_help_and_exit(options, 1);
        }
        return result;
    }
    catch (const cxxopts::OptionException &e) {
        std::cout << "error parsing options: " << e.what() << std::endl;
        exit(1);
    }
}