#include "Game.hpp"

#include "catch_wrap.hpp"
#include "test_printers.hpp"

#include "test_game_config.hpp"
#include "TestClient.hpp"

TEST_CASE("self-heal", "[game]")
{
    Game game{TestGameCfg};

    TestClient A{game, {1, 1}};
    TestClient B{game, {2, 2}};

    A.requestCast(Spell::Lightning, {2, 2});
    game.tick();

    REQUIRE(B.m_health < 100);
    auto oldHealth = B.m_health;

    B.requestCast(Spell::SelfHeal);
    game.tick();

    REQUIRE(B.m_health > oldHealth);

    B.requestCast(Spell::SelfHeal);
    game.tick();

    REQUIRE(B.m_health == 100);

    B.requestCast(Spell::SelfHeal);
    game.tick();

    REQUIRE(B.m_health == 100);
}