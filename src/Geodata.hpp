#pragma once

#include <cassert>
#include <vector>

#include "types.hpp"
#include "math.hpp"

struct Geodata
{
    Geodata(int cx, int cy)
        : m_table(cx * cy)
        , m_cx{cx}
        , m_cy{cy}
    {}

    void addWall(const Point& pt)
    {
        if (pt.x > 0) m_table[idx(moveRel(pt, Dir::Left))] |= dirMask(Dir::Right);
        if (pt.y > 0) m_table[idx(moveRel(pt, Dir::Up))] |= dirMask(Dir::Down);
        if (pt.x < m_cx - 1) m_table[idx(moveRel(pt, Dir::Right))] |= dirMask(Dir::Left);
        if (pt.y < m_cy - 1) m_table[idx(moveRel(pt, Dir::Down))] |= dirMask(Dir::Up);
    }

    bool canMove(const Point& pt, Dir moveDir) const
    {
        assert(pt.inside(m_cx, m_cy));
        auto cellInfo = m_table[idx(pt)];
        return (cellInfo & dirMask(moveDir)) == 0;
    }

private:
    int idx(const Point& pt) const
    {
        return pt.x + pt.y * m_cx;
    }

    static std::uint8_t dirMask(Dir direction)
    {
        return 1 << static_cast<int>(direction);
    }

    std::vector<std::uint8_t> m_table;
    int m_cx, m_cy;
};

