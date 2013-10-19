#pragma once

#include <cstdint>

using ObjectId = std::uint32_t;

enum class PlayerState
{
    Idle, MovingOut, MovingIn,
};