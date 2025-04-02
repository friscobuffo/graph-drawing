#ifndef MY_GRAPH_FILE_LOADER_H
#define MY_GRAPH_FILE_LOADER_H

#include <fstream>
#include <sstream>

#include "graph.hpp"

SimpleGraph* loadSimpleUndirectedGraphFromFile(std::string filename) {
    int nodesNumber{};
    std::ifstream infile(filename);
    if (infile.is_open()) {
        std::string line;
        std::getline(infile, line);
        nodesNumber = stoi(line);
        SimpleGraph* graph = new SimpleGraph();
        for (int i = 0; i < nodesNumber; ++i)
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
        return graph;
    }
    std::cout << "Unable to open file\n";
    return nullptr;
}

template <GraphTrait T>
void save_undirected_graph_to_file(const T& graph, std::string filename) {
    std::ofstream outfile(filename);
    if (outfile.is_open()) {
        outfile << graph.size() << std::endl;
        for (int i = 0; i < graph.size(); i++) {
            for (auto& edge : graph.get_node(i).get_edges()) {
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