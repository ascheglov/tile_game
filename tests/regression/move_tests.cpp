#include "Game.hpp"

#include "catch_wrap.hpp"
#include "test_printers.hpp"

#include "TestClient.hpp"

TEST_CASE("move alone", "[game]")
{
    Game game;
    game.setSpawns({{3, 2}});

    TestClient A(game);
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
    game.setSpawns({{3, 2}, {3, 3}});

    TestClient A(game);
    TestClient B(game);

    A.requestMove(Dir::Right);

    REQUIRE(B.see[1].m_state == PlayerState::MovingOut);
    REQUIRE(B.see[1].m_moveDir == Dir::Right);

    game.tick();

    REQUIRE(B.see[1].m_state == PlayerState::MovingIn);
    REQUIRE(B.see[1].m_pos == Point(4, 2));

    game.tick();

    REQUIRE(B.see[1].m_state == PlayerState::Idle);
}

TEST_CASE("spawn and see move out", "[game]")
{
    Game game;
    game.setSpawns({{3, 2}, {3, 3}});

    TestClient A(game);

    A.requestMove(Dir::Right);

    TestClient B(game);

    REQUIRE(B.see[1].m_state == PlayerState::MovingOut);
    REQUIRE(B.see[1].m_moveDir == Dir::Right);
}

TEST_CASE("spawn and see move in", "[game]")
{
    Game game;
    game.setSpawns({{3, 2}, {3, 3}});

    TestClient A(game);

    A.requestMove(Dir::Right);
    game.tick();

    TestClient B(game);

    REQUIRE(B.see[1].m_state == PlayerState::MovingIn);
    REQUIRE(B.see[1].m_moveDir == Dir::Right);
    REQUIRE(B.see[1].m_pos == Point(4, 2));
}

TEST_CASE("spawn after move and see idle", "[game]")
{
    Game game;
    game.setSpawns({{3, 2}, {3, 3}});

    TestClient A(game);

    A.requestMove(Dir::Right);
    game.tick();
    game.tick();

    TestClient B(game);
    REQUIRE(B.see[1].m_state == PlayerState::Idle);
}

TEST_CASE("move out of view area", "[game]")
{
    Game game;
    game.setSpawns({{3, 2}, {1, 2}});

    TestClient A(game);
    TestClient B(game);

    REQUIRE_FALSE(A.see.empty());
    REQUIRE_FALSE(B.see.empty());

    A.requestMove(Dir::Right);
    game.tick();

    REQUIRE(A.see.empty());
    REQUIRE(B.see.empty());

    game.tick(); // just in case
    REQUIRE(A.see.empty());
    REQUIRE(B.see.empty());
}

TEST_CASE("move into view area", "[game]")
{
    Game game;
    game.setSpawns({{4, 2}, {1, 2}});

    TestClient A(game);
    TestClient B(game);

    REQUIRE(A.see.empty());
    REQUIRE(B.see.empty());

    A.requestMove(Dir::Left);
    game.tick();

    REQUIRE_FALSE(A.see.empty());
    REQUIRE(A.see[2].m_state == PlayerState::Idle);
    REQUIRE(A.see[2].m_pos == Point(1, 2));

    REQUIRE_FALSE(B.see.empty());
    REQUIRE(B.see[1].m_state == PlayerState::MovingIn);
    REQUIRE(B.see[1].m_moveDir == Dir::Left);
    REQUIRE(B.see[1].m_pos == Point(3, 2));

    game.tick();
    REQUIRE_FALSE(B.see.empty());
}

TEST_CASE("move not in view area", "[game]")
{
    Game game;
    game.setSpawns({{1, 1}, {5, 5}});

    TestClient A(game);
    TestClient B(game);

    REQUIRE(B.see.empty());
    A.requestMove(Dir::Right);
    game.tick();
    game.tick();
    REQUIRE(B.see.empty());
}

TEST_CASE("move across world boundaries", "[game]")
{
    Game game;
    game.setSpawns({{0, 0}, {7, 7}});

    TestClient A(game);
    A.requestMove(Dir::Left);
    REQUIRE(A.m_state == PlayerState::Idle);
    A.requestMove(Dir::Up);
    REQUIRE(A.m_state == PlayerState::Idle);

    TestClient B(game);
    B.requestMove(Dir::Right);
    REQUIRE(B.m_state == PlayerState::Idle);
    B.requestMove(Dir::Down);
    REQUIRE(B.m_state == PlayerState::Idle);
}

TEST_CASE("ignore move request when moving", "[game]")
{
    Game game;
    game.setSpawns({{1, 1}});
    TestClient A(game);
    A.requestMove(Dir::Right);
    REQUIRE(A.m_state == PlayerState::MovingOut);
    REQUIRE(A.m_moveDir == Dir::Right);

    A.requestMove(Dir::Left);
    REQUIRE(A.m_state == PlayerState::MovingOut);
    REQUIRE(A.m_moveDir == Dir::Right);

    game.tick();
    REQUIRE(A.m_state == PlayerState::MovingIn);

    A.requestMove(Dir::Left);
    REQUIRE(A.m_state == PlayerState::MovingIn);
    REQUIRE(A.m_moveDir == Dir::Right);
}