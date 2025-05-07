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

std::unique_ptr<SimpleGraph> load_simple_undirected_graph_from_txt_file(std::string filename) {
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

std::unique_ptr<SimpleGraph> load_simple_undirected_graph_from_gml_file(const std::string& filename) {
    SimpleGraph* graph = new SimpleGraph();
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file: " + filename);
    std::string line;
    bool in_node_block = false, in_edge_block = false;
    int node_id = -1, source = -1, target = -1;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;
        if (token == "node") {
            in_node_block = true;
            node_id = -1;
        }
        else if (token == "edge") {
            in_edge_block = true;
            source = target = -1;
        }
        else if (token == "id" && in_node_block) {
            iss >> node_id;
            graph->add_node(); // Add node to graph
        }
        else if (token == "source" && in_edge_block)
            iss >> source;
        else if (token == "target" && in_edge_block)
            iss >> target;
        else if (token == "]") {
            if (in_node_block)
                in_node_block = false;
            else if (in_edge_block) {
                if (source != -1 && target != -1)
                    graph->add_undirected_edge(source, target);
                in_edge_block = false;
            }
        }
    }
    file.close();
    return std::unique_ptr<SimpleGraph>(graph);
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
    const std::vector<std::vector<int>>& cycles, const std::string filename
) {
    std::ofstream outfile(filename);
    if (outfile.is_open()) {
        for (const auto& cycle: cycles) {
            for (int i = 0; i < cycle.size(); ++i) {
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

inline std::vector<std::vector<int>> load_cycles_from_file(std::string filename) {
    std::ifstream infile(filename);
    if (infile.is_open()) {
        std::vector<std::vector<int>> cycles;
        std::string line;
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            std::vector<int> cycle;
            int node;
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