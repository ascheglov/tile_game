#include "Game.hpp"

#include "catch_wrap.hpp"
#include "test_printers.hpp"

#include "TestClient.hpp"

TEST_CASE("move alone", "[game]")
{
    Game game;
    game.m_spawns.set({{3, 2}});

    TestClient A(game);
    game.newPlayer(A);

    REQUIRE(A.m_state == PlayerState::Idle);

    A.requestMove(Dir::Right);

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
    Game game;
    game.m_spawns.set({{3, 2}, {3, 3}});

    TestClient A(game);
    game.newPlayer(A);

    TestClient B(game);
    game.newPlayer(B);

    A.requestMove(Dir::Right);

    REQUIRE(B.see[1].state == PlayerState::MovingOut);
    REQUIRE(B.see[1].moveDir == Dir::Right);

    game.tick();

    REQUIRE(B.see[1].state == PlayerState::MovingIn);
    REQUIRE(B.see[1].pos == Point(4, 2));

    game.tick();

    REQUIRE(B.see[1].state == PlayerState::Idle);
}

TEST_CASE("spawn and see move out", "[game]")
{
    Game game;
    game.m_spawns.set({{3, 2}, {3, 3}});

    TestClient A(game);
    game.newPlayer(A);

    A.requestMove(Dir::Right);

    TestClient B(game);
    game.newPlayer(B);

    REQUIRE(B.see[1].state == PlayerState::MovingOut);
    REQUIRE(B.see[1].moveDir == Dir::Right);
}

TEST_CASE("spawn and see move in", "[game]")
{
    Game game;
    game.m_spawns.set({{3, 2}, {3, 3}});

    TestClient A(game);
    game.newPlayer(A);

    A.requestMove(Dir::Right);
    game.tick();

    TestClient B(game);
    game.newPlayer(B);

    REQUIRE(B.see[1].state == PlayerState::MovingIn);
    REQUIRE(B.see[1].moveDir == Dir::Right);
    REQUIRE(B.see[1].pos == Point(4, 2));
}

TEST_CASE("spawn after move and see idle", "[game]")
{
    Game game;
    game.m_spawns.set({{3, 2}, {3, 3}});

    TestClient A(game);
    game.newPlayer(A);

    A.requestMove(Dir::Right);
    game.tick();
    game.tick();

    TestClient B(game);
    game.newPlayer(B);

    REQUIRE(B.see[1].state == PlayerState::Idle);
}

TEST_CASE("move out of view area", "[game]")
{
    Game game;
    game.m_spawns.set({{3, 2}, {1, 2}});

    TestClient A(game);
    game.newPlayer(A);

    TestClient B(game);
    game.newPlayer(B);

    REQUIRE_FALSE(B.see.empty());

    A.requestMove(Dir::Right);
    game.tick();

    REQUIRE(B.see.empty());

    game.tick(); // just in case
    REQUIRE(B.see.empty());
}

TEST_CASE("move into view area", "[game]")
{
    Game game;
    game.m_spawns.set({{4, 2}, {1, 2}});

    TestClient A(game);
    game.newPlayer(A);

    TestClient B(game);
    game.newPlayer(B);

    REQUIRE(B.see.empty());

    A.requestMove(Dir::Left);
    game.tick();

    REQUIRE_FALSE(B.see.empty());
    REQUIRE(B.see[1].state == PlayerState::MovingIn);
    REQUIRE(B.see[1].moveDir == Dir::Left);
    REQUIRE(B.see[1].pos == Point(3, 2));

    game.tick();
    REQUIRE_FALSE(B.see.empty());
}

TEST_CASE("move not in view area")
{
    Game game;
    game.m_spawns.set({{1, 1}, {5, 5}});

    TestClient A(game);
    game.newPlayer(A);

    TestClient B(game);
    game.newPlayer(B);

    REQUIRE(B.see.empty());
    A.requestMove(Dir::Right);
    game.tick();
    game.tick();
    REQUIRE(B.see.empty());
}