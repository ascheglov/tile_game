#pragma once

#include <cassert>
#include <algorithm>
#include <atomic>
#include <deque>
#include <functional>
#include <list>
#include <memory>
#include <numeric>
#include <thread>
#include <unordered_map>

#include "GameCfg.hpp"
#include "types.hpp"
#include "events.hpp"
#include "math.hpp"
#include "Object.hpp"
#include "Geodata.hpp"
#include "World.hpp"
#include "ObjectManager.hpp"

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
     
        InitInfo initInfo;
        initInfo.m_id = obj.m_id;
        initInfo.m_name = obj.m_name;
        initInfo.m_pos = obj.m_pos;
        initInfo.m_health = obj.m_health;
        obj.m_eventHandler->init(initInfo);

        assert(!m_world.objectAt(pos));
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

        if (m_world.objectAt(destPt))
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
        int hpDelta = std::accumulate(begin(obj.m_healthDelta), end(obj.m_healthDelta), 0);
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
        m_objects.for_each([&](Object& obj)
        {
            if (distance(pt, obj.m_pos) <= m_cfg.playerViewRadius)
                callback(obj);
        });
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
        m_objects.parallel_for_each([&](Object& obj, ThrdIdx threadIdx)
        {
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
        });

        m_objects.parallel_for_each([&](Object& obj, ThrdIdx threadIdx)
        {
            updateHealth(obj);

            if (obj.m_erased)
            {
                onDisconnect(obj);
                m_objects.eraseObject(obj, threadIdx);
            }
        });

        m_objects.mergeErasedObjectsLists();
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

    ObjectManager m_objects{m_cfg.threadsCount};
    World m_world{m_cfg.worldCX, m_cfg.worldCY};
    ticks_t m_now{0};
};