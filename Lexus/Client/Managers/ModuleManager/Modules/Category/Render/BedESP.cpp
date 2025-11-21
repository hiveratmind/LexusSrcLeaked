#include "BedESP.h"
#include <string>

BedESP::BedESP() : Module(Category::Render, "BedESP", "Draws chams on all beds anywhere"),
fillColor(UIColor(255, 0, 0, 100)),
lineColor(UIColor(255, 255, 255, 255))
{
    addSetting(new ColorSetting("Fill Color", "Color of bed fill", &fillColor, fillColor));
    addSetting(new ColorSetting("Line Color", "Color of bed outline", &lineColor, lineColor));
}

void BedESP::LevelRenderHook() {
    LocalPlayer* localPlayer = g_Data.getLocalPlayer();
    if (!localPlayer) return;

    for (auto it = blockList.begin(); it != blockList.end();) {
        BlockPos pos = it->first;
        Block* block = g_Data.clientInstance->getRegion()->getBlock(pos);
        if (!block || !block->blockLegacy || block->blockLegacy->blockId == 0) {
            it = blockList.erase(it);
            continue;
        }
        std::string name = block->blockLegacy->blockName;
        bool valid = name.find("bed") != std::string::npos;
        if (!valid) {
            it = blockList.erase(it);
        }
        else {
            ++it;
        }
    }

    if (blockList.empty()) return;

    std::unordered_set<BlockPos, BlockPosHash> rendered;

    for (const auto& entry : blockList) {
        const BlockPos& pos = entry.first;
        if (rendered.find(pos) != rendered.end()) continue;

        Block* block = g_Data.clientInstance->getRegion()->getBlock(pos);
        if (!block) continue;

        AABB box1;
        BlockPos nonConstPos = pos;
        block->blockLegacy->getOutline(block, g_Data.clientInstance->getRegion(), &nonConstPos, &box1);

        std::vector<BlockPos> neighbors = {
            BlockPos(pos.x + 1, pos.y, pos.z),
            BlockPos(pos.x - 1, pos.y, pos.z),
            BlockPos(pos.x, pos.y, pos.z + 1),
            BlockPos(pos.x, pos.y, pos.z - 1)
        };

        bool merged = false;
        AABB mergedBox = box1;

        for (const BlockPos& neighborPos : neighbors) {
            if (blockList.find(neighborPos) == blockList.end()) continue;

            Block* neighborBlock = g_Data.clientInstance->getRegion()->getBlock(neighborPos);
            if (!neighborBlock) continue;
            std::string neighborName = neighborBlock->blockLegacy->blockName;
            if (neighborName.find("bed") == std::string::npos) continue;

            AABB box2;
            BlockPos nonConstNeighborPos = neighborPos;
            neighborBlock->blockLegacy->getOutline(neighborBlock, g_Data.clientInstance->getRegion(), &nonConstNeighborPos, &box2);

            mergedBox.lower.x = std::min(mergedBox.lower.x, box2.lower.x);
            mergedBox.lower.y = std::min(mergedBox.lower.y, box2.lower.y);
            mergedBox.lower.z = std::min(mergedBox.lower.z, box2.lower.z);

            mergedBox.upper.x = std::max(mergedBox.upper.x, box2.upper.x);
            mergedBox.upper.y = std::max(mergedBox.upper.y, box2.upper.y);
            mergedBox.upper.z = std::max(mergedBox.upper.z, box2.upper.z);

            rendered.insert(neighborPos);

            merged = true;
            break;
        }

        rendered.insert(pos);

        RenderUtils::Draw3dBox(mergedBox, fillColor, lineColor, 1.f);
    }
}