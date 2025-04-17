#ifndef MY_GRAPH_GENERATORS_H
#define MY_GRAPH_GENERATORS_H

#include <memory>

#include "graph.hpp"

std::unique_ptr<SimpleGraph> generate_connected_random_graph_degree_max_4(size_t number_of_nodes, size_t number_of_edges);

// n*m grid, n, m > 1
std::unique_ptr<SimpleGraph> generate_grid_graph(size_t n, size_t m);

// num_nodes > 1
std::unique_ptr<SimpleGraph> generate_triangle_graph(size_t num_nodes);

#endif