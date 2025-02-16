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

template <GraphTrait T>
std::vector<std::vector<size_t>> computeAllCyclesWithNode(const T& graph, size_t node_index, std::vector<bool>& taboo_nodes) {
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
        for (auto& edge : graph.getNodes()[current].getEdges()) {
            size_t neighbor = edge.getTo();
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