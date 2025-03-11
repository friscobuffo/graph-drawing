#include <chrono>

#include "core/graph/graph.hpp"
#include "core/graph/file_loader.hpp"
#include "orthogonal/drawing_builder.hpp"
#include "core/graph/generators.hpp"

#include "core/graph/algorithms.hpp"

int main() {
    // SimpleGraph* graph = generate_connected_random_graph_degree_max_4(45, 60);
    SimpleGraph* graph = generate_connected_random_graph_degree_max_4(70, 110);
    // SimpleGraph* graph = loadSimpleUndirectedGraphFromFile("example-graphs/g8.txt");
    // SimpleGraph *graph = generate_grid_graph(15, 10);
    // SimpleGraph *graph = generate_triangle_graph(2);

    auto start_inc = std::chrono::high_resolution_clock::now();
    make_rectilinear_drawing_incremental_basis(*graph);
    auto end_inc = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_inc = end_inc - start_inc;
    std::cout << "make_rectilinear_drawing_incremental_basis\ntime: " << elapsed_inc.count() << " seconds\n";

    // auto start_inc_trips = std::chrono::high_resolution_clock::now();
    // make_rectilinear_drawing_incremental_triplets(*graph);
    // auto end_inc_trips = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double> elapsed_inc_trips = end_inc_trips - start_inc_trips;
    // std::cout << "make_rectilinear_drawing_incremental_triplets\ntime: " << elapsed_inc_trips.count() << " seconds\n";

    // auto start_inc_no_cycles = std::chrono::high_resolution_clock::now();
    // make_rectilinear_drawing_incremental_no_cycles(*graph);
    // auto end_inc_no_cycles = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double> elapsed_inc_no_cycles = end_inc_no_cycles - start_inc_no_cycles;
    // std::cout << "make_rectilinear_drawing_incremental_no_cycles\ntime: " << elapsed_inc_no_cycles.count() << " seconds\n";

    // auto start_all = std::chrono::high_resolution_clock::now();
    // make_rectilinear_drawing_all_cycles(*graph);
    // auto end_all = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double> elapsed_all = end_all - start_all;
    // std::cout << "make_rectilinear_drawing_all_cycles\ntime: " << elapsed_all.count() << " seconds\n";

    delete graph;
    return 0;
}
