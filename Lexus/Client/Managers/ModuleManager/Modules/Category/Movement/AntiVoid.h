#pragma once
#include "../../ModuleBase/Module.h"

class AntiVoid : public Module {
public:
    AntiVoid();
    float lastY = 0.f;
    bool lastYInitialized = false;
    float fallDistance = 5.f;
    bool teleportOnce = false;
    bool toggleFreecam = false;
private:
    std::vector<Vec3<int>> onGroundPositions;
    bool teleported = false;
    bool canTeleport = true;
    uint64_t lastPlaceTime = 0;
public:
    void onEnable() override;
    void onDisable() override;
    void OnTick() override;
    std::string getModeText() override;
private:
    bool isOverVoid(const Vec3<float>& pos);
    bool isGoodBlock(const Vec3<int>& pos);
    void teleportPlayerBack();
};
