#pragma once

#include "types.hpp"
#include "math.hpp"

struct FullPlayerInfo
{
    ObjectId m_id;
    Point m_pos;
    PlayerState m_state;
    Dir m_moveDir;
};

struct MoveInfo
{
    ObjectId id;
    Dir moveDir;
};

struct EventHandler
{
    virtual void init(ObjectId assignedId, const Point& pos) = 0;

    virtual void seePlayer(const FullPlayerInfo& info) = 0;

    virtual void disconnect() = 0;
    virtual void seeDisappear(ObjectId id) = 0;

    virtual void seeBeginMove(const MoveInfo& info) = 0;
    virtual void seeCrossCellBorder(ObjectId id) = 0;
    virtual void seeStop(ObjectId id) = 0;

protected:
    ~EventHandler() = default;
};