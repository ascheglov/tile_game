#pragma once

#include "types.hpp"
#include "math.hpp"

struct FullPlayerInfo
{
    ObjectId m_id;
    Point m_pos;
    PlayerState m_state;
    Dir m_moveDir;
    Spell m_spell;
};

struct MoveInfo
{
    ObjectId id;
    Dir moveDir;
};

struct CastInfo
{
    ObjectId m_id;
    Spell m_spell;
};

struct SpellEffect
{
    SpellEffect(Spell spell, Point pos) : m_pos{pos}
    {
        switch (spell)
        {
        case Spell::Lightning: m_effect = Effect::Lightning; break;
        default: assert(!"unknown spell");
        }
    }

    Point m_pos;
    Effect m_effect;
};

struct EventHandler
{
    virtual void init(ObjectId assignedId, const Point& pos, int health) = 0;

    virtual void seePlayer(const FullPlayerInfo& info) = 0;

    virtual void disconnect() = 0;
    virtual void seeDisappear(ObjectId id) = 0;

    virtual void seeBeginMove(const MoveInfo& info) = 0;
    virtual void seeCrossCellBorder(ObjectId id) = 0;
    virtual void seeStop(ObjectId id) = 0;

    virtual void seeBeginCast(const CastInfo& info) = 0;
    virtual void seeEndCast(ObjectId id) = 0;
    
    virtual void seeEffect(const SpellEffect& effect) = 0;

    virtual void healthChange(int newHP) = 0;
protected:
    ~EventHandler() = default;
};