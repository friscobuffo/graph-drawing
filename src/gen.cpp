#include <iostream>
#include <filesystem>

#include "core/graph/graph.hpp"
#include "core/graph/file_loader.hpp"
#include "core/graph/generators.hpp"
#include "config/config.hpp"

int main() {
    auto config = parse_config("gen_config.txt");
    std::string generated_graphs_folder = config["generated_graphs_folder"];
    std::filesystem::create_directories(generated_graphs_folder);
    for (const auto& entry : std::filesystem::directory_iterator(generated_graphs_folder))
        std::filesystem::remove_all(entry.path());

    const double min_density = 1.1;
    const double max_density = 1.9;
    for (int number_of_nodes = 10; number_of_nodes <= 100; ++number_of_nodes) {
        std::string sub_folder = generated_graphs_folder + std::to_string(number_of_nodes) + "/";
        std::filesystem::create_directories(sub_folder);
        for (int i = 1; i <= 100; ++i) {
            double density = min_density + (max_density - min_density) * i / 100;
            size_t number_of_edges = static_cast<size_t>(density * number_of_nodes);
            auto graph = generate_connected_random_graph_degree_max_4_uniform(number_of_nodes, number_of_edges);
            std::string filename =
                sub_folder + "graph_" + std::to_string(i) + "_n" +
                std::to_string(number_of_nodes) + "_m" + 
                std::to_string(number_of_edges) + ".txt";
            save_undirected_graph_to_file(*graph, filename);
        }
    }

    return 0;
}