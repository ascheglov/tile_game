#include "Game.hpp"

#include "catch_wrap.hpp"
#include "test_printers.hpp"

#include "test_game_config.hpp"
#include "TestClient.hpp"

TEST_CASE("spawn one", "[game]")
{
    Game game{TestGameCfg};

    TestClient A(game, "A", {3, 2});

    REQUIRE(A.m_id);
    REQUIRE(A.m_name == "A");
    REQUIRE(A.m_pos == Point(3, 2));
    REQUIRE(A.seeNothing());
}

TEST_CASE("spawn two nearby", "[game]")
{
    Game game{TestGameCfg};

    TestClient A(game, "A", {3, 1});
    TestClient B(game, "B", {3, 2});

    REQUIRE(A.m_id != B.m_id);
    REQUIRE(B.m_pos == Point(3, 2));
    REQUIRE(B.see("A").m_pos == Point(3, 1));

    REQUIRE(A.see("B").m_pos == Point(3, 2));
}

TEST_CASE("spawn two apart", "[game]")
{
    Game game{TestGameCfg};

    TestClient A(game, "A", {0, 0});
    TestClient B(game, "B", {7, 7});

    REQUIRE(A.seeNothing());
    REQUIRE(B.seeNothing());
}

TEST_CASE("spawn on occupied cell", "[game]")
{
    Game game{TestGameCfg};

    TestClient A{game, "A", {1, 1}};
    TestClient B{game, "B", {2, 1}};
    REQUIRE(B.doSee("A"));

    TestClient C{game, "C", {1, 1}};
    REQUIRE_FALSE(B.doSee("A"));
    REQUIRE(B.doSee("C"));
}