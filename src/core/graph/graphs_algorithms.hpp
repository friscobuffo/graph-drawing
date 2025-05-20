#ifndef MY_GRAPHS_ALGORITHMS_H
#define MY_GRAPHS_ALGORITHMS_H

#include <vector>
#include <optional>

#include "graph.hpp"

bool is_graph_connected(const Graph& graph);

bool is_graph_undirected(const Graph& graph);

std::vector<std::vector<int>> compute_all_cycles_with_node_in_undirected_graph(
    const Graph& graph,
    const GraphNode& node,
    std::unordered_set<int>& taboo_nodes
);

std::vector<std::vector<int>> compute_all_cycles_in_undirected_graph(const Graph& graph);

std::optional<std::vector<int>> find_a_cycle_directed_graph(const Graph& graph);

std::vector<std::vector<int>> compute_cycle_basis(const Graph& graph);

std::vector<int> make_topological_ordering(const Graph& graph);

int compute_embedding_genus(
    int number_of_nodes, int number_of_edges, int number_of_faces, int connected_components
);

bool are_cycles_equivalent(
    const std::vector<int>& cycle1,
    const std::vector<int>& cycle2
);

std::vector<std::unique_ptr<Graph>> compute_connected_components(const Graph& graph);

#endif