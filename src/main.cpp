#include <iostream>

#include "core/graph/graph.hpp"
#include "orthogonal/drawing_builder.hpp"
#include "core/graph/file_loader.hpp"
#include "core/graph/generators.hpp"
#include "config/config.hpp"
#include "baseline-ogdf/drawer.hpp"

int main() {
    auto config = parse_config("config.txt");
    const std::string filename = config["output_svg_shape_metrics"];
    auto graph = load_simple_undirected_graph_from_txt_file(config["input_graph_file"]);
    auto result = make_rectilinear_drawing_incremental_disjoint_paths<SimpleGraph>(*graph);
    node_positions_to_svg(
        *result.positions,
        *result.augmented_graph,
        filename
    );
    create_drawing(*graph, "output.svg");
    return 0;
}
