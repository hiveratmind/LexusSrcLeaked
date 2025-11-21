#include "ChestESP.h"
#include <string>
#include <algorithm>

StorageESP::StorageESP()
    : Module(Category::Render, "StorageESP", "ESP for chests, shulkers, and more"),
    chestFillColor(UIColor(200, 125, 0, 50)),
    chestLineColor(UIColor(200, 125, 0, 255)),
    shulkerFillColor(UIColor(140, 0, 180, 50)),
    shulkerLineColor(UIColor(140, 0, 180, 255)),
    hopperFillColor(UIColor(128, 128, 128, 50)),
    hopperLineColor(UIColor(128, 128, 128, 255)),
    dropperFillColor(UIColor(192, 192, 192, 50)),
    dropperLineColor(UIColor(192, 192, 192, 255)),
    dispenserFillColor(UIColor(169, 169, 169, 50)),
    dispenserLineColor(UIColor(169, 169, 169, 255)),
    showHopper(true),
    showDropper(true),
    showDispenser(true),
    tracer2D(true)
{
    addSetting(new ColorSetting("Chest Fill Color", "Chest fill color", &chestFillColor, chestFillColor));
    addSetting(new ColorSetting("Chest Line Color", "Chest line color", &chestLineColor, chestLineColor));
    addSetting(new ColorSetting("Shulker Fill Color", "Shulker fill color", &shulkerFillColor, shulkerFillColor));
    addSetting(new ColorSetting("Shulker Line Color", "Shulker line color", &shulkerLineColor, shulkerLineColor));
    addSetting(new ColorSetting("Hopper Fill Color", "Hopper fill color", &hopperFillColor, hopperFillColor));
    addSetting(new ColorSetting("Hopper Line Color", "Hopper line color", &hopperLineColor, hopperLineColor));
    addSetting(new ColorSetting("Dropper Fill Color", "Dropper fill color", &dropperFillColor, dropperFillColor));
    addSetting(new ColorSetting("Dropper Line Color", "Dropper line color", &dropperLineColor, dropperLineColor));
    addSetting(new ColorSetting("Dispenser Fill Color", "Dispenser fill color", &dispenserFillColor, dispenserFillColor));
    addSetting(new ColorSetting("Dispenser Line Color", "Dispenser line color", &dispenserLineColor, dispenserLineColor));

    addSetting(new BoolSetting("Show Hopper", "Render hoppers", &showHopper, showHopper));
    addSetting(new BoolSetting("Show Dropper", "Render droppers", &showDropper, showDropper));
    addSetting(new BoolSetting("Show Dispenser", "Render dispensers", &showDispenser, showDispenser));

    addSetting(new BoolSetting("2D Tracers", "Draw 2D lines to blocks", &tracer2D, tracer2D));
}

void StorageESP::LevelRenderHook() {
    LocalPlayer* localPlayer = g_Data.getLocalPlayer();
    if (!localPlayer) return;

    Vec3<float> playerPos = localPlayer->getPos();

    for (auto it = blockList.begin(); it != blockList.end();) {
        BlockPos pos = it->first;

        float dx = static_cast<float>(pos.x) + 0.5f - playerPos.x;
        float dy = static_cast<float>(pos.y) + 0.5f - playerPos.y;
        float dz = static_cast<float>(pos.z) + 0.5f - playerPos.z;
        float distSq = dx * dx + dy * dy + dz * dz;

        if (distSq > 90000.f) {
            it = blockList.erase(it);
            continue;
        }

        Block* block = g_Data.clientInstance->getRegion()->getBlock(pos);
        if (!block || !block->blockLegacy || block->blockLegacy->blockId == 0) {
            it = blockList.erase(it);
            continue;
        }

        std::string name = block->blockLegacy->blockName;
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);

        bool valid = (name.find("chest") != std::string::npos && name.find("ender") == std::string::npos)
            || name.find("shulker") != std::string::npos
            || (showHopper && (block->blockLegacy->blockId == 154 || name.find("hopper") != std::string::npos))
            || (showDropper && name.find("dropper") != std::string::npos)
            || (showDispenser && (block->blockLegacy->blockId == 23 || name.find("dispenser") != std::string::npos));

        if (!valid) {
            it = blockList.erase(it);
            continue;
        }

        AABB box;
        block->blockLegacy->getOutline(block, g_Data.clientInstance->getRegion(), &pos, &box);

        RenderUtils::Draw3dBox(box, it->second.fillColor, it->second.lineColor, 1.f);

        ++it;
    }
}

void StorageESP::MCRHook(MinecraftUIRenderContext* renderCtx) {
    if (!tracer2D || blockList.empty()) return;
    if (!g_Data.getLocalPlayer()) return;
    Vec2<float> screenSize(
        renderCtx->clientInstance->getguiData()->windowSizeScaled.x,
        renderCtx->clientInstance->getguiData()->windowSizeScaled.y
    );
    Vec2<float> center(screenSize.x / 2.f, screenSize.y / 2.f);

    for (const auto& entry : blockList) {
        const BlockPos& pos = entry.first;
        const BlockVisualInfo& vis = entry.second;

        Block* block = g_Data.clientInstance->getRegion()->getBlock(pos);
        if (!block || !block->blockLegacy) continue;

        std::string name = block->blockLegacy->blockName;
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);

        float tracerHeight = 0.4375f;
        if (name.find("shulker") != std::string::npos) {
            tracerHeight = 0.5f;
        }

        Vec3<float> target(pos.x + 0.5f, pos.y + tracerHeight, pos.z + 0.5f);
        Vec2<float> screenPos;

        if (RenderUtils::WorldToScreen(target, screenPos)) {
            RenderUtils::DrawLine(center, screenPos, 0.2f, UIColor(vis.fillColor.r, vis.fillColor.g, vis.fillColor.b, 255));
        }
    }
}
