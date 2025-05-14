#include "file_loader.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

std::unique_ptr<Shape> load_shape_from_file(const std::string& filename) {
    auto shape = std::make_unique<Shape>();
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
        return shape;
    }
    throw std::runtime_error("Unable to open shape file: " + filename);
}

void save_shape_to_file(const Graph& graph, const Shape& shape, std::string filename) {
    std::ofstream outfile(filename);
    if (outfile.is_open()) {
        for (const auto& node : graph.get_nodes()) {
            for (const auto& edge : node.get_edges()) {
                int neighbor_id = edge.get_to().get_id();
                outfile << node.get_id() << " " << neighbor_id << " ";
                outfile << direction_to_string(shape.get_direction(node.get_id(), neighbor_id)) << std::endl;
            }
        }
        outfile.close();
    } else
        throw std::runtime_error("Unable to save shape: " + filename);
}