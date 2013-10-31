#include "Game.hpp"

#include "catch_wrap.hpp"
#include "test_printers.hpp"

#include "test_game_config.hpp"
#include "TestClient.hpp"

TEST_CASE("disconnect single client", "[game]")
{
    Game game{TestGameCfg};

    TestClient A(game, "A", {3, 2});
    REQUIRE(A.m_isConnected);

    A.requestDisconnect();
    game.tick();
    REQUIRE_FALSE(A.m_isConnected);
}

TEST_CASE("see other disconnects", "[game]")
{
    Game game{TestGameCfg};

    TestClient A(game, "A", {1, 1});
    TestClient B(game, "B", {2, 2});

    REQUIRE_FALSE(A.seeNothing());
    B.requestDisconnect();
    game.tick();
    REQUIRE(A.seeNothing());
}

TEST_CASE("other disconnects far away", "[game]")
{
    Game game{TestGameCfg};

    TestClient A(game, "A", {1, 1});
    TestClient B(game, "B", {7, 7});

    REQUIRE(A.seeNothing());
    B.requestDisconnect();
    game.tick();
    REQUIRE(A.seeNothing());
}