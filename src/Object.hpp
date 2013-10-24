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

    Point moveDest() const
    {
        auto pt = m_pos;
        moveRel(pt, m_moveDir);
        return pt;
    }

    FullPlayerInfo getFullInfo() const
    {
        FullPlayerInfo inf;
        inf.m_id = m_id;
        inf.m_pos = m_pos;
        inf.m_state = m_state;
        inf.m_moveDir = m_moveDir;
        return inf;
    }

    MoveInfo getMoveInfo() const
    {
        MoveInfo inf = {m_id, m_moveDir};
        return inf;
    }
};