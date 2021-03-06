#include "Game.hpp"

#include "catch.hpp"
#include "test_printers.hpp"

#include "test_game_config.hpp"
#include "TestClient.hpp"

TEST_CASE("move alone", "[game]")
{
    Game game{TestGameCfg};

    TestClient A(game, "A", {3, 2});
    REQUIRE(A.m_state == PlayerState::Idle);

    A.requestMove(Dir::Right);
    game.tick();

    REQUIRE(A.m_state == PlayerState::MovingOut);
    REQUIRE(A.m_moveDir == Dir::Right);

    game.tick();

    REQUIRE(A.m_state == PlayerState::MovingIn);
    REQUIRE(A.m_pos == Point(4, 2));

    game.tick();

    REQUIRE(A.m_state == PlayerState::Idle);
}

TEST_CASE("observe move", "[game]")
{
    Game game{TestGameCfg};

    TestClient A(game, "A", {3, 2});
    TestClient B(game, "B", {3, 3});

    A.requestMove(Dir::Right);
    game.tick();

    REQUIRE(B.see("A").m_state == PlayerState::MovingOut);
    REQUIRE(B.see("A").m_moveDir == Dir::Right);

    game.tick();

    REQUIRE(B.see("A").m_state == PlayerState::MovingIn);
    REQUIRE(B.see("A").m_pos == Point(4, 2));

    game.tick();

    REQUIRE(B.see("A").m_state == PlayerState::Idle);
}

TEST_CASE("spawn and see move out", "[game]")
{
    Game game{TestGameCfg};

    TestClient A(game, "A", {3, 2});

    A.requestMove(Dir::Right);
    game.tick();

    TestClient B(game, "B", {3, 3});

    REQUIRE(B.see("A").m_state == PlayerState::MovingOut);
    REQUIRE(B.see("A").m_moveDir == Dir::Right);
}

TEST_CASE("spawn and see move in", "[game]")
{
    Game game{TestGameCfg};

    TestClient A(game, "A", {3, 2});

    A.requestMove(Dir::Right);
    game.tick();
    game.tick();

    TestClient B(game, "B", {3, 3});

    REQUIRE(B.see("A").m_state == PlayerState::MovingIn);
    REQUIRE(B.see("A").m_moveDir == Dir::Right);
    REQUIRE(B.see("A").m_pos == Point(4, 2));
}

TEST_CASE("spawn after move and see idle", "[game]")
{
    Game game{TestGameCfg};

    TestClient A(game, "A", {3, 2});

    A.requestMove(Dir::Right);
    game.tick();
    game.tick();
    game.tick();

    TestClient B(game, "B", {3, 3});
    REQUIRE(B.see("A").m_state == PlayerState::Idle);
}

TEST_CASE("move out of view area", "[game]")
{
    Game game{TestGameCfg};

    TestClient A(game, "A", {3, 2});
    TestClient B(game, "B", {1, 2});

    REQUIRE_FALSE(A.seeNothing());
    REQUIRE_FALSE(B.seeNothing());

    A.requestMove(Dir::Right);
    game.tick();
    game.tick();

    REQUIRE(A.seeNothing());
    REQUIRE(B.seeNothing());

    game.tick(); // just in case
    REQUIRE(A.seeNothing());
    REQUIRE(B.seeNothing());
}

TEST_CASE("move into view area", "[game]")
{
    Game game{TestGameCfg};

    TestClient A(game, "A", {4, 2});
    TestClient B(game, "B", {1, 2});

    REQUIRE(A.seeNothing());
    REQUIRE(B.seeNothing());

    A.requestMove(Dir::Left);
    game.tick();
    game.tick();

    REQUIRE_FALSE(A.seeNothing());
    REQUIRE(A.see("B").m_state == PlayerState::Idle);
    REQUIRE(A.see("B").m_pos == Point(1, 2));

    REQUIRE_FALSE(B.seeNothing());
    REQUIRE(B.see("A").m_state == PlayerState::MovingIn);
    REQUIRE(B.see("A").m_moveDir == Dir::Left);
    REQUIRE(B.see("A").m_pos == Point(3, 2));

    game.tick();
    REQUIRE_FALSE(B.seeNothing());
}

