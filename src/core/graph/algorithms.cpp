#include "algorithms.hpp"

template <GraphTrait T>
bool isConnected(const T& graph) {
    if (graph.size() == 0) return true;
    std::vector<bool> visited(graph.size(), false);
    std::vector<size_t> stack;
    stack.push_back(0);
    while (!stack.empty()) {
        size_t nodeIndex = stack.back();
        stack.pop_back();
        visited[nodeIndex] = true;
        for (auto& edge : graph.getNodes()[nodeIndex].getEdges())
            if (!visited[edge.getTo()])
                stack.push_back(edge.getTo());
    }
    for (bool visit : visited) if (!visit) return false;
    return true;
}