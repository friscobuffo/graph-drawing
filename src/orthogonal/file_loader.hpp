#ifndef MY_SHAPE_FILE_LOADER_H
#define MY_SHAPE_FILE_LOADER_H

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>

#include "shape.hpp"
#include "../core/graph/graph.hpp"

std::unique_ptr<Shape> load_shape_from_file(std::string filename) {
    Shape* shape = new Shape();
    std::ifstream infile(filename);
    if (infile.is_open()) {
        std::string line;
        int from_index, to_index;
        std::string direction;
        while (std::getline(infile, line)) {
            if (line.find("//") == 0)
                continue;
            std::istringstream iss(line);
            if (iss >> from_index >> to_index >> direction)
                shape->set_direction(from_index, to_index, string_to_direction(direction));
        }
        infile.close();
        return std::unique_ptr<Shape>(shape);
    }
    throw std::runtime_error("Unable to open shape file: " + filename);
}

template <GraphTrait T>
void save_shape_to_file(const T& graph, const Shape& shape, std::string filename) {
    std::ofstream outfile(filename);
    if (outfile.is_open()) {
        for (int i = 0; i < graph.size(); i++) {
            for (const auto& edge : graph.get_node(i).get_edges()) {
                int j = edge.get_to();
                outfile << i << " " << j << " ";
                outfile << direction_to_string(shape.get_direction(i, j)) << std::endl;
            }
        }
        outfile.close();
    } else
        throw std::runtime_error("Unable to save shape: " + filename);
}

#endif