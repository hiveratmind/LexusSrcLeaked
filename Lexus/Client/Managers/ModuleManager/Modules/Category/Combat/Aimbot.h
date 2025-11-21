#pragma once
#include "../../ModuleBase/Module.h"

class Aimbot : public Module {
public:
    Aimbot();

    void onEnable() override;
    void onDisable() override;
    void RotationUpdateHook(LocalPlayer* player);
    std::string getModeText() override;
private:
    float maxReach = 6.f;
    Vec2<float> currentRot;

    Actor* lastTarget = nullptr;
    float verticalOffset = 0.f;
};