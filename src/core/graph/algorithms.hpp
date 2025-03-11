#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <vector>
#include <functional>
#include <optional>

#include "graph.hpp"

template <GraphTrait T>
bool is_connected(const T& graph);

template <GraphTrait T>
std::vector<std::vector<size_t>> compute_all_cycles_with_node_in_undirected_graph(
    const T& graph,
    size_t node_index,
    std::vector<bool>& taboo_nodes
);

template <GraphTrait T>
std::vector<std::vector<size_t>> compute_all_cycles_in_undirected_graph(const T& graph);

template <GraphTrait T>
std::optional<std::vector<size_t>> find_a_cycle_directed_graph(const T& graph);

template <GraphTrait T>
std::vector<std::vector<size_t>> compute_cycle_basis(const T& graph);

template <GraphTrait T>
std::vector<size_t> make_topological_ordering(const T& graph);

template <GraphTrait T>
bool is_edge_in_graph(const T& graph, int i, int j);

template <GraphTrait T>
std::vector<std::vector<size_t>> compute_cycles_in_undirected_graph_triplets(const T& graph);

#include "algorithms.ipp"

#endif