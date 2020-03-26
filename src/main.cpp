#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iterator>

using matrix = std::vector<std::vector<double>>;

matrix parseCSV(const std::string &input_csv_file) {
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

void save_matrix (const matrix& matrix_to_save) {
    std::ofstream output("file.csv");

    output << matrix_to_save.size() << std::endl;
    output << matrix_to_save.at(0).size() << std::endl;
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(5);
    for (auto row : matrix_to_save) {
        oss.str(std::string());
        std::copy(row.begin(), row.end(), std::ostream_iterator<double>(oss, ";"));
        output << oss.str() << std::endl;
    }
}

int main() {
    matrix matrix_a = parseCSV(R"(M:\C++\Projects\ProgWspRozprZad3MacierzeCpp\matrix_a.csv)");
    matrix matrix_b = parseCSV(R"(M:\C++\Projects\ProgWspRozprZad3MacierzeCpp\matrix_b.csv)");
    auto output_matrix = multiply_matrixes(matrix_a, matrix_b);
    std::cout << std::fixed;
    std::cout << std::setprecision(5);
    for (std::vector<double> &row : output_matrix) {
        for (double &val : row) {
            std::cout << val << ";";
        }
        std::cout << std::endl;
    }
    save_matrix(output_matrix);
    return 0;
}
