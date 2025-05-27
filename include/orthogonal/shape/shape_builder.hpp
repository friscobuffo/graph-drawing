#ifndef MY_SHAPE_BUILDER_H
#define MY_SHAPE_BUILDER_H

#include <vector>

#include "core/graph/graph.hpp"
#include "orthogonal/shape/shape.hpp"

Shape build_shape(
    Graph& graph, 
    GraphAttributes& attributes,
    std::vector<std::vector<int>>& cycles,
    bool randomize = false
);

Shape build_shape_special(
    Graph& graph, 
    GraphAttributes& attributes,
    std::vector<std::vector<int>>& cycles,
    bool randomize = false
);

#endif