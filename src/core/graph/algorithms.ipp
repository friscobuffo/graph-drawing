#include <algorithm>
#include <queue>

#include "../tree/tree.hpp"
#include "../tree/algorithms.hpp"

template <GraphTrait T>
bool is_connected(const T& graph) {
    if (graph.size() == 0) return true;
    std::vector<bool> visited(graph.size(), false);
    std::vector<size_t> stack;
    stack.push_back(0);
    while (!stack.empty()) {
        size_t nodeIndex = stack.back();
        stack.pop_back();
        visited[nodeIndex] = true;
        for (auto& edge : graph.get_nodes()[nodeIndex].get_edges())
            if (!visited[edge.get_to()])
                stack.push_back(edge.get_to());
    }
    for (bool visit : visited) if (!visit) return false;
    return true;
}

template <GraphTrait T>
std::vector<std::vector<size_t>> compute_all_cycles_in_undirected_graph(const T& graph) {
    std::vector<std::vector<size_t>> allCycles;
    std::vector<bool> tabooNodes(graph.size(), false);
    for (size_t i = 0; i < graph.size(); ++i) {
        std::vector<std::vector<size_t>> cycles = compute_all_cycles_with_node_in_undirected_graph(graph, i, tabooNodes);
        for (auto& cycle : cycles)
            allCycles.push_back(cycle);
        tabooNodes[i] = true;
    }
    return allCycles;
}

template <GraphTrait T>
std::vector<std::vector<size_t>> compute_all_cycles_with_node_in_undirected_graph(const T& graph, size_t node_index, std::vector<bool>& taboo_nodes) {
    std::vector<std::vector<size_t>> cycles;
    std::vector<size_t> path;
    std::vector<bool> visited(graph.size(), false);

    std::function<void(size_t, size_t)> dfs = [&](size_t current, size_t start) {
        if (taboo_nodes[current]) return;
        if (visited[current]) {
            if (current == start && path.size() > 2)
                cycles.push_back(path);
            return;
        }
        visited[current] = true;
        path.push_back(current);
        for (auto& edge : graph.get_nodes()[current].get_edges()) {
            size_t neighbor = edge.get_to();
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
    for (auto& edge : graph.get_nodes()[u].get_edges()) {
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
std::optional<std::vector<size_t>> find_a_cycle_directed_graph(const T& graph) {
    std::vector<int> state(graph.size(), 0);
    std::vector<int> parent(graph.size(), -1);
    int cycle_start = -1, cycle_end = -1;
    for (int i = 0; i < graph.size(); ++i)
        if (state[i] == 0)
            if (dfs_find_cycle(i, graph, state, parent, cycle_start, cycle_end))
                break;
    if (cycle_start == -1) return {};
    std::vector<size_t> cycle;
    for (int v = cycle_end; v != cycle_start; v = parent[v])
        cycle.push_back(v);
    cycle.push_back(cycle_start);
    std::reverse(cycle.begin(), cycle.end());
    return cycle;
}

template <GraphTrait T>
std::vector<std::vector<size_t>> compute_cycle_basis(const T& graph) {
    SimpleTree* spanning = build_simple_spanning_tree(graph);
    std::vector<std::vector<size_t>> cycles;
    for (int i = 0; i < graph.size(); ++i) {
        for (auto& edge : graph.get_nodes()[i].get_edges()) {
            int j = edge.get_to();
            if (i > j) continue;
            if (is_edge_in_tree(*spanning, i, j)) continue;
            size_t common_ancestor = compute_common_ancestor(*spanning, i, j);
            std::vector<size_t> path1 = get_path_from_root(*spanning, i);
            std::vector<size_t> path2 = get_path_from_root(*spanning, j);
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
std::vector<size_t> make_topological_ordering(const T& graph) {
    std::vector<int> in_degree(graph.size(), 0);
    for (int u = 0; u < graph.size(); ++u)
        for (auto& edge : graph.get_nodes()[u].get_edges()) {
            int v = edge.get_to();
            in_degree[v]++;
        }
    std::queue<int> queue;
    std::vector<size_t> topological_order;
    for (int i = 0; i < graph.size(); ++i)
        if (in_degree[i] == 0)
            queue.push(i);
    int count = 0;
    while (!queue.empty()) {
        int u = queue.front();
        ++count;
        queue.pop();
        topological_order.push_back(u);
        for (auto& edge : graph.get_nodes()[u].get_edges()) {
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
    for (int k = 0; k < graph.get_nodes()[i].get_edges().size(); ++k)
        if (graph.get_nodes()[i].get_edges()[k].get_to() == j)
            return true;
    return false;
}