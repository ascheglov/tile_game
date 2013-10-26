#include "Game.hpp"

#include "catch_wrap.hpp"
#include "test_printers.hpp"

#include "test_game_config.hpp"
#include "TestClient.hpp"

TEST_CASE("hit with lightning", "[game]")
{
    Game game{TestGameCfg};

    TestClient A{game, {1, 1}};
    TestClient B{game, {2, 2}};

    REQUIRE(B.m_health == 100);
    REQUIRE_FALSE(A.see.empty());

    A.requestCast(Spell::Lightning, {2, 2});
    game.tick();
    game.tick();

    REQUIRE(B.m_health < 100);

    A.requestCast(Spell::Lightning, {2, 2});
    game.tick();
    game.tick();

    REQUIRE_FALSE(B.m_isConnected);
    REQUIRE(A.see.empty());
}