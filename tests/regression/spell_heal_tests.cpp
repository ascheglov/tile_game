#include "Game.hpp"

#include "catch.hpp"
#include "test_printers.hpp"

#include "test_game_config.hpp"
#include "TestClient.hpp"

TEST_CASE("self-heal", "[game]")
{
    Game game{TestGameCfg};

    TestClient A{game, "A", {1, 1}};
    TestClient B{game, "B", {2, 2}};

    A.requestCast(Spell::Lightning, {2, 2});
    game.tick();
    game.tick();

    REQUIRE(B.m_health < 100);
    auto oldHealth = B.m_health;

    B.requestCast(Spell::SelfHeal);
    game.tick();
    game.tick();

    REQUIRE(B.m_health > oldHealth);

    B.requestCast(Spell::SelfHeal);
    game.tick();
    game.tick();

    REQUIRE(B.m_health == 100);

    B.requestCast(Spell::SelfHeal);
    game.tick();
    game.tick();

    REQUIRE(B.m_health == 100);
}