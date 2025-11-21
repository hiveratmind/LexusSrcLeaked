#include "ESP.h" //ive gotten into the habbit of putting comments

ESP::ESP() : Module(Category::Render, "ESP", "Highlights player/entity") {
    addSetting(new ColorSetting("Color", "NULL", &color, UIColor(255, 255, 255), false));
    addSetting(new SliderSetting<int>("Alpha", "NULL", &alpha, 40, 0, 255));
    addSetting(new SliderSetting<int>("LineAlpha", "NULL", &lineAlpha, 135, 0, 255));
    addSetting(new BoolSetting("Mobs", "NULL", &mobs, false));
    addSetting(new BoolSetting("2D ESP", "NULL", &esp2D, false));
}

void ESP::LevelRenderHook() {
    if (esp2D) return;
    LocalPlayer* localPlayer = g_Data.getLocalPlayer();
    for (auto* entity : ActorUtils::getActorList(true)) {
        if (!entity) continue;
        if (!entity->getActorTypeComponent()) continue;
        if (!entity->stateVector) continue;
        if (!entity->isAlive()) continue;
        uint32_t entityId = entity->getActorTypeComponent()->id;
        if (TargetUtil::ValidCheck(entity, mobs)) {
            AABBShapeComponent* entityAABBShape = entity->aabbShape;
            RenderUtils::Draw3dBox(entityAABBShape->aabb, UIColor(color.r, color.g, color.b, alpha), UIColor(color.r, color.g, color.b, lineAlpha));
        }
    }
}

void ESP::MCRHook(MinecraftUIRenderContext* renderCtx) {
    if (!esp2D) return;
    if (!g_Data.canUseMoveKeys()) return;
    if (!g_Data.getLocalPlayer()) return;

    LocalPlayer* localPlayer = g_Data.getLocalPlayer();
    for (auto* entity : ActorUtils::getActorList(true)) {
        if (!TargetUtil::ValidCheck(entity, mobs)) continue;

        draw2DEspBox(entity, renderCtx);
    }
}

void ESP::draw2DEspBox(Actor* entity, MinecraftUIRenderContext* renderCtx) {
    AABB aabb = entity->aabbShape->aabb;

    Vec3<float> corners[8] = {
        {aabb.lower.x, aabb.lower.y, aabb.lower.z},
        {aabb.upper.x, aabb.lower.y, aabb.lower.z},
        {aabb.lower.x, aabb.upper.y, aabb.lower.z},
        {aabb.upper.x, aabb.upper.y, aabb.lower.z},
        {aabb.lower.x, aabb.lower.y, aabb.upper.z},
        {aabb.upper.x, aabb.lower.y, aabb.upper.z},
        {aabb.lower.x, aabb.upper.y, aabb.upper.z},
        {aabb.upper.x, aabb.upper.y, aabb.upper.z}
    };

    float minX = 1e9f, minY = 1e9f;
    float maxX = -1e9f, maxY = -1e9f;

    bool anyPointVisible = false;

    for (int i = 0; i < 8; i++) {
        Vec2<float> screenPos;
        if (RenderUtils::WorldToScreen(corners[i], screenPos)) {
            anyPointVisible = true;
            if (screenPos.x < minX) minX = screenPos.x;
            if (screenPos.y < minY) minY = screenPos.y;
            if (screenPos.x > maxX) maxX = screenPos.x;
            if (screenPos.y > maxY) maxY = screenPos.y;
        }
    }

    if (!anyPointVisible) return;

    float margin = 0.3f; 

    float centerX = (minX + maxX) * 0.5f;
    float centerY = (minY + maxY) * 0.5f;
    float halfWidth = (maxX - minX) * 0.5f;
    float halfHeight = (maxY - minY) * 0.5f;

    Vec4<float> coloredRect(
        centerX - halfWidth + margin,
        centerY - halfHeight + margin,
        centerX + halfWidth - margin,
        centerY + halfHeight - margin
    );

    Vec4<float> blackRect(
        centerX - halfWidth - margin,
        centerY - halfHeight - margin,
        centerX + halfWidth + margin,
        centerY + halfHeight + margin
    );

    UIColor blackOutline(0, 0, 0, 255);
    RenderUtils::DrawRect(blackRect, blackOutline, 1.3f);

    UIColor lineColor(color.r, color.g, color.b, lineAlpha);
    RenderUtils::DrawRect(coloredRect, lineColor, 1.0f);
}
