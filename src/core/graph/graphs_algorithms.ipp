#include <algorithm>
#include <queue>
#include <functional>
#include <limits>
#include <utility>

#include "graphs_algorithms.hpp"
#include "../tree/tree.hpp"
#include "../tree/tree_algorithms.hpp"

template <GraphTrait T>
bool is_connected(const T& graph) {
    if (graph.size() == 0) return true;
    std::vector<bool> visited(graph.size(), false);
    std::vector<int> stack;
    stack.push_back(0);
    while (!stack.empty()) {
        int node_index = stack.back();
        stack.pop_back();
        visited[node_index] = true;
        for (auto& edge : graph.get_node(node_index).get_edges())
            if (!visited[edge.get_to()])
                stack.push_back(edge.get_to());
    }
    for (bool visit : visited) if (!visit) return false;
    return true;
}

template <GraphTrait T>
std::vector<std::vector<int>> compute_all_cycles_in_undirected_graph(const T& graph) {
    std::vector<std::vector<int>> all_cycles;
    std::vector<bool> taboo_nodes(graph.size(), false);
    for (int i = 0; i < graph.size(); ++i) {
        std::vector<std::vector<int>> cycles = compute_all_cycles_with_node_in_undirected_graph(graph, i, taboo_nodes);
        for (auto& cycle : cycles)
            all_cycles.push_back(cycle);
        taboo_nodes[i] = true;
    }
    return all_cycles;
}

template <GraphTrait T>
std::vector<std::vector<int>> compute_all_cycles_with_node_in_undirected_graph(const T& graph, int node_index, std::vector<bool>& taboo_nodes) {
    std::vector<std::vector<int>> cycles;
    std::vector<int> path;
    std::vector<bool> visited(graph.size(), false);
    std::function<void(int, int)> dfs = [&](int current, int start) {
        if (taboo_nodes[current]) return;
        if (visited[current]) {
            if (current == start && path.size() > 2)
                cycles.push_back(path);
            return;
        }
        visited[current] = true;
        path.push_back(current);
        for (auto& edge : graph.get_node(current).get_edges()) {
            int neighbor = edge.get_to();
            if (path.size() > 2 && neighbor == start)
                cycles.push_back(path);
            else
                dfs(neighbor, start);
        }
        visited[current] = false;
        path.pop_back();
    };
    dfs(node_index, node_index);
    return cycles;
}

template <GraphTrait T>
bool dfs_find_cycle(int u, const T& graph, std::vector<int>& state, std::vector<int>& parent, int& cycle_start, int& cycle_end) {
    state[u] = 1; // mark as visiting (gray)
    for (auto& edge : graph.get_node(u).get_edges()) {
        int v = edge.get_to();
        if (state[v] == 0) { // unvisited
            parent[v] = u;
            if (dfs_find_cycle(v, graph, state, parent, cycle_start, cycle_end))
                return true;
        } else if (state[v] == 1) {
            cycle_start = v;
            cycle_end = u;
            return true;
        }
    }
    state[u] = 2; // mark as fully processed (black)
    return false;
}

template <GraphTrait T>
std::optional<std::vector<int>> find_a_cycle_directed_graph(const T& graph) {
    std::vector<int> state(graph.size(), 0);
    std::vector<int> parent(graph.size(), -1);
    int cycle_start = -1, cycle_end = -1;
    for (int i = 0; i < graph.size(); ++i)
        if (state[i] == 0)
            if (dfs_find_cycle(i, graph, state, parent, cycle_start, cycle_end))
                break;
    if (cycle_start == -1) return {};
    std::vector<int> cycle;
    for (int v = cycle_end; v != cycle_start; v = parent[v])
        cycle.push_back(v);
    cycle.push_back(cycle_start);
    std::reverse(cycle.begin(), cycle.end());
    return cycle;
}

template <GraphTrait T>
std::vector<std::vector<int>> compute_cycle_basis(const T& graph) {
    SimpleTree* spanning = build_simple_spanning_tree(graph);
    std::vector<std::vector<int>> cycles;
    for (int i = 0; i < graph.size(); ++i) {
        for (auto& edge : graph.get_node(i).get_edges()) {
            int j = edge.get_to();
            if (i > j) continue;
            if (is_edge_in_tree(*spanning, i, j)) continue;
            int common_ancestor = compute_common_ancestor(*spanning, i, j);
            std::vector<int> path1 = get_path_from_root(*spanning, i);
            std::vector<int> path2 = get_path_from_root(*spanning, j);
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
    delete spanning;
    return cycles;
}

template <GraphTrait T>
std::vector<int> make_topological_ordering(const T& graph) {
    std::vector<int> in_degree(graph.size(), 0);
    for (int u = 0; u < graph.size(); ++u)
        for (auto& edge : graph.get_node(u).get_edges()) {
            int v = edge.get_to();
            in_degree[v]++;
        }
    std::queue<int> queue;
    std::vector<int> topological_order;
    for (int i = 0; i < graph.size(); ++i)
        if (in_degree[i] == 0)
            queue.push(i);
    int count = 0;
    while (!queue.empty()) {
        int u = queue.front();
        ++count;
        queue.pop();
        topological_order.push_back(u);
        for (auto& edge : graph.get_node(u).get_edges()) {
            int v = edge.get_to();
            if (--in_degree[v] == 0)
                queue.push(v);
        }
    }
    if (count != graph.size())
        throw std::runtime_error("Graph contains cycle");
    return topological_order;
}

template <GraphTrait T>
bool is_edge_in_graph(const T& graph, int i, int j) {
    for (int k = 0; k < graph.get_node(i).get_degree(); ++k)
        if (graph.get_node(i).get_edge(k).get_to() == j)
            return true;
    return false;
}

template <GraphTrait T>
std::vector<std::vector<int>> compute_all_faces_of_embedding(const T& embedding) {
    std::vector<std::vector<int>> faces;
    std::vector<std::vector<bool>> visited(
        embedding.size(), std::vector<bool>(embedding.size(), false)
    );
    for (int i = 0; i < embedding.size(); ++i) {
        for (const auto& edge : embedding.get_node(i).get_edges()) {
            int j = edge.get_to();
            if (visited[i][j]) continue;
            std::vector<int> face;
            while (!visited[i][j]) {
                face.push_back(j);
                visited[i][j] = true;
                int i_position = 0;
                for (const auto& edge2 : embedding.get_node(j).get_edges()) {
                    int k = edge2.get_to();
                    if (k == i) break;
                    ++i_position;
                }
                int next_edge_position = (i_position + 1) % embedding.get_node(j).get_degree();
                const auto& next_edge = embedding.get_node(j).get_edge(next_edge_position);
                i = j;
                j = next_edge.get_to();
            }
            if (face.size() > 2)
                faces.push_back(face);
        }
    }
    return faces;
}

inline int compute_embedding_genus(
    int number_of_nodes, int number_of_edges, int number_of_faces, int connected_components
) {
    // f - e + v = 2(p - g)
    // f - e + v = 2p - 2g
    // 2g = 2p - f + e - v
    // g = p - (f - e + v) / 2
    return connected_components - (number_of_faces - number_of_edges + number_of_nodes)/2;
}

inline bool are_cycles_equivalent(
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