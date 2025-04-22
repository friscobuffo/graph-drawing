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

#include "core/graph/graph.hpp"
#include "core/graph/file_loader.hpp"
#include "orthogonal/drawing_builder.hpp"
#include "orthogonal/file_loader.hpp"
#include "config/config.hpp"
#include "baseline-ogdf/drawer.hpp"

std::tuple<int, int, int, int, double> test_shape_metrics_approach(const SimpleGraph &graph)
{
    auto start = std::chrono::high_resolution_clock::now();
    auto result = make_rectilinear_drawing_incremental_basis<SimpleGraph>(graph);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    return std::make_tuple(result.crossings, result.bends, result.area, result.total_edge_length, elapsed.count());
}

std::tuple<int, int, int, int, double> test_ogdf_approach(const SimpleGraph &graph)
{
    auto start = std::chrono::high_resolution_clock::now();
    auto result = create_drawing(graph, "ogdf_output.svg", "ogdf_output.gml");
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    return std::make_tuple(result.crossings, result.bends, result.area, result.total_edge_length, elapsed.count());
}

void save_stats(
    std::ofstream &results_file,
    std::tuple<int, int, int, int, double> &results_shape_metrics,
    std::tuple<int, int, int, int, double> &results_ogdf,
    const std::string &graph_name)
{
    results_file << graph_name << ",";
    results_file << std::get<0>(results_shape_metrics) << ",";
    results_file << std::get<0>(results_ogdf) << ",";
    results_file << std::get<1>(results_shape_metrics) << ",";
    results_file << std::get<1>(results_ogdf) << ",";
    results_file << std::get<2>(results_shape_metrics) << ",";
    results_file << std::get<2>(results_ogdf) << ",";
    results_file << std::get<3>(results_shape_metrics) << ",";
    results_file << std::get<3>(results_ogdf) << ",";
    results_file << std::get<4>(results_shape_metrics) << ",";
    results_file << std::get<4>(results_ogdf) << std::endl;
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

void compare_approaches_in_folder(std::string& folder_path, std::ofstream& results_file) {
    auto txt_files = collect_txt_files(folder_path);
    std::atomic<int> number_of_comparisons_done{0};
    std::mutex cout_mutex, file_mutex;
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

                // Increment and get the current comparison number
                int current_number = ++number_of_comparisons_done;

                // Thread-safe console output
                {
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    std::cout << "Processing comparison #" << current_number 
                              << " - " << graph_filename << std::endl;
                }

                // Load and process graph
                auto graph = load_simple_undirected_graph_from_txt_file(entry_path);
                auto result_shape_metrics = test_shape_metrics_approach(*graph);
                auto result_ogdf = test_ogdf_approach(*graph);

                // Thread-safe file writing
                {
                    std::lock_guard<std::mutex> lock(file_mutex);
                    save_stats(results_file, result_shape_metrics, result_ogdf, graph_filename);
                }
            }
        });
    }

    // Wait for all threads to complete
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    std::cout << "All comparisons done." << std::endl;
    std::cout << "Threads used: " << num_threads << std::endl;
}

void compare_approaches(std::unordered_map<std::string, std::string>& config) {
    std::cout << "Comparing approaches..." << std::endl;
    std::string test_results_filename = config["output_result_filename"];
    if (std::filesystem::exists(test_results_filename)) {
        std::cout << "File " << test_results_filename << " already exists." << std::endl;
        std::cout << "Do you want to delete it? (y/n): ";
        char answer;
        std::cin >> answer;
        if (answer == 'y' || answer == 'Y') {
            std::filesystem::remove(test_results_filename);
        } else {
            std::cout << "File not deleted." << std::endl;
            return;
        }
    }
    std::ofstream result_file(test_results_filename);
    if (result_file.is_open()) {
        result_file << "graph_name,";
        result_file << "shape_metrics_crossings,";
        result_file << "ogdf_crossings,";
        result_file << "shape_metrics_bends,";
        result_file << "ogdf_bends,";
        result_file << "shape_metrics_area,";
        result_file << "ogdf_area,";
        result_file << "shape_metrics_total_edge_length,";
        result_file << "ogdf_total_edge_length,";
        result_file << "shape_metrics_time,";
        result_file << "ogdf_time" << std::endl;
    }
    std::string test_graphs_folder = config["test_graphs_folder"];
    compare_approaches_in_folder(test_graphs_folder, result_file);
    std::cout << std::endl;
    result_file.close();
}

int main() {
    auto config = parse_config("config.txt");
    compare_approaches(config);

    return 0;
}
