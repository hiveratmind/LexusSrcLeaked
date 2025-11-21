#include "Scaffold.h"
#include <cmath>
#include <chrono>

Scaffold::Scaffold() : Module(Category::Player, "Scaffold", "Automatically places blocks under the player") {
    addSetting(new SliderSetting<int>("Delay", "Delay between block placements (ms)", &delay, 0, 0, 1000));
    addSetting(new BoolSetting("Strict", "Strict rotations", &strict, false));

    lastPlaceTime = 0;
    currentRot = { 0.f, 0.f };
}

Vec3<int> Scaffold::getBlockUnderFeet(LocalPlayer* p) {
    if (!p) return { 0, -1337, 0 };

    auto pos = p->getPos();
    auto vel = p->stateVector ? p->stateVector->velocity : Vec3<float>{ 0.f, 0.f, 0.f };

    float x = vel.x, z = vel.z;
    float spd = std::sqrt(x * x + z * z);
    if (spd > 0.01f) {
        pos.x += (x / spd) * 0.3f;
        pos.z += (z / spd) * 0.3f;
    }

    int px = static_cast<int>(std::floor(pos.x));
    int py = static_cast<int>(std::floor(pos.y - 2.0f));
    int pz = static_cast<int>(std::floor(pos.z));

    Vec3<int> foot = { px, py, pz };

    auto region = g_Data.getClientInstance()->getRegion();
    if (!region) return { 0, -1337, 0 };

    auto b = region->getBlock(foot);
    if (b && b->blockLegacy && b->blockLegacy->blockId == 0) {
        for (int i = 0; i < 6; i++) {
            int ox = px, oy = py, oz = pz;
            if (i == 0) oy--;
            else if (i == 1) oy++;
            else if (i == 2) oz--;
            else if (i == 3) oz++;
            else if (i == 4) ox--;
            else if (i == 5) ox++;

            auto neighbor = region->getBlock({ ox, oy, oz });
            if (neighbor && neighbor->blockLegacy && neighbor->blockLegacy->blockId != 0) {
                return foot;
            }
        }
    }
    else {
        return { 0, -1337, 0 };
    }

    for (int dx = -1; dx <= 1; dx++) {
        for (int dz = -1; dz <= 1; dz++) {
            if (dx == 0 && dz == 0) continue;

            int cx = px + dx, cz = pz + dz;
            Vec3<int> checkPos = { cx, py, cz };
            auto fb = region->getBlock(checkPos);
            if (fb && fb->blockLegacy && fb->blockLegacy->blockId == 0) {
                for (int i = 0; i < 6; i++) {
                    int ox = cx, oy = py, oz = cz;
                    if (i == 0) oy--;
                    else if (i == 1) oy++;
                    else if (i == 2) oz--;
                    else if (i == 3) oz++;
                    else if (i == 4) ox--;
                    else if (i == 5) ox++;

                    auto neighbor = region->getBlock({ ox, oy, oz });
                    if (neighbor && neighbor->blockLegacy && neighbor->blockLegacy->blockId != 0) {
                        return checkPos;
                    }
                }
            }
        }
    }

    return { 0, -1337, 0 };
}

int Scaffold::findBlockHotbarSlot(LocalPlayer* player) {
    if (!player || !player->playerInventory || !player->playerInventory->container) return -1;
    Container* hotbar = player->playerInventory->container;
    for (int slot = 0; slot < 9; slot++) {
        ItemStack* stack = hotbar->getItem(slot);
        if (stack && stack->item.get() && stack->isBlockType()) {
            return slot;
        }
    }
    return -1;
}

bool Scaffold::isBlockPlaceable(LocalPlayer* player, const Vec3<int>& pos) {
    if (!player) return false;
    BlockSource* region = g_Data.getClientInstance()->getRegion();
    if (!region) return false;

    Block* block = region->getBlock(pos);
    if (!block || !block->blockLegacy) return false;
    return block->blockLegacy->blockId == 0;
}

int getPlaceableBlockFace(BlockSource* region, const Vec3<int>& pos) {
    static const Vec3<int> directions[6] = {
        {0, -1, 0}, {0, 1, 0}, {0, 0, -1}, {0, 0, 1}, {-1, 0, 0}, {1, 0, 0}
    };
    for (int side = 0; side < 6; ++side) {
        Vec3<int> neighborPos = {
            pos.x + directions[side].x,
            pos.y + directions[side].y,
            pos.z + directions[side].z
        };
        Block* neighborBlock = region->getBlock(neighborPos);
        if (neighborBlock && neighborBlock->blockLegacy && neighborBlock->blockLegacy->blockId != 0) {
            return side;
        }
    }
    return -1;
}

bool spoofSwitch(int slot) {
    auto player = g_Data.getLocalPlayer();
    if (!player || !player->playerInventory || !player->playerInventory->container) return false;
    auto item = player->playerInventory->container->getItem(slot);
    if (!item) return false;

    auto packetShared = MinecraftPackets::createPacket(PacketID::MobEquipment);
    if (!packetShared) return false;
    auto* pkt = reinterpret_cast<MobEquipmentPacket*>(packetShared.get());
    pkt->mSlot = slot;
    pkt->mSelectedSlot = slot;
    pkt->mContainerId = 0;
    pkt->mSlotByte = slot;
    pkt->mSelectedSlotByte = slot;
    pkt->mContainerIdByte = 0;

    auto sender = player->level ? player->level->getPacketSender() : nullptr;
    if (!sender) return false;

    sender->send(pkt);
    sender->sendToServer(pkt);
    return true;
}

