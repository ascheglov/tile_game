#pragma once

#include <cassert>
#include <string>
#include "math.hpp"

struct TestCanvas
{
    int m_cx, m_cy;
    std::string m_str;

    TestCanvas(int cx, int cy) : m_cx(cx), m_cy(cy)
    {
        m_str.resize((cx + 1) * cy, '0');
        for (auto y = 0; y != cy; ++y)
            at(cx, y) = '\n';
    }

    char& at(int x, int y)
    {
        return m_str[x + y * (m_cx + 1)];
    }

    void draw(const Point& pt)
    {
        assert(pt.inside(m_cx, m_cy));
        ++at(pt.x, pt.y);
    }
};
