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

#include "core/graph/graph.hpp"
#include "core/graph/file_loader.hpp"
#include "orthogonal/drawing_builder.hpp"
#include "core/graph/generators.hpp"
#include "orthogonal/orthogonal_algorithms.hpp"
#include "orthogonal/file_loader.hpp"
#include "core/graph/graphs_algorithms.hpp"
#include "config/config.hpp"
#include "baseline-ogdf/drawer.hpp"

std::unique_ptr<SimpleGraph> read_gml(const std::string& filename) {
    SimpleGraph* graph = new SimpleGraph();
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    std::string line;
    bool in_node_block = false, in_edge_block = false;
    int node_id = -1, source = -1, target = -1;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;
        if (token == "node") {
            in_node_block = true;
            node_id = -1;
        }
        else if (token == "edge") {
            in_edge_block = true;
            source = target = -1;
        }
        else if (token == "id" && in_node_block) {
            iss >> node_id;
            graph->add_node(); // Add node to graph
        }
        else if (token == "source" && in_edge_block) {
            iss >> source;
        }
        else if (token == "target" && in_edge_block) {
            iss >> target;
        }
        else if (token == "]") {
            if (in_node_block) {
                in_node_block = false;
            }
            else if (in_edge_block) {
                if (source != -1 && target != -1) {
                    if (graph->get_nodes()[source].get_edges().size() < 4 && graph->get_nodes()[target].get_edges().size() < 4) {
                        graph->add_edge(source, target);
                        graph->add_edge(target, source);
                    }
                }
                in_edge_block = false;
            }
        }
    }

    // for(int i = 0; i < graph->get_nodes().size(); ++i) {
    //     while(graph->get_nodes()[i].get_edges().size() > 4) {
    //         int j = rand() % graph->get_nodes()[i].get_edges().size();
    //         graph->remove_edge(graph->get_nodes()[i].get_edges()[j]);
    //     }
    //     std::cout << std::endl;
    // }

    file.close();
    return std::unique_ptr<SimpleGraph>(graph);
}

std::unique_ptr<ColoredNodesGraph> parse_shape_file(const std::string& shape_file) {
    std::ifstream infile(shape_file);
    std::string line;
    std::set<int> nodes;
    std::vector<std::pair<int, int>> edges;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        int u, v;
        std::string direction;
        if (!(iss >> u >> v >> direction)) {
            std::cerr << "Invalid line: " << line << std::endl;
            continue;
        }
        edges.emplace_back(u, v);
        nodes.insert(u);
        nodes.insert(v);
    }
    ColoredNodesGraph* colored_graph = new ColoredNodesGraph();
    for (int i = 0; i < nodes.size(); ++i)
        colored_graph->add_node(Color::BLACK);
    for (const auto &[u, v] : edges)
        colored_graph->add_edge(u, v);
    return std::unique_ptr<ColoredNodesGraph>(colored_graph);
}

std::tuple<int, int, int, double> test_shape_metrics_approach(
    const SimpleGraph& graph, const std::string& svg_filename
) {
    auto start = std::chrono::high_resolution_clock::now();
    auto result = make_rectilinear_drawing_incremental_disjoint_paths<SimpleGraph>(
        graph, svg_filename
    );
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    return std::make_tuple(result.crossings, result.bends, result.area, elapsed.count());
}

std::tuple<int, int, int, double> test_ogdf_approach(
    const std::string& input_graph_file, const std::string& svg_filename
) {
    auto start = std::chrono::high_resolution_clock::now();
    auto result = create_drawing(input_graph_file);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    return std::make_tuple(result.crossings, result.bends, result.area, elapsed.count());
}

void save_stats(
    const std::string& folder_path, const std::string& metric,
    const auto value_shape_metrics, const auto value_ogdf
) {
    std::string filename = std::format("{}{}.txt", folder_path, metric);
    std::ofstream out_file(filename, std::ios::app);
    if (out_file.is_open()) {
        out_file << value_shape_metrics << "," << value_ogdf << std::endl;
        out_file.close();
    }
    else {
        std::cerr << "Error: Could not open for writing!" << std::endl;
    }
}

void compare_approaches(std::unordered_map<std::string, std::string>& config) {
    std::string test_results_folder = config["tests_results_folder"];
    for (const auto &entry : std::filesystem::directory_iterator(config["test_input_folder"])) {
        if (entry.path().extension() == ".gml") {
            const std::string input_file = entry.path().string();
            const std::string graph_filename = entry.path().stem().string();
            auto graph = read_gml(input_file);


            // SHAPE-METRICS
            std::string svg_filename_shape_metrics = std::format("{}{}-shape-metrics.svg", test_results_folder, graph_filename);
            auto result_shape_metrics = test_shape_metrics_approach(
                *graph,
                svg_filename_shape_metrics
            );

            // OGDF
            std::string svg_filename_ogdf = std::format("{}{}-ogdf.svg", test_results_folder, graph_filename);
            auto result_ogdf = test_ogdf_approach(input_file, svg_filename_ogdf);

            save_stats(test_results_folder, "crossings", get<0>(result_shape_metrics), get<0>(result_ogdf));
            save_stats(test_results_folder, "bends", get<1>(result_shape_metrics), get<1>(result_ogdf));
            save_stats(test_results_folder, "running_time", get<3>(result_shape_metrics), get<3>(result_ogdf));

            std::cout << "Done with file: " << input_file << std::endl;
        }
    }
}

int main() {
    auto config = parse_config("stats_config.txt");
    compare_approaches(config);
    return 0;
}
