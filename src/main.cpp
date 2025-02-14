#include "core/graph/graph.hpp"
#include "core/graph/algorithms.hpp"

int main() {
    SimpleGraph graph;
    graph.addSimpleNode();
    graph.addSimpleNode();
    graph.addSimpleNode();

    graph.addUndirectedSimpleEdge(0, 1);
    graph.addUndirectedSimpleEdge(1, 2);


    const SimpleGraph& constGraph = graph;
    graph.print();

    // graph.addNode(std::make_unique<Node<Edge>>());
    // std::cout << isConnected(constGraph) << std::endl;
    return 0;
}