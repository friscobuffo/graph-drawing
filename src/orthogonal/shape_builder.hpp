#ifndef MY_SHAPE_BUILDER_H
#define MY_SHAPE_BUILDER_H

#include <vector>
#include <tuple>
#include <memory>

#include "../core/graph/graph.hpp"
#include "shape.hpp"
#include "../sat/cnf_builder.hpp"

const Shape* _build_shape(ColoredNodesGraph& colored_graph, std::vector<std::vector<size_t>>& cycles);

template <GraphTrait T>
const Shape* build_shape(const T& graph, std::vector<std::vector<size_t>>& cycles) {
    ColoredNodesGraph colored_graph{};
    for (int i = 0; i < graph.size(); i++)
        colored_graph.addNode(Color::BLACK);
    for (int i = 0; i < graph.size(); i++)
        for (auto& edge : graph.getNodes()[i].getEdges()) {
            int j = edge.getTo();
            colored_graph.addEdge(i, j);
        }
    return _build_shape(colored_graph, cycles);
}

#endif