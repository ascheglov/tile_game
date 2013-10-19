#pragma once

#include "types.hpp"
#include "math.hpp"
#include "events.hpp"
#include "Game.hpp"

struct TestClient : EventHandler, FullPlayerInfo
{
    Game* m_game;

    bool m_isConnected{false};

    std::unordered_map<ObjectId, FullPlayerInfo> see;

    TestClient(Game& game) : m_game{&game}
    {
        game.newPlayer(*this);
    }

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
    FullPlayerInfo& findInfo(ObjectId id)
    {
        if (id == m_id) return *this;
        assert(see.count(id) != 0);
        return see[id];
    }

    virtual void init(ObjectId assignedId, const Point& pos) override
    {
        assert(!m_isConnected);
        m_isConnected = true;
        m_id = assignedId;
        m_pos = pos;
        m_state = PlayerState::Idle;
    }

    virtual void seePlayer(const FullPlayerInfo& info) override
    {
        assert(m_id != info.m_id);
        assert(see.count(info.m_id) == 0);
        see[info.m_id] = info;
    }

    virtual void disconnect() override
    {
        m_isConnected = false;
    }

    virtual void seeDisappear(ObjectId id) override
    {
        assert(m_id != id);
        assert(see.count(id) != 0);
        see.erase(id);
    }

    virtual void seeBeginMove(const MoveInfo& info) override
    {
        auto& localInfo = findInfo(info.id);
        localInfo.m_state = PlayerState::MovingOut;
        localInfo.m_moveDir = info.moveDir;
    }

    virtual void seeCrossCellBorder(ObjectId id) override
    {
        auto& localInfo = findInfo(id);
        localInfo.m_state = PlayerState::MovingIn;
        moveRel(localInfo.m_pos, localInfo.m_moveDir);
    }

    virtual void seeStop(ObjectId id) override
    {
        auto& localInfo = findInfo(id);
        localInfo.m_state = PlayerState::Idle;
    }
};