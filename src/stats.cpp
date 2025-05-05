#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>
#include <tuple>
#include <memory>
#include <format>
#include <filesystem>
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_set>

#include "core/graph/graph.hpp"
#include "core/graph/file_loader.hpp"
#include "orthogonal/drawing_builder.hpp"
#include "orthogonal/file_loader.hpp"
#include "config/config.hpp"
#include "baseline-ogdf/drawer.hpp"
#include "core/csv.hpp"

std::unordered_set<std::string> graphs_already_in_csv;

std::tuple<DrawingResult, double> test_shape_metrics_approach(
    const SimpleGraph &graph, const std::string &svg_output_filename
) {
    auto start = std::chrono::high_resolution_clock::now();
    DrawingResult result = make_rectilinear_drawing_incremental_basis<SimpleGraph>(graph);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    node_positions_to_svg(*result.positions, *result.augmented_graph, svg_output_filename);
    if (check_if_drawing_has_overlappings(*result.augmented_graph, *result.positions))
        throw std::runtime_error("Drawing has overlappings");
    return std::make_tuple(
        std::move(result),
        elapsed.count()
    );
}

std::tuple<int, int, int, int, int, int, double, double, double> test_ogdf_approach(
    const SimpleGraph &graph, const std::string &svg_output_filename
) {
    auto start = std::chrono::high_resolution_clock::now();
    auto result = create_drawing(graph, svg_output_filename, "");
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    return std::make_tuple(
        result.crossings,
        result.bends,
        result.area,
        result.total_edge_length,
        result.max_edge_length,
        result.max_bends_per_edge,
        result.edge_length_stddev,
        result.bends_stddev,
        elapsed.count()
    );
}

void save_stats(std::ofstream &results_file, DrawingResult& results_shape_metrics, double shape_metrics_time, std::tuple<int, int, int, int, int, int, double, double, double> &results_ogdf, const std::string &graph_name) {
    results_file << graph_name << ",";
    results_file << results_shape_metrics.crossings << ",";
    results_file << std::get<0>(results_ogdf) << ",";
    results_file << results_shape_metrics.bends << ",";
    results_file << std::get<1>(results_ogdf) << ",";
    results_file << results_shape_metrics.area << ",";
    results_file << std::get<2>(results_ogdf) << ",";
    results_file << results_shape_metrics.total_edge_length << ",";
    results_file << std::get<3>(results_ogdf) << ",";
    results_file << results_shape_metrics.max_edge_length << ",";
    results_file << std::get<4>(results_ogdf) << ",";
    results_file << results_shape_metrics.max_bends_per_edge << ",";
    results_file << std::get<5>(results_ogdf) << ",";
    results_file << results_shape_metrics.edge_length_stddev << ",";
    results_file << std::get<6>(results_ogdf) << ",";
    results_file << results_shape_metrics.bends_stddev << ",";
    results_file << std::get<7>(results_ogdf) << ",";
    results_file << shape_metrics_time << ",";
    results_file << std::get<8>(results_ogdf) << ",";
    results_file << results_shape_metrics.initial_number_of_cycles << ",";
    results_file << results_shape_metrics.number_of_added_cycles << ",";
    results_file << results_shape_metrics.number_of_useless_bends;
    results_file << std::endl;
}

std::vector<std::string> collect_txt_files(const std::string& folder_path) {
    std::vector<std::string> txt_files;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(folder_path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            txt_files.push_back(entry.path().string());
        }
    }
    return txt_files;
}

void compare_approaches_in_folder(std::string& folder_path, std::ofstream& results_file, std::string& output_svgs_folder) {
    auto txt_files = collect_txt_files(folder_path);
    std::atomic<int> number_of_comparisons_done{0};
    std::mutex input_output_lock;
    std::atomic<size_t> index{0};

    unsigned num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;

    for (unsigned i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            while (true) {
                // Get the next file index atomically
                size_t current = index.fetch_add(1, std::memory_order_relaxed);
                if (current >= txt_files.size()) break;
                const auto& entry_path = txt_files[current];
                const std::string graph_filename = std::filesystem::path(entry_path).stem().string();
                int current_number = ++number_of_comparisons_done;
                {
                    std::lock_guard<std::mutex> lock(input_output_lock);
                    std::cout << "Processing comparison #" << current_number 
                              << " - " << graph_filename << std::endl;
                    if (graphs_already_in_csv.find(graph_filename) != graphs_already_in_csv.end()) {
                        std::cout << "Graph " << graph_filename << " already processed." << std::endl;
                        continue;
                    }
                }
                std::unique_ptr<SimpleGraph> graph;
                {
                    std::lock_guard<std::mutex> lock(input_output_lock);
                    std::cout << "Loading graph from file: " << entry_path << std::endl;
                    graph = load_simple_undirected_graph_from_txt_file(entry_path);
                }
                const std::string svg_output_filename_shape_metrics
                    = output_svgs_folder + graph_filename + "_shape_metrics.svg";
                const std::string svg_output_filename_ogdf
                    = output_svgs_folder + graph_filename + "_ogdf.svg";
                try {
                    auto result_shape_metrics = test_shape_metrics_approach(*graph, svg_output_filename_shape_metrics);
                    auto result_ogdf = test_ogdf_approach(*graph, svg_output_filename_ogdf);
                    {
                        std::lock_guard<std::mutex> lock(input_output_lock);
                        save_stats(results_file, std::get<0>(result_shape_metrics), std::get<1>(result_shape_metrics), result_ogdf, graph_filename);
                    }
                }
                catch (const std::exception& e) {
                    std::lock_guard<std::mutex> lock(input_output_lock);
                    std::cout << "Error processing graph " << graph_filename << std::endl;
                    throw;
                }
            }
        });
    }
    for (auto& t : threads)
        if (t.joinable()) t.join();
    std::cout << "All comparisons done." << std::endl;
    std::cout << "Threads used: " << num_threads << std::endl;
}

void compare_approaches(std::unordered_map<std::string, std::string>& config) {
    std::cout << "Comparing approaches..." << std::endl;
    std::string test_results_filename = config["output_result_filename"];
    bool append_mode = false;
    if (std::filesystem::exists(test_results_filename)) {
        std::cout << "File " << test_results_filename << " already exists." << std::endl;
        std::cout << "Do you want to append to it? (y/n): ";
        char answer;
        std::cin >> answer;
        if (answer != 'y' && answer != 'Y') {
            std::cout << "Aborting." << std::endl;
            return;
        }
        append_mode = true;
        auto csv_data = parse_csv(test_results_filename);
        for (const auto& row : csv_data.rows)
            if (row.size() > 0)
                graphs_already_in_csv.insert(row[0]);
    }
    std::ofstream result_file;
    if (append_mode) {
        result_file.open(test_results_filename, std::ios_base::app);
    } else {
        result_file.open(test_results_filename);
        if (result_file.is_open()) {
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
        }
    }
    if (!result_file.is_open()) {
        std::cerr << "Error: Could not open result file " << test_results_filename << std::endl;
        return;
    }
    std::string output_svgs_folder = config["output_svgs_folder"];
    if (!std::filesystem::exists(output_svgs_folder)) {
        if (!std::filesystem::create_directories(output_svgs_folder)) {
            std::cerr << "Error: Could not create directory " << output_svgs_folder << std::endl;
            return;
        }
    }
    std::string test_graphs_folder = config["test_graphs_folder"];
    compare_approaches_in_folder(test_graphs_folder, result_file, output_svgs_folder);
    std::cout << std::endl;
    result_file.close();
}

int main() {
    auto config = parse_config("config.txt");
    compare_approaches(config);

    return 0;
}
