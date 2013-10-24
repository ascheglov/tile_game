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

TEST_CASE("move", "[math]")
{
    Point pt{1, 11};
    CHECK(moveRel(pt, Dir::Right) == Point(2, 11));
    CHECK(moveRel(pt, Dir::Up) == Point(1, 10));
    CHECK(moveRel(pt, Dir::Left) == Point(0, 11));
    CHECK(moveRel(pt, Dir::Down) == Point(1, 12));
}

TEST_CASE("arc180", "[math]")
{
    Point origin{4, 3};
    auto R = 2;
    TestCanvas cnv{8, 7};

    auto&& drawArc = [&](Dir dir)
    {
        forArc180(origin, R, dir, [&](Point pt) { cnv.draw(pt); });
    };

    SECTION("Right")
    {
        drawArc(Dir::Right);
        REQUIRE(cnv.m_str == ""
            "00000000\n"
            "00001000\n"
            "00000100\n"
            "00000010\n"
            "00000100\n"
            "00001000\n"
            "00000000\n"
            );
    }

    SECTION("Up")
    {
        drawArc(Dir::Up);
        REQUIRE(cnv.m_str == ""
            "00000000\n"
            "00001000\n"
            "00010100\n"
            "00100010\n"
            "00000000\n"
            "00000000\n"
            "00000000\n"
            );
    }

    SECTION("Left")
    {
        drawArc(Dir::Left);
        REQUIRE(cnv.m_str == ""
            "00000000\n"
            "00001000\n"
            "00010000\n"
            "00100000\n"
            "00010000\n"
            "00001000\n"
            "00000000\n"
            );
    }

    SECTION("Down")
    {
        drawArc(Dir::Down);
        REQUIRE(cnv.m_str == ""
            "00000000\n"
            "00000000\n"
            "00000000\n"
            "00100010\n"
            "00010100\n"
            "00001000\n"
            "00000000\n"
            );
    }
}