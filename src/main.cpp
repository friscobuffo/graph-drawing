#include <iostream>

#include "core/graph/graph.hpp"
#include "orthogonal/drawing_builder.hpp"
#include "core/graph/file_loader.hpp"
#include "core/graph/generators.hpp"
#include "config/config.hpp"
#include "baseline-ogdf/drawer.hpp"

void prova0() {
    Graph graph;
    for (int i = 0; i < 12; ++i)
        graph.add_node(i);
    graph.add_undirected_edge(0, 1);
    graph.add_undirected_edge(3, 1);
    graph.add_undirected_edge(3, 2);
    graph.add_undirected_edge(0, 2);
    graph.add_undirected_edge(3, 4);
    graph.add_undirected_edge(4, 5);
    graph.add_undirected_edge(5, 6);
    graph.add_undirected_edge(7, 6);
    graph.add_undirected_edge(0, 8);
    graph.add_undirected_edge(0, 7);
    graph.add_undirected_edge(0, 10);
    graph.add_undirected_edge(10, 11);
    graph.add_undirected_edge(3, 11);
    graph.add_undirected_edge(3, 9);
    graph.add_undirected_edge(8, 9);
    auto result = make_orthogonal_drawing_sperimental(graph);
}

void prova1() {
    
    Graph graph;
    for (int i = 0; i < 16; ++i)
        graph.add_node(i);
    graph.add_undirected_edge(0, 1);
    graph.add_undirected_edge(1, 2);
    graph.add_undirected_edge(2, 3);
    graph.add_undirected_edge(3, 0);
    graph.add_undirected_edge(4, 5);
    graph.add_undirected_edge(5, 6);
    graph.add_undirected_edge(6, 2);
    graph.add_undirected_edge(7, 8);
    graph.add_undirected_edge(8, 9);
    graph.add_undirected_edge(9, 11);
    graph.add_undirected_edge(10, 11);
    graph.add_undirected_edge(6, 12);
    graph.add_undirected_edge(12, 10);
    graph.add_undirected_edge(10, 2);
    graph.add_undirected_edge(13, 14);
    graph.add_undirected_edge(14, 7);
    graph.add_undirected_edge(13, 0);
    graph.add_undirected_edge(15, 13);
    graph.add_undirected_edge(15, 4);
    graph.add_undirected_edge(0, 7);
    graph.add_undirected_edge(3, 8);
    graph.add_undirected_edge(2, 9);
    graph.add_undirected_edge(1, 5);
    graph.add_undirected_edge(4, 0);
    auto result = make_orthogonal_drawing_sperimental(graph);
}

int main() {
    prova1();
    Config config("config.txt");
    const std::string& filename = config.get("output_svg_shape_metrics");
    auto graph = load_graph_from_txt_file(config.get("input_graph_file"));

    auto result_ogdf = create_drawing(*graph, config.get("output_svg_ogdf"));
    std::cout << "OGDF:\n";
    std::cout << "Area: " << result_ogdf.area << "\n";
    std::cout << "Crossings: " << result_ogdf.crossings << "\n";
    std::cout << "Bends: " << result_ogdf.bends << "\n";
    std::cout << "Total edge length: " << result_ogdf.total_edge_length << "\n";

    srand(0);
    auto result = make_orthogonal_drawing_low_degree(*graph);
    node_positions_to_svg(
        result.positions,
        *result.augmented_graph,
        result.attributes,
        filename
    );
    std::cout << "Shape metrics:\n";
    std::cout << "Area: " << result.area << "\n";
    std::cout << "Crossings: " << result.crossings << "\n";
    std::cout << "Bends: " << result.bends << "\n";
    std::cout << "Total edge length: " << result.total_edge_length << "\n";
    return 0;
}
