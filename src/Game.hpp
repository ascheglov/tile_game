#pragma once

#include <cassert>
#include <functional>
#include <list>
#include <memory>
#include <unordered_map>

#include "types.hpp"
#include "events.hpp"
#include "math.hpp"
#include "Object.hpp"

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

    void remove(ObjectId id)
    {
        assert(m_objects.count(id) != 0);
        m_objects.erase(id);
    }

    std::unordered_map<ObjectId, std::unique_ptr<Object>> m_objects;

private:
    ObjectId m_lastObjectId{0};
};

struct TimerQueue
{
    using ticks_t = std::uint32_t;  // 1.36 years at 100 ticks per second

    ticks_t m_now{0};

    template<typename Callback>
    void add(ObjectId id, unsigned delay, Callback&& callback)
    {
        m_queue.emplace_back(id, m_now + delay, callback);
    }

    template<typename Id2Obj>
    void tick(Id2Obj&& id2obj)
    {
        ++m_now;
        if (m_queue.empty())
            return;

        auto it = m_queue.begin();
        auto next = it;
        for (; it != m_queue.end(); it = next)
        {
            next = it; ++next;

            if (it->elapsed(m_now))
            {
                it->callback(id2obj(it->id));
                m_queue.erase(it);
            }
        }
    }

    struct Timer
    {
        template<typename Callback>
        Timer(ObjectId id, ticks_t deadline, Callback&& callback)
            : deadline(deadline)
            , id(id)
            , callback(std::forward<Callback>(callback))
        {}

        bool elapsed(ticks_t now) const { return now >= deadline; }

        ticks_t deadline;
        ObjectId id;
        std::function<void(Object&)> callback;
    };

    std::list<Timer> m_queue;
};

struct GameCfg
{
    int worldCX{8};
    int worldCY{8};
    int playerViewRadius{2};
    int moveTicks{1};
};

class World
{
public:
    explicit World(unsigned cx, unsigned cy)
        : m_cx(cx), m_cy(cy)
    {
        m_objectTable.resize(m_cy);
        for (auto& v : m_objectTable)
            v.resize(cx, '0');
    }

    ObjectId objectAt(const Point& pt) const
    {
        return isValidPoint(pt) ? at(pt) - '0' : 0;
    }

    void addObject(ObjectId id, const Point& pt)
    {
        assert(objectAt(pt) == 0);
        assert(id < 50); // otherwise it won't fit in `char`
        at(pt) = static_cast<char>('0' + id);
    }

    void removeObject(const Point& pt)
    {
        assert(objectAt(pt) != 0);
        at(pt) = '0';
    }

private:

    bool isValidPoint(const Point& pt) const
    {
        return pt.inside(m_cx, m_cy);
    }

    char& at(const Point& pt)
    {
        assert(isValidPoint(pt));
        return m_objectTable[pt.y][pt.x];
    }

    char at(const Point& pt) const
    {
        assert(isValidPoint(pt));
        return m_objectTable[pt.y][pt.x];
    }

    int m_cx, m_cy;
    std::vector<std::string> m_objectTable;
};

class Game
{
public:
    GameCfg m_cfg;

    void newPlayer(EventHandler& eventHandler, Point pos)
    {
        if (auto occupantId = m_world.objectAt(pos))
        {
            disconnect(m_objects.getObject(occupantId));
        }

        auto&& obj = m_objects.newObject();
        obj.m_eventHandler = &eventHandler;

        obj.m_pos = pos;
     
        obj.m_eventHandler->init(obj.m_id, obj.m_pos);

        assert(m_world.objectAt(pos) == 0);
        m_world.addObject(obj.m_id, obj.m_pos);

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

    void disconnect(Object& obj)
    {
        obj.m_eventHandler->disconnect();
        
        forObjectsAround(obj.m_pos, [&](Object& otherObj)
        {
            if (&obj != &otherObj && otherObj.m_eventHandler)
            {
                otherObj.m_eventHandler->seeDisappear(obj.m_id);
            }
        });
        
        assert(m_world.objectAt(obj.m_pos) == obj.m_id);
        m_world.removeObject(obj.m_pos);

        m_objects.remove(obj.m_id);
    }

    bool canMove(Object& obj, Dir direction)
    {
        auto pt = obj.m_pos;
        moveRel(pt, direction);

        if (!pt.inside(m_cfg.worldCX, m_cfg.worldCY))
            return false;
             
        if (m_world.objectAt(pt) != 0)
            return false;

        return true;
    }

    void beginMove(Object& obj, Dir direction)
    {
        if (obj.m_state != PlayerState::Idle)
            return;

        if (!canMove(obj, direction))
            return;

        obj.m_state = PlayerState::MovingOut;
        obj.m_moveDir = direction;

        m_timers.add(obj.m_id, m_cfg.moveTicks, [this](Object& o){ onCrossCellBorder(o); });

        auto&& moveInfo = obj.getMoveInfo();
        forObjectsAround(obj.m_pos, [&](Object& otherObj)
        {
            if (otherObj.m_eventHandler)
                otherObj.m_eventHandler->seeBeginMove(moveInfo);
        });
    }

    void onCrossCellBorder(Object& obj)
    {
        auto oldPos = obj.m_pos;

        assert(obj.m_state == PlayerState::MovingOut);
        obj.m_state = PlayerState::MovingIn;
        moveRel(obj.m_pos, obj.m_moveDir);

        m_world.removeObject(oldPos);
        m_world.addObject(obj.m_id, obj.m_pos);

        m_timers.add(obj.m_id, m_cfg.moveTicks, [this](Object& o){ onStopMove(o); });

        auto&& fullInfo = obj.getFullInfo();
        forObjectsAround(obj.m_pos, [&](Object& otherObj)
        {
            bool seeAppears = isOnArc180(obj.m_pos, m_cfg.playerViewRadius, obj.m_moveDir, otherObj.m_pos);

            if (seeAppears)
                obj.m_eventHandler->seePlayer(otherObj.getFullInfo());

            if (otherObj.m_eventHandler)
            {
                if (seeAppears)
                {
                    otherObj.m_eventHandler->seePlayer(fullInfo);
                }
                else
                {
                    otherObj.m_eventHandler->seeCrossCellBorder(obj.m_id);
                }
            }
        });

        forArc180(oldPos, m_cfg.playerViewRadius, oppositeDir(obj.m_moveDir), [&](const Point& pt)
        {
            if (auto otherObjPtr = objectAt(pt))
            {
                auto&& otherObj = *otherObjPtr;

                obj.m_eventHandler->seeDisappear(otherObj.m_id);

                if (otherObj.m_eventHandler)
                    otherObj.m_eventHandler->seeDisappear(obj.m_id);
            }
        });
    }

    void onStopMove(Object& obj)
    {
        assert(obj.m_state == PlayerState::MovingIn);
        obj.m_state = PlayerState::Idle;

        forObjectsAround(obj.m_pos, [&](Object& otherObj)
        {
            if (otherObj.m_eventHandler)
                otherObj.m_eventHandler->seeStop(obj.m_id);
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

    void tick()
    {
        auto&& id2obj = [this](ObjectId id) -> Object& { return m_objects.getObject(id); };
        m_timers.tick(id2obj);
    }

    Object* objectAt(const Point& pt)
    {
        if (auto id = m_world.objectAt(pt))
            return &m_objects.getObject(id);

        return nullptr;
    }

    ObjectManager m_objects;
    TimerQueue m_timers;
    World m_world{m_cfg.worldCX, m_cfg.worldCY};
};