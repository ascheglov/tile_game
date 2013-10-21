#pragma once

#include <conio.h>
#include <iostream>
#include <sstream>
#include <memory>

#include "Game.hpp"

#include <websocket-cpp/ServerImpl.hpp>

struct Connection : EventHandler
{
    Connection(websocket::ConnectionId id, websocket::ServerImpl& srv)
        : m_connId{id}, m_server{&srv} 
    {}

    websocket::ConnectionId m_connId;
    websocket::ServerImpl* m_server;
    ObjectId m_objId{0};

    struct Packet
    {
        std::stringstream ss;
        Packet(const char* type)
        {
            ss << "{\"type\":\"" << type << '"';
        }

        Packet& field(const char* name, int value)
        {
            ss << ",\"" << name << "\":" << value;
            return *this;
        }

        void send(Connection* conn)
        {
            ss << '}';
            conn->m_server->send(conn->m_connId, ss.str(), false);
        }
    };

    virtual void init(ObjectId assignedId, const Point& pos) override
    {
        m_objId = assignedId;

        Packet p("init");
        p.field("id", assignedId);
        p.field("x", pos.x);
        p.field("y", pos.y);
        p.send(this);
    }

    virtual void seePlayer(const FullPlayerInfo& info) override
    {
        Packet p("see_player");
        p.field("id", info.m_id);
        p.field("dir", static_cast<int>(info.m_moveDir));
        p.field("state", static_cast<int>(info.m_state));
        p.field("x", info.m_pos.x);
        p.field("y", info.m_pos.y);
        p.send(this);
    }

    virtual void disconnect() override
    {
        Packet p("disconnect");
        p.send(this);
    }

    virtual void seeDisappear(ObjectId id) override
    {
        Packet p("see_disappear");
        p.field("id", id);
        p.send(this);
    }

    virtual void seeBeginMove(const MoveInfo& info) override
    {
        Packet p("see_begin_move");
        p.field("id", info.id);
        p.field("dir", static_cast<int>(info.moveDir));
        p.send(this);
    }

    virtual void seeCrossCellBorder(ObjectId id) override
    {
        Packet p("see_cross_cell");
        p.field("id", id);
        p.send(this);
    }

    virtual void seeStop(ObjectId id) override
    {
        Packet p("see_stop");
        p.field("id", id);
        p.send(this);
    }
};

struct Server
{
    std::unique_ptr<websocket::ServerImpl> m_wsServer;
    Game m_game;

    std::unordered_map<websocket::ConnectionId, std::unique_ptr<Connection>> m_conn;

    void start()
    {
        std::vector<Point> spawns;
        spawns.emplace_back(1, 1);
        spawns.emplace_back(2, 2);
        spawns.emplace_back(3, 3);
        m_game.setSpawns(spawns);

        m_wsServer = std::make_unique<websocket::ServerImpl>(
            "127.0.0.1", 4080, std::cout,
            [this](websocket::Event event, websocket::ConnectionId id, std::string msg){
                switch (event)
                {
                case websocket::Event::NewConnection:
                    onNewConnection(id);
                    break;

                case websocket::Event::Message:
                    onMessage(id, msg);
                    break;

                case websocket::Event::Disconnect:
                    onDisconnect(id);
                    break;
                }
            });
    }

    void run()
    {
        std::cout << "press [h] for help\n";

        for (;;)
        {
            m_game.tick();
            if (!poll_input())
                break;

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::cout << "stopping...\n";
        m_wsServer->stop();
    }

private:
    bool poll_input()
    {
        if (!_kbhit())
            return true;

        switch (_getch())
        {
        case 'q': return false;
        
        case '?': case 'h':
            std::cout << R"(
q - quit
h or ? - this message
p - list players
)";
            break;

        case 'p':
            listPlayers();
            break;
        }
        return true;
    }

    void listPlayers()
    {
        std::cout << "\n" << m_game.m_objects.m_objects.size() << " players:\n";
        for (auto&& objPair : m_game.m_objects.m_objects)
        {
            auto& obj = *objPair.second;
            std::cout << '#' << obj.m_id
                << " (" << obj.m_pos.x << ", " << obj.m_pos.y << ")\n";
        }
    }

    void onNewConnection(websocket::ConnectionId connId)
    {
        assert(m_conn.count(connId) == 0);
        m_conn[connId] = std::make_unique<Connection>(connId, *m_wsServer);
        m_game.newPlayer(*m_conn[connId]);
    }

    void onMessage(websocket::ConnectionId connId, const std::string& msg)
    {
        auto objId = m_conn[connId]->m_objId;
        assert(objId != 0);
        auto&& obj = m_game.m_objects.getObject(objId);

        std::stringstream msgStream(msg);
        std::string verb; msgStream >> verb;
        if (verb == "move")
        {
            int dir;
            msgStream >> dir;
            m_game.beginMove(obj, static_cast<Dir>(dir));
        }
        else if(verb == "close")
        {
            m_game.disconnect(obj);
        }
        else
        {
            std::cout << "unknown packet: " << msg << '\n';
        }
    }

    void onDisconnect(websocket::ConnectionId connId)
    {
        if (auto objId = m_conn[connId]->m_objId)
            m_game.disconnect(m_game.m_objects.getObject(objId));

        m_conn.erase(connId);
    }
};