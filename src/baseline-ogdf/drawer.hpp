#ifndef DRAWER_H
#define DRAWER_H

struct OGDFResult {
    int crossings;
    int bends;
    int area;
};

OGDFResult create_drawing(
    const SimpleGraph& graph,
    const std::string svg_output_filename = "",
    const std::string gml_output_filename = ""
);

#endif
