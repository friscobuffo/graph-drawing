#ifndef MY_NODE_H
#define MY_NODE_H

#include <concepts>
#include <iostream>
#include <string>
#include <vector>

#include "../utils.hpp"
#include "edge.hpp"

template <typename T>
concept GraphNodeTrait = requires(T node, const T constNode) {
    requires PrintTrait<T>;
    typename T::GraphEdgeType;
    requires GraphEdgeTrait<typename T::GraphEdgeType>;
    
    { node.get_index() } -> std::convertible_to<int>;
    { node.set_index(0) } -> std::same_as<void>;
    { constNode.get_edges() } -> std::same_as<const Container<typename T::GraphEdgeType>&>;
    { constNode.get_edge(0) } -> std::same_as<const typename T::GraphEdgeType&>;
    { node.get_edge(0) } -> std::same_as<typename T::GraphEdgeType&>;
    { constNode.get_degree() } -> std::convertible_to<size_t>;
    { node.add_edge(std::declval<typename T::GraphEdgeType*>()) } -> std::same_as<void>;
    { node.remove_edge(0) } -> std::same_as<void>;
    { node.get_edges() } -> std::same_as<Container<typename T::GraphEdgeType>&>;
};

template <typename T>
struct GraphNode {
    using GraphEdgeType = T;
private:
    int m_index = -1;
    Container<T> m_edges;
public:
    GraphNode() {}
    int get_index() const { return m_index; }
    void set_index(int index) { m_index = index; }
    Container<T>& get_edges() { return m_edges; }
    const T& get_edge(size_t index) const { return m_edges[index]; }
    T& get_edge(size_t index) { return m_edges[index]; }
    const Container<T>& get_edges() const { return m_edges; }
    size_t get_degree() const { return m_edges.size(); }
    void add_edge(T* edge) { m_edges.add_element(std::unique_ptr<T>(edge)); }
    void remove_edge(size_t neighbor_index) {
        for (size_t i = 0; i < get_edges().size(); i++)
            if (get_edges()[i].get_to() == neighbor_index) {
                m_edges.remove_element(i);
                return;
            }
        throw std::runtime_error("Edge not found");
    }
    std::string to_string() const {
        std::string result = "Node " + std::to_string(m_index) + " edges: ";
        for (auto& edge : m_edges)
            result += "(" + edge.to_string() + ")";
        return result;
    }
    void print() const { std::cout << to_string() << std::endl; }
};

static_assert(GraphNodeTrait<GraphNode<GraphEdge>>);

template <typename T>
struct ColoredNode {
    using GraphEdgeType = T;
private:
    GraphNode<T> m_node{};
    Color m_color;
public:
    ColoredNode(Color color) : m_color(color) {}
    int get_index() const { return m_node.get_index(); }
    void set_index(int index) { m_node.set_index(index); }
    Container<T>& get_edges() { return m_node.get_edges(); }
    const Container<T>& get_edges() const { return m_node.get_edges(); }
    const T& get_edge(size_t index) const { return m_node.get_edge(index); }
    T& get_edge(size_t index) { return m_node.get_edge(index); }
    size_t get_degree() const { return m_node.get_degree(); }
    void add_edge(T* edge) { m_node.add_edge(edge); }
    void remove_edge(size_t neighbor_index) { m_node.remove_edge(neighbor_index); }
    Color get_color() const { return m_color; }
    std::string to_string() const {
        std::string result = "ColoredNode " + color_to_string(m_color) + " " + std::to_string(m_node.get_index()) + " edges:";
        for (auto& edge : get_edges())
            result += " " + edge.to_string();
        return result;
    }
    void print() const { std::cout << to_string() << std::endl; }
};

static_assert(GraphNodeTrait<ColoredNode<GraphEdge>>);

struct SimpleGraphNode {
    using GraphEdgeType = GraphEdge;
private:
    GraphNode<GraphEdge> m_node;
public:
    SimpleGraphNode() {}
    int get_index() const { return m_node.get_index(); }
    void set_index(int index) { m_node.set_index(index); }
    Container<GraphEdge>& get_edges() { return m_node.get_edges(); }
    const Container<GraphEdge>& get_edges() const { return m_node.get_edges(); }
    const GraphEdge& get_edge(size_t index) const { return m_node.get_edge(index); }
    GraphEdge& get_edge(size_t index) { return m_node.get_edge(index); }
    size_t get_degree() const { return m_node.get_degree(); }
    void add_edge(GraphEdge* edge) { m_node.add_edge(edge); }
    void remove_edge(size_t neighbor_index) { m_node.remove_edge(neighbor_index); }
    std::string to_string() const { return m_node.to_string(); }
    void print() const { m_node.print(); }
};

static_assert(GraphNodeTrait<SimpleGraphNode>);

#endif