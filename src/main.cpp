#include "core/graph/graph.hpp"
#include "core/graph/algorithms.hpp"
#include "core/tree/algorithms.hpp"
#include "core/graph/file_loader.hpp"

#include "orthogonal/shape_builder.hpp"
#include "sat/glucose.hpp"
#include "orthogonal/shape.hpp"

#include <iostream>

void prova1() {
    SimpleGraph graph;
    graph.addNode();
    graph.addNode();
    graph.addNode();

    graph.addUndirectedEdge(0, 1);
    graph.addUndirectedEdge(1, 2);
    graph.addUndirectedEdge(2, 0);

    SimpleTree* spanning = buildSimpleSpanningTree(graph);
    spanning->print();
    delete spanning;

    const SimpleGraph& constGraph = graph;
    graph.print();

    std::cout << isConnected(constGraph) << std::endl;
    auto cycles = computeAllCycles(constGraph);
    for (auto& cycle : cycles) {
        for (auto& node : cycle) {
            std::cout << node << " ";
        }
        std::cout << std::endl;
    }

    SimpleTree tree;
    tree.addNode();
    tree.addNode();
    tree.addNode();
    tree.addChild(0, 1);
    tree.addChild(0, 2);

    tree.print();

    SimpleGraph* fromFile = loadSimpleUndirectedGraphFromFile("example-graphs/g1.txt");
    fromFile->print();

    auto cyclesFromFile = computeAllCycles(*fromFile);
    std::cout << cyclesFromFile.size() << std::endl;
    delete fromFile;
}

int main() {
    // prova1();

    SimpleGraph* fromFile = loadSimpleUndirectedGraphFromFile("example-graphs/g8.txt");
    auto cyclesFromFile = computeAllCycles(*fromFile);

    const Shape* shape = build_shape(*fromFile, cyclesFromFile);

    delete fromFile;
    delete shape;

    return 0;
}