#include "Game.hpp"

#include "catch_wrap.hpp"
#include "test_printers.hpp"

#include "TestClient.hpp"

TEST_CASE("disconnect single client", "[game]")
{
    Game game;

    TestClient A(game, {3, 2});
    REQUIRE(A.m_isConnected);

    A.requestDisconnect();
    REQUIRE_FALSE(A.m_isConnected);
}

TEST_CASE("see other disconnects", "[game]")
{
    Game game;

    TestClient A(game, {1, 1});
    TestClient B(game, {2, 2});

    REQUIRE_FALSE(A.see.empty());
    B.requestDisconnect();
    REQUIRE(A.see.empty());
}

TEST_CASE("other disconnects far away", "[game]")
{
    Game game;

    TestClient A(game, {1, 1});
    TestClient B(game, {7, 7});

    REQUIRE(A.see.empty());
    B.requestDisconnect();
    REQUIRE(A.see.empty());
}