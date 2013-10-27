#pragma once

#include "types.hpp"
#include "math.hpp"
#include "events.hpp"
#include "Game.hpp"

struct TestClient : EventHandler, FullPlayerInfo
{
    Game* m_game;

    bool m_isConnected{false};

    std::unordered_map<ObjectId, FullPlayerInfo> see;

    int m_health;

    TestClient(Game& game, Point pos) : m_game{&game}
    {
        game.newPlayer(*this, pos);
    }

    void requestDisconnect()
    {
        ActionData ad;
        ad.m_action = Action::Disconnect;
        m_game->enqueueAction(m_id, ad);
    }

    void requestMove(Dir direction)
    {
        ActionData ad;
        ad.m_action = Action::Move;
        ad.m_moveDir = direction;
        m_game->enqueueAction(m_id, ad);
    }

    void requestCast(Spell spell, Point dest = {})
    {
        ActionData ad;
        ad.m_action = Action::Cast;
        ad.m_spell = spell;
        ad.m_castDest = dest;
        m_game->enqueueAction(m_id, ad);
    }

    Effect seeEffect(Point pt)
    {
        dropExpiredEffects();
        
        auto it = m_effects.find(pt);
        if (it == m_effects.end())
            return Effect::None;

        return it->second.first;
    }

private:
    FullPlayerInfo& findInfo(ObjectId id)
    {
        if (id == m_id) return *this;
        assert(see.count(id) != 0);
        return see[id];
    }

    virtual void init(ObjectId assignedId, const Point& pos, int health) override
    {
        assert(!m_isConnected);
        m_isConnected = true;
        m_id = assignedId;
        m_pos = pos;
        m_health = health;
        m_state = PlayerState::Idle;
    }

    virtual void seePlayer(const FullPlayerInfo& info) override
    {
        assert(m_id != info.m_id);
        assert(see.count(info.m_id) == 0);
        see[info.m_id] = info;
    }

    virtual void disconnect() override
    {
        m_isConnected = false;
    }

    virtual void seeDisappear(ObjectId id) override
    {
        assert(m_id != id);
        assert(see.count(id) != 0);
        see.erase(id);
    }

    virtual void seeBeginMove(const MoveInfo& info) override
    {
        auto& localInfo = findInfo(info.id);
        localInfo.m_state = PlayerState::MovingOut;
        localInfo.m_moveDir = info.moveDir;
    }

    virtual void seeCrossCellBorder(ObjectId id) override
    {
        auto& localInfo = findInfo(id);
        localInfo.m_state = PlayerState::MovingIn;
        localInfo.m_pos = moveRel(localInfo.m_pos, localInfo.m_moveDir);
    }

    virtual void seeStop(ObjectId id) override
    {
        auto& localInfo = findInfo(id);
        localInfo.m_state = PlayerState::Idle;
    }

    virtual void seeBeginCast(const CastInfo& info) override
    {
        auto& localInfo = findInfo(info.m_id);
        localInfo.m_state = PlayerState::Casting;
        localInfo.m_spell = info.m_spell;
    }

    virtual void seeEndCast(ObjectId id) override
    {
        auto& localInfo = findInfo(id);
        localInfo.m_state = PlayerState::Idle;
    }

    virtual void seeEffect(const SpellEffect& effect) override
    {
        dropExpiredEffects();
        auto&& effectVal = std::make_pair(effect.m_effect, now() + 1);
        m_effects.emplace(effect.m_pos, effectVal);
    }

    virtual void healthChange(int newHP) override
    {
        m_health = newHP;
    }

    ticks_t now() const { return m_game->now(); }

    void dropExpiredEffects()
    {
        decltype(m_effects) newEffects;
        for (auto&& val : m_effects)
        {
            if (val.second.second > now())
                newEffects.insert(val);
        }
        m_effects = newEffects;
    }

    std::unordered_map<Point, std::pair<Effect, ticks_t>> m_effects;
};