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
    
    { node.getIndex() } -> std::convertible_to<int>;
    { node.setIndex(0) } -> std::same_as<void>;
    { constNode.getEdges() } -> std::same_as<const Container<typename T::GraphEdgeType>&>;
    { node.addEdge(std::declval<typename T::GraphEdgeType*>()) } -> std::same_as<void>;
    { node.getEdges() } -> std::same_as<Container<typename T::GraphEdgeType>&>;
};

template <typename T>
struct GraphNode {
    using GraphEdgeType = T;
private:
    int m_index = -1;
    Container<T> m_edges;
public:
    GraphNode() {}
    int getIndex() const { return m_index; }
    void setIndex(int index) { m_index = index; }
    Container<T>& getEdges() { return m_edges; }
    const Container<T>& getEdges() const { return m_edges; }
    void addEdge(T* edge) { m_edges.addElement(std::unique_ptr<T>(edge)); }
    std::string toString() const {
        std::string result = "Node " + std::to_string(m_index) + " edges: ";
        for (auto& edge : m_edges)
            result += "(" + edge.toString() + ")";
        return result;
    }
    void print() const { std::cout << toString() << std::endl; }
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
    int getIndex() const { return m_node.getIndex(); }
    void setIndex(int index) { m_node.setIndex(index); }
    Container<T>& getEdges() { return m_node.getEdges(); }
    const Container<T>& getEdges() const { return m_node.getEdges(); }
    void addEdge(T* edge) { m_node.addEdge(edge); }
    Color getColor() const { return m_color; }
    std::string toString() const {
        std::string result = "ColoredNode " + color2string(m_color) + " " + std::to_string(m_node.getIndex()) + " edges:";
        for (auto& edge : getEdges())
            result += " " + edge.toString();
        return result;
    }
    void print() const { std::cout << toString() << std::endl; }
};

static_assert(GraphNodeTrait<ColoredNode<GraphEdge>>);

struct SimpleGraphNode {
    using GraphEdgeType = GraphEdge;
private:
    GraphNode<GraphEdge> m_node;
public:
    SimpleGraphNode() {}
    int getIndex() const { return m_node.getIndex(); }
    void setIndex(int index) { m_node.setIndex(index); }
    Container<GraphEdge>& getEdges() { return m_node.getEdges(); }
    const Container<GraphEdge>& getEdges() const { return m_node.getEdges(); }
    void addEdge(GraphEdge* edge) { m_node.addEdge(edge); }
    std::string toString() const { return m_node.toString(); }
    void print() const { m_node.print(); }
};

static_assert(GraphNodeTrait<SimpleGraphNode>);

#endif