#pragma once

#include <iostream>
#include <memory>
#include <unordered_map>

#include "Game.hpp"

#include "Connection.hpp"
#include <websocket-cpp/Server.hpp>
#include <websocket-cpp/server_src.hpp>

class Server
{
public:
    void start(const std::string& ip, unsigned short port, std::ostream& log)
    {
        m_wsServer.start(ip, port, log);
    }

    void tick()
    {
        pollConnections();
        m_game.tick();
    }

    void stop()
    {
        m_wsServer.stop();
    }

private:
    void pollConnections()
    {
        websocket::Event event;
        websocket::ConnectionId id;
        std::string msg;

        while (m_wsServer.poll(event, id, msg))
        {
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
        }
    }

    void onNewConnection(websocket::ConnectionId connId)
    {
        assert(m_conn.count(connId) == 0);
        m_conn[connId] = std::make_unique<Connection>(connId, m_wsServer);
        Point pos{(int)connId % m_game.m_cfg.worldCX, (int)connId % m_game.m_cfg.worldCY};
        m_game.newPlayer(*m_conn[connId], pos);
    }

    void onMessage(websocket::ConnectionId connId, const std::string& msg)
    {
        auto objId = m_conn[connId]->objId();
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
        if (auto objId = m_conn[connId]->objId())
            m_game.disconnect(m_game.m_objects.getObject(objId));

        m_conn.erase(connId);
    }

    websocket::Server m_wsServer;
    Game m_game;

    std::unordered_map<websocket::ConnectionId, std::unique_ptr<Connection>> m_conn;
};