#ifndef MY_GRAPHS_ALGORITHMS_H
#define MY_GRAPHS_ALGORITHMS_H

#include <vector>
#include <optional>

#include "graph.hpp"

template <GraphTrait T>
bool is_connected(const T& graph);

template <GraphTrait T>
bool is_graph_undirected(const T& graph);

template <GraphTrait T>
std::vector<std::vector<int>> compute_all_cycles_with_node_in_undirected_graph(
    const T& graph,
    int node_index,
    std::vector<bool>& taboo_nodes
);

template <GraphTrait T>
std::vector<std::vector<int>> compute_all_cycles_in_undirected_graph(const T& graph);

template <GraphTrait T>
std::optional<std::vector<int>> find_a_cycle_directed_graph(const T& graph);

template <GraphTrait T>
std::vector<std::vector<int>> compute_cycle_basis(const T& graph);

template <GraphTrait T>
std::vector<int> make_topological_ordering(const T& graph);

template <GraphTrait T>
bool is_edge_in_graph(const T& graph, int i, int j);

template <GraphTrait T>
std::vector<std::vector<int>> compute_all_faces_of_embedding(const T& embedding);

inline int compute_embedding_genus(
    int number_of_nodes, int number_of_edges, int number_of_faces, int connected_components
);

inline bool are_cycles_equivalent(
    const std::vector<int>& cycle1,
    const std::vector<int>& cycle2
);

template <GraphTrait T>
std::vector<LabeledNodeGraph<Int>> compute_biconnected_components(const T& graph);

#include "graphs_algorithms.ipp"

#endif