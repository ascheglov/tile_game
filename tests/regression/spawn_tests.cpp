#include "Game.hpp"

#include "catch_wrap.hpp"
#include "test_printers.hpp"

#include "TestClient.hpp"

TEST_CASE("spawn one", "[game]")
{
    Game game;
    game.m_spawns.set({{3, 2}});

    TestClient A(game);
    game.newPlayer(A);

    REQUIRE(A.m_id == 1);
    REQUIRE(A.m_pos == Point(3, 2));
    REQUIRE(A.see.empty());
}

TEST_CASE("spawn two nearby", "[game]")
{
    Game game;
    game.m_spawns.set({{3, 1}, {3, 2}});

    TestClient A(game);
    game.newPlayer(A);

    TestClient B(game);
    game.newPlayer(B);

    REQUIRE(B.m_id == 2);
    REQUIRE(B.m_pos == Point(3, 2));
    REQUIRE(B.see[1].pos == Point(3, 1));

    REQUIRE(A.see[2].pos == Point(3, 2));
}

TEST_CASE("spawn two apart", "[game]")
{
    Game game;
    game.m_spawns.set({{1, 1}, {10, 10}});

    TestClient A(game);
    game.newPlayer(A);

    TestClient B(game);
    game.newPlayer(B);

    REQUIRE(A.see.empty());
    REQUIRE(B.see.empty());
}