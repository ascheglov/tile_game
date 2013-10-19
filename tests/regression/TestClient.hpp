#pragma once

#include "types.hpp"
#include "math.hpp"
#include "events.hpp"
#include "Game.hpp"

struct TestClient : EventHandler
{
    Game* m_game;

    bool m_isConnected{false};

    ObjectId m_id;
    Point m_pos;

    std::unordered_map<ObjectId, FullPlayerInfo> see;

    TestClient(Game& game) : m_game{&game} {}

    void requestDisconnect()
    {
        auto& obj = m_game->m_objects.getObject(m_id);
        m_game->disconnect(obj);
    }

private: // EventHandler implementation
    virtual void init(ObjectId assignedId, const Point& pos) override
    {
        m_isConnected = true;
        m_id = assignedId;
        m_pos = pos;
    }

    virtual void seePlayer(const FullPlayerInfo& info) override
    {
        assert(see.count(info.id) == 0);
        see[info.id] = info;
    }

    virtual void disconnect() override
    {
        m_isConnected = false;
    }

    virtual void seeDisconnect(ObjectId id) override
    {
        assert(see.count(id) != 0);
        see.erase(id);
    }
};