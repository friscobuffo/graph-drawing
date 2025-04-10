#ifndef SHAPE_ALGORITHMS_H
#define SHAPE_ALGORITHMS_H

#include "../core/graph/graph.hpp"
#include "shape.hpp"

template <GraphTrait T>
const SimpleGraph* compute_embedding_from_shape(const T& graph, const Shape& shape);

#include "orthogonal_algorithms.ipp"

#endif