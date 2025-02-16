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
            graph->addNode();
        int fromIndex, toIndex;
        while (std::getline(infile, line)) {
            if (line.find("//") == 0)
                continue;
            std::istringstream iss(line);
            if (iss >> fromIndex >> toIndex)
                graph->addUndirectedEdge(fromIndex, toIndex);
        }
        infile.close();
        return graph;
    }
    std::cout << "Unable to open file\n";
    return nullptr;
}
#endif