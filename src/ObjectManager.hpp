#pragma once

#include <cassert>
#include <atomic>
#include <deque>
#include <list>
#include <thread>

#include "build_config.hpp"
#include "types.hpp"
#include "Object.hpp"

class ObjectManager
{
public:
    ObjectManager(unsigned threadsCount) : m_threadsCount{threadsCount} {}

    Object& newObject()
    {
        if (m_free.empty())
        {
            m_arr.emplace_back(ObjectId(m_arr.size()));
            return m_arr.back();
        }

        auto idx = m_free.front();
        m_free.pop_front();
        auto& el = m_arr[idx];
        assert(el.m_isFree);
        ++el.m_id.f.version;
        el = Object(el.m_id);
        return el;
    }

    ObjectAPI* findObject(ObjectId id)
    {
        auto idx = id.f.index;
        if (idx >= m_arr.size())
            return nullptr;
        auto&& el = m_arr[idx];
        return (!el.m_isFree && el.m_id == id) ? &el : nullptr;
    }

    void eraseObject(Object& obj, ThrdIdx threadIdx)
    {
        assert(threadIdx < MaxThreads);

        auto idx = obj.m_id.f.index;
        assert(idx < m_arr.size());
        m_freeLists[threadIdx].push_front(idx);
        obj.m_isFree = true;
    }

    void mergeErasedObjectsLists()
    {
        for (auto& lst : m_freeLists)
            m_free.splice(m_free.end(), lst);
    }

    template<typename F>
    void parallel_for_each(F&& f)
    {
        assert(m_threadsCount == 1);
        auto n = m_arr.size();
        if (n <= ThreadBlockSize)
        {
            for_idx(0, n, f, 0);
            return;
        }

        std::atomic<unsigned> nextBlockBase{0};

        auto&& threadFn = [&](unsigned threadIdx)
        {
            for (;;)
            {
                auto blockBase = nextBlockBase.fetch_add(ThreadBlockSize);
                if (blockBase >= n)
                    return;

                for_idx(blockBase, ThreadBlockSize, f, threadIdx);
            }
        };

        std::vector<std::thread> threads;
        for (auto threadIdx = 1u; threadIdx < m_threadsCount; ++threadIdx)
            threads.emplace_back(threadFn, threadIdx);

        threadFn(0);

        for (auto&& th : threads)
            th.join();
    }

    template<typename F>
    void for_each(F&& f)
    {
        for_idx(0, m_arr.size(), f);
    }

private:
    template<typename F, typename... Ts>
    void for_idx(unsigned base, unsigned n, F&& f, Ts&&... ts)
    {
        auto lastIdx = base + n;
        if (lastIdx > m_arr.size()) lastIdx = m_arr.size();

        for (auto it = begin(m_arr) + base, E = begin(m_arr) + lastIdx; it != E; ++it)
        {
            if (!it->m_isFree)
                f(*it, ts...);
        }
    }

    std::deque<Object> m_arr;
    std::array<std::list<unsigned>, MaxThreads> m_freeLists;
    std::list<unsigned> m_free;

    unsigned m_threadsCount;
};
