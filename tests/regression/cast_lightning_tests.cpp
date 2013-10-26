#include "Game.hpp"

#include "catch_wrap.hpp"
#include "test_printers.hpp"

#include "test_game_config.hpp"
#include "TestClient.hpp"

TEST_CASE("cast lightning", "[game]")
{
    Game game{TestGameCfg};

    TestClient A{game, {1, 1}};
    
    A.requestCast(Spell::Lightning, {2, 1});

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

    TestClient A{game, {1, 1}};
    TestClient B{game, {2, 2}};
    REQUIRE_FALSE(B.see.empty());

    A.requestCast(Spell::Lightning, {2, 1});

    REQUIRE(B.see[1].m_state == PlayerState::Casting);
    REQUIRE(B.see[1].m_spell == Spell::Lightning);

    game.tick();

    REQUIRE(B.see[1].m_state == PlayerState::Idle);
    REQUIRE(B.seeEffect({2, 1}) == Effect::Lightning);

    game.tick();

    REQUIRE(B.seeEffect({2, 1}) == Effect::None);
}

TEST_CASE("observe only cast effect (lightning)", "[game]")
{
    Game game{TestGameCfg};

    TestClient A{game, {1, 1}};
    TestClient B{game, {3, 3}};

    REQUIRE(B.see.empty());
    A.requestCast(Spell::Lightning, {2, 2});
    game.tick();

    REQUIRE(B.seeEffect({2, 2}) == Effect::Lightning);

    game.tick();

    REQUIRE(B.seeEffect({2, 2}) == Effect::None);
}

TEST_CASE("see nothing (lightning cast)", "[game]")
{
    Game game{TestGameCfg};

    TestClient A{game, {1, 1}};
    TestClient B{game, {4, 4}};

    REQUIRE(B.see.empty());
    A.requestCast(Spell::Lightning, {2, 2});
    game.tick();

    REQUIRE(B.seeEffect({2, 2}) == Effect::None);
}

TEST_CASE("spawn and see cast", "[game]")
{
    Game game{TestGameCfg};

    TestClient A{game, {1, 1}};
    A.requestCast(Spell::Lightning, {2, 2});

    TestClient B{game, {1, 2}};
    REQUIRE(B.see[1].m_state == PlayerState::Casting);
    REQUIRE(B.see[1].m_spell == Spell::Lightning);
}

TEST_CASE("spawn after cast and and see nothing", "[game]")
{
    Game game{TestGameCfg};

    TestClient A{game, {1, 1}};
    A.requestCast(Spell::Lightning, {2, 2});
    game.tick();

    TestClient B{game, {1, 2}};
    REQUIRE(B.see[1].m_state == PlayerState::Idle);
    REQUIRE(B.seeEffect({2, 2}) == Effect::None);
    REQUIRE(A.seeEffect({2, 2}) == Effect::Lightning);
}