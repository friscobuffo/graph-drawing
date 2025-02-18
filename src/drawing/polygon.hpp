#ifndef MY_POLYGON_H
#define MY_POLYGON_H

#include <vector>
#include <iostream>

class Point2D {
public:
    double x, y;
    Point2D(double x, double y);
    double distance(const Point2D& other);
    Point2D operator+(const Point2D& other) const {
        return Point2D(x + other.x, y + other.y);
    }
    Point2D operator-(const Point2D& other) const {
        return Point2D(x + other.x, y + other.y);
    }
    Point2D operator*(const double scalar) const {
        return Point2D(x * scalar, y * scalar);
    }
    Point2D operator/(const double scalar) const {
        return Point2D(x / scalar, y / scalar);
    }
    bool operator==(const Point2D& other) const {
        return x == other.x && y == other.y;
    }
    bool operator!=(const Point2D& other) const {
        return !(*this == other);
    }
    // Operatore di confronto lessicografico (prima x, poi y)
    bool operator<(const Point2D& p) const {
        return x < p.x || (x == p.x && y < p.y);
    }
    friend std::ostream& operator<<(std::ostream& os, const Point2D& p) {
        os << "(" << p.x << ", " << p.y << ")";
        return os;
    }
};

class Path2D {
public:
    std::vector<Point2D> points;
    void addPoint(Point2D& p) {
        points.push_back(p);
    }
};

class Line2D {
public:
    Point2D p1, p2;
    Line2D(Point2D& p1, Point2D& p2);
    bool isPointOnLine(Point2D& p);
    bool isIntersecting(Line2D& l);
    bool operator==(const Line2D& other) const {
        return p1 == other.p1 && p2 == other.p2;
    }
    bool operator!=(const Line2D& other) const {
        return !(*this == other);
    }
    friend std::ostream& operator<<(std::ostream& os, const Line2D& l) {
        os << "[" << l.p1 << " - " << l.p2 << "]";
        return os;
    }
};

class Polygon2D {
private:
    std::vector<Point2D> m_points;
public:
    Polygon2D(std::vector<Point2D>& points);
    std::vector<Point2D>& getPoints();
    bool isOnBoundary(Point2D& p);
    bool isInside(Point2D& p);
    bool isInside(Line2D& l);
    Path2D computePathInside(Point2D& p1, Point2D& p2);
    std::vector<Point2D> computeConvexHull();
};

#endif