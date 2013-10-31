#include "Game.hpp"

#include "catch_wrap.hpp"
#include "test_printers.hpp"

#include "test_game_config.hpp"
#include "TestClient.hpp"

TEST_CASE("cast lightning", "[game]")
{
    Game game{TestGameCfg};

    TestClient A{game, "A", {1, 1}};
    
    A.requestCast(Spell::Lightning, {2, 1});
    game.tick();

    REQUIRE(A.m_state == PlayerState::Casting);
    REQUIRE(A.m_spell == Spell::Lightning);

    game.tick();

    REQUIRE(A.m_state == PlayerState::Idle);
    REQUIRE(A.seeEffect({2, 1}) == Effect::Lightning);

    game.tick();

    REQUIRE(A.seeEffect({2, 1}) == Effect::None);
}

TEST_CASE("observe cast lightning", "[game]")
{
    Game game{TestGameCfg};

    TestClient A{game, "A", {1, 1}};
    TestClient B{game, "B", {2, 2}};
    REQUIRE_FALSE(B.seeNothing());

    A.requestCast(Spell::Lightning, {2, 1});
    game.tick();

    REQUIRE(B.see("A").m_state == PlayerState::Casting);
    REQUIRE(B.see("A").m_spell == Spell::Lightning);

    game.tick();

    REQUIRE(B.see("A").m_state == PlayerState::Idle);
    REQUIRE(B.seeEffect({2, 1}) == Effect::Lightning);

    game.tick();

    REQUIRE(B.seeEffect({2, 1}) == Effect::None);
}

TEST_CASE("observe only cast effect (lightning)", "[game]")
{
    Game game{TestGameCfg};

    TestClient A{game, "A", {1, 1}};
    TestClient B{game, "B", {3, 3}};

    REQUIRE(B.seeNothing());
    A.requestCast(Spell::Lightning, {2, 2});
    game.tick();
    game.tick();

    REQUIRE(B.seeEffect({2, 2}) == Effect::Lightning);

    game.tick();

    REQUIRE(B.seeEffect({2, 2}) == Effect::None);
}

TEST_CASE("see nothing (lightning cast)", "[game]")
{
    Game game{TestGameCfg};

    TestClient A{game, "A", {1, 1}};
    TestClient B{game, "B", {4, 4}};

    REQUIRE(B.seeNothing());
    A.requestCast(Spell::Lightning, {2, 2});
    game.tick();
    game.tick();

    REQUIRE(B.seeEffect({2, 2}) == Effect::None);
}

TEST_CASE("spawn and see cast", "[game]")
{
    Game game{TestGameCfg};

    TestClient A{game, "A", {1, 1}};
    A.requestCast(Spell::Lightning, {2, 2});
    game.tick();

    TestClient B{game, "B", {1, 2}};
    REQUIRE(B.see("A").m_state == PlayerState::Casting);
    REQUIRE(B.see("A").m_spell == Spell::Lightning);
}

TEST_CASE("spawn after cast and and see nothing", "[game]")
{
    Game game{TestGameCfg};

    TestClient A{game, "A", {1, 1}};
    A.requestCast(Spell::Lightning, {2, 2});
    game.tick();
    game.tick();

    TestClient B{game, "B", {1, 2}};
    REQUIRE(B.see("A").m_state == PlayerState::Idle);
    REQUIRE(B.seeEffect({2, 2}) == Effect::None);
    REQUIRE(A.seeEffect({2, 2}) == Effect::Lightning);
}