#pragma once

#include "Game.hpp"

#include "PacketBuilder.hpp"
#include <websocket-cpp/Server.hpp>

class Connection : public EventHandler
{
public:
    Connection(websocket::ConnectionId id, websocket::Server& srv)
        : m_connId{id}, m_server{&srv}
    {}

    ObjectId objId() const { return m_objId; }

private:
    void send(PacketBuilder& pb)
    {
        m_server->sendText(m_connId, pb.close());
    }

    virtual void init(ObjectId assignedId, const Point& pos) override
    {
        m_objId = assignedId;

        PacketBuilder p("init");
        p.field("id", assignedId);
        p.field("x", pos.x);
        p.field("y", pos.y);
        send(p);
    }

    virtual void seePlayer(const FullPlayerInfo& info) override
    {
        PacketBuilder p("see_player");
        p.field("id", info.m_id);
        p.field("dir", static_cast<int>(info.m_moveDir));
        p.field("state", static_cast<int>(info.m_state));
        p.field("x", info.m_pos.x);
        p.field("y", info.m_pos.y);
        send(p);
    }

    virtual void disconnect() override
    {
        PacketBuilder p("disconnect");
        send(p);
    }

    virtual void seeDisappear(ObjectId id) override
    {
        PacketBuilder p("see_disappear");
        p.field("id", id);
        send(p);
    }

    virtual void seeBeginMove(const MoveInfo& info) override
    {
        PacketBuilder p("see_begin_move");
        p.field("id", info.id);
        p.field("dir", static_cast<int>(info.moveDir));
        send(p);
    }

    virtual void seeCrossCellBorder(ObjectId id) override
    {
        PacketBuilder p("see_cross_cell");
        p.field("id", id);
        send(p);
    }

    virtual void seeStop(ObjectId id) override
    {
        PacketBuilder p("see_stop");
        p.field("id", id);
        send(p);
    }

    websocket::ConnectionId m_connId;
    websocket::Server* m_server;
    ObjectId m_objId{0};
};
