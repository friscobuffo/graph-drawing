#ifndef MY_DRAWING_BUILDER_H
#define MY_DRAWING_BUILDER_H

#include "shape.hpp"
#include "../core/graph/graph.hpp"
#include "../core/graph/node.hpp"
#include "../core/utils.hpp"
#include "../core/graph/algorithms.hpp"
#include "../drawing/svg_drawer.hpp"
#include "../drawing/linear_scale.hpp"
#include "shape_builder.hpp"

class NodesPositions {
private:
    std::vector<std::pair<int, int>> m_positions;
public:
    void set_position_x(size_t node, size_t position);
    void set_position_y(size_t node, size_t position);
    int get_position_x(size_t node) const;
    int get_position_y(size_t node) const;
};

const NodesPositions* build_nodes_positions(const Shape& shape, const ColoredNodesGraph& graph);

template <GraphTrait T>
void make_rectilinear_drawing(const T& graph) {
    auto cycles = compute_all_cycles(graph);
    // auto cycles = compute_cycle_basis(graph);
    ColoredNodesGraph colored_graph{};
    for (int i = 0; i < graph.size(); i++)
        colored_graph.add_node(Color::BLACK);
    for (int i = 0; i < graph.size(); i++) {
        assert(graph.get_nodes()[i].get_edges().size() <= 4);
        for (auto& edge : graph.get_nodes()[i].get_edges()) {
            int j = edge.get_to();
            colored_graph.add_edge(i, j);
        }
    }
    const Shape* shape = build_shape(colored_graph, cycles);
    const NodesPositions* positions = build_nodes_positions(*shape, colored_graph);
    int max_x = 0;
    int max_y = 0;
    for (size_t i = 0; i < colored_graph.size(); ++i) {
        max_x = std::max(max_x, positions->get_position_x(i));
        max_y = std::max(max_y, positions->get_position_y(i));
    }
    SvgDrawer drawer{800, 600};
    ScaleLinear scale_x = ScaleLinear(0, max_x+2, 0, 800);
    ScaleLinear scale_y = ScaleLinear(0, max_y+2, 0, 600);
    std::vector<Point2D> points;
    for (size_t i = 0; i < colored_graph.size(); ++i) {
        double x = scale_x.map(positions->get_position_x(i)+1);
        double y = scale_y.map(positions->get_position_y(i)+1);
        points.push_back(Point2D{x, y});
    }
    for (size_t i = 0; i < colored_graph.size(); ++i)
        for (auto& edge : colored_graph.get_nodes()[i].get_edges()) {
            size_t j = edge.get_to();
            Line2D line{points[i], points[j]};
            drawer.add(line);
        }
    for (size_t i = 0; i < colored_graph.size(); ++i) {
        Color color = colored_graph.get_nodes()[i].get_color();
        drawer.add(points[i], color_to_string(color), std::to_string(i));
    }
    drawer.saveToFile("graph.svg");
    delete shape;
    delete positions;
}

#endif