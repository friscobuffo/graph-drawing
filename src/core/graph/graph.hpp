#ifndef MY_GRAPH_H
#define MY_GRAPH_H

#include <vector>
#include <iostream>
#include <string>
#include <memory>
#include <concepts>

#include "node.hpp"

template <typename T>
concept GraphTrait = requires(T graph, const T constGraph) {
    requires PrintTrait<T>;
    typename T::NodeType;
    requires NodeTrait<typename T::NodeType>;
    
    { constGraph.getNodes() } -> std::same_as<const Container<typename T::NodeType>&>;
    { graph.addNode(std::declval<typename T::NodeType*>()) } -> std::same_as<void>;
    { graph.addEdge(0, std::declval<typename T::NodeType::EdgeType*>()) } -> std::same_as<void>;
    { graph.size() } -> std::convertible_to<size_t>;
    { graph.getNodes() } -> std::same_as<Container<typename T::NodeType>&>;
    { constGraph.size() } -> std::convertible_to<size_t>;
};

template <typename T>
struct Graph {
    using NodeType = T;
private:
    Container<T> m_nodes;
public:
    Graph() {}
    Container<T>& getNodes() { return m_nodes; }
    const Container<T>& getNodes() const { return m_nodes; }
    void addNode(T* node) {
        node->setIndex(size());
        m_nodes.addElement(std::unique_ptr<T>(node));
    }
    void addEdge(size_t from, T::EdgeType* edge) {
        m_nodes[from].addEdge(edge);
    }
    size_t size() const { return m_nodes.size(); }
    std::string toString() const {
        std::string result = "Graph: {";
        for (auto& node : m_nodes) {
            result += "["+node.toString() + "] ";
        }
        return result + "}";
    }
    void print() const {
        std::cout << toString() << std::endl;
    }
};

static_assert(GraphTrait<Graph<Node<Edge>>>);

struct SimpleGraph {
    using NodeType = Node<Edge>;
private:
    Graph<Node<Edge>> m_graph;
public:
    SimpleGraph() {}
    Container<Node<Edge>>& getNodes() { return m_graph.getNodes(); }
    const Container<Node<Edge>>& getNodes() const { return m_graph.getNodes(); }
    void addNode(Node<Edge>* node) { m_graph.addNode(node); }
    void addEdge(size_t from, Edge* edge) {
        m_graph.addEdge(from, edge);
    }
    size_t size() const { return m_graph.size(); }
    void addSimpleNode() {
        m_graph.addNode(new Node<Edge>());
    }
    void addSimpleEdge(size_t from, size_t to) {
        m_graph.addEdge(from, new Edge(to));
    }
    void addUndirectedSimpleEdge(size_t from, size_t to) {
        addSimpleEdge(from, to);
        addSimpleEdge(to, from);
    }
    std::string toString() const {
        return m_graph.toString();
    }
    void print() const {
        m_graph.print();
    }
};

static_assert(GraphTrait<SimpleGraph>);

#endif