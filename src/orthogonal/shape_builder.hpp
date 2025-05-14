#ifndef MY_SHAPE_BUILDER_H
#define MY_SHAPE_BUILDER_H

#include <vector>
#include <cassert>
#include <utility>

#include "../core/graph/graph.hpp"
#include "shape.hpp"

std::unique_ptr<Shape> build_shape(
    Graph& graph, 
    GraphAttributes& attributes,
    std::vector<std::vector<int>>& cycles
);

#endif