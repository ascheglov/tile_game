#pragma once

#include <cassert>
#include <vector>
#include "types.hpp"
#include "math.hpp"

class World
{
public:
    explicit World(int cx, int cy)
        : m_cx(cx), m_cy(cy)
    {
        m_objectTable.resize(m_cy);
        for (auto& v : m_objectTable)
            v.resize(cx);
    }

    ObjectId objectAt(const Point& pt) const
    {
        return isValidPoint(pt) ? getAt(pt) : ObjectId{};
    }

    void addObject(ObjectId id, const Point& pt)
    {
        assert(isValidPoint(pt));
        assert(!objectAt(pt));
        setAt(pt, id);
    }

    void removeObject(const Point& pt)
    {
        assert(objectAt(pt));
        setAt(pt, {});
    }

    void lockCell(ObjectId lockOwner, const Point& pt)
    {
        addObject(makeLockId(lockOwner), pt);
    }

    void moveToCell(const Point& from, const Point& dest)
    {
        assert(isLocked(dest));
        auto id = objectAt(from);
        assert(id == withoutReserved(objectAt(dest)));
        removeObject(from);
        removeObject(dest);
        addObject(id, dest);
    }

private:
    static const std::uint8_t CellLockFlag = 0x80;

    static ObjectId withoutReserved(ObjectId id)
    {
        id.f.reserved = 0;
        return id;
    }

    ObjectId makeLockId(ObjectId id) const
    {
        id.f.reserved |= CellLockFlag;
        return id;
    }

    bool isLocked(const Point& pt)
    {
        return (objectAt(pt).f.reserved & CellLockFlag) != 0;
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
