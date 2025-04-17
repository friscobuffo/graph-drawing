#include <iostream>

#include "core/graph/graph.hpp"
#include "orthogonal/drawing_builder.hpp"
#include "core/graph/generators.hpp"

int main() {
    const std::string filename = "drawing.svg";
    int test_number = 0;
    while (true) {
        std::cout << "\rtest number: " << ++test_number << std::flush;
        auto graph = generate_connected_random_graph_degree_max_4(12, 16);
        auto result = make_rectilinear_drawing_incremental_disjoint_paths<SimpleGraph>(*graph);
        node_positions_to_svg(
            *result.positions,
            *result.augmented_graph,
            filename
        );
        if (result.number_of_added_cycles > 0) {
            std::cout << std::endl;
            graph->print();
            break;
        }
        break;
    }
    return 0;
}