#pragma once

#include "types.hpp"
#include "events.hpp"
#include "math.hpp"

struct Object
{
    Object(ObjectId id) : m_id{id} {}

    ObjectId m_id;
    Point m_pos;
    PlayerState m_state{PlayerState::Idle};

    Dir m_moveDir;

    EventHandler* m_eventHandler{nullptr};

    FullPlayerInfo getFullInfo() const
    {
        FullPlayerInfo inf;
        inf.id = m_id;
        inf.pos = m_pos;
        inf.state = m_state;
        inf.moveDir = m_moveDir;
        return inf;
    }

    MoveInfo getMoveInfo() const
    {
        MoveInfo inf = {m_id, m_moveDir};
        return inf;
    }
};