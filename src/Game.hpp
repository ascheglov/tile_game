#pragma once

#include <cassert>
#include <functional>
#include <list>
#include <memory>
#include <numeric>
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
        m_objects[m_lastObjectId] = std::make_shared<Object>(m_lastObjectId);
        return *m_objects[m_lastObjectId];
    }

    ObjectAPI* findObject(ObjectId id)
    {
        auto it = m_objects.find(id);
        return it == m_objects.end() ? nullptr : it->second.get();
    }

    std::unordered_map<ObjectId, std::shared_ptr<Object>> m_objects;

    template<typename F>
    void parallel_for(unsigned threads, F&& f)
    {
        assert(threads == 1);
        for (auto&& p : m_objects)
            f(p.second, 0);
    }

private:
    ObjectId m_lastObjectId{0};
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
            v.resize(cx);
    }

    ObjectId objectAt(const Point& pt) const
    {
        return isValidPoint(pt) ? getAt(pt) : 0;
    }

    void addObject(ObjectId id, const Point& pt)
    {
        assert(isValidPoint(pt));
        assert(objectAt(pt) == 0);
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
        m_objectTable[pt.y][pt.x] = id;
    }

    ObjectId getAt(const Point& pt) const
    {
        assert(isValidPoint(pt));
        return m_objectTable[pt.y][pt.x];
    }

    int m_cx, m_cy;
    std::vector<std::vector<ObjectId>> m_objectTable;
};

class Game
{
public:
    Game(const GameCfg& cfg) : m_cfg{cfg} {}

    Game(const Game&) = delete;
    void operator=(const Game&) = delete;

    const GameCfg& m_cfg;
    Geodata m_geodata{m_cfg.worldCX, m_cfg.worldCY};

    ticks_t now() const { return m_now; }

    void tick()
    {
        ++m_now;
        updateObjects();
    }

    void enqueueAction(ObjectId id, ActionData action)
    {
        if (auto objPtr = m_objects.findObject(id))
            objPtr->setNextAction(action);
    }

    void newPlayer(EventHandler& eventHandler, Point pos, std::string name)
    {
        if (auto objPtr = objectAt(pos))
        {
            onDisconnect(objPtr->asObject());
            objPtr->disconnect();
        }

        auto&& obj = m_objects.newObject();
        obj.m_eventHandler = &eventHandler;

        obj.m_name = std::move(name);
        obj.m_pos = pos;
     
        obj.m_eventHandler->init(obj.m_id, obj.m_pos, obj.m_health);

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
private:
    void onDisconnect(const Object& obj)
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

        setTimer(obj, m_cfg.moveTicks, [this](Object& o, ThrdIdx){ onCrossCellBorder(o); });

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

        setTimer(obj, m_cfg.moveTicks, [this](Object& o, ThrdIdx){ onStopMove(o); });

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

        setTimer(obj, m_cfg.castTicks,
            [this](Object& o, ThrdIdx threadIdx){ onEndCast(o, threadIdx); });

        auto&& castInfo = obj.getCastInfo();
        forObjectsAround(obj.m_pos, [&](Object& otherObj)
        {
            if (otherObj.m_eventHandler)
                otherObj.m_eventHandler->seeBeginCast(castInfo);
        });
    }

    void onEndCast(Object& obj, ThrdIdx threadIdx)
    {
        assert(obj.m_state == PlayerState::Casting);
        obj.m_state = PlayerState::Idle;

        forObjectsAround(obj.m_pos, [&](Object& otherObj)
        {
            if (otherObj.m_eventHandler)
                otherObj.m_eventHandler->seeEndCast(obj.m_id);
        });

        switch (obj.m_spell)
        {
        case Spell::Lightning:
            castAtPoint(obj.m_spell, obj.m_castDest, threadIdx);
            break;

        case Spell::SelfHeal:
            castSelfHeal(obj);
            break;

        default:
            assert(!"unknown spell");
        }
    }

