#include <chrono>

#include "core/graph/graph.hpp"
#include "core/graph/file_loader.hpp"
#include "orthogonal/drawing_builder.hpp"
#include "core/graph/generators.hpp"
#include "orthogonal/orthogonal_algorithms.hpp"
#include "core/graph/graphs_algorithms.hpp"

int main() {
    // SimpleGraph* graph = generate_connected_random_graph_degree_max_4(45, 60);
    // SimpleGraph* graph = generate_connected_random_graph_degree_max_4(55, 80);
    // SimpleGraph* graph = generate_connected_random_graph_degree_max_4(70, 110);
    // SimpleGraph* graph = generate_connected_random_graph_degree_max_4(150, 220);
    SimpleGraph* graph = loadSimpleUndirectedGraphFromFile("example-graphs/random_graph_augmented.txt");
    // SimpleGraph* graph = loadSimpleUndirectedGraphFromFile("example-graphs/prova2.txt");

    // auto start_inc_basis = std::chrono::high_resolution_clock::now();
    // auto result = make_rectilinear_drawing_incremental_basis(*graph);
    // auto end_inc_basis = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double> elapsed_inc_basis = end_inc_basis - start_inc_basis;
    // std::cout << "make_rectilinear_drawing_incremental_basis\ntime: " << elapsed_inc_basis.count() << " seconds\n";
    // save_undirected_graph_to_file(
    //     *std::get<0>(result),
    //     "example-graphs/prova2.txt"
    // );

    // const ColoredNodesGraph* augmented_graph = std::get<0>(result);
    // const Shape* shape = std::get<1>(result);
    // const SimpleGraph* embedding = compute_embedding_from_shape(*augmented_graph, *shape);
    // auto faces = compute_all_faces_of_embedding(*embedding);
    // int genus = compute_embedding_genus(
    //     embedding->size(),
    //     embedding->get_number_of_edges()/2,
    //     faces.size(),
    //     1
    // );
    // std::cout << "Genus: " << genus << std::endl;
    // auto start_inc_faces = std::chrono::high_resolution_clock::now();
    // auto result_faces = make_rectilinear_drawing_incremental(*augmented_graph, faces);
    // auto end_inc_faces = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double> elapsed_inc_faces = end_inc_faces - start_inc_faces;
    // std::cout << "make_rectilinear_drawing_incremental_faces\ntime: " << elapsed_inc_faces.count() << " seconds\n";
    
    // delete shape;
    // delete embedding;
    // delete std::get<0>(result_faces);
    // delete std::get<1>(result_faces);

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
    std::cout << "Cycles found: " << cycles.size() << "\n";

    auto start_inc_special = std::chrono::high_resolution_clock::now();
    auto result_special = make_rectilinear_drawing_incremental(*graph, cycles);
    auto end_inc_special = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_inc_special = end_inc_special - start_inc_special;
    std::cout << "make_rectilinear_drawing_incremental_special\ntime: " << elapsed_inc_special.count() << " seconds\n";

    // delete std::get<0>(result_special);
    // delete std::get<1>(result_special);

    delete graph;
    return 0;
}