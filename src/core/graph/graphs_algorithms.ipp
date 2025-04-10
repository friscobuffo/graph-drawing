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
    std::vector<size_t> stack;
    stack.push_back(0);
    while (!stack.empty()) {
        size_t node_index = stack.back();
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
        for (auto& edge : graph.get_node(current).get_edges()) {
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
        for (auto& edge : graph.get_node(i).get_edges()) {
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
        for (auto& edge : graph.get_node(u).get_edges()) {
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
std::vector<size_t> cycle_with_triplet_bfs(const T& graph, size_t i, size_t j, size_t k) {
    std::vector<size_t> cycle;
    std::vector<int> prev(graph.size(), -1);
    prev[j] = i;
    prev[k] = j;
    std::queue<size_t> queue;
    queue.push(k);
    bool found_prev = false;
    while (!queue.empty()) {
        size_t node_index = queue.front();
        queue.pop();
        for (auto& edge : graph.get_node(node_index).get_edges()) {
            size_t neighbor = edge.get_to();
            if (neighbor == i) {
                found_prev = true;
                prev[i] = node_index;
                break;
            }
            if (prev[neighbor] != -1) continue;
            prev[neighbor] = node_index;
            queue.push(neighbor);
        }
        if (found_prev) break;
    }
    if (!found_prev) return {}; // No cycle found
    size_t v = i;
    while (true) {
        cycle.push_back(v);
        if (v == j) break; // Completed the cycle
        v = prev[v];
    }
    std::reverse(cycle.begin(), cycle.end());
    return cycle;
}

template <GraphTrait T>
std::vector<std::vector<size_t>> compute_cycles_in_undirected_graph_triplets(const T& graph) {
    bool triplets[graph.size()][graph.size()][graph.size()];
    for (int i = 0; i < graph.size(); ++i)
        for (int j = 0; j < graph.size(); ++j)
            for (int k = 0; k < graph.size(); ++k)
                triplets[i][j][k] = false;
    for (int i = 0; i < graph.size(); ++i) {
        if (graph.get_node(i).get_degree() < 2) continue;
        int number_of_edges = graph.get_node(i).get_degree();
        for (int j = 0; j < number_of_edges-1; ++j) {
            int u = graph.get_node(i).get_edge(j).get_to();
            for (int k = j + 1; k < number_of_edges; ++k) {
                int v = graph.get_node(i).get_edge(k).get_to();
                triplets[u][i][v] = true;
                triplets[v][i][u] = true;
            }
        }
    }
    std::vector<std::vector<size_t>> cycles;
    for (int i = 0; i < graph.size(); ++i)
        for (int j = 0; j < graph.size(); ++j)
            for (int k = 0; k < graph.size(); ++k) {
                if (i < k) continue;
                if (triplets[i][j][k]) {
                    std::vector<size_t> cycle = cycle_with_triplet_bfs(graph, i, j, k);
                    // triplets[i][j][k] = false;
                    // triplets[k][j][i] = false;
                    // for (int i_ = 0; i_ < cycle.size(); ++i_) {
                    //     int j_ = (i_ + 1) % cycle.size();
                    //     int k_ = (i_ + 2) % cycle.size();
                    //     triplets[cycle[i_]][cycle[j_]][cycle[k_]] = false;
                    //     triplets[cycle[k_]][cycle[j_]][cycle[i_]] = false;
                    // }
                    if (!cycle.empty())
                        cycles.push_back(cycle);
                }
            }
    return cycles;
}


template <GraphTrait T>
std::vector<std::vector<size_t>> compute_smallest_cycle_between_pair_nodes(const T& graph) {
    bool is_pair_added[graph.size()][graph.size()];
    for (int i = 0; i < graph.size(); ++i)
        for (int j = 0; j < graph.size(); ++j)
            is_pair_added[i][j] = false;
    auto all_cycles = compute_all_cycles_in_undirected_graph(graph);
    // sort cycles by length
    std::sort(all_cycles.begin(), all_cycles.end(), [](const std::vector<size_t>& a, const std::vector<size_t>& b) {
        return a.size() < b.size();
    });
    std::vector<std::vector<size_t>> cycles;
    for (auto& cycle : all_cycles) {
        bool found_new_pair = false;
        for (int i = 0; i < cycle.size(); ++i) {
            int j = (i + 1) % cycle.size();
            if (!is_pair_added[cycle[i]][cycle[j]]) {
                found_new_pair = true;
                is_pair_added[cycle[i]][cycle[j]] = true;
                is_pair_added[cycle[j]][cycle[i]] = true;
            }
        }
        if (found_new_pair)
            cycles.push_back(cycle);
    }
    return cycles;
}

template <GraphTrait T>
std::vector<std::vector<size_t>> compute_all_faces_of_embedding(const T& embedding) {
    std::vector<std::vector<size_t>> faces;
    std::vector<std::vector<bool>> visited(
        embedding.size(), std::vector<bool>(embedding.size(), false)
    );
    for (size_t i = 0; i < embedding.size(); ++i) {
        for (const auto& edge : embedding.get_node(i).get_edges()) {
            size_t j = edge.get_to();
            if (visited[i][j]) continue;
            std::vector<size_t> face;
            while (!visited[i][j]) {
                face.push_back(j);
                visited[i][j] = true;
                int i_position = 0;
                for (const auto& edge2 : embedding.get_node(j).get_edges()) {
                    size_t k = edge2.get_to();
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

template <typename GraphTrait>
std::vector<std::vector<size_t>> find_disjoint_paths(
    const GraphTrait& graph, const int source, const int target
) {
    const int n = graph.size();
    // In our flow network, we “split” each vertex into two:
    // - v_in is represented as index v
    // - v_out is represented as index v+n
    // The total number of nodes in the flow network is 2*n.
    const int N = 2 * n;
    const int INF = 1000; // A large capacity for the source/target splitting edge.
    
    struct FlowEdge {
        int capacity, flow;
        int rev; // index of the reverse edge in the adjacent list of 'to'
        std::string to_string() const {
            return "capacity: " + std::to_string(capacity) +
                   ", flow: " + std::to_string(flow);
        }
        void print() const {
            std::cout << to_string() << std::endl;
        }
    };
    
    // Build the flow network.
    LabeledEdgeGraph<FlowEdge> flow_graph;
    for (int i = 0; i < N; ++i) flow_graph.add_node();
    
    // For every vertex v, add an edge from v_in (v) to v_out (v+n).
    // For vertices other than s and t we use capacity 1; for s and t we use INF.
    for (int v = 0; v < n; v++) {
        int cap = (v == source || v == target) ? INF : 1;
        int degree_1 = flow_graph.get_node(v).get_degree();
        int degree_2 = flow_graph.get_node(v+n).get_degree();
        flow_graph.add_edge(v, v+n, FlowEdge{cap, 0, degree_2});
        flow_graph.add_edge(v+n, v, FlowEdge{0,   0, degree_1});
    }
    
    // Now add edges corresponding to the undirected edges of the original graph.
    // Since each edge is represented twice, we add an edge only once (e.g. when u < v).
    for (int u = 0; u < n; u++) {
        for (const auto& edge : graph.get_node(u).get_edges()) {
            int v = edge.get_to();
            if (u < v) {
                // In the flow network, add an edge from u_out (u+n) to v_in (v)
                // and an edge from v_out (v+n) to u_in (u) with capacity 1.
                int degree_1 = flow_graph.get_node(v).get_degree();
                int degree_2 = flow_graph.get_node(u+n).get_degree();

                flow_graph.add_edge(u+n, v, FlowEdge{1, 0, degree_1});
                flow_graph.add_edge(v, u+n, FlowEdge{0, 0, degree_2});

                int degree_3 = flow_graph.get_node(v+n).get_degree();
                int degree_4 = flow_graph.get_node(u).get_degree();

                flow_graph.add_edge(v+n, u, FlowEdge{1, 0, degree_4});
                flow_graph.add_edge(u, v+n, FlowEdge{0, 0, degree_3});
            }
        }
    }
    
    // For the flow network, choose:
    int source_flow = source + n; // s_out
    int sink_flow   = target;     // t_in
    
    // Now run Edmonds–Karp (BFS-based max flow) to compute the maximum flow.
    int max_flow = 0;
    while (true) {
        std::vector<int> parent(N, -1);
        std::vector<int> parent_edge(N, -1);
        std::queue<int> q;
        q.push(source_flow);
        std::vector<bool> visited(N, false);
        visited[source_flow] = true;
        while (!q.empty() && !visited[sink_flow]) {
            int u = q.front();
            q.pop();
            for (int i = 0; i < flow_graph.get_node(u).get_degree(); i++) {
                auto& edge = flow_graph.get_node(u).get_edges()[i];
                FlowEdge& label = edge.get_label();
                if (!visited[edge.get_to()] && label.capacity - label.flow > 0) {
                    visited[edge.get_to()] = true;
                    parent[edge.get_to()] = u;
                    parent_edge[edge.get_to()] = i;
                    q.push(edge.get_to());
                }
            }
        }
        if (!visited[sink_flow])
            break; // No more augmenting paths.
        
        int flow = INF;
        int cur = sink_flow;
        while (cur != source_flow) {
            int p = parent[cur];
            int edge_idx = parent_edge[cur];
            int cap = flow_graph.get_node(p).get_edges()[edge_idx].get_label().capacity;
            int f = flow_graph.get_node(p).get_edges()[edge_idx].get_label().flow;
            flow = std::min(flow, cap - f);
            cur = p;
        }
        
        cur = sink_flow;
        while (cur != source_flow) {
            int p = parent[cur];
            int edge_idx = parent_edge[cur];
            flow_graph.get_node(p).get_edges()[edge_idx].get_label().flow += flow;
            int rev_idx = flow_graph.get_node(p).get_edges()[edge_idx].get_label().rev;
            flow_graph.get_node(cur).get_edges()[rev_idx].get_label().flow -= flow;
            cur = p;
        }
        
        max_flow += flow;
    }
    // If we did not get at least two units of flow, then two disjoint paths do not exist.
    if (max_flow < 1)
        return {};
    
    // To extract the two disjoint paths, we “peel off” paths carrying flow.
    // We follow edges with positive flow from source_flow to sink_flow and subtract the used flow.
    auto extract_flow_path = [&](int start, int end) -> std::vector<int> {
        std::vector<int> path;
        std::vector<bool> used(N, false);
        std::function<bool(int)> dfs = [&](int u) -> bool {
            if (u == end) {
                path.push_back(u);
                return true;
            }
            used[u] = true;
            for (auto &edge : flow_graph.get_node(u).get_edges()) {
                auto& label = edge.get_label();
                if (label.flow > 0 && !used[edge.get_to()]) {
                    if (dfs(edge.get_to())) {
                        label.flow -= 1; // use up one unit of flow along this edge
                        // Also update the reverse edge
                        flow_graph.get_node(edge.get_to()).get_edges()[label.rev].get_label().flow += 1;
                        path.push_back(u);
                        return true;
                    }
                }
            }
            return false;
        };
        dfs(start);
        std::reverse(path.begin(), path.end());
        return path;
    };
    
    // Extract two paths from the flow network.
    std::vector<std::vector<int>> flow_paths;
    for (int i = 0; i < max_flow; i++)
        flow_paths.push_back(extract_flow_path(source_flow, sink_flow));
    
    // Convert the flow network nodes back to the original graph’s vertex indices.
    // Recall that for a node v, v_in is v and v_out is (v + n). Consecutive nodes that differ only
    // by the splitting can be merged.
    auto convert_path = [&](const std::vector<int>& fpath) -> std::vector<size_t> {
        std::vector<size_t> orig_path;
        for (int node : fpath) {
            int v = (node < n) ? node : (node - n);
            if (orig_path.empty() || orig_path.back() != v)
                orig_path.push_back(v);
        }
        return orig_path;
    };
    
    std::vector<std::vector<size_t>> paths;
    for (const auto& flow_path : flow_paths)
        paths.push_back(convert_path(flow_path));
    
    return paths;
}