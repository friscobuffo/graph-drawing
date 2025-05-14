#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include "file_loader.hpp"

std::unique_ptr<Graph> load_graph_from_txt_file(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile)
        throw std::runtime_error("Could not open file: " + filename);
    auto graph = std::make_unique<Graph>();
    std::string line;
    enum Section { NONE, NODES, EDGES } section = NONE;
    while (std::getline(infile, line)) {
        if (line == "nodes:") {
            section = NODES;
        } else if (line == "edges:") {
            section = EDGES;
        } else if (!line.empty()) {
            std::istringstream iss(line);
            if (section == NODES) {
                int node_id;
                if (iss >> node_id)
                    graph->add_node(node_id);
            } else if (section == EDGES) {
                int from, to;
                if (iss >> from >> to)
                    graph->add_edge(from, to);
            }
        }
    }
    infile.close();
    return graph;
}

std::unique_ptr<Graph> load_undirected_graph_from_gml_file(const std::string& filename) {
    auto graph = std::make_unique<Graph>();
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
            graph->add_node(node_id); // Add node to graph
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
    return graph;
}

void save_graph_to_file(const Graph& graph, const std::string& filename) {
    std::ofstream outfile(filename);
    if (!outfile)
        throw std::runtime_error("Could not write to file: " + filename);
    outfile << "nodes:\n";
    for (auto node : graph.get_nodes())
        outfile << node.get_id() << '\n';
    outfile << "edges:\n";
    for (const auto& edge : graph.get_edges())
        outfile << edge.get_from().get_id() << ' ' << edge.get_to().get_id() << '\n';
    outfile.close();
}

void save_cycles_to_file(
    const std::vector<std::vector<int>>& cycles, const std::string& filename
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

std::vector<std::vector<int>> load_cycles_from_file(const std::string& filename) {
    std::ifstream infile(filename);
    if (infile.is_open()) {
        std::vector<std::vector<int>> cycles;
        std::string line;
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            std::vector<int> cycle;
            int node;
            while (iss >> node)
                cycle.push_back(node);
            cycles.push_back(cycle);
        }
        infile.close();
        return cycles;
    }
    throw std::runtime_error("Unable to open file: " + filename);
}















