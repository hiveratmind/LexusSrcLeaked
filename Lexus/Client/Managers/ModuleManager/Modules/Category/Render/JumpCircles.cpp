#include "JumpCircles.h"
#include "../../../../../../SDK/Game.h"
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include "../../../ModuleManager.h"

int thesegments = 16;

JumpCircle::JumpCircle() : Module(Category::Render, "JumpCircle", "Shows invisible targets when you jump") {
    addSetting(new SliderSetting<float>("Scale", "Base circle size", &scale, 1.0f, 0.1f, 3.0f));
    addSetting(new SliderSetting<float>("Test", "Shrink factor", &test, 30.f, 0.001f, 400.f));
    addSetting(new SliderSetting<float>("Speed", "Growth speed", &speed, 1.0f, 0.1f, 10.0f));
    addSetting(new SliderSetting<float>("Fade", "Alpha fade speed", &fade, 2.0f, 0.1f, 6.0f));
    addSetting(new SliderSetting<int>("Segments", "Circle segments.", &thesegments, 16, 4, 128));
    addSetting(new BoolSetting("Affect Others", "Also show circles for other players", &affectOthers, false));
    addSetting(new ColorSetting("Render Color", "Color of render", &mainColor, UIColor(255, 255, 255)));
}

void JumpCircle::OnTick() {
    auto localPlayer = g_Data.getLocalPlayer();
    if (!localPlayer) return;

    if (localPlayer->isOnGround() && !prevStatus && groundTicks > 5) {
        auto pos = localPlayer->getPos();
        pos.y -= 1.6f;
        circles.emplace_back(pos, 0.f, 1.f);
    }

    groundTicks = !localPlayer->isOnGround() ? groundTicks + 1 : 0;
    prevStatus = localPlayer->isOnGround();

    if (affectOthers) {
        Level* level = localPlayer->getlevel();
        if (!level) return;

        const auto& entities = level->getRuntimeActorList();
        static std::unordered_map<Actor*, bool> entityPrevStatus;
        static std::unordered_map<Actor*, int> entityGroundTicks;

        for (Actor* entity : entities) {
            if (!entity || entity == localPlayer) continue;
            if (!TargetUtil::ValidCheck(entity, false)) continue;

            bool isOnGround = entity->isOnGround();

            if (entityPrevStatus.find(entity) == entityPrevStatus.end()) {
                entityPrevStatus[entity] = isOnGround;
                entityGroundTicks[entity] = 0;
            }

            if (isOnGround && !entityPrevStatus[entity] && entityGroundTicks[entity] > 5) {
                auto pos = entity->getPos();
                pos.y -= 1.6f;
                circles.emplace_back(pos, 0.f, 1.f);
            }

            entityGroundTicks[entity] = !isOnGround ? entityGroundTicks[entity] + 1 : 0;
            entityPrevStatus[entity] = isOnGround;
        }
    }

    circles.erase(std::remove_if(circles.begin(), circles.end(), [](const Circle& c) {
        return c.shouldRemove();
        }), circles.end());
}

void JumpCircle::LevelRenderHook() {
    LocalPlayer* localPlayer = g_Data.getLocalPlayer();
    if (!localPlayer || circles.empty()) return;

    for (auto& circle : circles) {
        circle.render(scale, speed / 10.f, fade / 100.f, test);
    }
}

void Circle::render(float circleScale, float speed, float alphaSpeed, float test) {
    int segments = (thesegments >= 4 && thesegments <= 128) ? thesegments : 16;
    if (test <= 0.f) test = 30.f;

    for (int i = 0; i < 16; i++) {
        float layerAlpha = alpha * (1.f - i / 16.f);

        auto mamamam = ModuleManager::getModule<JumpCircle>();

        UIColor layerColor(mamamam->mainColor.r, mamamam->mainColor.g, mamamam->mainColor.b, static_cast<int>(layerAlpha * 255));
        float layerScaleOuter = scale - i / test;
        float layerScaleInner = layerScaleOuter * 0.85f;

        Vec3<float> rotationAngles(0.f, 0.f, 0.f);

        RenderUtils::drawRing3d(pos, layerScaleInner, layerScaleOuter, layerColor, UIColor(0, 0, 0, 0), 5.f, rotationAngles, segments);
    }

    scale += (circleScale - scale) * speed * RenderUtils::deltaTime;
    alpha = std::fmaxf(alpha - alphaSpeed, 0.f);
}

bool Circle::shouldRemove() const {
    return alpha < 0.05f;
}
