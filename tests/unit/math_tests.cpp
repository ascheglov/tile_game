#include "math.hpp"

#include "catch_wrap.hpp"
#include "test_printers.hpp"
#include "TestCanvas.hpp"

TEST_CASE("distance", "[math]")
{
    Point origin{4, 3};
    TestCanvas cnv{8, 7};
    for (auto y = 0; y != cnv.m_cy; ++y)
        for (auto x = 0; x != cnv.m_cx; ++x)
        {
            Point pt{x, y};
            if (distance(origin, pt) <= 2)
                cnv.draw(pt);
        }

    REQUIRE(cnv.m_str == ""
        "00000000\n"
        "00001000\n"
        "00011100\n"
        "00111110\n"
        "00011100\n"
        "00001000\n"
        "00000000\n"
        );
}