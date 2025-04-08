#include "orthogonal_algorithms.hpp"

template <GraphTrait T>
const SimpleGraph* compute_embedding_from_shape(const T& graph, const Shape& shape) {
    SimpleGraph* embedding = new SimpleGraph();
    for (int i = 0; i < graph.size(); ++i)
        embedding->add_node();
    for (int i = 0; i < graph.size(); ++i) {
        int neighbor = shape.has_node_a_left_neighbor(i);
        if (neighbor != -1)
            embedding->add_edge(i, neighbor);
        neighbor = shape.has_node_a_up_neighbor(i);
        if (neighbor != -1)
            embedding->add_edge(i, neighbor);
        neighbor = shape.has_node_a_right_neighbor(i);
        if (neighbor != -1)
            embedding->add_edge(i, neighbor);
        neighbor = shape.has_node_a_down_neighbor(i);
        if (neighbor != -1)
            embedding->add_edge(i, neighbor);
    }
    return embedding;
}