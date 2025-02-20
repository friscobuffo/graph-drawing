#include "tree.hpp"
#include "../graph/graph.hpp"

#include <list>
#include <cassert>

template <GraphTrait T>
SimpleTree* build_simple_spanning_tree(const T& graph) {
    SimpleTree* tree = new SimpleTree();
    for (int i = 0; i < graph.size(); ++i) tree->add_node();
    std::vector<bool> visited(graph.size(), false);
    visited[0] = true;
    std::list<size_t> queue;
    queue.push_back(0);
    while (!queue.empty()) {
        size_t node_index = queue.front();
        queue.pop_front();
        for (auto& edge : graph.get_nodes()[node_index].get_edges()) {
            size_t neighbor_index = edge.get_to();
            if (!visited[neighbor_index]) {
                visited[neighbor_index] = true;
                tree->add_child(node_index, neighbor_index);
                queue.push_back(neighbor_index);
            }
        }
    }
    for (int i = 0; i < graph.size(); ++i)
        assert(visited[i]);
    return tree;
}

template <TreeTrait T>
std::vector<size_t> get_path_from_root(const T& tree, size_t node) {
    std::vector<size_t> path;
    while (node != -1) {
        path.push_back(node);
        node = tree.get_nodes()[node].get_parent();
    }
    std::reverse(path.begin(), path.end());
    return path;
}

template <TreeTrait T>
size_t compute_common_ancestor(const T& tree, size_t node1, size_t node2) {
    std::vector<size_t> path1 = get_path_from_root(tree, node1);
    std::vector<size_t> path2 = get_path_from_root(tree, node2);
    size_t i = 0;
    while (i < path1.size() && i < path2.size() && path1[i] == path2[i]) ++i;
    return path1[i-1];
}

template <TreeTrait T>
bool is_edge_in_tree(const T& tree, size_t node1, size_t node2) {
    const auto& node_1 = tree.get_nodes()[node1];
    const auto& node_2 = tree.get_nodes()[node2];
    return node_1.get_parent() == node2 || node_2.get_parent() == node1;
}