#ifndef MY_GRAPH_H
#define MY_GRAPH_H

#include <vector>
#include <iostream>
#include <string>
#include <memory>
#include <concepts>

#include "node.hpp"
#include "edge.hpp"

template <typename T>
concept GraphTrait = requires(T graph, const T constGraph) {
    requires PrintTrait<T>;
    typename T::GraphNodeType;
    requires GraphNodeTrait<typename T::GraphNodeType>;
    
    { constGraph.get_nodes() } -> std::same_as<const Container<typename T::GraphNodeType>&>;
    { constGraph.get_node(0) } -> std::same_as<const typename T::GraphNodeType&>;
    { graph.get_node(0) } -> std::same_as<typename T::GraphNodeType&>;
    { graph.add_node(std::declval<typename T::GraphNodeType*>()) } -> std::same_as<void>;
    { graph.add_edge(0, std::declval<typename T::GraphNodeType::GraphEdgeType*>()) } -> std::same_as<void>;
    { constGraph.size() } -> std::same_as<int>;
    { graph.remove_edge(0, 0) } -> std::same_as<void>;
    { constGraph.get_number_of_edges() } -> std::same_as<int>;
};

template <typename T>
struct Graph {
    using GraphNodeType = T;
private:
    Container<T> m_nodes;
public:
    Graph() {}
    const Container<T>& get_nodes() const { return m_nodes; }
    const T& get_node(int index) const { return m_nodes[index]; }
    T& get_node(int index) { return m_nodes[index]; }
    void add_node(T* node) {
        node->set_index(size());
        m_nodes.add_element(std::unique_ptr<T>(node));
    }
    void add_edge(int from, T::GraphEdgeType* edge) { m_nodes[from].add_edge(edge); }
    int size() const { return m_nodes.size(); }
    void remove_edge(int from, int to) {
        m_nodes[from].remove_edge(to);
    }
    int get_number_of_edges() const {
        int count = 0;
        for (const auto& node : m_nodes)
            count += node.get_edges().size();
        return count;
    }
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
    const Container<GraphNode<GraphEdge>>& get_nodes() const { return m_graph.get_nodes(); }
    const GraphNode<GraphEdge>& get_node(int index) const { return m_graph.get_node(index); }
    GraphNode<GraphEdge>& get_node(int index) { return m_graph.get_node(index); }
    void add_node(GraphNode<GraphEdge>* node) { m_graph.add_node(node); }
    void add_edge(int from, GraphEdge* edge) { m_graph.add_edge(from, edge); }
    int size() const { return m_graph.size(); }
    void add_node() { m_graph.add_node(new GraphNode<GraphEdge>()); }
    void add_edge(int from, int to) { m_graph.add_edge(from, new GraphEdge(to)); }
    void remove_edge(int from, int to) { m_graph.remove_edge(from, to); }
    void add_undirected_edge(int from, int to) {
        add_edge(from, to);
        add_edge(to, from);
    }
    void remove_undirected_edge(int from, int to) {
        m_graph.remove_edge(from, to);
        m_graph.remove_edge(to, from);
    }
    int get_number_of_edges() const { return m_graph.get_number_of_edges(); }
    std::string to_string() const { return m_graph.to_string(); }
    void print() const { m_graph.print(); }
};

static_assert(GraphTrait<SimpleGraph>);

struct ColoredNodesGraph {
    using GraphNodeType = ColoredNode<GraphEdge>;
private:
    Graph<ColoredNode<GraphEdge>> m_graph{};
public:
    const Container<ColoredNode<GraphEdge>>& get_nodes() const { return m_graph.get_nodes(); }
    const ColoredNode<GraphEdge>& get_node(int index) const { return m_graph.get_node(index); }
    ColoredNode<GraphEdge>& get_node(int index) { return m_graph.get_node(index); }
    void add_node(ColoredNode<GraphEdge>* node) { m_graph.add_node(node); }
    void add_edge(int from, GraphEdge* edge) { m_graph.add_edge(from, edge); }
    int size() const { return m_graph.size(); }
    void add_node(Color color) { m_graph.add_node(new ColoredNode<GraphEdge>(color)); }
    void add_edge(int from, int to) { m_graph.add_edge(from, new GraphEdge(to)); }
    void remove_edge(int from, int to) { m_graph.remove_edge(from, to); }
    void add_undirected_edge(int from, int to) {
        add_edge(from, to);
        add_edge(to, from);
    }
    void remove_undirected_edge(int from, int to) {
        m_graph.remove_edge(from, to);
        m_graph.remove_edge(to, from);
    }
    int get_number_of_edges() const { return m_graph.get_number_of_edges(); }
    std::string to_string() const { return m_graph.to_string(); }
    void print() const { m_graph.print(); }
};

static_assert(GraphTrait<ColoredNodesGraph>);

template <typename T>
struct LabeledEdgeGraph {
    using GraphNodeType = GraphNode<LabeledGraphEdge<T>>;
private:
    Graph<GraphNode<LabeledGraphEdge<T>>> m_graph{};
public:
    const Container<GraphNode<LabeledGraphEdge<T>>>& get_nodes() const { return m_graph.get_nodes(); }
    const GraphNode<LabeledGraphEdge<T>>& get_node(int index) const { return m_graph.get_node(index); }
    GraphNode<LabeledGraphEdge<T>>& get_node(int index) { return m_graph.get_node(index); }
    void add_node(GraphNode<LabeledGraphEdge<T>>* node) { m_graph.add_node(node); }
    void add_edge(int from, LabeledGraphEdge<T>* edge) { m_graph.add_edge(from, edge); }
    int size() const { return m_graph.size(); }
    void add_node() { m_graph.add_node(new GraphNode<LabeledGraphEdge<T>>()); }
    void add_edge(int from, int to, T label) {
        m_graph.add_edge(from, new LabeledGraphEdge<T>(to, label));
    }
    void remove_edge(int from, int to) { m_graph.remove_edge(from, to); }
    int get_number_of_edges() const { return m_graph.get_number_of_edges(); }
    std::string to_string() const { return m_graph.to_string(); }
    void print() const { m_graph.print(); }
};

static_assert(GraphTrait<LabeledEdgeGraph<Int>>);

static_assert(GraphTrait<Graph<LabeledGraphNode<GraphEdge, Int>>>);

template <typename T>
struct LabeledNodeGraph {
    using GraphNodeType = LabeledGraphNode<GraphEdge, T>;
private:
    Graph<LabeledGraphNode<GraphEdge, T>> m_graph{};
public:
    const Container<LabeledGraphNode<GraphEdge, T>>& get_nodes() const { return m_graph.get_nodes(); }
    const LabeledGraphNode<GraphEdge, T>& get_node(int index) const { return m_graph.get_node(index); }
    LabeledGraphNode<GraphEdge, T>& get_node(int index) { return m_graph.get_node(index); }
    void add_node(LabeledGraphNode<GraphEdge, T>* node) { m_graph.add_node(node); }
    void add_edge(int from, GraphEdge* edge) { m_graph.add_edge(from, edge); }
    int size() const { return m_graph.size(); }
    void add_node(T label) { m_graph.add_node(new LabeledGraphNode<GraphEdge, T>(label)); }
    void add_edge(int from, int to) { m_graph.add_edge(from, new GraphEdge(to)); }
    void remove_edge(int from, int to) { m_graph.remove_edge(from, to); }
    int get_number_of_edges() const { return m_graph.get_number_of_edges(); }
    std::string to_string() const { return m_graph.to_string(); }
    void print() const { m_graph.print(); }
};

static_assert(GraphTrait<LabeledNodeGraph<Int>>);

#endif