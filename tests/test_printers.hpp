#pragma once

#include <ostream>

#include "math.hpp"
#include "types.hpp"

inline std::ostream& operator<<(std::ostream& o, const Point& pt)
{
    o << '(' << pt.x << ", " << pt.y << ')';
    return o;
}

inline std::ostream& operator<<(std::ostream& o, Dir d)
{
    auto idx = static_cast<unsigned>(d);
    const char* table[] {"Right", "Up", "Left", "Down"};
    o << (idx < _countof(table) ? table[idx] : "UNDEFINED");
    return o;
}

inline std::ostream& operator<<(std::ostream& o, PlayerState ps)
{
    auto idx = static_cast<unsigned>(ps);
    const char* table[] {"Idle", "MovingOut", "MovingIn"};
    o << (idx < _countof(table) ? table[idx] : "UNDEFINED");
    return o;
}
