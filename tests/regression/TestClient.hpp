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
    PlayerState m_state{PlayerState::Idle};
    Dir m_moveDir;

    std::unordered_map<ObjectId, FullPlayerInfo> see;

    TestClient(Game& game) : m_game{&game} {}

    void requestDisconnect()
    {
        auto& obj = m_game->m_objects.getObject(m_id);
        m_game->disconnect(obj);
    }

    void requestMove(Dir direction)
    {
        auto& obj = m_game->m_objects.getObject(m_id);
        m_game->beginMove(obj, direction);
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

    virtual void seeBeginMove(const MoveInfo& info) override
    {
        if (info.id == m_id)
        {
            m_state = PlayerState::MovingOut;
            m_moveDir = info.moveDir;
        }
        else
        {
            see[info.id].state = PlayerState::MovingOut;
            see[info.id].moveDir = info.moveDir;
        }
    }

    virtual void seeCrossCellBorder(ObjectId id) override
    {
        if (id == m_id)
        {
            m_state = PlayerState::MovingIn;
            moveRel(m_pos, m_moveDir);
        }
        else
        {
            see[id].state = PlayerState::MovingIn;
            moveRel(see[id].pos, see[id].moveDir);
        }
    }

    virtual void seeStop(ObjectId id) override
    {
        if (id == m_id)
        {
            m_state = PlayerState::Idle;
        }
        else
        {
            see[id].state = PlayerState::Idle;
        }
    }
};