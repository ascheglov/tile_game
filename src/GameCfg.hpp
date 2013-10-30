#pragma once

#include <array>

struct GameCfg
{
    int worldCX{8};
    int worldCY{8};
    int playerViewRadius{2};
    int moveTicks{1};
    int castTicks{1};
    std::array<int, 2> spellHpDelta{{-51, +26}};
    unsigned threadsCount{1};
};

