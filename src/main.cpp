#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <filesystem>
#include "argument_parsing.cpp"

using matrix = std::vector<std::vector<double>>;
namespace fs = std::filesystem;

matrix parseCSV(const fs::path &input_csv_file) {
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

matrix multiply_matrixes(const matrix &matrix_a, const matrix &matrix_b) {
    matrix output_matrix(matrix_a.size(), std::vector<double>(matrix_b.at(0).size()));
    for (std::size_t row = 0; row < output_matrix.size(); row++) {
        for (std::size_t col = 0; col < output_matrix.at(0).size(); col++) {
            for (std::size_t inner = 0; inner < matrix_b.size(); inner++) {
                output_matrix.at(row).at(col) += matrix_a.at(row).at(inner) * matrix_b.at(inner).at(col);
            }
        }
    }
    return output_matrix;
}

void save_matrix (const matrix& matrix_to_save, const fs::path &output_file_path) {
    std::ofstream output(output_file_path);

    output << matrix_to_save.size() << std::endl;
    output << matrix_to_save.at(0).size() << std::endl;
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(5);
    for (auto row : matrix_to_save) {
        oss.str(std::string());
        std::copy(row.begin(), row.end()-1, std::ostream_iterator<double>(oss, ";"));
        std::copy(row.end()-1, row.end(), std::ostream_iterator<double>(oss));
        output << oss.str() << std::endl;
    }
}

bool check_matrix_sizes(const matrix& matrix_a, const matrix& matrix_b) {
    return matrix_a.size() == matrix_b.at(0).size() && matrix_a.at(0).size() == matrix_b.size();
}

int main(int argc, char *argv[]) {
    auto result = parse_arguments(argc, argv);
    fs::path matrix_a_path = fs::absolute(fs::path(result["matrix_a"].as<std::string>()));
    fs::path matrix_b_path = fs::absolute(fs::path(result["matrix_b"].as<std::string>()));
    fs::path output_file_path(result["output"].as<std::string>());
    if (matrix_a_path.is_relative()) {
        matrix_a_path.relative_path();
    }
    if(!fs::exists(matrix_a_path) || !fs::exists(matrix_b_path)) {
        printf("One of the input files doesnt exists, check them\n");
        printf("Provided matrix a path: %s\n", matrix_a_path.c_str());
        printf("Provided matrix b path: %s\n", matrix_b_path.c_str());
        exit(1);
    }
    std::string output_path = result["output"].as<std::string>();

    matrix matrix_a = parseCSV(matrix_a_path);
    matrix matrix_b = parseCSV(matrix_b_path);
    if (!check_matrix_sizes(matrix_a, matrix_b)) {
        return -1;
    }
    matrix output_matrix = multiply_matrixes(matrix_a, matrix_b);
    std::cout << std::fixed;
    std::cout << std::setprecision(5);
    save_matrix(output_matrix, output_file_path);
    return 0;
}
