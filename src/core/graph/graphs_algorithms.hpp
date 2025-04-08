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

#include "graphs_algorithms.ipp"










#include <vector>
#include <queue>
#include <algorithm>
#include <limits>
#include <optional>
#include <utility>
#include <functional>

// The following template function finds disjoint paths (both edge‐ and vertex‑disjoint)
// between nodes s and t in an undirected graph. The function returns an optional vector of
// vectors (each a sequence of vertex indices representing a path). If no paths
// do not exists, it returns std::nullopt.

template <typename GraphTrait>
std::optional<std::vector<std::vector<size_t>>>
find_disjoint_paths(const GraphTrait& graph, size_t s, size_t t) {
    size_t n = graph.size();
    // In our flow network, we “split” each vertex into two:
    // - v_in is represented as index v
    // - v_out is represented as index v+n
    // The total number of nodes in the flow network is 2*n.
    size_t N = 2 * n;
    const int INF = 1000000; // A large capacity for the source/target splitting edge.
    
    // Define our flow edge structure.
    struct FlowEdge {
        int to, capacity, flow;
        int rev; // index of the reverse edge in the adjacent list of 'to'
    };
    
    // Build an adjacency list for the flow network.
    std::vector<std::vector<FlowEdge>> flowGraph(N);
    
    auto addEdge = [&](int u, int v, int cap) {
        FlowEdge a { v, cap, 0, (int)flowGraph[v].size() };
        FlowEdge b { u, 0, 0, (int)flowGraph[u].size() };
        flowGraph[u].push_back(a);
        flowGraph[v].push_back(b);
    };
    
    // For every vertex v, add an edge from v_in (v) to v_out (v+n).
    // For vertices other than s and t we use capacity 1; for s and t we use INF.
    for (size_t v = 0; v < n; v++) {
        int cap = (v == s || v == t) ? INF : 1;
        addEdge(v, v + n, cap);
    }
    
    // Now add edges corresponding to the undirected edges of the original graph.
    // Since each edge is represented twice, we add an edge only once (e.g. when u < v).
    for (size_t u = 0; u < n; u++) {
        for (const auto& edge : graph.get_node(u).get_edges()) {
            size_t v = edge.get_to();
            if (u < v) {
                // In the flow network, add an edge from u_out (u+n) to v_in (v)
                // and an edge from v_out (v+n) to u_in (u) with capacity 1.
                addEdge(u + n, v, 1);
                addEdge(v + n, u, 1);
            }
        }
    }
    
    // For the flow network, choose:
    //   source = s_out (s + n)
    //   sink   = t_in  (t)
    int source_flow = s + n; // s_out
    int sink_flow   = t;     // t_in
    
    // Now run Edmonds–Karp (BFS-based max flow) to compute the maximum flow.
    int max_flow = 0;
    while (true) {
        std::vector<int> parent(N, -1);
        std::vector<int> parentEdge(N, -1);
        std::queue<int> q;
        q.push(source_flow);
        std::vector<bool> visited(N, false);
        visited[source_flow] = true;
        while (!q.empty() && !visited[sink_flow]) {
            int u = q.front();
            q.pop();
            for (int i = 0; i < flowGraph[u].size(); i++) {
                FlowEdge &e = flowGraph[u][i];
                if (!visited[e.to] && e.capacity - e.flow > 0) {
                    visited[e.to] = true;
                    parent[e.to] = u;
                    parentEdge[e.to] = i;
                    q.push(e.to);
                }
            }
        }
        if (!visited[sink_flow])
            break; // No more augmenting paths.
        
        int flow = INF;
        int cur = sink_flow;
        while (cur != source_flow) {
            int p = parent[cur];
            int edge_idx = parentEdge[cur];
            flow = std::min(flow, flowGraph[p][edge_idx].capacity - flowGraph[p][edge_idx].flow);
            cur = p;
        }
        
        cur = sink_flow;
        while (cur != source_flow) {
            int p = parent[cur];
            int edge_idx = parentEdge[cur];
            flowGraph[p][edge_idx].flow += flow;
            int rev_idx = flowGraph[p][edge_idx].rev;
            flowGraph[cur][rev_idx].flow -= flow;
            cur = p;
        }
        
        max_flow += flow;
    }
    // If we did not get at least two units of flow, then two disjoint paths do not exist.
    if (max_flow < 1)
        return std::nullopt;
    
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
            for (auto &e : flowGraph[u]) {
                if (e.flow > 0 && !used[e.to]) {
                    if (dfs(e.to)) {
                        e.flow -= 1; // use up one unit of flow along this edge
                        // Also update the reverse edge
                        flowGraph[e.to][e.rev].flow += 1;
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
            size_t v = (node < n) ? node : (node - n);
            if (orig_path.empty() || orig_path.back() != v)
                orig_path.push_back(v);
        }
        return orig_path;
    };
    
    std::vector<std::vector<size_t>> paths;
    for (const auto& flow_path : flow_paths)
        paths.push_back(convert_path(flow_path));
    
    return std::make_optional(paths);
}

#endif