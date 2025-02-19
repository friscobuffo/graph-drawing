#ifndef MY_SHAPE_BUILDER_H
#define MY_SHAPE_BUILDER_H

#include <vector>
#include <cassert>

#include "../core/graph/graph.hpp"
#include "shape.hpp"

const Shape* _build_shape(ColoredNodesGraph& colored_graph, std::vector<std::vector<size_t>>& cycles);

template <GraphTrait T>
const Shape* build_shape(const T& graph, std::vector<std::vector<size_t>>& cycles) {
    ColoredNodesGraph colored_graph{};
    for (int i = 0; i < graph.size(); i++)
        colored_graph.add_node(Color::BLACK);
    for (int i = 0; i < graph.size(); i++) {
        assert(graph.get_nodes()[i].get_edges().size() <= 4);
        for (auto& edge : graph.get_nodes()[i].get_edges()) {
            int j = edge.get_to();
            colored_graph.add_edge(i, j);
        }
    }
    return _build_shape(colored_graph, cycles);
}

#endif