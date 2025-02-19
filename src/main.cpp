#include "core/graph/graph.hpp"
#include "core/graph/file_loader.hpp"
#include "orthogonal/drawing_builder.hpp"

int main() {
    SimpleGraph* graph = loadSimpleUndirectedGraphFromFile("example-graphs/g3.txt");
    make_rectilinear_drawing(*graph);
    delete graph;
    
}