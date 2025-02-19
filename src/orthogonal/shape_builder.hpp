#ifndef MY_SHAPE_BUILDER_H
#define MY_SHAPE_BUILDER_H

#include <vector>
#include <cassert>

#include "../core/graph/graph.hpp"
#include "shape.hpp"

const Shape* build_shape(ColoredNodesGraph& graph, std::vector<std::vector<size_t>>& cycles);

#endif