TEST_CASE("move not in view area", "[game]")
{
    Game game{TestGameCfg};

    TestClient A(game, "A", {1, 1});
    TestClient B(game, "B", {5, 5});

    REQUIRE(B.seeNothing());
    A.requestMove(Dir::Right);
    game.tick();
    game.tick();
    game.tick();
    REQUIRE(B.seeNothing());
}

TEST_CASE("move across world boundaries", "[game]")
{
    Game game{TestGameCfg};

    TestClient A(game, "A", {0, 0});
    A.requestMove(Dir::Left);
    game.tick();
    REQUIRE(A.m_state == PlayerState::Idle);
    A.requestMove(Dir::Up);
    game.tick();
    REQUIRE(A.m_state == PlayerState::Idle);

    TestClient B(game, "B", {7, 7});
    B.requestMove(Dir::Right);
    game.tick();
    REQUIRE(B.m_state == PlayerState::Idle);
    B.requestMove(Dir::Down);
    game.tick();
    REQUIRE(B.m_state == PlayerState::Idle);
}

TEST_CASE("ignore move request when moving", "[game]")
{
    Game game{TestGameCfg};

    TestClient A(game, "A", {1, 1});
    A.requestMove(Dir::Right);
    game.tick();

    A.requestMove(Dir::Left);
    game.tick();
    REQUIRE(A.m_state == PlayerState::MovingIn);
    REQUIRE(A.m_moveDir == Dir::Right);
}

TEST_CASE("move to occupied cell", "[game]")
{
    Game game{TestGameCfg};

    TestClient A{game, "A", {1, 1}};
    TestClient B{game, "B", {2, 1}};

    A.requestMove(Dir::Right);
    game.tick();
    REQUIRE(A.m_state == PlayerState::Idle);
}

TEST_CASE("two move to same cell", "[game]")
{
    Game game{TestGameCfg};

    TestClient A{game, "A", {1, 1}};
    TestClient B{game, "B", {3, 1}};

    A.requestMove(Dir::Right);
    game.tick();
    REQUIRE(A.m_state == PlayerState::MovingOut);

    B.requestMove(Dir::Left);
    game.tick();
    REQUIRE(B.m_state == PlayerState::Idle);

    A.requestDisconnect();
    game.tick();
    B.requestMove(Dir::Left);
    game.tick();
    REQUIRE(B.m_state == PlayerState::MovingOut);
}

TEST_CASE("move after another", "[game]")
{
    Game game{TestGameCfg};

    TestClient A{game, "A", {1, 1}};
    TestClient B{game, "B", {2, 1}};

    B.requestMove(Dir::Right);
    game.tick();
    game.tick();
    
    A.requestMove(Dir::Right);
    game.tick();
    REQUIRE(A.m_state == PlayerState::MovingOut);
}

TEST_CASE("move near a wall", "[game]")
{
    Game game{TestGameCfg};
    game.m_geodata.addWall({1, 1});

    auto&& testMove = [&](Point spawnPt, Dir moveDir) -> PlayerState
    {
        TestClient A{game, "A", spawnPt};
        A.requestMove(moveDir);
        game.tick();
        return A.m_state;
    };

    SECTION("move into wall at Right")
    {
        REQUIRE(testMove({0, 1}, Dir::Right) == PlayerState::Idle);
    }

    SECTION("move into wall at Down")
    {
        REQUIRE(testMove({1, 0}, Dir::Down) == PlayerState::Idle);
    }

    SECTION("move near wall")
    {
        REQUIRE(testMove({1, 0}, Dir::Right) == PlayerState::MovingOut);
    }
}