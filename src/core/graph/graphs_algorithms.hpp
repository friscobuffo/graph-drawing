#ifndef MY_GRAPHS_ALGORITHMS_H
#define MY_GRAPHS_ALGORITHMS_H

#include <vector>
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

template <GraphTrait T>
std::vector<std::vector<size_t>> compute_smallest_cycle_between_pair_nodes(const T& graph);

template <GraphTrait T>
std::vector<std::vector<size_t>> compute_all_faces_of_embedding(const T& embedding);

inline int compute_embedding_genus(
    int number_of_nodes, int number_of_edges, int number_of_faces, int connected_components
);

// The following template function finds disjoint paths (both edge‐ and vertex‑disjoint)
// between nodes s and t in an undirected graph. The function returns a vector of
// vectors (each a sequence of vertex indices representing a path).
template <typename GraphTrait>
std::vector<std::vector<size_t>> find_disjoint_paths(
    const GraphTrait& graph, size_t s, size_t t);

#include "graphs_algorithms.ipp"

#endif