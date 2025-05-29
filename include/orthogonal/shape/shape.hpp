#ifndef MY_SHAPE_HPP
#define MY_SHAPE_HPP

#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "core/utils.hpp"

enum class Direction { LEFT, RIGHT, UP, DOWN };

const std::string direction_to_string(const Direction direction);

Direction string_to_direction(const std::string& direction);

Direction opposite_direction(const Direction direction);

Direction rotate_90_degrees(const Direction direction);

class Shape {
 private:
  std::unordered_map<std::pair<int, int>, Direction, int_pair_hash> m_shape;
  std::unordered_map<int, std::tuple<int, int, int, int>>
      m_node_neighbors_direction;

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
  bool has_node_a_right_neighbor(const int node) const;
  bool has_node_a_left_neighbor(const int node) const;
  bool has_node_a_up_neighbor(const int node) const;
  bool has_node_a_down_neighbor(const int node) const;
  int get_right_neighbor(const int node) const;
  int get_left_neighbor(const int node) const;
  int get_up_neighbor(const int node) const;
  int get_down_neighbor(const int node) const;
  void remove_direction(const int i, const int j);
  std::string to_string() const;
  void print() const;
};

#endif