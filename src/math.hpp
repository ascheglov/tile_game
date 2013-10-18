#pragma once

#include <cmath>

struct Point
{
    Point() {}
    Point(int x, int y) : x(x), y(y) {}
    int x, y;

    bool positive() const { return x >= 0 && y >= 0; }
    bool inside(int cx, int cy) const { return positive() && x < cx && y < cy; }
};

inline bool operator==(const Point& lhs, const Point& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline bool operator!=(const Point& lhs, const Point& rhs) { return !(lhs == rhs); }

inline int distance(const Point& a, const Point& b)
{
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}
