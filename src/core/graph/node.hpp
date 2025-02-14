#ifndef MY_NODE_H
#define MY_NODE_H

#include <concepts>
#include <iostream>
#include <string>
#include <vector>

#include "../utils.hpp"
#include "edge.hpp"

template <typename T>
concept NodeTrait = requires(T node, const T constNode) {
    requires PrintTrait<T>;
    typename T::EdgeType;
    requires EdgeTrait<typename T::EdgeType>;
    
    { node.getIndex() } -> std::convertible_to<int>;
    { node.setIndex(0) } -> std::same_as<void>;
    { constNode.getEdges() } -> std::same_as<const Container<typename T::EdgeType>&>;
    { node.addEdge(std::declval<typename T::EdgeType*>()) } -> std::same_as<void>;
    { node.getEdges() } -> std::same_as<Container<typename T::EdgeType>&>;
};

template <typename T>
struct Node {
    using EdgeType = T;
private:
    int m_index = -1;
    Container<T> m_edges;
public:
    Node() {}
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

static_assert(NodeTrait<Node<Edge>>);

template <typename T>
struct ColoredNode {
    using EdgeType = T;
private:
    Node<T> m_node{};
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

static_assert(NodeTrait<ColoredNode<Edge>>);

struct SimpleNode {
    using EdgeType = Edge;
private:
    Node<Edge> m_node;
public:
    SimpleNode() {}
    int getIndex() const { return m_node.getIndex(); }
    void setIndex(int index) { m_node.setIndex(index); }
    Container<Edge>& getEdges() { return m_node.getEdges(); }
    const Container<Edge>& getEdges() const { return m_node.getEdges(); }
    void addEdge(Edge* edge) { m_node.addEdge(edge); }
    std::string toString() const { return m_node.toString(); }
    void print() const { m_node.print(); }
};

static_assert(NodeTrait<SimpleNode>);

#endif