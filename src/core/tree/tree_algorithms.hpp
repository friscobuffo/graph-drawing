#ifndef MY_TREE_ALGORITHMS_H
#define MY_TREE_ALGORITHMS_H

#include <vector>

#include "../graph/graph.hpp"
#include "tree.hpp"

template <GraphTrait T>
SimpleTree* build_simple_spanning_tree(const T& graph);

template <TreeTrait T>
std::vector<int> get_path_from_root(const T& tree, int node);

template <TreeTrait T>
int compute_common_ancestor(const T& tree, int node1, int node2);

template <TreeTrait T>
bool is_edge_in_tree(const T& tree, int node1, int node2);

#include "tree_algorithms.ipp"

#endif