#pragma once

#include <cassert>
#include <memory>
#include <unordered_map>

#include "types.hpp"
#include "events.hpp"
#include "math.hpp"
#include "Object.hpp"

struct SpawnPoints
{
    void set(std::vector<Point> spawns) { m_spawns = std::move(spawns); }

    Point get()
    {
        assert(!m_spawns.empty());
        auto pt = m_spawns[m_next];
        m_next = (m_next + 1) % m_spawns.size();
        return pt;
    }

private:
    std::vector<Point> m_spawns;
    unsigned m_next{0};
};

class ObjectManager
{
public:
    Object& newObject()
    {
        ++m_lastObjectId;
        m_objects[m_lastObjectId] = std::make_unique<Object>(m_lastObjectId);
        return *m_objects[m_lastObjectId];
    }

    Object& getObject(ObjectId id)
    {
        assert(m_objects.count(id) != 0);
        return *m_objects[id];
    }

    std::unordered_map<ObjectId, std::unique_ptr<Object>> m_objects;

private:
    ObjectId m_lastObjectId{0};
};

struct GameCfg
{
    int playerViewRadius{2};
};

class Game
{
public:
    GameCfg m_cfg;

    void newPlayer(EventHandler& eventHandler)
    {
        auto&& obj = m_objects.newObject();
        obj.m_eventHandler = &eventHandler;

        obj.m_pos = m_spawns.get();
     
        obj.m_eventHandler->init(obj.m_id, obj.m_pos);

        auto&& newPlayerInfo = obj.getFullInfo();

        forObjectsAround(obj.m_pos, [&](Object& otherObj)
        {
            if (&obj != &otherObj)
            {
                obj.m_eventHandler->seePlayer(otherObj.getFullInfo());

                if (otherObj.m_eventHandler)
                    otherObj.m_eventHandler->seePlayer(newPlayerInfo);
            }
        });
    }

    template<typename Callback>
    void forObjectsAround(Point pt, Callback&& callback)
    {
        for (auto&& objPair : m_objects.m_objects)
        {
            auto& obj = *objPair.second;
            if (distance(pt, obj.m_pos) <= m_cfg.playerViewRadius)
                callback(obj);
        }
    }

    ObjectManager m_objects;
    SpawnPoints m_spawns;
};