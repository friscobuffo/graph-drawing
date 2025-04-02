#include <chrono>

#include "core/graph/graph.hpp"
#include "core/graph/file_loader.hpp"
#include "orthogonal/drawing_builder.hpp"
#include "core/graph/generators.hpp"

int main() {
    // SimpleGraph* graph = generate_connected_random_graph_degree_max_4(45, 60);
    // SimpleGraph* graph = generate_connected_random_graph_degree_max_4(55, 80);
    // SimpleGraph* graph = generate_connected_random_graph_degree_max_4(70, 110);
    // SimpleGraph* graph = loadSimpleUndirectedGraphFromFile("example-graphs/random_graph_augmented.txt");
    SimpleGraph* graph = new SimpleGraph();
    graph->add_node();
    graph->add_node();
    graph->add_node();
    graph->add_undirected_edge(0, 1);
    graph->add_undirected_edge(1, 2);
    graph->add_undirected_edge(2, 0);
    // const SimpleGraph *graph = generate_grid_graph(15, 10);
    // SimpleGraph *graph = generate_triangle_graph(15);

    auto start_inc_basis = std::chrono::high_resolution_clock::now();
    make_rectilinear_drawing_incremental_basis(*graph);
    auto end_inc_basis = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_inc_basis = end_inc_basis - start_inc_basis;
    std::cout << "make_rectilinear_drawing_incremental_basis\ntime: " << elapsed_inc_basis.count() << " seconds\n";

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

    // auto start_inc_pairs = std::chrono::high_resolution_clock::now();
    // make_rectilinear_drawing_incremental_pairs(*graph);
    // auto end_inc_pairs = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double> elapsed_inc_pairs = end_inc_pairs - start_inc_pairs;
    // std::cout << "make_rectilinear_drawing_incremental_pairs\ntime: " << elapsed_inc_pairs.count() << " seconds\n";

    delete graph;
    return 0;
}
