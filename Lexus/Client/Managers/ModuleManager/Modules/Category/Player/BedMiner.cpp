#include "BedMiner.h"
#include "PacketMine.h"
#include "../../Client/Client.h"

BedMiner::BedMiner() : Module(Category::World, "BedMiner", "Mines beds using PacketMine") {
    addSetting(new SliderSetting<float>("Horizontal Range", "Range to search for beds", &horizontalRange, 6.f, 1.f, 12.f));
}

void BedMiner::OnTick() {
    auto lp = g_Data.getLocalPlayer();
    if (!lp) return;

    auto region = g_Data.getClientInstance()->getRegion();
    auto packetMine = ModuleManager::getModule<PacketMine>();
    if (!packetMine || !packetMine->isEnabled()) return;

    BlockPos playerPos = lp->getPos().toInt();
    if (packetMine->getMiningPos() != BlockPos(0, 0, 0)) return;

    int range = static_cast<int>(horizontalRange);
    for (int x = -range; x <= range; x++) {
        for (int y = -3; y <= 3; y++) {
            for (int z = -range; z <= range; z++) {
                BlockPos checkPos = BlockPos(playerPos.x + x, playerPos.y + y, playerPos.z + z);
                Block* block = region->getBlock(checkPos);
                if (!block || !block->blockLegacy) continue;

                std::string name = block->blockLegacy->blockName;
                if (name.find("bed") != std::string::npos && name.find("bedrock") == std::string::npos) {
                    bool idk = false;
                    lp->getgameMode()->startDestroyBlock(checkPos, 1, idk);
                    packetMine->mine(checkPos, 1);
                    return;
                }
            }
        }
    }
}
