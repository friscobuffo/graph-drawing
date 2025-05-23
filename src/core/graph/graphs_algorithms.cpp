#include <algorithm>
#include <queue>
#include <functional>
#include <limits>
#include <utility>
#include <unordered_set>

#include "graphs_algorithms.hpp"
#include "../tree/tree.hpp"
#include "../tree/tree_algorithms.hpp"

bool is_graph_connected(const Graph& graph) {
    if (graph.size() == 0) return true;
    std::unordered_set<int> visited;
    std::vector<const GraphNode*> stack;
    stack.push_back(&(*graph.get_nodes().begin()));
    while (!stack.empty()) {
        auto* node = stack.back();
        stack.pop_back();
        visited.insert(node->get_id());
        for (auto& edge : node->get_edges())
            if (!visited.contains(edge.get_to().get_id()))
                stack.push_back(&edge.get_to());
    }
    for (auto& node : graph.get_nodes())
        if (!visited.contains(node.get_id())) return false;
    return true;
}

bool is_graph_undirected(const Graph& graph) {
    for (const auto& node : graph.get_nodes())
        for (const auto& edge : node.get_edges())
            if (!graph.has_edge(edge.get_to().get_id(), node.get_id()))
                return false;
    return true;
}

std::vector<std::vector<int>> compute_all_cycles_in_undirected_graph(const Graph& graph) {
    std::vector<std::vector<int>> all_cycles;
    std::unordered_set<int> taboo_nodes;
    for (const auto& node : graph.get_nodes()) {
        std::vector<std::vector<int>> cycles = compute_all_cycles_with_node_in_undirected_graph(graph, node, taboo_nodes);
        for (auto& cycle : cycles)
            all_cycles.push_back(cycle);
        taboo_nodes.insert(node.get_id());
    }
    return all_cycles;
}

std::vector<std::vector<int>> compute_all_cycles_with_node_in_undirected_graph(
    const Graph& graph, const GraphNode& node, std::unordered_set<int>& taboo_nodes
) {
    std::vector<std::vector<int>> cycles;
    std::vector<int> path;
    std::unordered_set<int> visited;
    std::function<void(const GraphNode&, const GraphNode&)> dfs = [&](const GraphNode& current, const GraphNode& start) {
        if (taboo_nodes.contains(current.get_id())) return;
        if (visited.contains(current.get_id())) {
            if (current.get_id() == start.get_id() && path.size() > 2)
                cycles.push_back(path);
            return;
        }
        visited.insert(current.get_id());
        path.push_back(current.get_id());
        for (auto& edge : current.get_edges()) {
            const auto& neighbor = edge.get_to();
            if (path.size() > 2 && &neighbor == &start)
                cycles.push_back(path);
            else
                dfs(neighbor, start);
        }
        visited.insert(current.get_id());
        path.pop_back();
    };
    dfs(node, node);
    return cycles;
}

bool dfs_find_cycle(
    const GraphNode& node, const Graph& graph, std::unordered_map<int,int>& state,
    std::unordered_map<int,int>& parent, int& cycle_start, int& cycle_end
) {
    state[node.get_id()] = 1; // mark as visiting (gray)
    for (const auto& edge : node.get_edges()) {
        const auto& neighbor = edge.get_to();
        if (!state.contains(neighbor.get_id())) { // unvisited
            parent[neighbor.get_id()] = node.get_id();
            if (dfs_find_cycle(neighbor, graph, state, parent, cycle_start, cycle_end))
                return true;
        } else if (state[neighbor.get_id()] == 1) {
            cycle_start = neighbor.get_id();
            cycle_end = node.get_id();
            return true;
        }
    }
    state[node.get_id()] = 2; // mark as fully processed (black)
    return false;
}

std::optional<std::vector<int>> find_a_cycle_directed_graph(const Graph& graph) {
    std::unordered_map<int,int> state;
    std::unordered_map<int,int> parent;
    int cycle_start = -1, cycle_end = -1;
    for (const auto& node : graph.get_nodes())
        if (!state.contains(node.get_id()))
            if (dfs_find_cycle(node, graph, state, parent, cycle_start, cycle_end))
                break;
    if (cycle_start == -1) return {};
    std::vector<int> cycle;
    for (int v = cycle_end; v != cycle_start; v = parent[v])
        cycle.push_back(v);
    cycle.push_back(cycle_start);
    std::reverse(cycle.begin(), cycle.end());
    return cycle;
}

