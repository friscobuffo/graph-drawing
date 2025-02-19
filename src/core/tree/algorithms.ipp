#include "tree.hpp"
#include "../graph/graph.hpp"

#include <list>

template <GraphTrait T>
SimpleTree* buildSimpleSpanningTree(const T& graph) {
    SimpleTree* tree = new SimpleTree();
    for (int i = 0; i < graph.size(); ++i) tree->add_node();
    std::vector<bool> visited(graph.size(), false);
    visited[0] = true;
    std::list<size_t> queue;
    queue.push_back(0);
    while (!queue.empty()) {
        size_t nodeIndex = queue.back();
        queue.pop_back();
        for (auto& edge : graph.get_nodes()[nodeIndex].get_edges()) {
            size_t neighborIndex = edge.get_to();
            if (!visited[neighborIndex]) {
                visited[neighborIndex] = true;
                tree->add_child(nodeIndex, neighborIndex);
            }
        }
    }
    return tree;
}