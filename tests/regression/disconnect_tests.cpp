#include "Game.hpp"

#include "catch_wrap.hpp"
#include "test_printers.hpp"

#include "TestClient.hpp"

TEST_CASE("disconnect single client", "[game]")
{
    Game game;
    game.m_spawns.set({{3, 2}});

    TestClient A(game);
    REQUIRE(A.m_isConnected);

    A.requestDisconnect();
    REQUIRE_FALSE(A.m_isConnected);
}

TEST_CASE("see other disconnects", "[game]")
{
    Game game;
    game.m_spawns.set({{1, 1}, {2, 2}});

    TestClient A(game);
    TestClient B(game);

    REQUIRE_FALSE(A.see.empty());
    B.requestDisconnect();
    REQUIRE(A.see.empty());
}

TEST_CASE("other disconnects far away", "[game]")
{
    Game game;
    game.m_spawns.set({{1, 1}, {10, 10}});

    TestClient A(game);
    TestClient B(game);

    REQUIRE(A.see.empty());
    B.requestDisconnect();
    REQUIRE(A.see.empty());
}