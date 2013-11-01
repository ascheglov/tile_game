#pragma once

#include <cstdint>
#include <type_traits>

struct ObjectId
{
    static const std::uint8_t NonEmptyFlag = 0x8;

    ObjectId() : value{0} {}

    explicit ObjectId(std::uint32_t index) : value{0}
    {
        f.index = index;
        f.flags = NonEmptyFlag;
    }

    union
    {
        std::uint64_t value;

        struct
        {
            std::uint32_t index;
            std::uint8_t version;
            std::uint8_t flags;
            std::uint16_t reserved;
        } f;
    };

    explicit operator bool() const { return (f.flags & NonEmptyFlag) != 0; }
};

inline bool operator==(const ObjectId& lhs, const ObjectId& rhs) { return lhs.value == rhs.value; }
inline bool operator!=(const ObjectId& lhs, const ObjectId& rhs) { return !(lhs == rhs); }

static_assert(sizeof(std::declval<ObjectId>().f) == 8, "");

namespace std
{
    template<> struct hash<ObjectId>
    {
        std::size_t operator()(const ObjectId& id) const
        {
            return std::hash<std::uint64_t>()(id.value);
        }
    };
}

using ticks_t = std::uint32_t;  // 1.36 years at 100 ticks per second

using ThrdIdx = unsigned;

enum class Action
{
    None, Move, Cast, Disconnect,
};

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
