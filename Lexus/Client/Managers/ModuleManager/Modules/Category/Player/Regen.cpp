#include "Regen.h"
#include "PacketMine.h"
#include "../../Client/Client.h"
#include <chrono>

Regen::Regen() : Module(Category::World, "Regen", "Mines redstone using PacketMine [for zeqa]") {
    addSetting(new SliderSetting<float>("Horizontal Range", "Range to search for redstone", &horizontalRange, 6.f, 1.f, 12.f));
}

BlockPos lastTarget = BlockPos(0, 0, 0);
std::chrono::steady_clock::time_point mineStartTime = std::chrono::steady_clock::now();

void Regen::OnTick() {
    auto lp = g_Data.getLocalPlayer();
    if (!lp) return;

    auto region = g_Data.getClientInstance()->getRegion();
    auto packetMine = ModuleManager::getModule<PacketMine>();
    auto aura = ModuleManager::getModule<KillAura>();

    if (!packetMine || !packetMine->isEnabled()) return;

    if (g_Data.getLocalPlayer()->getAttribute(AttributeId::Absorption) && g_Data.getLocalPlayer()->getAttribute(AttributeId::Absorption)->currentValue >= 10.f) { currentMode = "Idle"; return; }

    if (aura->isEnabled() && !aura->targetList.empty()) {
        currentMode = "Idle";
        return;
    }

    Vec3 playerVec = lp->getPos();
    BlockPos playerPos = playerVec.toInt();
    float range = horizontalRange;

    if (lastTarget != BlockPos(0, 0, 0)) {
        float dist = playerVec.dist(Vec3{ (float)lastTarget.x, (float)lastTarget.y, (float)lastTarget.z });
        if (dist <= range + 0.5f) {
            Block* block = region->getBlock(lastTarget);
            if (block && block->blockLegacy && block->blockLegacy->blockName.find("redstone") != std::string::npos) {
                auto now = std::chrono::steady_clock::now();
                float elapsed = std::chrono::duration<float>(now - mineStartTime).count();
                if (elapsed > 1.5f) {
                    packetMine->reset();
                    lastTarget = BlockPos(0, 0, 0);
                    currentMode = "Reset";
                    return;
                }
                currentMode = "Mining";
                return;
            }
        }
        lastTarget = BlockPos(0, 0, 0);
    }

    BlockPos closestPos = BlockPos(0, 0, 0);
    float closestDist = FLT_MAX;

    for (int x = -range; x <= range; x++) {
        for (int y = -3; y <= 3; y++) {
            for (int z = -range; z <= range; z++) {
                BlockPos checkPos(playerPos.x + x, playerPos.y + y, playerPos.z + z);
                Block* block = region->getBlock(checkPos);
                if (!block || !block->blockLegacy) continue;

                std::string name = block->blockLegacy->blockName;
                if (name.find("redstone") != std::string::npos && name.find("bedrock") == std::string::npos) {
                    float dist = playerVec.dist(Vec3{ (float)checkPos.x, (float)checkPos.y, (float)checkPos.z });
                    if (dist < closestDist) {
                        closestDist = dist;
                        closestPos = checkPos;
                    }
                }
            }
        }
    }

    if (closestPos != BlockPos(0, 0, 0)) {
        bool idk = false;
        lp->getgameMode()->startDestroyBlock(closestPos, 1, idk);
        packetMine->mine(closestPos, 1);
        lastTarget = closestPos;
        mineStartTime = std::chrono::steady_clock::now();
        currentMode = "Acquiring";
        return;
    }

    currentMode = "Idle";
}

std::string Regen::getModeText() {
    return currentMode;
}

void Regen::onEnable() {}

void Regen::onDisable() {}
