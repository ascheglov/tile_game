#pragma once

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <functional>

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

namespace std
{
    template<>
    struct hash<Point>
    {
        std::size_t operator()(const Point& pt) const
        {
            return static_cast<std::size_t>(pt.x ^ (pt.y << 16));
        }
    };
}

inline int distance(const Point& a, const Point& b)
{
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

enum class Dir
{
    Right, Up, Left, Down,
};

const auto DirCount = 4;

inline Dir oppositeDir(Dir direction)
{
    auto dirIdx = static_cast<unsigned>(direction);
    return static_cast<Dir>((dirIdx + DirCount / 2) % DirCount);
}

inline Dir leftDir(Dir direction)
{
    auto dirIdx = static_cast<unsigned>(direction);
    return static_cast<Dir>((dirIdx + 1) % DirCount);
}

inline Dir rightDir(Dir direction)
{
    auto dirIdx = static_cast<unsigned>(direction);
    return static_cast<Dir>((dirIdx - 1) % DirCount);
}

inline Point moveRel(const Point& pt, Dir dir, int distance = 1)
{
    int ofsX[] {1, 0, -1, 0};
    int ofsY[] {0, -1, 0, 1};
    auto moved = pt;
    moved.x += ofsX[(int)dir] * distance;
    moved.y += ofsY[(int)dir] * distance;
    return moved;
}

template<typename Callback>
void forArc180(const Point& origin, int radius, Dir dir, Callback&& callback)
{
    auto pt = moveRel(origin, leftDir(dir), radius);;    
    for (auto n = 0; n < radius; ++n)
    {
        callback(pt);
        pt = moveRel(pt, dir);
        pt = moveRel(pt, rightDir(dir));
    }

    for (auto n = 0; n < radius + 1; ++n)
    {
        callback(pt);
        pt = moveRel(pt, oppositeDir(dir));
        pt = moveRel(pt, rightDir(dir));
    }
}

inline bool isInFov180(const Point& origin, Dir dir, const Point& pt)
{
    switch (dir)
    {
    case Dir::Right: return pt.x >= origin.x;
    case Dir::Up: return pt.y <= origin.y;
    case Dir::Left: return pt.x <= origin.x;
    case Dir::Down: return pt.y >= origin.y;
    }

    return false;
}

inline bool isOnArc180(const Point& origin, int radius, Dir dir, const Point& pt)
{
    if (distance(origin, pt) != radius)
        return false;

    return isInFov180(origin, dir, pt);
}