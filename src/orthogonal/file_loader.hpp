#ifndef MY_SHAPE_FILE_LOADER_H
#define MY_SHAPE_FILE_LOADER_H

#include <string>
#include <memory>

#include "shape.hpp"
#include "../core/graph/graph.hpp"

std::unique_ptr<Shape> load_shape_from_file(const std::string& filename);

void save_shape_to_file(const Graph& graph, const Shape& shape, std::string filename);

#endif