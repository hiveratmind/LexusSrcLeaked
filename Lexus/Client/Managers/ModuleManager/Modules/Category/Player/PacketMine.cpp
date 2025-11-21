#include "PacketMine.h"
#include "../../Client/Client.h"

bool _instantX = false;

PacketMine::PacketMine() : Module(Category::World, "PacketMine", "Automatically mines blocks you click") {
    addSetting(new SliderSetting<float>("Mine Speed", "Mining speed multiplier", &mineSpeed, 1.f, 0.1f, 5.f));
    addSetting(new SliderSetting<float>("Mine Range", "Block mining distance", &mineRange, 6.f, 1.f, 12.f));
    addSetting(new BoolSetting("Switch Back", "Switch back to original slot after mining", &switchBack, true));
    addSetting(new EnumSetting("Multitask", "Controls mining while using items", { "Normal", "Strict", "None" }, &multiTask, 0));
    addSetting(new BoolSetting("Instant", "Bypasses instant break", &_instantX, false));
}

void PacketMine::reset() {
    currentBlock = { BlockPos(0, 0, 0), -1 };
    restoringSlot = false;
    lastSlot = -1;
    if (auto lp = g_Data.getLocalPlayer()) {
        if (auto gm = lp->gameMode)
            gm->destroyProgress = 0.f;
    }
}

bool PacketMine::isValid(const BlockPos& pos) const {
    if (pos == BlockPos(0, 0, 0)) return false;
    auto lp = g_Data.getLocalPlayer();
    if (!lp) return false;
    auto block = g_Data.getClientInstance()->getRegion()->getBlock(pos);
    if (!block || !block->blockLegacy || block->blockLegacy->canBeBuiltOver(g_Data.getClientInstance()->getRegion(), pos))
        return false;
    if (block->blockLegacy->blockId == 7) return false;
    if (pos.CastTo<float>().dist(lp->getPos()) > mineRange) return false;
    return true;
}

std::pair<float, int> PacketMine::getBestTool(const BlockPos& pos) {
    auto lp = g_Data.getLocalPlayer();
    if (!lp) return { 0.f, lp->playerInventory->selectedSlot };
    auto gm = lp->gameMode;
    auto region = g_Data.getClientInstance()->getRegion();
    auto block = region->getBlock(pos);

    float bestSpeed = gm->getDestroyRate(block);
    int bestSlot = lp->playerInventory->selectedSlot;

    for (int i = 0; i < 9; i++) {
        lp->playerInventory->selectedSlot = i;
        float rate = gm->getDestroyRate(block);
        if (rate > bestSpeed) {
            bestSpeed = rate;
            bestSlot = i;
        }
    }

    lp->playerInventory->selectedSlot = bestSlot;
    return { bestSpeed, bestSlot };
}

void PacketMine::mine(const BlockPos& pos, uint8_t face) {
    if (!isValid(pos)) return;
    auto lp = g_Data.getLocalPlayer();
    if (!lp) return;
    auto gm = lp->gameMode;
    gm->destroyProgress = 0.f;
    currentBlock = { pos, face };
}

