#ifndef MY_SHAPE_BUILDER_H
#define MY_SHAPE_BUILDER_H

#include <vector>
#include <cassert>
#include <utility>

#include "../core/graph/graph.hpp"
#include "shape.hpp"

Shape* build_shape(ColoredNodesGraph& graph, std::vector<std::vector<int>>& cycles);

#endif