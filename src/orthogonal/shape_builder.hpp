#ifndef MY_SHAPE_BUILDER_H
#define MY_SHAPE_BUILDER_H

#include <vector>
#include <cassert>
#include <utility>

#include "../core/graph/graph.hpp"
#include "shape.hpp"

std::pair<const Shape*,int> build_shape(ColoredNodesGraph& graph, std::vector<std::vector<size_t>>& cycles);

#endif