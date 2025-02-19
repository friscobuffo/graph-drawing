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
    typename T::GraphNodeType;
    requires GraphNodeTrait<typename T::GraphNodeType>;
    
    { constGraph.getNodes() } -> std::same_as<const Container<typename T::GraphNodeType>&>;
    { graph.addNode(std::declval<typename T::GraphNodeType*>()) } -> std::same_as<void>;
    { graph.addEdge(0, std::declval<typename T::GraphNodeType::GraphEdgeType*>()) } -> std::same_as<void>;
    { constGraph.size() } -> std::convertible_to<size_t>;
};

template <typename T>
struct Graph {
    using GraphNodeType = T;
private:
    Container<T> m_nodes;
public:
    Graph() {}
    const Container<T>& getNodes() const { return m_nodes; }
    void addNode(T* node) {
        node->setIndex(size());
        m_nodes.addElement(std::unique_ptr<T>(node));
    }
    void addEdge(size_t from, T::GraphEdgeType* edge) { m_nodes[from].addEdge(edge); }
    size_t size() const { return m_nodes.size(); }
    std::string to_string() const {
        std::string result = "Graph: {";
        for (auto& node : m_nodes)
            result += "["+node.to_string() + "] ";
        return result + "}";
    }
    void print() const { std::cout << to_string() << std::endl; }
};

static_assert(GraphTrait<Graph<GraphNode<GraphEdge>>>);

struct SimpleGraph {
    using GraphNodeType = GraphNode<GraphEdge>;
private:
    Graph<GraphNode<GraphEdge>> m_graph{};
public:
    const Container<GraphNode<GraphEdge>>& getNodes() const { return m_graph.getNodes(); }
    void addNode(GraphNode<GraphEdge>* node) { m_graph.addNode(node); }
    void addEdge(size_t from, GraphEdge* edge) { m_graph.addEdge(from, edge); }
    size_t size() const { return m_graph.size(); }
    void addNode() { m_graph.addNode(new GraphNode<GraphEdge>()); }
    void addEdge(size_t from, size_t to) { m_graph.addEdge(from, new GraphEdge(to)); }
    void addUndirectedEdge(size_t from, size_t to) {
        addEdge(from, to);
        addEdge(to, from);
    }
    std::string to_string() const { return m_graph.to_string(); }
    void print() const { m_graph.print(); }
};

static_assert(GraphTrait<SimpleGraph>);

struct ColoredNodesGraph {
    using GraphNodeType = ColoredNode<GraphEdge>;
private:
    Graph<ColoredNode<GraphEdge>> m_graph{};
public:
    const Container<ColoredNode<GraphEdge>>& getNodes() const { return m_graph.getNodes(); }
    void addNode(ColoredNode<GraphEdge>* node) { m_graph.addNode(node); }
    void addEdge(size_t from, GraphEdge* edge) { m_graph.addEdge(from, edge); }
    size_t size() const { return m_graph.size(); }
    void addNode(Color color) { m_graph.addNode(new ColoredNode<GraphEdge>(color)); }
    void addEdge(size_t from, size_t to) { m_graph.addEdge(from, new GraphEdge(to)); }
    void addUndirectedEdge(size_t from, size_t to) {
        addEdge(from, to);
        addEdge(to, from);
    }
    std::string to_string() const { return m_graph.to_string(); }
    void print() const { m_graph.print(); }
};

static_assert(GraphTrait<SimpleGraph>);

#endif