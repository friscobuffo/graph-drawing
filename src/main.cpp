#include <chrono>
#include <functional>
#include <iostream>

#include "core/graph/graph.hpp"
#include "core/graph/file_loader.hpp"
#include "orthogonal/drawing_builder.hpp"
#include "core/graph/generators.hpp"
#include "orthogonal/orthogonal_algorithms.hpp"
#include "core/graph/graphs_algorithms.hpp"
#include "config/config.hpp"

#include "orthogonal/file_loader.hpp"
#include <cassert>

// template <GraphTrait G, typename Func>
// void time_function(Func& func, const G& graph, const std::string& func_name) {
//     std::cout << "start: " << func_name << "\n";
//     auto start = std::chrono::high_resolution_clock::now();
//     DrawingResult result = func(graph);
//     auto end = std::chrono::high_resolution_clock::now();
//     std::chrono::duration<double> elapsed = end - start;
//     std::cout << "time: " << elapsed.count() << " seconds\n";
// }

int main() {
    // auto graph = generate_connected_random_graph_degree_max_4(45, 60);
    // auto graph = generate_connected_random_graph_degree_max_4(55, 80);
    // auto graph = generate_connected_random_graph_degree_max_4(70, 110);
    // auto graph = generate_connected_random_graph_degree_max_4(150, 220);
    /*
    auto graph = load_simple_undirected_graph_from_file(
        "example-graphs/random_graph_augmented.txt"
    );
    time_function(
        make_rectilinear_drawing_incremental_basis<SimpleGraph>,
        *graph,
        "incremental from basis"
    );
    time_function(
        make_rectilinear_drawing_incremental_disjoint_paths<SimpleGraph>,
        *graph,
        "incremental from disjoint paths"
    );
    */

    const std::string filename = "drawing.svg";

    int test_number = 0;
    while (true) {
        std::cout << "\rtest number: " << ++test_number << std::flush;
        auto graph = generate_connected_random_graph_degree_max_4(12, 16);
        auto result = make_rectilinear_drawing_incremental_disjoint_paths<SimpleGraph>(*graph, filename);
        if (result.number_of_added_cycles > 0) {
            std::cout << std::endl;
            graph->print();
            break;
        }
        break;
    }

    return 0;
}