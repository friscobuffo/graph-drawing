#include "tree.hpp"
#include "../graph/graph.hpp"

#include <list>

template <GraphTrait T>
SimpleTree* buildSimpleSpanningTree(const T& graph) {
    SimpleTree* tree = new SimpleTree();
    for (int i = 0; i < graph.size(); ++i) tree->addNode();
    std::vector<bool> visited(graph.size(), false);
    visited[0] = true;
    std::list<size_t> queue;
    queue.push_back(0);
    while (!queue.empty()) {
        size_t nodeIndex = queue.back();
        queue.pop_back();
        for (auto& edge : graph.getNodes()[nodeIndex].getEdges()) {
            size_t neighborIndex = edge.getTo();
            if (!visited[neighborIndex]) {
                visited[neighborIndex] = true;
                tree->addChild(nodeIndex, neighborIndex);
            }
        }
    }
    return tree;
}