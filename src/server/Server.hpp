#pragma once

#include <iostream>
#include <memory>
#include <unordered_map>

#include "Game.hpp"

#include "Connection.hpp"
#include "PacketBuilder.hpp"
#include <websocket-cpp/Server.hpp>
#include <websocket-cpp/server_src.hpp>

class Server
{
public:
    Server(const GameCfg& cfg, const std::string& worldMap)
        : m_gameCfg{cfg}
        , m_worldMap{worldMap}
    {
        parseMap();
    }

    void start(const std::string& ip, unsigned short port, std::ostream& log)
    {
        m_wsServer.start(ip, port, log);
    }

    void tick()
    {
        m_game.tick();
        pollConnections();
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

        m_conn[connId]->sendWorldMap(m_gameCfg.worldCX, m_worldMap);

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
        else if (verb == "cast")
        {
            int spell, x, y;
            msgStream >> spell >> x >> y;
            m_game.beginCast(obj, static_cast<Spell>(spell), {x, y});
        }
        else if (verb == "close")
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

    void parseMap()
    {
        assert((int)m_worldMap.size() == m_gameCfg.worldCX * m_gameCfg.worldCY);
        auto cellIdx = 0;         
        for (auto y = 0; y != m_gameCfg.worldCY; ++y)
        {
            for (auto x = 0; x != m_gameCfg.worldCX; ++x, ++cellIdx)
            {
                Point pt{x, y};

                switch (m_worldMap[cellIdx])
                {
                case '.': break; // empty cell

                case 'W':
                    m_game.m_geodata.addWall(pt);
                    break;

                case '?':
                    m_spawns.push_back(pt);
                    break;

                default:
                    assert(!"unknown cell type");
                }
            }
        }
    }

    websocket::Server m_wsServer;

    const GameCfg& m_gameCfg;
    Game m_game{m_gameCfg};

    std::unordered_map<websocket::ConnectionId, std::unique_ptr<Connection>> m_conn;

    std::string m_worldMap;
    std::vector<Point> m_spawns;
};