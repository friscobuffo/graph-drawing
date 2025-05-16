#include <chrono>
#include <iostream>
#include <vector>
#include <thread>
#include <filesystem>
#include <unordered_set>
#include <utility>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>

#include "core/graph/graph.hpp"
#include "core/graph/file_loader.hpp"
#include "orthogonal/drawing_builder.hpp"
#include "config/config.hpp"
#include "baseline-ogdf/drawer.hpp"
#include "core/csv.hpp"

std::unordered_set<std::string> graphs_already_in_csv;

auto test_shape_metrics_approach(
    const Graph &graph, const std::string &svg_output_filename
) {
    auto start = std::chrono::high_resolution_clock::now();
    DrawingResult result = make_rectilinear_drawing_incremental_basis(graph);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    node_positions_to_svg(
        result.positions, *result.augmented_graph, 
        result.attributes, svg_output_filename
    );
    if (check_if_drawing_has_overlappings(*result.augmented_graph, result.positions))
        throw std::runtime_error("Drawing has overlappings");
    return std::make_pair(
        std::move(result),
        elapsed.count()
    );
}

auto test_ogdf_approach(
    const Graph &graph, const std::string &svg_output_filename
) {
    auto start = std::chrono::high_resolution_clock::now();
    auto result = create_drawing(graph, svg_output_filename, "");
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    return std::make_pair(
        std::move(result),
        elapsed.count()
    );
}

void save_stats(
    int file_descriptor,
    DrawingResult& results_shape_metrics, double shape_metrics_time,
    OGDFResult& results_ogdf, double ogdf_time,
    const std::string& graph_name
) {
    std::string result_line = graph_name + "," +
        std::to_string(results_shape_metrics.crossings) + "," +
        std::to_string(results_ogdf.crossings) + "," +
        std::to_string(results_shape_metrics.bends) + "," +
        std::to_string(results_ogdf.bends) + "," +
        std::to_string(results_shape_metrics.area) + "," +
        std::to_string(results_ogdf.area) + "," +
        std::to_string(results_shape_metrics.total_edge_length) + "," +
        std::to_string(results_ogdf.total_edge_length) + "," +
        std::to_string(results_shape_metrics.max_edge_length) + "," +
        std::to_string(results_ogdf.max_edge_length) + "," +
        std::to_string(results_shape_metrics.max_bends_per_edge) + "," +
        std::to_string(results_ogdf.max_bends_per_edge) + "," +
        std::to_string(results_shape_metrics.edge_length_stddev) + "," +
        std::to_string(results_ogdf.edge_length_stddev) + "," +
        std::to_string(results_shape_metrics.bends_stddev) + "," +
        std::to_string(results_ogdf.bends_stddev) + "," +
        std::to_string(shape_metrics_time) + "," +
        std::to_string(ogdf_time) + "," +
        std::to_string(results_shape_metrics.initial_number_of_cycles) + "," +
        std::to_string(results_shape_metrics.number_of_added_cycles) + "," +
        std::to_string(results_shape_metrics.number_of_useless_bends) + "\n";
    write(file_descriptor, result_line.c_str(), result_line.size());
}

std::vector<std::string> collect_txt_files(const std::string& folder_path) {
    std::vector<std::string> txt_files;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(folder_path))
        if (entry.is_regular_file() && entry.path().extension() == ".txt")
            txt_files.push_back(entry.path().string());
    return txt_files;
}

