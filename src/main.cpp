#include "core/graph/graph.hpp"
#include "core/graph/algorithms.hpp"
#include "core/tree/algorithms.hpp"
#include "core/graph/file_loader.hpp"

#include "orthogonal/shape_builder.hpp"
#include "sat/glucose.hpp"
#include "orthogonal/shape.hpp"
#include "orthogonal/drawing_builder.hpp"

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

#include "drawing/svg_drawer.hpp"
#include "drawing/linear_scale.hpp"

int main() {
    // prova1();

    SimpleGraph* fromFile = loadSimpleUndirectedGraphFromFile("example-graphs/g6.txt");
    auto cyclesFromFile = computeAllCycles(*fromFile);

    const Shape* shape = build_shape(*fromFile, cyclesFromFile);
    if (shape == nullptr) {
        std::cerr << "Error: Could not build the shape.\n";
        return 1;
    }
    const NodesPositions* positions = build_nodes_positions(*shape, *fromFile);
    int max_x = 0;
    int max_y = 0;
    for (size_t i = 0; i < fromFile->size(); ++i) {
        max_x = std::max(max_x, positions->get_position_x(i));
        max_y = std::max(max_y, positions->get_position_y(i));
    }
    SvgDrawer drawer{800, 600};
    ScaleLinear scale_x = ScaleLinear(0, max_x+2, 0, 800);
    ScaleLinear scale_y = ScaleLinear(0, max_y+2, 0, 600);
    std::vector<Point2D> points;
    for (size_t i = 0; i < fromFile->size(); ++i) {
        double x = scale_x.map(positions->get_position_x(i)+1);
        double y = scale_y.map(positions->get_position_y(i)+1);
        points.push_back(Point2D{x, y});
        std::cout << i << " -> " << x << " " << y << std::endl;
    }
    for (size_t i = 0; i < fromFile->size(); ++i)
        for (auto& edge : fromFile->getNodes()[i].getEdges()) {
            size_t j = edge.getTo();
            Line2D line{points[i], points[j]};
            drawer.add(line);
        }
    for (size_t i = 0; i < fromFile->size(); ++i)
        drawer.add(points[i], "black", std::to_string(i));
    drawer.saveToFile("graph.svg");
    delete fromFile;
    delete shape;
    delete positions;

    return 0;
}