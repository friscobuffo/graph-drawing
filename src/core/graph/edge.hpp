#ifndef MY_EDGE_H
#define MY_EDGE_H

#include <concepts>
#include <iostream>
#include <string>

#include "../utils.hpp"

template <typename T>
concept GraphEdgeTrait = requires(T edge) {
    requires PrintTrait<T>;
    { edge.get_to() } -> std::same_as<int>;
};

struct GraphEdge {
private:
    int m_to;
public:
    GraphEdge(int to) : m_to(to) {}
    int get_to() const { return m_to; }
    std::string to_string() const {
        return "Edge to " + std::to_string(m_to);
    }
    void print() const { std::cout << to_string() << std::endl; }
};

static_assert(GraphEdgeTrait<GraphEdge>);

struct ColoredGraphEdge {
private:
    GraphEdge m_edge;
    Color m_color;
public:
    ColoredGraphEdge(int to, Color color) : m_edge(to), m_color(color) {}
    int get_to() const { return m_edge.get_to(); }
    Color get_color() const { return m_color; }
    std::string to_string() const {
        return "ColoredEdge " + color_to_string(m_color) + " -> " + std::to_string(m_edge.get_to());
    }
    void print() const { std::cout << to_string() << std::endl; }
};

static_assert(GraphEdgeTrait<ColoredGraphEdge>);
static_assert(ColorTrait<ColoredGraphEdge>);

struct WeightedGraphEdge {
private:
    GraphEdge m_edge;
    double m_weight;
public:
    WeightedGraphEdge(int to, double weight) : m_edge(to), m_weight(weight) {}
    int get_to() const { return m_edge.get_to(); }
    double get_weight() const { return m_weight; }
    std::string to_string() const {
        return "ColoredEdge " + std::to_string(m_weight) + " -> " + std::to_string(m_edge.get_to());
    }
    void print() const { std::cout << to_string() << std::endl; }
};

static_assert(GraphEdgeTrait<WeightedGraphEdge>);
static_assert(WeightTrait<WeightedGraphEdge>);

template <PrintTrait T>
struct LabeledGraphEdge {
private:
    GraphEdge m_edge;
    T m_label;
public:
    LabeledGraphEdge(int to, T label) : m_edge(to), m_label(label) {}
    int get_to() const { return m_edge.get_to(); }
    const T& get_label() const { return m_label; }
    T& get_label() { return m_label; }
    std::string to_string() const {
        return m_edge.to_string() + " with label " + m_label.to_string();
    }
    void print() const { std::cout << to_string() << std::endl; }
};

static_assert(GraphEdgeTrait<LabeledGraphEdge<Int>>);

#endif