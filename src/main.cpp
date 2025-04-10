#include <chrono>
#include <functional>
#include <iostream>

#include "core/graph/graph.hpp"
#include "core/graph/file_loader.hpp"
#include "orthogonal/drawing_builder.hpp"
#include "core/graph/generators.hpp"
#include "orthogonal/orthogonal_algorithms.hpp"
#include "core/graph/graphs_algorithms.hpp"

template <GraphTrait G, typename Func>
void time_function(Func& func, const G& graph, const std::string& func_name) {
    auto start = std::chrono::high_resolution_clock::now();
    auto result = func(graph);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << func_name << "\ntime: " << elapsed.count() << " seconds\n";
}

int main() {
    // SimpleGraph* graph = generate_connected_random_graph_degree_max_4(45, 60);
    // SimpleGraph* graph = generate_connected_random_graph_degree_max_4(55, 80);
    // SimpleGraph* graph = generate_connected_random_graph_degree_max_4(70, 110);
    // SimpleGraph* graph = generate_connected_random_graph_degree_max_4(150, 220);
    auto graph = load_simple_undirected_graph_from_file("example-graphs/random_graph_augmented.txt");
    
    time_function(make_rectilinear_drawing_incremental_basis<SimpleGraph>, *graph, "incremental from basis");

    std::vector<std::vector<size_t>> cycles;
    for (int i = 0; i < graph->size()-1; i++) {
        if (graph->get_node(i).get_degree() <= 2) continue;
        for (int j = i + 1; j < graph->size(); j++) {
            if (graph->get_node(j).get_degree() <= 2) continue;
            auto paths = find_disjoint_paths(*graph, i, j);
            if (paths.size() <= 1) continue;
            for (int p1 = 0; p1 < paths.size()-1; ++p1) {
                auto& path1 = paths[p1];
                for (int p2 = p1+1; p2 < paths.size(); ++p2) {
                    auto& path2 = paths[p2];
                    std::vector<size_t> cycle(path1.begin(), path1.end());
                    for (int k = path2.size()-2; k > 0; --k)
                        cycle.push_back(path2[k]);
                    cycles.push_back(cycle);
                }
            }
        }
    }

    auto start_inc_special = std::chrono::high_resolution_clock::now();
    auto result_special = make_rectilinear_drawing_incremental(*graph, cycles);
    auto end_inc_special = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_inc_special = end_inc_special - start_inc_special;
    std::cout << "make_rectilinear_drawing_incremental_special\ntime: " << elapsed_inc_special.count() << " seconds\n";

    return 0;
}