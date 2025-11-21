#pragma once
#include "../../ModuleBase/Module.h"
#include <unordered_map>

struct BlockPosHash2 {
    size_t operator()(const BlockPos& pos) const noexcept {
        size_t hx = std::hash<int>()(pos.x);
        size_t hy = std::hash<int>()(pos.y);
        size_t hz = std::hash<int>()(pos.z);
        return hx ^ (hy << 1) ^ (hz << 2);
    }
};

struct BlockVisualInfo {
    UIColor fillColor;
    UIColor lineColor;
};

class StorageESP : public Module {
public:
    StorageESP();
    void LevelRenderHook() override;
    void MCRHook(MinecraftUIRenderContext* renderCtx) override;
    bool tracer2D;

    UIColor chestFillColor;
    UIColor chestLineColor;
    UIColor shulkerFillColor;
    UIColor shulkerLineColor;

    UIColor hopperFillColor;
    UIColor hopperLineColor;
    UIColor dropperFillColor;
    UIColor dropperLineColor;
    UIColor dispenserFillColor;
    UIColor dispenserLineColor;

    bool showHopper;
    bool showDropper;
    bool showDispenser;

    std::unordered_map<BlockPos, BlockVisualInfo, BlockPosHash2> blockList;
};
