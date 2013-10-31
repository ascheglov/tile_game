#pragma once

#include <array>
#include <string>

#include "types.hpp"
#include "events.hpp"
#include "math.hpp"

struct Object;

struct ActionData
{
    Action m_action{Action::None};
    Dir m_moveDir;
    Spell m_spell;
    Point m_castDest;

    bool empty() const { return m_action == Action::None; }
    void clear() { m_action = Action::None; }
};

class TableBase
{
    friend class ObjectManager;
    bool m_isFree{false};
};

struct ObjectAPI : TableBase
{
    ObjectAPI(ObjectId id) : m_id{id} {}

    ObjectId m_id;
    EventHandler* m_eventHandler{nullptr};

    virtual const Object& asObject() const = 0;

    virtual void setNextAction(ActionData ad) = 0;
    virtual void disconnect() = 0;

    virtual void modifyHP(int delta, ThrdIdx threadIdx) = 0;

protected:
    ~ObjectAPI() = default;
};

struct Object : public ObjectAPI
{
    Object(ObjectId id)
        : ObjectAPI{id}
        , m_healthDelta{} // put it here as a workaround for VC++2013RC ICE
    {}

    std::string m_name;

    Point m_pos;
    PlayerState m_state{PlayerState::Idle};

    Dir m_moveDir;

    Spell m_spell;
    Point m_castDest;

    int m_health{100};

    ActionData m_nextAction;

    ticks_t m_timerDeadline;
    std::function<void(Object&, ThrdIdx)> m_timerCallback;

    volatile bool m_erased{false};

    std::array<int, MaxThreads> m_healthDelta;

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
        inf.m_name = m_name;
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

private: // ObjectAPI implementation
    virtual const Object& asObject() const override { return *this; }

    virtual void setNextAction(ActionData ad) override
    {
        m_nextAction = std::move(ad);
    }

    virtual void disconnect() override { m_erased = true; }
    
    virtual void modifyHP(int delta, ThrdIdx threadIdx)
    {
        m_healthDelta[threadIdx] += delta;
    }
};