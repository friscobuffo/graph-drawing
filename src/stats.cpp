#include <atomic>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>
#include <unordered_set>
#include <vector>

#include "baseline-ogdf/drawer.hpp"
#include "config/config.hpp"
#include "core/csv.hpp"
#include "core/graph/file_loader.hpp"
#include "core/graph/graph.hpp"
#include "orthogonal/drawing_builder.hpp"
#include "orthogonal/drawing_stats.hpp"
#include "orthogonal/file_loader.hpp"

std::unordered_set<std::string> graphs_already_in_csv;
int total_fails = 0;

auto test_shape_metrics_approach(const Graph& graph,
                                 const std::string& svg_output_filename) {
  auto start = std::chrono::high_resolution_clock::now();
  DrawingResult result = make_orthogonal_drawing_sperimental(graph);
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  node_positions_to_svg(result.positions, *result.augmented_graph,
                        result.attributes, svg_output_filename);
  // if (check_if_drawing_has_overlappings(*result.augmented_graph,
  //                                       result.positions))
  //   throw std::runtime_error("Drawing has overlappings");
  return std::make_pair(std::move(result), elapsed.count());
}

auto test_ogdf_approach(const Graph& graph,
                        const std::string& svg_output_filename) {
  auto start = std::chrono::high_resolution_clock::now();
  auto result = create_drawing(graph, svg_output_filename, "");
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  return std::make_pair(result, elapsed.count());
}

void save_stats(std::ofstream& results_file,
                const DrawingResult& results_shape_metrics,
                double shape_metrics_time, const OGDFResult& results_ogdf,
                double ogdf_time, const std::string& graph_name) {
  auto stats_shape_metrics =
      compute_all_orthogonal_stats(results_shape_metrics);
  results_file << graph_name << ",";
  results_file << stats_shape_metrics.crossings << ",";
  results_file << results_ogdf.crossings << ",";
  results_file << stats_shape_metrics.bends << ",";
  results_file << results_ogdf.bends << ",";
  results_file << stats_shape_metrics.area << ",";
  results_file << results_ogdf.area << ",";
  results_file << stats_shape_metrics.total_edge_length << ",";
  results_file << results_ogdf.total_edge_length << ",";
  results_file << stats_shape_metrics.max_edge_length << ",";
  results_file << results_ogdf.max_edge_length << ",";
  results_file << stats_shape_metrics.max_bends_per_edge << ",";
  results_file << results_ogdf.max_bends_per_edge << ",";
  results_file << stats_shape_metrics.edge_length_stddev << ",";
  results_file << results_ogdf.edge_length_stddev << ",";
  results_file << stats_shape_metrics.bends_stddev << ",";
  results_file << results_ogdf.bends_stddev << ",";
  results_file << shape_metrics_time << ",";
  results_file << ogdf_time << ",";
  results_file << results_shape_metrics.initial_number_of_cycles << ",";
  results_file << results_shape_metrics.number_of_added_cycles << ",";
  results_file << results_shape_metrics.number_of_useless_bends;
  results_file << std::endl;
}

