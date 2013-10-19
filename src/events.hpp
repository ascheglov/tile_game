#pragma once

#include "types.hpp"
#include "math.hpp"

struct FullPlayerInfo
{
    ObjectId id;
    Point pos;
};

struct EventHandler
{
    virtual void init(ObjectId assignedId, const Point& pos) = 0;

    virtual void seePlayer(const FullPlayerInfo& info) = 0;

    virtual void disconnect() = 0;
    virtual void seeDisconnect(ObjectId id) = 0;

protected:
    ~EventHandler() = default;
};