#include "shape.hpp"

#include <cassert>

void Shape::set_direction(const int i, const int j, const Direction direction) {
    if (m_shape.contains(std::make_pair(i, j))) {
        std::string error = "Direction already set for this pair: (" +
            std::to_string(i) + ", " + std::to_string(j) + ") -> " +
            direction_to_string(m_shape.at(std::make_pair(i, j))) + " vs " +
            direction_to_string(direction);
        throw std::invalid_argument(error);
    }
    m_shape[std::make_pair(i, j)] = direction;
    if (m_node_neighbors_direction.contains(i)) {
        switch (direction) {
            case Direction::LEFT: std::get<0>(m_node_neighbors_direction[i]) = j; break;
            case Direction::RIGHT: std::get<1>(m_node_neighbors_direction[i]) = j; break;
            case Direction::DOWN: std::get<2>(m_node_neighbors_direction[i]) = j; break;
            case Direction::UP: std::get<3>(m_node_neighbors_direction[i]) = j; break;
        }
    }
    else {
        switch (direction) {
            case Direction::LEFT:
                m_node_neighbors_direction[i] = std::make_tuple(j, -1, -1, -1);
                break;
            case Direction::RIGHT:
                m_node_neighbors_direction[i] = std::make_tuple(-1, j, -1, -1);
                break;
            case Direction::DOWN:
                m_node_neighbors_direction[i] = std::make_tuple(-1, -1, j, -1);
                break;
            case Direction::UP:
                m_node_neighbors_direction[i] = std::make_tuple(-1, -1, -1, j);
                break;
        }
    }
}

Direction Shape::get_direction(const int i, const int j) const {
    return m_shape.at(std::make_pair(i, j));
}

bool Shape::contains(const int i, const int j) const {
    return m_shape.contains(std::make_pair(i, j));
}

bool Shape::is_up(const int i, const int j) const {
    return get_direction(i, j) == Direction::UP;
}

bool Shape::is_down(const int i, const int j) const {
    return get_direction(i, j) == Direction::DOWN;
}

bool Shape::is_right(const int i, const int j) const {
    return get_direction(i, j) == Direction::RIGHT;
}

bool Shape::is_left(const int i, const int j) const {
    return get_direction(i, j) == Direction::LEFT;
}

bool Shape::is_horizontal(const int i, const int j) const {
    return is_right(i, j) || is_left(i, j);
}

bool Shape::is_vertical(const int i, const int j) const {
    return is_up(i, j) || is_down(i, j);
}

int Shape::has_node_a_left_neighbor(const int node) const {
    return std::get<0>(m_node_neighbors_direction.at(node));
}

int Shape::has_node_a_right_neighbor(const int node) const {
    return std::get<1>(m_node_neighbors_direction.at(node));
}

int Shape::has_node_a_down_neighbor(const int node) const {
    return std::get<2>(m_node_neighbors_direction.at(node));
}

int Shape::has_node_a_up_neighbor(const int node) const {
    return std::get<3>(m_node_neighbors_direction.at(node));
}

std::string Shape::to_string() const {
    std::string result = "Shape: {";
    for (auto& [pair, direction] : m_shape) {
        result += "(" + std::to_string(pair.first) + ", " + std::to_string(pair.second) + ") -> " + direction_to_string(direction) + " ";
    }
    result += "}";
    return result;
}

void Shape::print() const {
    std::cout << to_string() << std::endl;
}