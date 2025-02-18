#ifndef MY_SVG_DRAWER_H
#define MY_SVG_DRAWER_H

#include <sstream>
#include "polygon.hpp"
#include "linear_scale.hpp"

class SvgDrawer {
private:
    std::stringstream m_svg;
    int m_width, m_height;
    ScaleLinear m_scaleY;
public:
    SvgDrawer(int width, int height);
    void add(Point2D& point, std::string color = "black");
    void add(Line2D& line, std::string color = "black");
    void add(Polygon2D& polygon, std::string color = "black");
    void add(Path2D& path, std::string color = "black");
    void addAndSmooth(Path2D& path, std::string color = "black");
    void saveToFile(const std::string& filename);
};

#endif