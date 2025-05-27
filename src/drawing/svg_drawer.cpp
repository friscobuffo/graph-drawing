#include "drawing/svg_drawer.hpp"

#include <fstream>

SvgDrawer::SvgDrawer(int width, int height)
: m_width(width), m_height(height), m_scaleY(0, height, height, 0) {
    m_svg << "<svg height=\"" << m_height << "\" width=\"" << m_width << "\" ";
    m_svg << "xmlns=\"http://www.w3.org/2000/svg\">" << std::endl;
    m_svg << "<rect width=\"" << m_width << "\" height=\"" << m_height << "\" ";
    m_svg << "x=\"0\" y=\"0\" fill=\"white\" />";
}

void SvgDrawer::add(Point2D& point, std::string color, std::string label) {
    m_svg << "<circle cx=\"" << point.x << "\" cy=\"" << m_scaleY.map(point.y) << "\" ";
    m_svg << "r=\"4\" fill=\"" << color << "\" />" << std::endl;
    if (!label.empty()) {
        m_svg << "<text x=\"" << point.x+2 << "\" y=\"" << m_scaleY.map(point.y+2) << "\" ";
        m_svg << "font-family=\"Verdana\" font-size=\"16\" fill=\"black\">" << label << "</text>" << std::endl;
    }
}

void SvgDrawer::add(Line2D& line, std::string color) {
    m_svg << "<line x1=\"" << line.p1.x << "\" y1=\"" << m_scaleY.map(line.p1.y) << "\" ";
    m_svg << "x2=\"" << line.p2.x << "\" y2=\"" << m_scaleY.map(line.p2.y) << "\" ";
    m_svg << "style=\"stroke:" << color << ";stroke-width:1\" />" << std::endl;
}

void SvgDrawer::add(Polygon2D& polygon, std::string color) {
    m_svg << "<polygon points=\"";
    for (const auto& point : polygon.getPoints())
        m_svg << point.x << "," << m_scaleY.map(point.y) << " ";
    m_svg << "\" style=\"fill:white;stroke:" << color << ";stroke-width:1\" />" << std::endl;
    for (Point2D& point : polygon.getPoints())
        add(point);
}

void SvgDrawer::add(Path2D& path, std::string color) {
    m_svg << "<path d=\"";
    for (int i = 0; i < path.points.size(); i++) {
        if (i == 0)
            m_svg << "M" << path.points[i].x << "," << m_scaleY.map(path.points[i].y) << " ";
        else
            m_svg << "L" << path.points[i].x << "," << m_scaleY.map(path.points[i].y) << " ";
    }
    m_svg << "\" style=\"fill:none;stroke:" << color << ";stroke-width:1\" />" << std::endl;
}

void SvgDrawer::addAndSmooth(Path2D& path, std::string color) {
    m_svg << "<path d=\"";
    for (int i = 0; i < path.points.size(); i++) {
        if (i == 0)
            m_svg << "M" << path.points[i].x << "," << m_scaleY.map(path.points[i].y) << " ";
        else
            m_svg << "T" << path.points[i].x << "," << m_scaleY.map(path.points[i].y) << " ";
    }
    m_svg << "\" style=\"fill:none;stroke:" << color << ";stroke-width:1\" />" << std::endl;
}

void SvgDrawer::saveToFile(const std::string& filename) {
    std::ofstream svgFile(filename);
    svgFile << m_svg.str();
    svgFile << "</svg>" << std::endl;
}