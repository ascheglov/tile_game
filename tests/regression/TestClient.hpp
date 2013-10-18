#pragma once

#include "types.hpp"
#include "math.hpp"
#include "events.hpp"
#include "Game.hpp"

struct TestClient : EventHandler
{
    Game* m_game;
    
    ObjectId m_id;
    Point m_pos;

    std::unordered_map<ObjectId, FullPlayerInfo> see;

    TestClient(Game& game) : m_game{&game} {}

    virtual void init(ObjectId assignedId, const Point& pos) override
    {
        m_id = assignedId;
        m_pos = pos;
    }

    virtual void seePlayer(const FullPlayerInfo& info) override
    {
        assert(see.count(info.id) == 0);
        see[info.id] = info;
    }
};