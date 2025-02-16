#ifndef MY_TREE_ALGORITHMS_H
#define MY_TREE_ALGORITHMS_H

#include "../graph/graph.hpp"
#include "tree.hpp"

template <GraphTrait T>
SimpleTree* buildSimpleSpanningTree(const T& graph);

/*
    def compute_common_ancestor(self, node1, node2):
        while self.get_depth(node1) > self.get_depth(node2):
            node1 = self.get_parent(node1)
        while self.get_depth(node2) > self.get_depth(node1):
            node2 = self.get_parent(node2)
        while node1 != node2:
            node1 = self.get_parent(node1)
            node2 = self.get_parent(node2)
        return node1
    def get_path(self, node):
        path = []
        while node != -1:
            path.append(node)
            node = self.parent[node]
        return path[::-1]
    def is_edge_in_tree(self, node1, node2):
        return self.parent[node1] == node2 or self.parent[node2] == node1
    def __str__(self):
        return "\n".join([f"{i}: {self.parent[i]}" for i in range(self.graph.size())])

*/


#include "algorithms.ipp"

#endif