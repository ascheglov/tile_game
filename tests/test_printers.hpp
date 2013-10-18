#pragma once

#include <ostream>

#include "math.hpp"

inline std::ostream& operator<<(std::ostream& o, const Point& pt)
{
    o << '(' << pt.x << ", " << pt.y << ')';
    return o;
}
