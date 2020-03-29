#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <filesystem>
#include "argument_parsing.cpp"
#include <omp.h>
#include <list>
#include "../include/TextTable.h"

const int DECIMAL_PRECISION = 6;
const int OUTPUT_TIME_DECIMAL_PRECISION = 5;
using matrix = std::vector<std::vector<double>>;
namespace fs = std::filesystem;
using namespace std;

struct OutputResult {
    long double Tp;
    long double Ts;
    int process_count;
    bool is_schedule_static;
    int chunk_size;

};

void show_result(list<OutputResult> all_results) {
    TextTable table('-', '|', '+');
    table.add("No");
    table.add("Tp");
    table.add("Ts");
    table.add("Process count");
    table.add("Is schedule static?");
    table.add("Dynamic portion");
    table.endOfRow();
    int no = 0;
    for (OutputResult out_result : all_results) {
        no++;
        table.add(std::to_string(no));
        table.add(std::to_string(out_result.Tp));
        table.add(std::to_string(out_result.Ts));
        table.add(std::to_string(out_result.process_count));
        table.add(out_result.is_schedule_static > 0 ? "true" : "false");
        table.add(std::to_string(out_result.chunk_size));
        table.endOfRow();
    }
    std::cout << table << std::endl;
}

void save_matrix(const matrix &matrix_to_save, const std::string &output_file_path) {
    std::ofstream output(output_file_path);

    output << matrix_to_save.size() << std::endl;
    output << matrix_to_save.at(0).size() << std::endl;
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(DECIMAL_PRECISION);
    for (auto row : matrix_to_save) {
        oss.str(std::string());
        std::copy(row.begin(), row.end() - 1, std::ostream_iterator<double>(oss, ";"));
        std::copy(row.end() - 1, row.end(), std::ostream_iterator<double>(oss));
        output << oss.str() << std::endl;
    }
}

matrix multiply_matrixes_parallel(const matrix &matrix_a, const matrix &matrix_b, int processors_count,
                                  bool is_schedule_static, int chunk_size) {
    matrix output_matrix(matrix_a.size(), std::vector<double>(matrix_b.at(0).size()));
    int output_rows = output_matrix.size();
    int output_columns = output_matrix.at(0).size();
    int inner_size = matrix_b.size();
    if (is_schedule_static) {
        printf("Multiplying matrixes using static schedule with %d processors\n", processors_count);
#pragma omp parallel for num_threads(processors_count) collapse(3) default(none) schedule(static, chunk_size) \
        shared(matrix_a, matrix_b, output_matrix, output_rows, output_columns, inner_size, chunk_size)
        for (int row = 0; row < output_rows; row++) {
            for (int col = 0; col < output_columns; col++) {
                for (int inner = 0; inner < inner_size; inner++) {
                    output_matrix[row][col] += matrix_a[row][inner] * matrix_b[inner][col];
                }
            }
        }
    } else {
        printf("Multiplying matrixes using dynamic schedule with %d processors and %d dynamic portions\n",
               processors_count, chunk_size);
#pragma omp parallel for num_threads(processors_count) collapse(3) default(none) schedule(dynamic, chunk_size) \
        shared(matrix_a, matrix_b, output_matrix, output_rows, output_columns, inner_size, chunk_size)
        for (int row = 0; row < output_rows; row++) {
            for (int col = 0; col < output_columns; col++) {
                for (int inner = 0; inner < inner_size; inner++) {
                    output_matrix[row][col] += matrix_a[row][inner] * matrix_b[inner][col];
                }
            }
        }
    }
    return output_matrix;
}

matrix multiply_matrixes_sequential(const matrix &matrix_a, const matrix &matrix_b) {
    matrix output_matrix(matrix_a.size(), std::vector<double>(matrix_b.at(0).size()));
    int output_rows = output_matrix.size();
    int output_columns = output_matrix.at(0).size();
    int inner_size = matrix_b.size();
    printf("Multiplying matrixes using sequential method\n");
    for (int row = 0; row < output_rows; row++) {
        for (int col = 0; col < output_columns; col++) {
            for (int inner = 0; inner < inner_size; inner++) {
                output_matrix[row][col] += matrix_a[row][inner] * matrix_b[inner][col];
            }
        }
    }
    return output_matrix;
}

