#include "drawing_builder.hpp"

template <GraphTrait T>
DrawingResult make_rectilinear_drawing_incremental(
    const T& graph, std::vector<std::vector<int>>& cycles
) {
    ColoredNodesGraph* colored_graph = new ColoredNodesGraph{};
    for (int i = 0; i < graph.size(); i++)
        colored_graph->add_node(Color::BLACK);
    for (int i = 0; i < graph.size(); i++) {
        assert(graph.get_node(i).get_degree() <= 4);
        for (auto& edge : graph.get_node(i).get_edges()) {
            int j = edge.get_to();
            colored_graph->add_edge(i, j);
        }
    }
    Shape* shape = build_shape(*colored_graph, cycles);
    BuildingResult* result = build_nodes_positions(*shape, *colored_graph);
    int number_of_added_cycles = 0;
    while (result->type == BuildingResultType::CYCLES_TO_BE_ADDED) {
        for (auto& cycle_to_add : result->cycles_to_be_added)
            cycles.push_back(cycle_to_add);
        number_of_added_cycles += result->cycles_to_be_added.size();
        delete shape;
        delete result;
        shape = build_shape(*colored_graph, cycles);
        result = build_nodes_positions(*shape, *colored_graph);
    }
    const NodesPositions* positions = result->positions;
    ColoredNodesGraph* refined_colored_graph = new ColoredNodesGraph{};
    Shape* refined_shape = new Shape{};
    refine_result(*colored_graph, *shape, *refined_colored_graph, *refined_shape);
    int number_of_useless_bends = colored_graph->size() - refined_colored_graph->size();
    delete colored_graph;
    delete shape;
    delete positions;
    delete result;
    colored_graph = refined_colored_graph;
    shape = refined_shape;
    result = build_nodes_positions(*shape, *colored_graph);
    assert(result->type == BuildingResultType::OK);
    positions = result->positions;
    int number_of_corners = 0;

    auto* new_positions = compact_area_x(*colored_graph, *shape, *positions);
    delete positions;
    positions = new_positions;
    new_positions = compact_area_y(*colored_graph, *shape, *positions);
    delete positions;
    positions = new_positions;
    for (int i = graph.size(); i < colored_graph->size(); i++) {
        auto& node = colored_graph->get_node(i);
        assert(node.get_color() == Color::RED);
        number_of_corners++;
    }
    delete result;
    std::tuple<int, int, double> edge_length_metrics = compute_edge_length_metrics(*positions, *colored_graph);
    std::tuple<int, double> bends_metrics = compute_bends_metrics(*colored_graph);
    return {
        std::unique_ptr<const ColoredNodesGraph>(colored_graph),
        std::unique_ptr<const Shape>(shape),
        std::unique_ptr<const NodesPositions>(positions),
        compute_total_crossings(*positions, *colored_graph),
        number_of_corners,
        compute_total_area(*positions, *colored_graph),
        (int)cycles.size() - number_of_added_cycles,
        number_of_added_cycles,
        std::get<0>(edge_length_metrics),
        std::get<1>(edge_length_metrics),
        std::get<2>(edge_length_metrics),
        std::get<0>(bends_metrics),
        std::get<1>(bends_metrics),
        number_of_useless_bends,
    };
}
