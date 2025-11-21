#include "AntiVoid.h"

AntiVoid::AntiVoid() : Module(Category::Player, "AntiVoid", "Prevents you from falling into the void") {
    addSetting(new SliderSetting<float>("Fall Distance", "Distance you can fall before teleporting", &fallDistance, 5.f, 0.f, 15.f));
    addSetting(new BoolSetting("Teleport Once", "Only teleport once", &teleportOnce, false));

    onGroundPositions.clear();
    teleported = false;
    canTeleport = true;

    lastY = 0.f;
    lastYInitialized = false;
}

void AntiVoid::onEnable() {
    onGroundPositions.clear();
    teleported = false;
    canTeleport = true;

    lastYInitialized = false;
}

void AntiVoid::onDisable() {
    onGroundPositions.clear();
    teleported = false;
    canTeleport = true;

    lastYInitialized = false;
}

void AntiVoid::OnTick() {
    auto* player = g_Data.getLocalPlayer();
    if (!player) return;

    auto pos = player->getPos();

    if (!lastYInitialized) {
        lastY = pos.y;
        lastYInitialized = true;
    }

    float fallDistEstimate = 0.f;
    if (pos.y < lastY) {
        fallDistEstimate = lastY - pos.y;
    }
    else {
        fallDistEstimate = 0.f;
    }
    lastY = pos.y;

    if (player->isOnGround()) {
        canTeleport = true;
        teleported = false;

        float feetY = pos.y - 1.62f;

        int blockX = static_cast<int>(std::floor(pos.x));
        int blockY = static_cast<int>(std::floor(feetY - 1.f));
        int blockZ = static_cast<int>(std::floor(pos.z));

        Vec3<int> blockPos = { blockX, blockY, blockZ };
        onGroundPositions.push_back(blockPos);
        if (onGroundPositions.size() > 40)
            onGroundPositions.erase(onGroundPositions.begin());
        return;
    }

    bool hasTeleported = teleported && teleportOnce;

    if (fallDistEstimate > fallDistance && (!hasTeleported || !teleportOnce) && canTeleport) {
        Vec3<float> checkPos = { pos.x, pos.y - 1.62f, pos.z };

        if (isOverVoid(checkPos)) {
            player->displayClientMessage("AntiVoid: Teleporting back!");

            teleportPlayerBack();
        }
    }
}

bool AntiVoid::isGoodBlock(const Vec3<int>& pos) {
    auto region = g_Data.getClientInstance()->getRegion();
    if (!region) return false;

    auto block = region->getBlock(pos);
    if (!block || !block->blockLegacy) return false;

    int blockId = block->blockLegacy->blockId;
    bool isLiquid = (blockId >= 8 && blockId <= 11);
    return blockId != 0 && !isLiquid && block->blockLegacy->isSolid();
}

bool AntiVoid::isOverVoid(const Vec3<float>& pos) {
    auto region = g_Data.getClientInstance()->getRegion();
    if (!region) return false;

    Vec3<int> blockPos = {
        static_cast<int>(std::floor(pos.x)),
        static_cast<int>(std::floor(pos.y)),
        static_cast<int>(std::floor(pos.z))
    };

    for (int y = blockPos.y; y >= 0; y--) {
        auto block = region->getBlock({ blockPos.x, y, blockPos.z });
        if (!block || !block->blockLegacy) continue;
        if (block->blockLegacy->blockId != 0)
            return false;
    }

    return true;
}

void AntiVoid::teleportPlayerBack() {
    auto* player = g_Data.getLocalPlayer();
    if (!player) return;

    Vec3<int> bestBlockPos = { 0, 0, 0 };
    bool found = false;

    for (auto it = onGroundPositions.rbegin(); it != onGroundPositions.rend(); ++it) {
        if (isGoodBlock(*it)) {
            bestBlockPos = *it;
            found = true;
            break;
        }
    }

    if (!found) {
        player->displayClientMessage("AntiVoid: No safe position found to teleport!");
        return;
    }

    Vec3<float> currentPos = player->getPos();
    Vec3<float> targetPos = {
        bestBlockPos.x + 0.5f,
        static_cast<float>(bestBlockPos.y) + 1.0f + 1.62f,
        bestBlockPos.z + 0.5f
    };

    Vec3<float> direction = targetPos.sub(currentPos);
    float distance = direction.magnitude();

    if (distance > 0.1f) {
        direction = direction.normalize() * 0.8f; 
        player->lerpMotion(direction);
    }

    player->setFallDistance(0.0f);

    if (distance < 1.0f) {
        teleported = true;
        if (teleportOnce) {
            onGroundPositions.clear();
            canTeleport = false;
        }
    }
}

std::string AntiVoid::getModeText() {
    return std::to_string(fallDistance);
}
