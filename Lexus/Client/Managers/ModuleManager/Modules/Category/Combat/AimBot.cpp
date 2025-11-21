#include "Aimbot.h"
#include "../../Client/Client.h"
#include <random>

Aimbot::Aimbot() : Module(Category::Combat, "Aimbot", "Smoothly aims at the nearest target without attacking") {
    addSetting(new SliderSetting<float>("Max Reach", "Maximum aim distance", &maxReach, 6.f, 1.f, 12.f));
}

void Aimbot::onEnable() {
    currentRot = { 0.f, 0.f };
    lastTarget = nullptr;
    verticalOffset = 0.f;
}

void Aimbot::onDisable() {
    currentRot = { 0.f, 0.f };
    lastTarget = nullptr;
    verticalOffset = 0.f;
}

static float normalizeAngle4(float angle) {
    while (angle > 180.f) angle -= 360.f;
    while (angle < -180.f) angle += 360.f;
    return angle;
}

static float shortestAngleDiff4(float current, float target) {
    return normalizeAngle4(target - current);
}

static float lerpAngle4(float current, float target, float percent) {
    float diff = shortestAngleDiff4(current, target);
    return normalizeAngle4(current + diff * percent);
}

static float randomFloat(float min, float max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

void Aimbot::RotationUpdateHook(LocalPlayer* player) {
    if (!player || !player->rotation || !player->getActorHeadRotationComponent()) return;

    Actor* closest = nullptr;
    float closestDist = maxReach;
    Vec3 eyePos = player->getEyePos();

    for (auto* ent : ActorUtils::getActorList(true)) {
        if (!ent || ent == player || !ent->isAlive()) continue;
        if (!TargetUtil::ValidCheck(ent, false)) continue;

        float dist = player->getPos().dist(ent->getPos());
        if (dist > maxReach) continue;

        if (dist < closestDist) {
            closest = ent;
            closestDist = dist;
        }
    }

    if (!closest) {
        lastTarget = nullptr;
        return;
    }

    if (closest != lastTarget) {
        verticalOffset = randomFloat(-0.3f, 0.f); 
        lastTarget = closest;
    }

    Vec3 targetPos = closest->getPos();
    targetPos.y += verticalOffset;

    Vec2 targetRot = eyePos.CalcAngle(targetPos);

    float smoothing = randomFloat(0.035f, 0.165f);
    float jitterYaw = randomFloat(-0.2f, 0.2f);

    float newPitch = lerpAngle4(currentRot.x, targetRot.x, smoothing);
    float newYaw = lerpAngle4(currentRot.y, targetRot.y + jitterYaw, smoothing);

    newPitch = std::clamp(newPitch, -90.f, 90.f);

    currentRot.x = newPitch;
    currentRot.y = newYaw;

    player->rotation->presentRot = currentRot;
    player->getActorHeadRotationComponent()->headYaw = currentRot.y;
    g_Data.getLocalPlayer()->resetRot();
}

std::string Aimbot::getModeText() {
    return std::to_string(maxReach);
}