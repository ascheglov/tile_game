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

    void sendWorldMap(int cx, const std::string& worldMap)
    {
        PacketBuilder p("map");
        p.field("cx", cx);
        p.field("cells", worldMap);
        send(p);
    }

private:
    void send(PacketBuilder& pb)
    {
        m_server->sendText(m_connId, pb.close());
    }

    virtual void init(const InitInfo& info) override
    {
        m_objId = info.m_id;

        PacketBuilder p("init");
        p.field("id", info.m_id);
        p.field("x", info.m_pos.x);
        p.field("y", info.m_pos.y);
        p.field("hp", info.m_health);
        p.field("name", info.m_name);
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
        p.field("name", info.m_name);
        send(p);
    }

    virtual void disconnect() override
    {
        PacketBuilder p("disconnect");
        send(p);
        m_server->drop(m_connId);
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

    virtual void seeBeginCast(const CastInfo& info) override
    {
        PacketBuilder p("see_cast");
        p.field("id", info.m_id);
        p.field("spell", static_cast<int>(info.m_spell));
        send(p);
    }

    virtual void seeEndCast(ObjectId id) override
    {
        PacketBuilder p("see_end_cast");
        p.field("id", id);
        send(p);
    }

    virtual void seeEffect(const SpellEffect& effect) override
    {
        PacketBuilder p("see_effect");
        p.field("x", effect.m_pos.x);
        p.field("y", effect.m_pos.y);
        p.field("effect", static_cast<int>(effect.m_effect));
        send(p);
    }

    virtual void healthChange(int newHP) override
    {
        PacketBuilder p("hp_change");
        p.field("hp", newHP);
        send(p);
    }

    websocket::ConnectionId m_connId;
    websocket::Server* m_server;
    ObjectId m_objId{0};
};
