#include "Game.hpp"

#include "catch_wrap.hpp"
#include "test_printers.hpp"

#include "TestClient.hpp"

TEST_CASE("spawn one", "[game]")
{
    Game game;
    game.setSpawns({{3, 2}});

    TestClient A(game);

    REQUIRE(A.m_id == 1);
    REQUIRE(A.m_pos == Point(3, 2));
    REQUIRE(A.see.empty());
}

TEST_CASE("spawn two nearby", "[game]")
{
    Game game;
    game.setSpawns({{3, 1}, {3, 2}});

    TestClient A(game);
    TestClient B(game);

    REQUIRE(B.m_id == 2);
    REQUIRE(B.m_pos == Point(3, 2));
    REQUIRE(B.see[1].m_pos == Point(3, 1));

    REQUIRE(A.see[2].m_pos == Point(3, 2));
}

TEST_CASE("spawn two apart", "[game]")
{
    Game game;
    game.setSpawns({{0, 0}, {7, 7}});

    TestClient A(game);
    TestClient B(game);

    REQUIRE(A.see.empty());
    REQUIRE(B.see.empty());
}