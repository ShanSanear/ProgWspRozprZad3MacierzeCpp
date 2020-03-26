#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>

using matrix = std::vector<std::vector<double>>;

matrix parseCSV(const std::string& input_csv_file) {
    std::ifstream data(input_csv_file);
    std::string line;
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

matrix multiply_matrixes(const matrix& matrix_a, const matrix& matrix_b) {
    matrix output_matrix(matrix_a.size(), std::vector<double>(matrix_b.at(0).size()));
    for(std::size_t row = 0; row < output_matrix.size(); ++row) {
        for(std::size_t col = 0; col < output_matrix.at(0).size(); ++col) {
            for(std::size_t inner = 0; inner < matrix_b.size(); ++inner) {
                output_matrix.at(row).at(col) += matrix_a.at(row).at(inner) * matrix_b.at(inner).at(col);
            }
        }
    }
    return output_matrix;

}

int main() {
    matrix matrix_a = parseCSV(R"(M:\C++\Projects\ProgWspRozprZad3MacierzeCpp\matrix_a.csv)");
    matrix matrix_b = parseCSV(R"(M:\C++\Projects\ProgWspRozprZad3MacierzeCpp\matrix_b.csv)");
    auto output_matrix = multiply_matrixes(matrix_a, matrix_b);
    for (std::vector<double> &row : output_matrix) {
        for (double &val : row) {
            std::cout << val << std::endl;
        }
        std::cout << "Next line" << std::endl;
    }
    return 0;
}