void compare_approaches_in_folder(std::string& folder_path,
                                  std::ofstream& results_file,
                                  std::string& output_svgs_folder) {
  auto txt_files = collect_txt_files(folder_path);
  std::atomic<int> number_of_comparisons_done{0};
  std::mutex input_output_lock;
  std::atomic<int> index{0};
  unsigned num_threads = std::thread::hardware_concurrency();
  std::vector<std::thread> threads;
  for (unsigned i = 0; i < num_threads; ++i) {
    threads.emplace_back([&]() {
      while (true) {
        int current = index.fetch_add(1, std::memory_order_relaxed);
        if (current >= txt_files.size()) break;
        const auto& entry_path = txt_files[current];
        const std::string graph_filename =
            std::filesystem::path(entry_path).stem().string();
        int current_number =
            number_of_comparisons_done.fetch_add(1, std::memory_order_relaxed);
        if (graphs_already_in_csv.contains(graph_filename)) continue;
        std::unique_ptr<Graph> graph = load_graph_from_txt_file(entry_path);
        if (!is_graph_connected(*graph)) {
          std::lock_guard<std::mutex> lock(input_output_lock);
          std::cerr << "Graph " << graph_filename
                    << " is not connected, skipping." << std::endl;
          continue;
        }
        {
          std::lock_guard<std::mutex> lock(input_output_lock);
          std::cout << "Processing comparison #" << current_number << " - "
                    << graph_filename << std::endl;
        }
        const std::string svg_output_filename_shape_metrics =
            output_svgs_folder + graph_filename + "_shape_metrics.svg";
        const std::string svg_output_filename_ogdf =
            output_svgs_folder + graph_filename + "_ogdf.svg";
        try {
          auto result_shape_metrics = test_shape_metrics_approach(
              *graph, svg_output_filename_shape_metrics);
          auto result_ogdf =
              test_ogdf_approach(*graph, svg_output_filename_ogdf);
          {
            std::lock_guard<std::mutex> lock(input_output_lock);
            save_stats(results_file, result_shape_metrics.first,
                       result_shape_metrics.second, result_ogdf.first,
                       result_ogdf.second, graph_filename);
          }
        } catch (const std::exception& e) {
          std::lock_guard<std::mutex> lock(input_output_lock);
          std::cerr << "Error processing graph " << graph_filename << std::endl;
          if (std::string(e.what()) != "Graph contains cycle") {
            std::cerr << "Error: " << e.what() << std::endl;
            throw;
          } else {
            std::cout << "Graph contains cycle, skipping." << std::endl;
            total_fails++;
          }
        }
      }
    });
  }
  for (auto& t : threads)
    if (t.joinable()) t.join();
  std::cout << "All comparisons done." << std::endl;
  std::cout << "Threads used: " << num_threads << std::endl;
  std::cout << "Total fails: " << total_fails << std::endl;
  std::cout << "Total comparisons: " << number_of_comparisons_done.load() << std::endl;
}

void initialize_csv_file(std::ofstream& result_file) {
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
              << "shape_metrics_number_useless_bends" << std::endl;
}

void compare_approaches(const Config& config) {
  std::cout << "Comparing approaches..." << std::endl;
  std::string test_results_filename = config.get("output_result_filename");
  std::ofstream result_file;
  if (std::filesystem::exists(test_results_filename)) {
    std::cout << "File " << test_results_filename << " already exists."
              << std::endl;
    std::cout << "What do you want to do?" << std::endl;
    std::cout << "1. Overwrite the file" << std::endl;
    std::cout << "2. Append to the file" << std::endl;
    std::cout << "3. Abort" << std::endl;
    std::cout << "Please enter your choice (1/2/3): ";
    int choice;
    std::cin >> choice;
    if (choice == 1) {
      std::filesystem::remove(test_results_filename);
      result_file.open(test_results_filename);
      initialize_csv_file(result_file);
    } else if (choice == 2) {
      auto csv_data = parse_csv(test_results_filename);
      for (const auto& row : csv_data.rows)
        if (row.size() > 0) graphs_already_in_csv.insert(row[0]);
      result_file.open(test_results_filename, std::ios_base::app);
    } else {
      std::cout << "Aborting." << std::endl;
      return;
    }
  } else {
    result_file.open(test_results_filename);
    initialize_csv_file(result_file);
  }
  std::string output_svgs_folder = config.get("output_svgs_folder");
  if (!std::filesystem::exists(output_svgs_folder))
    if (!std::filesystem::create_directories(output_svgs_folder)) {
      std::cerr << "Error: Could not create directory " << output_svgs_folder
                << std::endl;
      return;
    }
  std::string test_graphs_folder = config.get("test_graphs_folder");
  compare_approaches_in_folder(test_graphs_folder, result_file,
                               output_svgs_folder);
  std::cout << std::endl;
  result_file.close();
}

int main() {
  Config config("config.txt");
  compare_approaches(config);
  return 0;
}