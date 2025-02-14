#ifndef MY_EDGE_H
#define MY_EDGE_H

#include <concepts>
#include <iostream>
#include <string>

#include "../utils.hpp"

template <typename T>
concept EdgeTrait = requires(T edge) {
    requires PrintTrait<T>;
    { edge.getTo() } -> std::convertible_to<int>;
};

struct Edge {
private:
    int m_to;
public:
    Edge(int to) : m_to(to) {}
    int getTo() const { return m_to; }
    std::string toString() const {
        return "Edge to " + std::to_string(m_to);
    }
    void print() const { std::cout << toString() << std::endl; }
};

static_assert(EdgeTrait<Edge>);

struct ColoredEdge {
private:
    Edge m_edge;
    Color m_color;
public:
    ColoredEdge(int to, Color color) : m_edge(to), m_color(color) {}
    int getTo() const { return m_edge.getTo(); }
    Color getColor() const { return m_color; }
    std::string toString() const {
        return "ColoredEdge " + color2string(m_color) + " -> " + std::to_string(m_edge.getTo());
    }
    void print() const { std::cout << toString() << std::endl; }
};

static_assert(EdgeTrait<ColoredEdge>);
static_assert(ColorTrait<ColoredEdge>);

struct WeightedEdge {
private:
    Edge m_edge;
    double m_weight;
public:
WeightedEdge(int to, double weight) : m_edge(to), m_weight(weight) {}
    int getTo() const { return m_edge.getTo(); }
    double getWeight() const { return m_weight; }
    std::string toString() const {
        return "ColoredEdge " + std::to_string(m_weight) + " -> " + std::to_string(m_edge.getTo());
    }
    void print() const { std::cout << toString() << std::endl; }
};

static_assert(EdgeTrait<WeightedEdge>);
static_assert(WeightTrait<WeightedEdge>);

#endif