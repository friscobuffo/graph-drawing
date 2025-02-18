#ifndef MY_DRAWING_BUILDER_H
#define MY_DRAWING_BUILDER_H


#include "shape.hpp"
#include "../core/graph/graph.hpp"
#include "../core/graph/node.hpp"
#include "../core/utils.hpp"

class NodesPositions {
private:
    std::vector<std::pair<int, int>> m_positions;
public:
    void set_position_x(size_t node, size_t position);
    void set_position_y(size_t node, size_t position);
    int get_position_x(size_t node) const;
    int get_position_y(size_t node) const;
};

template <GraphTrait T>
const NodesPositions* build_nodes_positions(const Shape& shape, const T& graph);

#include "drawing_builder.ipp"

#endif