std::vector<std::vector<int>> compute_cycle_basis(const Graph& graph) {
    auto spanning = build_spanning_tree(graph);
    std::vector<std::vector<int>> cycles;
    for (auto& node : graph.get_nodes()) {
        int node_id = node.get_id();
        for (auto& edge : node.get_edges()) {
            int neighbor_id = edge.get_to().get_id();
            if (node_id > neighbor_id) continue;
            if (spanning->has_edge(node_id, neighbor_id)) continue;
            int common_ancestor = compute_common_ancestor(*spanning, node_id, neighbor_id);
            std::vector<int> path1 = get_path_from_root(*spanning, node_id);
            std::vector<int> path2 = get_path_from_root(*spanning, neighbor_id);
            std::reverse(path1.begin(), path1.end());
            std::reverse(path2.begin(), path2.end());
            while (path1.back() != common_ancestor)
                path1.pop_back();
            while (path2.back() != common_ancestor)
                path2.pop_back();
            std::reverse(path1.begin(), path1.end());
            path1.insert(path1.end(), path2.begin(), path2.end());
            path1.pop_back();
            cycles.push_back(path1);
        }
    }
    return cycles;
}

std::vector<int> make_topological_ordering(const Graph& graph) {
    std::unordered_map<int,int> in_degree;
    for (const auto& node : graph.get_nodes()) {
        for (const auto& edge : node.get_edges()) {
            const auto& neighbor = edge.get_to();
            if (!in_degree.contains(neighbor.get_id()))
                in_degree[neighbor.get_id()] = 0;
            in_degree[neighbor.get_id()]++;
        }
    }
    std::queue<const GraphNode*> queue;
    std::vector<int> topological_order;
    for (const auto& node : graph.get_nodes())
        if (in_degree[node.get_id()] == 0)
            queue.push(&node);
    int count = 0;
    while (!queue.empty()) {
        const auto node = queue.front();
        ++count;
        queue.pop();
        topological_order.push_back(node->get_id());
        for (const auto& edge : node->get_edges()) {
            const auto& neighbor = edge.get_to();
            if (--in_degree[neighbor.get_id()] == 0)
                queue.push(&neighbor);
        }
    }
    if (count != graph.size())
        throw std::runtime_error("Graph contains cycle");
    return topological_order;
}

int compute_embedding_genus(
    int number_of_nodes, int number_of_edges, int number_of_faces, int connected_components
) {
    // f - e + v = 2(p - g)
    // f - e + v = 2p - 2g
    // 2g = 2p - f + e - v
    // g = p - (f - e + v) / 2
    return connected_components - (number_of_faces - number_of_edges + number_of_nodes)/2;
}

bool are_cycles_equivalent(
    const std::vector<int>& cycle1,
    const std::vector<int>& cycle2
) {
    if (cycle1.size() != cycle2.size()) return false;
    int v = cycle1[0];
    int v_pos_in_cycle2 = -1;
    for (int i = 0; i < cycle2.size(); ++i) {
        if (cycle2[i] == v) {
            v_pos_in_cycle2 = i;
            break;
        }
    }
    if (v_pos_in_cycle2 == -1) return false;
    bool are_the_same = true;
    for (int i = 0; i < cycle1.size(); ++i) {
        int j = (v_pos_in_cycle2 + i) % cycle2.size();
        if (cycle1[i] != cycle2[j]) {
            are_the_same = false;
            break;
        }
    }
    if (are_the_same) return true;
    // Check the reverse
    for (int i = 0; i < cycle1.size(); ++i) {
        int j = (v_pos_in_cycle2 - i + cycle2.size()) % cycle2.size();
        if (cycle1[i] != cycle2[j]) {
            return false;
        }
    }
    return true;
}

std::vector<std::unique_ptr<Graph>> compute_connected_components(const Graph& graph) {
    if (!is_graph_undirected(graph))
        throw std::runtime_error("Graph is not undirected");
    std::unordered_set<int> visited;
    std::vector<std::unique_ptr<Graph>> components;
    std::function<void(const GraphNode&, Graph& component)> explore_component = [&](
        const GraphNode& node, Graph& component) {
        visited.insert(node.get_id());
        for (const auto &edge : node.get_edges())
        {
            const auto &neighbor = edge.get_to();
            if (!component.has_node(neighbor.get_id()))
                component.add_node(neighbor.get_id());
            if (!component.has_edge(node.get_id(), neighbor.get_id()))
                component.add_undirected_edge(node.get_id(), neighbor.get_id());
            if (!visited.contains(neighbor.get_id()))
            {
                explore_component(neighbor, component);
            }
        }
    };
    for (const auto& node : graph.get_nodes())
        if (!visited.contains(node.get_id())) {
            auto new_component = std::make_unique<Graph>();
            new_component->add_node(node.get_id());
            explore_component(node, *new_component);
            components.push_back(std::move(new_component));
        }
    return std::move(components);
}
