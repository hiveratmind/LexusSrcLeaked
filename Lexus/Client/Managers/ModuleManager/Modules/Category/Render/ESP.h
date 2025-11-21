#pragma once
#include "../../ModuleBase/Module.h"

class ESP : public Module {
private:
    UIColor color;
    int alpha;
    int lineAlpha;
    bool mobs;
    bool esp2D;

public:
    ESP();

    void LevelRenderHook() override; // 3D ESP
    void MCRHook(MinecraftUIRenderContext* renderCtx) override; // 2D ESP

    void draw2DEspBox(Actor* entity, MinecraftUIRenderContext* renderCtx);
};