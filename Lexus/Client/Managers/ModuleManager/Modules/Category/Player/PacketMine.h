#pragma once
#include "../../ModuleBase/Module.h"

class PacketMine : public Module {
private:
    float mineSpeed = 1.f;
    float mineRange = 6.f;
    bool switchBack = true;
    int multiTask = 0;

    std::pair<BlockPos, uint8_t> currentBlock;

    int lastSlot = -1;
    bool restoringSlot = false;

public:
    PacketMine();
    void reset();
    void mine(const BlockPos& pos, uint8_t face);
    bool isValid(const BlockPos& pos) const;
    std::pair<float, int> getBestTool(const BlockPos& pos);

    BlockPos getMiningPos() const { return currentBlock.first; }
    uint8_t getMiningFace() const { return currentBlock.second; }

    void OnTick() override;
    void LevelRenderHook() override;
};
