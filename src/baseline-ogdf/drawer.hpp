#ifndef DRAWER_H
#define DRAWER_H

struct OGDFResult {
    int crossings;
    int bends;
    int area;
    int total_edge_length;
    int max_bends_per_edge;
    int max_edge_length;
    double bends_standard_deviation;
    double edge_length_standard_deviation;
};

OGDFResult create_drawing(
    const SimpleGraph& graph,
    const std::string svg_output_filename = "",
    const std::string gml_output_filename = ""
);

#endif
