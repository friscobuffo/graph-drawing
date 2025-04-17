#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>
#include <tuple>
#include <filesystem>

#include "core/graph/graph.hpp"
#include "core/graph/file_loader.hpp"
#include "orthogonal/drawing_builder.hpp"
#include "core/graph/generators.hpp"
#include "orthogonal/orthogonal_algorithms.hpp"
#include "orthogonal/file_loader.hpp"
#include "core/graph/graphs_algorithms.hpp"
#include "globals/globals.hpp"
#include "../baseline-ogdf/drawer.hpp"

SimpleGraph *read_gml(const std::string &filename) {
    SimpleGraph *graph = new SimpleGraph();
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
    return graph;
}

ColoredNodesGraph *parse_shape_file(const std::string shape_file) {
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
    ColoredNodesGraph *colored_graph = new ColoredNodesGraph();
    for (int i = 0; i < nodes.size(); ++i)
        colored_graph->add_node(Color::BLACK);
    for (const auto &[u, v] : edges)
        colored_graph->add_edge(u, v);
    return colored_graph;
}

template <typename Func, typename Arg>
std::tuple<int, int, int, double> time_function(
    Func &&func, Arg &&arg, const std::string &func_name
) {
    std::cout << "start: " << func_name << "\n";
    auto start = std::chrono::high_resolution_clock::now();
    auto result = std::invoke(std::forward<Func>(func), std::forward<Arg>(arg));
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "time: " << elapsed.count() << " seconds\n";
    return std::make_tuple(result.crossings, result.bends, result.area, elapsed.count());
}

void save_stats(const std::string metric, const auto value_shape_metrics, const auto value_ogdf) {
    std::string filename = std::format("{}{}.txt", stats_path, metric);
    std::ofstream out_file(filename, std::ios::app);
    if (out_file.is_open()) {
        out_file << value_shape_metrics << "," << value_ogdf << std::endl;
        out_file.close();
    }
    else {
        std::cerr << "Error: Could not open for writing!" << std::endl;
    }
}

void save_drawing_to_file(const std::string input_folder) {
    for (const auto &entry : std::filesystem::directory_iterator(input_folder)) {
        if (entry.path().extension() == ".gml") {

            std::string input_file = entry.path().string();
            // graph_file = entry.path().stem().string();

            // SHAPE-METRICS
            auto graph = read_gml(input_file);
            auto result_shape_metrics = time_function(make_rectilinear_drawing_incremental_disjoint_paths<SimpleGraph>,
                                                   *graph,
                                                      "incremental from disjoint paths");

            // OGDF
            auto result_ogdf = time_function(create_drawing, input_file, "create drawing");
            save_stats(crossings_file, get<0>(result_shape_metrics), get<0>(result_ogdf));
            save_stats(bends_file, get<1>(result_shape_metrics), get<1>(result_ogdf));
            // std::unique_ptr<Shape> shape = load_shape_from_file("shape.txt");
            // ColoredNodesGraph *colored_graph = parse_shape_file("shape.txt");
            // save_stats(area_file, get<2>(result_shape_metrics), get<2>(result_ogdf));
            // save_stats("area_2", get<2>(result_shape_metrics), area);
            save_stats(running_time_file, get<3>(result_shape_metrics), get<3>(result_ogdf));
            std::cout << "Done with file: " << input_file << std::endl;
        }
    }
}

int main() {
    std::string input_folder = "test-graphs/input/generated_gml/";
    save_drawing_to_file(input_folder);
    return 0;
}
