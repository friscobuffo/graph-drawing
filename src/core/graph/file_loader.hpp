#ifndef MY_GRAPH_FILE_LOADER_H
#define MY_GRAPH_FILE_LOADER_H

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>

#include "graph.hpp"

std::unique_ptr<SimpleGraph> load_simple_undirected_graph_from_file(std::string filename) {
    int nodes_number = -1;
    std::ifstream infile(filename);
    if (infile.is_open()) {
        std::string line;
        std::getline(infile, line);
        nodes_number = stoi(line);
        SimpleGraph* graph = new SimpleGraph();
        for (int i = 0; i < nodes_number; ++i)
            graph->add_node();
        int from_index, to_index;
        while (std::getline(infile, line)) {
            if (line.find("//") == 0)
                continue;
            std::istringstream iss(line);
            if (iss >> from_index >> to_index)
                graph->add_undirected_edge(from_index, to_index);
        }
        infile.close();
        return std::unique_ptr<SimpleGraph>(graph);
    }
    throw std::runtime_error("Unable to open file: " + filename);
}

template <GraphTrait T>
void save_undirected_graph_to_file(const T& graph, std::string filename) {
    std::ofstream outfile(filename);
    if (outfile.is_open()) {
        outfile << graph.size() << std::endl;
        for (int i = 0; i < graph.size(); i++) {
            for (const auto& edge : graph.get_node(i).get_edges()) {
                int j = edge.get_to();
                if (i < j)
                    outfile << i << " " << j << std::endl;
            }
        }
        outfile.close();
    } else
        throw std::runtime_error("Unable to save graph: " + filename);
}

template <GraphTrait T>
void save_directed_graph_to_file(const T& graph, std::string filename) {
    std::ofstream outfile(filename);
    if (outfile.is_open()) {
        outfile << graph.size() << std::endl;
        for (int i = 0; i < graph.size(); i++) {
            for (const auto& edge : graph.get_node(i).get_edges()) {
                int j = edge.get_to();
                outfile << i << " " << j << std::endl;
            }
        }
        outfile.close();
    } else
        throw std::runtime_error("Unable to save graph: " + filename);
}

inline void save_cycles_to_file(
    const std::vector<std::vector<size_t>>& cycles, const std::string filename
) {
    std::ofstream outfile(filename);
    if (outfile.is_open()) {
        for (const auto& cycle: cycles) {
            for (size_t i = 0; i < cycle.size(); ++i) {
                outfile << cycle[i];
                if (i != cycle.size() - 1)
                    outfile << " ";
            }
            outfile << std::endl;
        }
        outfile.close();
    } else
        throw std::runtime_error("Unable to save cycles: " + filename);
}

inline std::vector<std::vector<size_t>> load_cycles_from_file(std::string filename) {
    std::ifstream infile(filename);
    if (infile.is_open()) {
        std::vector<std::vector<size_t>> cycles;
        std::string line;
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            std::vector<size_t> cycle;
            size_t node;
            while (iss >> node) {
                cycle.push_back(node);
            }
            cycles.push_back(cycle);
        }
        infile.close();
        return cycles;
    }
    throw std::runtime_error("Unable to open file: " + filename);
}

#endif