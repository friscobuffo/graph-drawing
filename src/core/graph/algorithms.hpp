#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <vector>

#include "graph.hpp"

template <GraphTrait T>
bool isConnected(const T& graph);

// pub fn compute_all_cycles_with_node<G: GraphTrait>(
//     graph: &G,
//     starting_node_index: usize,
//     taboo_nodes: &Option<Vec<bool>>,
// ) -> Vec<Vec<usize>> {
//     let mut cycles: Vec<Vec<usize>> = vec![];
//     let mut path: Vec<usize> = vec![];
//     let mut visited: Vec<bool> = vec![false; graph.size()];

//     fn dfs<G: GraphTrait>(
//         graph: &G,
//         current: usize,
//         start: usize,
//         visited: &mut Vec<bool>,
//         path: &mut Vec<usize>,
//         cycles: &mut Vec<Vec<usize>>,
//         taboo_nodes: &Option<Vec<bool>>,
//     ) {
//         if let Some(taboo) = taboo_nodes {
//             if taboo[current] {
//                 return;
//             }
//         }

//         if visited[current] {
//             if current == start && path.len() > 2 {
//                 cycles.push(path.clone());
//             }
//             return;
//         }

//         visited[current] = true;
//         path.push(current);

//         for edge in graph.get_nodes()[current].get_edges() {
//             let neighbor = edge.get_to();
//             if path.len() > 2 && neighbor == start {
//                 cycles.push(path.clone());
//                 continue;
//             }
//             dfs(graph, neighbor, start, visited, path, cycles, taboo_nodes);
//         }
//         visited[current] = false;
//         path.pop();
//     }

//     dfs(
//         graph,
//         starting_node_index,
//         starting_node_index,
//         &mut visited,
//         &mut path,
//         &mut cycles,
//         taboo_nodes,
//     );

//     cycles
// }


// template <GraphTrait T>
// std::vector<std::vector<size_t>> computeAllCyclesWithNode(T& graph, size_t node_index, std::vector<bool>& taboo_nodes) {
//     std::vector<std::vector<size_t>> cycles;
//     std::vector<size_t> path;
//     std::vector<bool> visited(graph.size(), false);

//     std::function<void(size_t, size_t)> dfs = [&](size_t current, size_t start) {
//         if (taboo_nodes[current]) return;
//         if (visited[current]) {
//             if (current == start && path.size() > 2)
//                 cycles.push_back(path);
//             return;
//         }
//         visited[current] = true;
//         path.push_back(current);
//         for (auto& edge : graph.getNodes()[current].getEdges()) {
//             size_t neighbor = edge.getTo();
//             if (path.size() > 2 && neighbor == start)
//                 cycles.push_back(path);
//             else
//                 dfs(neighbor, start);
//         }
//         visited[current] = false;
//         path.pop_back();
//     };

//     dfs(node_index, node_index);
//     return cycles;

// }


template <GraphTrait T>
std::vector<std::vector<size_t>> computeAllCycles(const T& graph) {
    std::vector<std::vector<size_t>> allCycles;
    std::vector<bool> tabooNodes(graph.size(), false);
    for (size_t i = 0; i < graph.size(); ++i) {
        std::vector<std::vector<size_t>> cycles = computeAllCyclesWithNode(graph, i, tabooNodes);
        for (auto& cycle : cycles)
            allCycles.push_back(cycle);
        tabooNodes[i] = true;
    }
    return allCycles;
}

#endif



// pub fn shortest_weighted_path<G: GraphTrait>(graph: &G, from_index: usize, to_index: usize) -> Vec<usize>
// where
//     <<G as GraphTrait>::NodeType as NodeTrait>::EdgeType: WeightedTrait,
// {
//     let mut path: Vec<usize> = vec![];
//     let mut distances = vec![std::f64::INFINITY; graph.size()];
//     let mut previous = vec![-1; graph.size()];
//     let mut visited = vec![false; graph.size()];
//     distances[from_index] = 0.0;
//     for _ in graph.get_nodes() {
//         let mut u: i32 = -1;
//         for j in 0..graph.size() {
//             if !visited[j] && (u == -1 || distances[j] < distances[u as usize]) {
//                 u = j as i32;
//             }
//         }
//         if distances[u as usize] == std::f64::INFINITY {
//             break;
//         }
//         visited[u as usize] = true;
//         for edge in graph.get_nodes()[u as usize].get_edges() {
//             let v = edge.get_to();
//             let weight = edge.get_weight();
//             if distances[u as usize] + weight < distances[v] {
//                 distances[v] = distances[u as usize] + weight;
//                 previous[v] = u;
//             }
//         }
//     }
//     let mut at: i32 = to_index as i32;
//     while at != -1 {
//         path.push(at as usize);
//         at = previous[at as usize];
//     }
//     //reverse path
//     path.reverse();
//     if path.len() == 1 && path[0] != from_index {
//         path.clear();
//     }
//     path          
// }

// fn dfs_bipartition<G: GraphTrait>(graph: &G, node_index: usize, bipartition: &mut Vec<i32>) -> bool {
//     bipartition[node_index] = 0;
//     let mut stack = vec![node_index];
//     while let Some(node_index) = stack.pop() {
//         let node = &graph.get_nodes()[node_index];
//         for edge in node.get_edges() {
//             let neighbor_index = edge.get_to();
//             if bipartition[neighbor_index] == -1 {
//                 bipartition[neighbor_index] = 1 - bipartition[node_index];
//                 stack.push(neighbor_index);
//             } else if bipartition[neighbor_index] == bipartition[node_index] {
//                 return false;
//             }
//         }
//     }
//     true
// }

// pub fn compute_bipartition<G: GraphTrait>(graph: &G) -> Option<Vec<i32>> {
//     let mut bipartition = vec![-1; graph.size()];
//     for i in 0..graph.size() {
//         if bipartition[i] == -1 {
//             let check = dfs_bipartition(graph, i, &mut bipartition);
//             if !check {
//                 return None;
//             }
//         }
//     }
//     Some(bipartition)
// }