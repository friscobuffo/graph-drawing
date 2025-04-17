#include "generators.hpp"

#include <random>
#include <algorithm>
#include <iostream>
#include <unordered_set>

#include "graphs_algorithms.hpp"

std::unique_ptr<SimpleGraph> generate_connected_random_graph_degree_max_4(size_t number_of_nodes, size_t number_of_edges) {
    if (number_of_edges > 2*number_of_nodes)
        throw std::runtime_error("Number of edges is too large");
    SimpleGraph* graph = new SimpleGraph();
    for (int i = 0; i < number_of_nodes; ++i)
        graph->add_node();
    std::vector<std::vector<bool>> added_edge(number_of_nodes, std::vector<bool>(number_of_nodes, false));
    graph->add_undirected_edge(0, 1);
    added_edge[0][1] = true;
    added_edge[1][0] = true;
    size_t added_edges = 1;
    for (size_t i = 2; i < number_of_nodes; ++i) {
        size_t j = rand() % i;
        while (graph->get_nodes()[j].get_edges().size() >= 4)
            j = rand() % i;
        graph->add_undirected_edge(i, j);
        ++added_edges;
        added_edge[i][j] = true;
        added_edge[j][i] = true;
    }
    while (added_edges < number_of_edges) {
        size_t i = rand() % number_of_nodes;
        size_t j = rand() % number_of_nodes;
        if (i == j || added_edge[i][j])
            continue;
        if (graph->get_nodes()[i].get_edges().size() >= 4)
            continue;
        if (graph->get_nodes()[j].get_edges().size() >= 4)
            continue;
        graph->add_undirected_edge(i, j);
        added_edge[i][j] = true;
        added_edge[j][i] = true;
        ++added_edges;
    }
    return std::unique_ptr<SimpleGraph>(graph);
}

// n*m grid, n, m > 1
std::unique_ptr<SimpleGraph> generate_grid_graph(size_t n, size_t m) {
    int num_nodes = 2 * n + 2 * m - 4;
    SimpleGraph *graph = new SimpleGraph();
    for (int i = 0; i < num_nodes; ++i)
        graph->add_node();
    for (int i = 0; i < num_nodes - 1; ++i)
        graph->add_undirected_edge(i, i + 1);
    graph->add_undirected_edge(0, num_nodes - 1);
    for (int i = 1; i < n - 1; ++i)
        graph->add_undirected_edge(i, (2 * n) + m - i - 3);
    m -= 2;
    for (int i = 0; i < m; ++i)
        graph->add_undirected_edge(n + i, (2 * n) + (2 * m) - i - 1);
    return std::unique_ptr<SimpleGraph>(graph);
}

// num_nodes > 1
std::unique_ptr<SimpleGraph> generate_triangle_graph(size_t num_nodes) {
    num_nodes = 3 * num_nodes;
    SimpleGraph *graph = new SimpleGraph();
    for (int i = 0; i < num_nodes; ++i)
        graph->add_node();
    for (int i = 0; i < num_nodes - 3; ++i) {
        if (i % 3 == 2) {
            graph->add_undirected_edge(i, i + 3);
            graph->add_undirected_edge(i + 3, i - 2);
        }
        else {
            graph->add_undirected_edge(i, i + 3);
            graph->add_undirected_edge(i + 1, i + 3);
        }
    }
    return std::unique_ptr<SimpleGraph>(graph);
}