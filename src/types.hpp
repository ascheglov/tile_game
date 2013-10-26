#pragma once

#include <cstdint>

using ObjectId = std::uint32_t;

using ticks_t = std::uint32_t;  // 1.36 years at 100 ticks per second

enum class PlayerState
{
    Idle, MovingOut, MovingIn, Casting,
};

enum class Spell
{
    Lightning, SelfHeal,
};

enum class Effect
{
    None, Lightning,
};
