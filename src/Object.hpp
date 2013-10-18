#pragma once

#include "types.hpp"
#include "events.hpp"
#include "math.hpp"

struct Object
{
    Object(ObjectId id) : m_id{id} {}

    ObjectId m_id;
    Point m_pos;

    EventHandler* m_eventHandler{nullptr};

    FullPlayerInfo getFullInfo() const
    {
        FullPlayerInfo inf;
        inf.id = m_id;
        inf.pos = m_pos;
        return inf;
    }
};