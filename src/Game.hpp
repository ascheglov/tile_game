#pragma once

#include <cassert>
#include <functional>
#include <list>
#include <memory>
#include <unordered_map>

#include "GameCfg.hpp"
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

struct Geodata
{
    Geodata(int cx, int cy)
        : m_table(cx * cy)
        , m_cx{cx}
        , m_cy{cy}
    {}

    void addWall(const Point& pt)
    {
        if (pt.x > 0) m_table[idx(moveRel(pt, Dir::Left))] |= dirMask(Dir::Right);
        if (pt.y > 0) m_table[idx(moveRel(pt, Dir::Up))] |= dirMask(Dir::Down);
        if (pt.x < m_cx - 1) m_table[idx(moveRel(pt, Dir::Right))] |= dirMask(Dir::Left);
        if (pt.y < m_cy - 1) m_table[idx(moveRel(pt, Dir::Down))] |= dirMask(Dir::Up);
    }

    bool canMove(const Point& pt, Dir moveDir) const
    {
        assert(pt.inside(m_cx, m_cy));
        auto cellInfo = m_table[idx(pt)];
        return (cellInfo & dirMask(moveDir)) == 0;
    }

private:
    int idx(const Point& pt) const
    {
        return pt.x + pt.y * m_cx;
    }

    static std::uint8_t dirMask(Dir direction)
    {
        return 1 << static_cast<int>(direction);
    }

    std::vector<std::uint8_t> m_table;
    int m_cx, m_cy;
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
        return isValidPoint(pt) ? getAt(pt) : 0;
    }

    void addObject(ObjectId id, const Point& pt)
    {
        assert(isValidPoint(pt));
        assert(objectAt(pt) == 0);
        assert(realId(id) < 10); // otherwise it won't fit in [1; 9]
        setAt(pt, id);
    }

    void removeObject(const Point& pt)
    {
        assert(objectAt(pt) != 0);
        setAt(pt, 0);
    }

    void lockCell(ObjectId lockOwner, const Point& pt)
    {
        addObject(lockOwner | CellLockFlag, pt);
    }

    void moveToCell(const Point& from, const Point& dest)
    {
        assert(isLocked(dest));
        auto id = objectAt(from);
        assert(id == realId(objectAt(dest)));
        removeObject(from);
        removeObject(dest);
        addObject(id, dest);
    }

private:
    static const ObjectId CellLockFlag = 0x80000000;

    static ObjectId realId(ObjectId maskedId)
    {
        return maskedId & ~CellLockFlag;
    }

    bool isLocked(const Point& pt)
    {
        return (objectAt(pt) & CellLockFlag) != 0;
    }

    bool isValidPoint(const Point& pt) const
    {
        return pt.inside(m_cx, m_cy);
    }

    void setAt(const Point& pt, ObjectId id)
    {
        assert(isValidPoint(pt));
        if ((id & CellLockFlag) == 0)
            m_objectTable[pt.y][pt.x] = wchar_t(id + '0');
        else
            m_objectTable[pt.y][pt.x] = wchar_t(id - 1 + L'\x2460');
    }

    ObjectId getAt(const Point& pt) const
    {
        assert(isValidPoint(pt));
        auto ch = m_objectTable[pt.y][pt.x];
        return (ch < 0x100) ? (ch - '0') : ((ch - 0x2460 + 1) | CellLockFlag);
    }

    int m_cx, m_cy;
    std::vector<std::wstring> m_objectTable;
};

class Game
{
public:
    Game(const GameCfg& cfg) : m_cfg{cfg} {}

    Game(const Game&) = delete;
    void operator=(const Game&) = delete;

    const GameCfg& m_cfg;

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

        if (obj.m_state == PlayerState::MovingOut)
        {
            m_world.removeObject(moveRel(obj.m_pos, obj.m_moveDir));
        }

        m_objects.remove(obj.m_id);
    }

    bool canMoveTo(const Point& srcPt, Dir moveDir)
    {
        auto destPt = moveRel(srcPt, moveDir);

        if (!destPt.inside(m_cfg.worldCX, m_cfg.worldCY))
            return false;

        if (!m_geodata.canMove(srcPt, moveDir))
            return false;

        if (m_world.objectAt(destPt) != 0)
            return false;

        return true;
    }

    void beginMove(Object& obj, Dir direction)
    {
        if (obj.m_state != PlayerState::Idle)
            return;

        if (!canMoveTo(obj.m_pos, direction))
            return;

        auto destPoint = moveRel(obj.m_pos, direction);
        m_world.lockCell(obj.m_id, destPoint);

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
        obj.m_pos = moveRel(obj.m_pos, obj.m_moveDir);

        m_world.moveToCell(oldPos, obj.m_pos);

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

    void beginCast(Object& obj, Spell spell, const Point& dest)
    {
        if (obj.m_state != PlayerState::Idle)
            return;

        obj.m_state = PlayerState::Casting;
        obj.m_spell = spell;
        obj.m_castDest = dest;

        m_timers.add(obj.m_id, m_cfg.castTicks, [this](Object& o){ onEndCast(o); });

        auto&& castInfo = obj.getCastInfo();
        forObjectsAround(obj.m_pos, [&](Object& otherObj)
        {
            if (otherObj.m_eventHandler)
                otherObj.m_eventHandler->seeBeginCast(castInfo);
        });
    }

    void onEndCast(Object& obj)
    {
        assert(obj.m_state == PlayerState::Casting);
        obj.m_state = PlayerState::Idle;

        forObjectsAround(obj.m_pos, [&](Object& otherObj)
        {
            if (otherObj.m_eventHandler)
                otherObj.m_eventHandler->seeEndCast(obj.m_id);
        });

        SpellEffect effect{obj.m_spell, obj.m_castDest};
        forObjectsAround(effect.m_pos, [&](Object& otherObj)
        {
            if (otherObj.m_eventHandler)
                otherObj.m_eventHandler->seeEffect(effect);
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
    Geodata m_geodata{m_cfg.worldCX, m_cfg.worldCY};
};