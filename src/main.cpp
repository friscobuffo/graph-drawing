#include <iostream>

#include "core/graph/graph.hpp"
#include "orthogonal/drawing_builder.hpp"
#include "core/graph/file_loader.hpp"
#include "core/graph/generators.hpp"
#include "baseline-ogdf/drawer.hpp"


int main() {
    const std::string filename = "drawing.svg";
    int test_number = 0;
    while (true) {
        std::cout << "\rtest number: " << ++test_number << std::flush;
        auto graph = load_simple_undirected_graph_from_txt_file("generated-graphs/graph_92_n32_m58.txt");
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
        create_drawing(*graph, "output.svg");
        break;
    }
    return 0;
}
