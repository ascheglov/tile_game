#pragma once

#include "types.hpp"
#include "events.hpp"
#include "math.hpp"

struct ActionData
{
    Action m_action{Action::None};
    Dir m_moveDir;
    Spell m_spell;
    Point m_castDest;

    bool empty() const { return m_action == Action::None; }
    void clear() { m_action = Action::None; }
};

struct Object
{
    Object(ObjectId id) : m_id{id} {}

    ObjectId m_id;
    Point m_pos;
    PlayerState m_state{PlayerState::Idle};

    Dir m_moveDir;

    Spell m_spell;
    Point m_castDest;

    int m_health{100};

    ActionData m_nextAction;

    EventHandler* m_eventHandler{nullptr};

    bool m_erased{false};

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
        inf.m_spell = m_spell;
        return inf;
    }

    MoveInfo getMoveInfo() const
    {
        MoveInfo inf = {m_id, m_moveDir};
        return inf;
    }

    CastInfo getCastInfo() const
    {
        CastInfo inf = {m_id, m_spell};
        return inf;
    }
};