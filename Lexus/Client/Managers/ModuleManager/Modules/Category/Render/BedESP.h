#pragma once
#include "../../ModuleBase/Module.h"
#include <unordered_map>

struct BlockPosHash {
    size_t operator()(const BlockPos& pos) const noexcept {
        size_t hx = std::hash<int>()(pos.x);
        size_t hy = std::hash<int>()(pos.y);
        size_t hz = std::hash<int>()(pos.z);
        return hx ^ (hy << 1) ^ (hz << 2);
    }
};

class BedESP : public Module {
public:
    BedESP();
    void LevelRenderHook() override;
    UIColor fillColor;
    UIColor lineColor;
    std::unordered_map<BlockPos, UIColor, BlockPosHash> blockList;
};