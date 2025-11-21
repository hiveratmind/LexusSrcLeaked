#include "HackerDetector.h"
#include "../../Client/Client.h"
#include <cmath>

HackerDetector::HackerDetector() : Module(Category::Misc, "HackerDetector", "Detects suspicious player hacks") {}

void HackerDetector::onEnable() {
    trackedPlayers.clear();
}

void HackerDetector::onDisable() {
    trackedPlayers.clear();
}

float HackerDetector::getDownVelocity(Actor* player) {
    if (!player || !player->stateVector) return 0.f;
    return player->stateVector->velocity.y;
}

bool HackerDetector::isFastRotating(Actor* player, const PlayerInfo& info) {
    Vec2<float> currentRot = player->rotation->presentRot;
    float yawDiff = std::abs(currentRot.y - info.lastRot.y);
    if (yawDiff > 180.f) yawDiff = 360.f - yawDiff;
    return yawDiff >= 45.f;
}

float getHorizontalDistance(const Vec3<float>& a, const Vec3<float>& b) {
    float dx = a.x - b.x;
    float dz = a.z - b.z;
    return sqrtf(dx * dx + dz * dz);
}

bool HackerDetector::isSpeeding(Actor* player, const PlayerInfo& info) {
    float dist = getHorizontalDistance(player->getPos(), info.lastPos);
    return dist > 10.8f;
}

bool HackerDetector::hasInvalidPitch(Actor* player) {
    float pitch = player->rotation->presentRot.x;
    return pitch > 90.f || pitch < -90.f;
}

void HackerDetector::OnTick() {
    auto localPlayer = g_Data.getLocalPlayer();
    if (!localPlayer) return;

    auto level = localPlayer->level;
    if (!level) return;

    auto players = level->getRuntimeActorList();

    for (auto& entity : players) {
        if (!entity || entity == localPlayer) continue;
        if (!TargetUtil::ValidCheck(entity, false)) continue;

        PlayerInfo& info = trackedPlayers[entity];

        Vec3<float> currentPos = entity->getPos();
        Vec2<float> currentRot = entity->rotation->presentRot;
        std::string playerName = *entity->getNameTag();

        if (isFastRotating(entity, info)) {
            localPlayer->displayClientMessage("[HackerDetector] Fast Rotation detected on " + playerName);
        }

        if (isSpeeding(entity, info)) {
            localPlayer->displayClientMessage("[HackerDetector] Speed detected on " + playerName);
        }

        if (hasInvalidPitch(entity)) {
            localPlayer->displayClientMessage("[HackerDetector] Invalid Pitch detected on " + playerName);
        }

        info.lastPos = currentPos;
        info.lastRot = currentRot;
    }
}