    void castAtPoint(Spell spell, const Point& dest, ThrdIdx threadIdx)
    {
        if (auto victim = objectAt(dest))
        {
            damageObject(*victim, spell, threadIdx);
        }

        SpellEffect effect{spell, dest};
        createEffect(effect);
    }

    void castSelfHeal(Object& obj)
    {
        auto spellIdx = static_cast<unsigned>(Spell::SelfHeal);
        assert(spellIdx < m_cfg.spellHpDelta.size());
        auto hpDelta = m_cfg.spellHpDelta[spellIdx];
        obj.m_health += hpDelta;
        if (obj.m_health > 100)
            obj.m_health = 100;
        obj.m_eventHandler->healthChange(obj.m_health);
    }

    void createEffect(const SpellEffect& effect)
    {
        forObjectsAround(effect.m_pos, [&](Object& otherObj)
        {
            if (otherObj.m_eventHandler)
                otherObj.m_eventHandler->seeEffect(effect);
        });
    }

    void damageObject(ObjectAPI& obj, Spell spell, ThrdIdx threadIdx)
    {
        auto spellIdx = static_cast<unsigned>(spell);
        assert(spellIdx < m_cfg.spellHpDelta.size());
        auto hpDelta = m_cfg.spellHpDelta[spellIdx];
        obj.modifyHP(hpDelta, threadIdx);
    }

    void updateHealth(Object& obj)
    {
        auto&& arrFirst = begin(obj.m_healthDelta);
        int hpDelta = std::accumulate(arrFirst, arrFirst + m_cfg.threadsCount, 0);
        obj.m_healthDelta.fill(0);

        if (obj.m_health > -hpDelta)
        {
            obj.m_health += hpDelta;
            obj.m_eventHandler->healthChange(obj.m_health);
        }
        else
        {
            obj.m_erased = true;
        }
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

    void dispatchAction(Object& obj, const ActionData& a)
    {
        switch (a.m_action)
        {
        case Action::Disconnect:
            obj.m_erased = true;
            break;

        case Action::Move:
            beginMove(obj, a.m_moveDir);
            break;

        case Action::Cast:
            beginCast(obj, a.m_spell, a.m_castDest);
            break;

        case Action::None:
        default:
            assert(!"unknown action");
        }
    }

    void updateObjects()
    {
        decltype(m_objects.m_objects) newTable;
        m_objects.parallel_for(m_cfg.threadsCount, [&](const std::shared_ptr<Object>& objPtr, ThrdIdx threadIdx)
        {
            auto&& obj = *objPtr;

            if (!obj.m_nextAction.empty())
            {
                dispatchAction(obj, obj.m_nextAction);
                obj.m_nextAction.clear();
            }

            if (obj.m_timerCallback && now() >= obj.m_timerDeadline)
            {
                decltype(obj.m_timerCallback) callback;
                callback.swap(obj.m_timerCallback);
                callback(obj, threadIdx);
            }

            newTable[obj.m_id] = objPtr;
        });

        for (auto nextIter = begin(newTable), E = end(newTable); nextIter != E; )
        {
            auto currentIter = nextIter;
            ++nextIter;
            auto&& obj = *currentIter->second;
            
            updateHealth(obj);

            if (obj.m_erased)
            {
                onDisconnect(obj);
                newTable.erase(currentIter);
            }
        }

        m_objects.m_objects.swap(newTable);
    }

    ObjectAPI* objectAt(const Point& pt)
    {
        if (auto id = m_world.objectAt(pt))
        {
            auto objPtr = m_objects.findObject(id);
            assert(objPtr && "inconsistent World data");
            return objPtr;
        }

        return nullptr;
    }

    template<typename Callback>
    void setTimer(Object& obj, int delay, Callback&& callback)
    {
        assert(!obj.m_timerCallback);
        obj.m_timerDeadline = now() + delay;
        obj.m_timerCallback = std::forward<Callback>(callback);
    }

    ObjectManager m_objects;
    World m_world{m_cfg.worldCX, m_cfg.worldCY};
    ticks_t m_now{0};
};