matrix multiply_matrixes(matrix &matrix_a, matrix &matrix_b, long double &Tp, long double &Ts, int process_count,
                         bool is_schedule_static, int chunksize) {
    matrix output;
    long double start_time = omp_get_wtime();
    output = multiply_matrixes_sequential(matrix_a, matrix_b);
    long double end_time = omp_get_wtime();
    Ts = end_time - start_time;
    start_time = omp_get_wtime();
    output = multiply_matrixes_parallel(matrix_a, matrix_b, process_count, is_schedule_static, chunksize);
    end_time = omp_get_wtime();
    Tp = end_time - start_time;
    return output;
}

bool check_matrix_sizes(const matrix &matrix_a, const matrix &matrix_b) {
    printf("Matrix a rows: %llu\n", matrix_a.size());
    printf("Matrix a columns: %llu\n", matrix_a.at(0).size());
    printf("Matrix b rows: %llu\n", matrix_b.size());
    printf("Matrix b columns: %llu\n", matrix_b.at(0).size());
    return matrix_a.at(0).size() == matrix_b.size();
}

matrix parse_csv(const fs::path &input_csv_file) {
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


int main(int argc, char *argv[]) {
    printf("Starting\n");
    list<OutputResult> all_results;
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
    matrix matrix_a = parse_csv(matrix_a_path);
    printf("Loading matrix b\n");
    matrix matrix_b = parse_csv(matrix_b_path);
    if (!check_matrix_sizes(matrix_a, matrix_b)) {
        printf("Matrix sizes are incorrect\n");
        return -1;
    }
    std::string prompt;
    prompt = "N";
    int process_count = 1;
    long double Ts;
    long double Tp;
    bool is_schedule_static = false;
    int chunk_size = 0;
    while (!prompt.compare("N")) {
        chunk_size = 0;
        printf("Specify number of processes:\n");
        cin >> process_count;
        printf("Specify type of is_scheduled [static]/dynamic:\n");
        cin >> prompt;
        is_schedule_static = "dynamic" != prompt;
        printf("Specify chunk size:\n");
        cin >> chunk_size;
        printf("Multiplying matrixes\n");
        matrix output_matrix = multiply_matrixes(matrix_a, matrix_b, Tp, Ts, process_count, is_schedule_static,
                                                 chunk_size);
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(OUTPUT_TIME_DECIMAL_PRECISION);
        oss << "C_" << Ts << "_" << Tp << ".csv";
        printf("Saving matrix\n");
        save_matrix(output_matrix, oss.str());
        OutputResult output_result = {
                Tp, Ts, process_count, is_schedule_static, chunk_size
        };
        all_results.push_back(output_result);
        show_result(all_results);
        printf("Exit? [Y]/N:\n");

        std::cin >> prompt;
        if (prompt == "n") {
            prompt = "N";
        }
    }
    return 0;
}

//SCHEDULE
/* Klauzula schedule zwieksza mozliwosc sterowania w jaki sposob wykonywany jest program, jednak w przypadku niektórych
 * parametrów zrównoleglenie może spowodować spowolnienie działania programu, zamiast jego przyspieszenie.
 * Przykładowo w tym programie użycie 'dynamic' z małą wartością porcji dla każdego wątku powoduje znaczące
 * spowolnienie. Jednak przy większych wartościach program działa szybciej niż w wersji sekwencyjnej.
 */

//COLLAPSE
/* Klauzula collapse znacząco ułatwia optymalizację zagnieżdżonych pętli.
 * Przy użyciu zwykłego zagnieżdżania pętli, należałoby odpowiednio rozdzielić ilość procesorów pomiędzy nimi.
 * W przypadku 3 zagnieżdżeń, jak poniżej jest to znacząco utrudnione by było optymalne.
 * Collapse ułatwia to, przez co biblioteka sama zajmuje się rozdzieleniem wątków w najbardziej optymalny sposób,
 * oraz powoduje że kod jest znacznie czytelniejszy.
 * */