void compare_approaches_in_folder(
    const std::string& folder_path,
    int file_descriptor, 
    const std::string& output_svgs_folder
) {
    auto txt_files = collect_txt_files(folder_path);
    unsigned num_threads = std::thread::hardware_concurrency();
    for (unsigned i = 0; i < num_threads; ++i) {
        pid_t pid = fork();
        if (pid < 0) {
            std::cerr << "Fork failed" << std::endl;
            return;
        }
        if (pid == 0) { // Child process
            for (int index = 0; index < txt_files.size(); ++index) {
                if (index % num_threads != i) continue;
                const auto& entry_path = txt_files[index];
                const std::string graph_filename = std::filesystem::path(entry_path).stem().string();
                if (graphs_already_in_csv.contains(graph_filename))
                    continue;
                Graph graph;
                load_graph_from_txt_file(entry_path, graph);
                std::cout << "Processing comparison #" << index 
                    << " - " << graph_filename << std::endl;
                const std::string svg_output_filename_shape_metrics
                    = output_svgs_folder + graph_filename + "_shape_metrics.svg";
                const std::string svg_output_filename_ogdf
                    = output_svgs_folder + graph_filename + "_ogdf.svg";
                try {
                    srand(0);
                    auto result_shape_metrics = test_shape_metrics_approach(
                        graph, svg_output_filename_shape_metrics
                    );
                    auto result_ogdf = test_ogdf_approach(
                        graph, svg_output_filename_ogdf
                    );
                    save_stats(
                        file_descriptor,
                        result_shape_metrics.first,
                        result_shape_metrics.second,
                        result_ogdf.first,
                        result_ogdf.second,
                        graph_filename
                    );
                }
                catch (const std::exception& e) {
                    std::cout << "Error processing graph " << graph_filename << std::endl;
                    std::cout << "Exception: " << e.what() << std::endl;
                    kill(getppid(), SIGTERM); // Signal parent
                    kill(0, SIGTERM);         // Signal all processes in the same group
                    _exit(1);                 // Exit this child
                }
            }
            _exit(0); // Exit child process
        }
    }
    for (int i = 0; i < num_threads; i++) wait(nullptr);
    std::cout << "All comparisons done." << std::endl;
    std::cout << "Threads used: " << num_threads << std::endl;
}

void initialize_csv_file(const std::string& test_results_filename) {
    std::ofstream result_file;
    result_file.open(test_results_filename);
    if (!result_file.is_open())
        throw std::runtime_error("Error: Could not open result file");
    result_file << "graph_name,"
                << "shape_metrics_crossings,"
                << "ogdf_crossings,"
                << "shape_metrics_bends,"
                << "ogdf_bends,"
                << "shape_metrics_area,"
                << "ogdf_area,"
                << "shape_metrics_total_edge_length,"
                << "ogdf_total_edge_length,"
                << "shape_metrics_max_edge_length,"
                << "ogdf_max_edge_length,"
                << "shape_metrics_max_bends_per_edge,"
                << "ogdf_max_bends_per_edge,"
                << "shape_metrics_edge_length_stddev,"
                << "ogdf_edge_length_stddev,"
                << "shape_metrics_bends_stddev,"
                << "ogdf_bends_stddev,"
                << "shape_metrics_time,"
                << "ogdf_time,"
                << "shape_metrics_initial_number_cycles,"
                << "shape_metrics_number_added_cycles,"
                << "shape_metrics_number_useless_bends"
                << std::endl;
    result_file.close();
}

void compare_approaches(const Config& config) {
    std::cout << "Comparing approaches..." << std::endl;
    const std::string& test_results_filename = config.get("output_result_filename");
    std::ofstream result_file;
    if (std::filesystem::exists(test_results_filename)) {
        std::cout << "File " << test_results_filename << " already exists." << std::endl;
        std::cout << "What do you want to do?" << std::endl;
        std::cout << "1. Overwrite the file" << std::endl;
        std::cout << "2. Append to the file" << std::endl;
        std::cout << "3. Abort" << std::endl;
        std::cout << "Please enter your choice (1/2/3): ";
        int choice;
        std::cin >> choice;
        if (choice == 1) {
            std::filesystem::remove(test_results_filename);
            initialize_csv_file(test_results_filename);
        } else if (choice == 2) {
            auto csv_data = parse_csv(test_results_filename);
            for (const auto& row : csv_data.rows)
                if (row.size() > 0)
                    graphs_already_in_csv.insert(row[0]);
        } else {
            std::cout << "Aborting." << std::endl;
            return;
        }
    }
    else
        initialize_csv_file(test_results_filename);
    int file_desc = open(test_results_filename.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0644);
    const std::string& output_svgs_folder = config.get("output_svgs_folder");
    if (!std::filesystem::exists(output_svgs_folder))
        if (!std::filesystem::create_directories(output_svgs_folder)) {
            std::cerr << "Error: Could not create directory " << output_svgs_folder << std::endl;
            return;
        }
    const std::string& test_graphs_folder = config.get("test_graphs_folder");
    compare_approaches_in_folder(test_graphs_folder, file_desc, output_svgs_folder);
    std::cout << std::endl;
    close(file_desc);
}

int main() {
    Config config("config.txt");
    compare_approaches(config);
    return 0;
}
