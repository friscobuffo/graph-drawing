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
    }
}

struct pair_hash {
    template <typename T1, typename T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        std::size_t h1 = std::hash<T1>{}(p.first);
        std::size_t h2 = std::hash<T2>{}(p.second);

        // Combining hashes using a better mixing function
        return h1 ^ (h2 * 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

class Shape {
private:
    std::unordered_map<std::pair<size_t, size_t>, Direction, pair_hash> m_shape;
    std::unordered_map<size_t, std::tuple<bool, bool, bool, bool>> m_node_neighbors_direction; 
public:
    void set_direction(const size_t i, const size_t j, const Direction direction);
    Direction get_direction(const size_t i, const size_t j) const;
    bool contains(const size_t i, const size_t j) const;
    bool is_up(const size_t i, const size_t j) const;
    bool is_down(const size_t i, const size_t j) const;
    bool is_right(const size_t i, const size_t j) const;
    bool is_left(const size_t i, const size_t j) const;
    bool is_horizontal(const size_t i, const size_t j) const;
    bool is_Vertical(const size_t i, const size_t j) const;
    int has_node_a_right_neighbor(const size_t node) const;
    int has_node_a_left_neighbor(const size_t node) const;
    int has_node_a_up_neighbor(const size_t node) const;
    int has_node_a_down_neighbor(const size_t node) const;
    std::string to_string() const;
    void print() const;
};

static_assert(PrintTrait<Shape>);

#endif