#include <chrono>

#include "core/graph/graph.hpp"
#include "core/graph/file_loader.hpp"
#include "orthogonal/drawing_builder.hpp"
#include "core/graph/generators.hpp"

int main() {
    // SimpleGraph* graph = generate_connected_random_graph_degree_max_4(45, 60);
    SimpleGraph* graph = generate_connected_random_graph_degree_max_4(70, 110);
    // SimpleGraph* graph = loadSimpleUndirectedGraphFromFile("example-graphs/g8.txt");
    auto start_inc = std::chrono::high_resolution_clock::now();
    make_rectilinear_drawing_incremental(*graph);
    auto end_inc = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_inc = end_inc - start_inc;
    std::cout << "make_rectilinear_drawing_incremental\ntime: " << elapsed_inc.count() << " seconds\n";

    // auto start_all = std::chrono::high_resolution_clock::now();
    // make_rectilinear_drawing_all_cycles(*graph);
    // auto end_all = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double> elapsed_all = end_all - start_all;
    // std::cout << "make_rectilinear_drawing_all_cycles\ntime: " << elapsed_all.count() << " seconds\n";

    delete graph;
    return 0;
}