void Scaffold::placeBlock(LocalPlayer* player, GameMode* gameMode, const Vec3<int>& pos, int slot) {
    if (!player || !gameMode) return;
    if (slot < 0) return;

    int oldslot = player->playerInventory ? player->playerInventory->selectedSlot : -1;
    if (!spoofSwitch(slot)) return;

    if (player->playerInventory) player->playerInventory->selectedSlot = slot;
    PlayerUtil::tryPlaceBlock(pos, false, false);
    spoofSwitch(oldslot);

    if (player->playerInventory) player->playerInventory->selectedSlot = oldslot;
}

void Scaffold::OnTick() {
    LocalPlayer* player = g_Data.getLocalPlayer();
    if (!player) return;
    GameMode* gameMode = player->getgameMode();
    if (!gameMode) return;

    uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

    if (now - lastPlaceTime < static_cast<uint64_t>(delay)) return;

    int slot = findBlockHotbarSlot(player);
    if (slot == -1) return;

    Vec3<int> underFeetPos = getBlockUnderFeet(player);
    if (underFeetPos.y == -1337) return;

    if (isBlockPlaceable(player, underFeetPos)) {
        placeBlock(player, gameMode, underFeetPos, slot);
        lastPlaceTime = now;

        bool exists = false;
        for (auto& block : fadingBlocks) {
            if (block.pos.x == underFeetPos.x && block.pos.y == underFeetPos.y && block.pos.z == underFeetPos.z) {
                block.alpha = 1.0f;
                exists = true;
                break;
            }
        }
        if (!exists)
            fadingBlocks.emplace_back(underFeetPos, 1.0f);
    }
}

void Scaffold::LevelRenderHook() {
    float fadeSpeed = 1.5f * RenderUtils::deltaTime;
    for (size_t i = 0; i < fadingBlocks.size();) {
        auto& block = fadingBlocks[i];
        block.alpha -= fadeSpeed;
        if (block.alpha <= 0.01f) {
            fadingBlocks.erase(fadingBlocks.begin() + i);
            continue;
        }
        Vec3<float> lower(block.pos.x, block.pos.y, block.pos.z);
        Vec3<float> upper(lower.x + 1.0f, lower.y + 1.0f, lower.z + 1.0f);
        UIColor fillColor(255, 255, 255, static_cast<int>(50.0f * block.alpha));
        UIColor outlineColor(255, 255, 255, static_cast<int>(200.0f * block.alpha));
        RenderUtils::Draw3dBox(AABB(lower, upper), fillColor, outlineColor);
        ++i;
    }
}

float normalizeAngle3(float angle) {
    while (angle > 180.f) angle -= 360.f;
    while (angle < -180.f) angle += 360.f;
    return angle;
}

float shortestAngleDiff3(float current, float target) {
    return normalizeAngle3(target - current);
}

float lerpAngle3(float current, float target, float percent) {
    float diff = shortestAngleDiff3(current, target);
    return normalizeAngle3(current + diff * percent);
}

void Scaffold::RotationUpdateHook(LocalPlayer* player) {
    if (!player || !player->stateVector || !player->rotation) return;

    Vec3<float> velocity = player->stateVector->velocity;
    float speed = std::sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
    if (speed < 0.01f) return;

    float targetYaw = std::atan2(velocity.z, velocity.x) * (180.f / 3.14159265f) + 90.f;
    float targetPitch = 65.f;

    if (currentRot.x == 0.f && currentRot.y == 0.f) {
        currentRot.x = targetPitch;
        currentRot.y = targetYaw;
    }

    constexpr float lerpPercent = 0.15f;
    currentRot.y = lerpAngle3(currentRot.y, targetYaw, lerpPercent);
    currentRot.x = lerpAngle3(currentRot.x, targetPitch, lerpPercent);

    if (currentRot.x > 90.f) currentRot.x = 90.f;
    if (currentRot.x < -90.f) currentRot.x = -90.f;

    if (strict) {
        player->rotation->presentRot.y = currentRot.y;
    }
    player->rotation->presentRot.x = currentRot.x;

    auto headRotComp = player->getActorHeadRotationComponent();
    if (headRotComp)
        headRotComp->headYaw = currentRot.y;

    if (!strict) {
        auto bodyRotComp = player->getMobBodyRotationComponent();
        if (bodyRotComp)
            bodyRotComp->bodyYaw = currentRot.y;
    }
}

void Scaffold::SentPacketHook(Packet* packet, bool& cancel) {
    if (!g_Data.getLocalPlayer()) return;
    if (!strict) return;
    if (!packet) return;

    if (packet->getId() == PacketID::PlayerAuthInput) {
        auto* input = reinterpret_cast<PlayerAuthInputPacket*>(packet);
        if (input) {
            input->rotation.x = currentRot.x;
            input->rotation.y = currentRot.y;
        }
    }
    else if (packet->getId() == PacketID::MovePlayer) {
        auto* move = reinterpret_cast<MovePlayerPacket*>(packet);
        if (move) {
            move->headYaw = currentRot.y;
        }
    }
}
