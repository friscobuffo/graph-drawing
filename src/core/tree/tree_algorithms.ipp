#include <list>
#include <cassert>

#include "tree_algorithms.hpp"

template <GraphTrait T>
SimpleTree* build_simple_spanning_tree(const T& graph) {
    SimpleTree* tree = new SimpleTree();
    for (int i = 0; i < graph.size(); ++i) tree->add_node();
    std::vector<bool> visited(graph.size(), false);
    visited[0] = true;
    std::list<int> queue;
    queue.push_back(0);
    while (!queue.empty()) {
        int node_index = queue.front();
        queue.pop_front();
        for (auto& edge : graph.get_nodes()[node_index].get_edges()) {
            int neighbor_index = edge.get_to();
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
std::vector<int> get_path_from_root(const T& tree, int node) {
    std::vector<int> path;
    while (node != -1) {
        path.push_back(node);
        node = tree.get_nodes()[node].get_parent();
    }
    std::reverse(path.begin(), path.end());
    return path;
}

template <TreeTrait T>
int compute_common_ancestor(const T& tree, int node1, int node2) {
    std::vector<int> path1 = get_path_from_root(tree, node1);
    std::vector<int> path2 = get_path_from_root(tree, node2);
    int i = 0;
    while (i < path1.size() && i < path2.size() && path1[i] == path2[i]) ++i;
    return path1[i-1];
}

template <TreeTrait T>
bool is_edge_in_tree(const T& tree, int node1, int node2) {
    const auto& node_1 = tree.get_nodes()[node1];
    const auto& node_2 = tree.get_nodes()[node2];
    return node_1.get_parent() == node2 || node_2.get_parent() == node1;
}