#ifndef MY_GRAPH_FILE_LOADER_H
#define MY_GRAPH_FILE_LOADER_H

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <memory>

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
        int fromIndex, toIndex;
        while (std::getline(infile, line)) {
            if (line.find("//") == 0)
                continue;
            std::istringstream iss(line);
            if (iss >> fromIndex >> toIndex)
                graph->add_undirected_edge(fromIndex, toIndex);
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
        std::cout << "Unable to open file\n";
}

#endif