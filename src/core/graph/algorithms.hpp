#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <vector>
#include <functional>
#include <optional>

#include "graph.hpp"

template <GraphTrait T>
bool is_connected(const T& graph);

template <GraphTrait T>
std::vector<std::vector<size_t>> compute_all_cycles_with_node(
    const T& graph,
    size_t node_index,
    std::vector<bool>& taboo_nodes
);

template <GraphTrait T>
std::vector<std::vector<size_t>> compute_all_cycles(const T& graph);

template <GraphTrait T>
std::optional<std::vector<size_t>> find_a_cycle(const T& graph);

template <GraphTrait T>
std::vector<std::vector<size_t>> compute_cycle_basis(const T& graph);

#include "algorithms.ipp"

#endif