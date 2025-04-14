#ifndef DRAWER_H
#define DRAWER_H

struct OGDFResult
{
    int crossings;
    int bends;
    int area;
};

OGDFResult create_drawing(const std::string inputFile);

#endif
