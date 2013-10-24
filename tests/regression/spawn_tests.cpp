#include "Game.hpp"

#include "catch_wrap.hpp"
#include "test_printers.hpp"

#include "TestClient.hpp"

TEST_CASE("spawn one", "[game]")
{
    Game game;

    TestClient A(game, {3, 2});

    REQUIRE(A.m_id == 1);
    REQUIRE(A.m_pos == Point(3, 2));
    REQUIRE(A.see.empty());
}

TEST_CASE("spawn two nearby", "[game]")
{
    Game game;

    TestClient A(game, {3, 1});
    TestClient B(game, {3, 2});

    REQUIRE(B.m_id == 2);
    REQUIRE(B.m_pos == Point(3, 2));
    REQUIRE(B.see[1].m_pos == Point(3, 1));

    REQUIRE(A.see[2].m_pos == Point(3, 2));
}

TEST_CASE("spawn two apart", "[game]")
{
    Game game;

    TestClient A(game, {0, 0});
    TestClient B(game, {7, 7});

    REQUIRE(A.see.empty());
    REQUIRE(B.see.empty());
}

TEST_CASE("spawn on occupied cell", "[game]")
{
    Game game;

    TestClient A{game, {1, 1}};
    TestClient B{game, {2, 1}};
    REQUIRE(B.see.count(1) == 1);

    TestClient C{game, {1, 1}};
    REQUIRE(B.see.count(1) == 0);
    REQUIRE(B.see.count(3) == 1);
}