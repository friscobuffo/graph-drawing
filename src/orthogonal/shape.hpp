#ifndef MY_SHAPE_HPP
#define MY_SHAPE_HPP

#include <string>
#include <unordered_map>
#include <tuple>
#include <utility>

#include "../core/utils.hpp"

enum class Direction {
    LEFT,
    RIGHT,
    UP,
    DOWN
};

inline const std::string direction_to_string(const Direction direction) {
    switch (direction) {
        case Direction::LEFT: return "left";
        case Direction::RIGHT: return "right";
        case Direction::UP: return "up";
        case Direction::DOWN: return "down";
        default: return "unknown";
    }
}

inline Direction string_to_direction(const std::string& direction) {
    if (direction == "left") return Direction::LEFT;
    if (direction == "right") return Direction::RIGHT;
    if (direction == "up") return Direction::UP;
    if (direction == "down") return Direction::DOWN;
    throw std::invalid_argument("Invalid direction string: " + direction);
}

struct pair_hash {
    template <typename T1, typename T2>
    int operator()(const std::pair<T1, T2>& p) const {
        int h1 = std::hash<T1>{}(p.first);
        int h2 = std::hash<T2>{}(p.second);
        return h1 ^ (h2 * 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

class Shape {
private:
    std::unordered_map<std::pair<int, int>, Direction, pair_hash> m_shape;
    std::unordered_map<int, std::tuple<int, int, int, int>> m_node_neighbors_direction; 
public:
    void set_direction(const int i, const int j, const Direction direction);
    Direction get_direction(const int i, const int j) const;
    bool contains(const int i, const int j) const;
    bool is_up(const int i, const int j) const;
    bool is_down(const int i, const int j) const;
    bool is_right(const int i, const int j) const;
    bool is_left(const int i, const int j) const;
    bool is_horizontal(const int i, const int j) const;
    bool is_vertical(const int i, const int j) const;
    int has_node_a_right_neighbor(const int node) const;
    int has_node_a_left_neighbor(const int node) const;
    int has_node_a_up_neighbor(const int node) const;
    int has_node_a_down_neighbor(const int node) const;
    std::string to_string() const;
    void print() const;
};

static_assert(PrintTrait<Shape>);

#endif