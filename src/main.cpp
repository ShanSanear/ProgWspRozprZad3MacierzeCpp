#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <filesystem>
#include "argument_parsing.cpp"
#include <omp.h>

using matrix = std::vector<std::vector<double>>;
namespace fs = std::filesystem;

matrix parseCSV(const fs::path &input_csv_file) {
    //TODO check if this also can be parallelize
    std::ifstream data(input_csv_file);
    std::string line;
    // Skipping required dimension lines
    std::getline(data, line);
    std::getline(data, line);
    matrix parsedCsv;
    while (std::getline(data, line)) {
        std::vector<double> parsedRow;
        std::stringstream s(line);
        std::string cell;
        while (std::getline(s, cell, ';')) {
            parsedRow.push_back(std::stod(cell));
        }

        parsedCsv.push_back(parsedRow);
    }
    return parsedCsv;
};

matrix multiply_matrixes(const matrix &matrix_a, const matrix &matrix_b, int thread_number, int inner_thread_number) {
    matrix output_matrix(matrix_a.size(), std::vector<double>(matrix_b.at(0).size()));
    long double start_time = omp_get_wtime();
#pragma omp parallel num_threads(thread_number) default(none) shared(matrix_a, matrix_b, output_matrix, inner_thread_number)
    {
        for (std::size_t row = 0; row < output_matrix.size(); row++) {
            for (std::size_t col = 0; col < output_matrix.at(0).size(); col++) {
#pragma omp parallel for num_threads(inner_thread_number)
                for (std::size_t inner = 0; inner < matrix_b.size(); inner++) {
                    output_matrix.at(row).at(col) += matrix_a.at(row).at(inner) * matrix_b.at(inner).at(col);
                }
            }
        }
    }
    long double end_time = omp_get_wtime();
    printf("Multiplying took %Lf seconds\n", end_time - start_time);
    printf("Threads number: %d, inner threads number: %d\n", thread_number, inner_thread_number);
    return output_matrix;
}

void save_matrix(const matrix &matrix_to_save, const std::string &output_file_path) {
    std::ofstream output(output_file_path);

    output << matrix_to_save.size() << std::endl;
    output << matrix_to_save.at(0).size() << std::endl;
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);
    for (auto row : matrix_to_save) {
        oss.str(std::string());
        std::copy(row.begin(), row.end() - 1, std::ostream_iterator<double>(oss, ";"));
        std::copy(row.end() - 1, row.end(), std::ostream_iterator<double>(oss));
        output << oss.str() << std::endl;
    }
}

bool check_matrix_sizes(const matrix &matrix_a, const matrix &matrix_b) {
    //TODO there should be only one of those checks, but which one?
    printf("Matrix a rows: %llu\n", matrix_a.size());
    printf("Matrix a columns: %llu\n", matrix_a.at(0).size());
    printf("Matrix b rows: %llu\n", matrix_b.size());
    printf("Matrix b columns: %llu\n", matrix_b.at(0).size());
    return matrix_a.at(0).size() == matrix_b.size();
}

int main(int argc, char *argv[]) {
    printf("Starting\n");
    auto result = parse_arguments(argc, argv);
    fs::path matrix_a_path = fs::absolute(fs::path(result["matrix_a"].as<std::string>()));
    fs::path matrix_b_path = fs::absolute(fs::path(result["matrix_b"].as<std::string>()));
    if (!fs::exists(matrix_a_path) || !fs::exists(matrix_b_path)) {
        printf("One of the input files doesnt exists, check them\n");
        printf("Provided matrix a path: %s\n", matrix_a_path.string().c_str());
        printf("Provided matrix b path: %s\n", matrix_b_path.string().c_str());
        exit(1);
    }
    printf("Loading matrix a\n");
    matrix matrix_a = parseCSV(matrix_a_path);
    printf("Loading matrix b\n");
    matrix matrix_b = parseCSV(matrix_b_path);
    if (!check_matrix_sizes(matrix_a, matrix_b)) {
        printf("Matrix sizes are incorrect\n");
        return -1;
    }
    printf("Multiplying matrixes\n");
    matrix output_matrix = multiply_matrixes(matrix_a, matrix_b, 2, 4);
    output_matrix = multiply_matrixes(matrix_a, matrix_b, 1, 1);
    output_matrix = multiply_matrixes(matrix_a, matrix_b, 4, 2);
    output_matrix = multiply_matrixes(matrix_a, matrix_b, 8, 1);
    output_matrix = multiply_matrixes(matrix_a, matrix_b, 8, 8);
    double Ts = 0.22;
    double Tp = 0.22;
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(5);
    oss << "C_" << Ts << "_" << Tp << ".csv";
    printf("Saving matrixes\n");
    save_matrix(output_matrix, oss.str());
    return 0;
}
