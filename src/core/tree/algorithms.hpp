#ifndef MY_TREE_ALGORITHMS_H
#define MY_TREE_ALGORITHMS_H

#include <vector>

#include "../graph/graph.hpp"
#include "tree.hpp"

template <GraphTrait T>
SimpleTree* build_simple_spanning_tree(const T& graph);


template <TreeTrait T>
std::vector<size_t> get_path_from_root(const T& tree, size_t node);

template <TreeTrait T>
size_t compute_common_ancestor(const T& tree, size_t node1, size_t node2);

template <TreeTrait T>
bool is_edge_in_tree(const T& tree, size_t node1, size_t node2);

#include "algorithms.ipp"

#endif