void PacketMine::OnTick() {
    static bool instantActive = false;
    LocalPlayer* lp = g_Data.getLocalPlayer();
    if (!lp) return;
    auto gm = lp->gameMode;
    if (!gm) return;

    Minecraft* mc = g_Data.clientInstance->minecraft;
    if (!mc) return;

    bool moving = g_Data.isKeyDown('W') || g_Data.isKeyDown('A') ||
        g_Data.isKeyDown('S') || g_Data.isKeyDown('D');

    // Reset timer if we are moving or not mining with _instantX
    if (!_instantX || currentBlock.first == BlockPos(0, 0, 0) || moving) {
        if (instantActive) {
            *mc->minecraftTimer = 20.f;
            *mc->minecraftRenderTimer = 20.f;
            instantActive = false;
        }
    }

    if (multiTask == 1 && lp->getItemUseDuration() > 0) {
        gm->destroyProgress = 0.f;
        currentBlock = { BlockPos(0, 0, 0), -1 };
        return;
    }

    if (!isValid(currentBlock.first)) {
        currentBlock = { BlockPos(0, 0, 0), -1 };
        gm->destroyProgress = 0.f;
        return;
    }

    auto [rate, bestSlot] = getBestTool(currentBlock.first);

    if (gm->destroyProgress < 1.f) {
        gm->destroyProgress += rate * mineSpeed;
        if (gm->destroyProgress > 1.f)
            gm->destroyProgress = 1.f;

        // Apply instant mining only if _instantX and not moving
        if (_instantX && !moving) {
            *mc->minecraftTimer = 1000.f;
            *mc->minecraftRenderTimer = 1000.f;
            instantActive = true;
        }
    }
    else {
        if (multiTask == 0 && lp->getItemUseDuration() > 0)
            return;

        auto spoof = MinecraftPackets::createPacket(PacketID::MobEquipment);
        auto* pkt = reinterpret_cast<MobEquipmentPacket*>(spoof.get());
        pkt->mSlot = bestSlot;
        pkt->mSelectedSlot = bestSlot;
        pkt->mContainerId = 0;
        pkt->mSlotByte = bestSlot;
        pkt->mSelectedSlotByte = bestSlot;
        pkt->mContainerIdByte = 0;
        lp->level->getPacketSender()->sendToServer(pkt);

        int oldSlot = lp->playerInventory->selectedSlot;
        lp->playerInventory->selectedSlot = bestSlot;
        gm->destroyBlock(currentBlock.first, currentBlock.second);

        if (switchBack) {
            lp->playerInventory->selectedSlot = oldSlot;
            auto spoofBack = MinecraftPackets::createPacket(PacketID::MobEquipment);
            auto* pktBack = reinterpret_cast<MobEquipmentPacket*>(spoofBack.get());
            pktBack->mSlot = oldSlot;
            pktBack->mSelectedSlot = oldSlot;
            pktBack->mContainerId = 0;
            pktBack->mSlotByte = oldSlot;
            pktBack->mSelectedSlotByte = oldSlot;
            pktBack->mContainerIdByte = 0;
            lp->level->getPacketSender()->sendToServer(pktBack);
        }

        gm->destroyProgress = 0.f;
        currentBlock = { BlockPos(0, 0, 0), -1 };

        if (_instantX && !moving) {
            *mc->minecraftTimer = 20.f;
            *mc->minecraftRenderTimer = 20.f;
            instantActive = false;
        }
    }
}


UIColor UIColorlerp(const UIColor& start, const UIColor& end, float t) {
    t = std::clamp(t, 0.f, 1.f);
    return UIColor(
        static_cast<uint8_t>(start.r + (end.r - start.r) * t),
        static_cast<uint8_t>(start.g + (end.g - start.g) * t),
        static_cast<uint8_t>(start.b + (end.b - start.b) * t),
        static_cast<uint8_t>(start.a + (end.a - start.a) * t)
    );
}

void PacketMine::LevelRenderHook() {
    auto lp = g_Data.getLocalPlayer();
    if (!lp) return;
    auto gm = lp->gameMode;
    if (!gm) return;

    float progress = gm->destroyProgress;
    if (progress <= 0.f || currentBlock.first == BlockPos(0, 0, 0))
        return;

    auto block = g_Data.getClientInstance()->getRegion()->getBlock(currentBlock.first);
    AABB aabb;
    block->blockLegacy->getOutline(block, g_Data.getClientInstance()->getRegion(), &currentBlock.first, &aabb);

    UIColor fill = UIColorlerp(UIColor(255, 0, 0), UIColor(0, 255, 0), progress);
    fill.a = 75;
    UIColor outline = fill;
    outline.a = 255;

    RenderUtils::Draw3dBox(aabb, fill, outline, progress);
}
