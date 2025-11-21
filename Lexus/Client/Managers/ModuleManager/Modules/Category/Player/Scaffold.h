#pragma once
#include "../../ModuleBase/Module.h"

class Scaffold : public Module {
private:
    uint64_t lastPlaceTime = 0;
    int delay = 100;
    float renderAlpha = 0.0f;
    struct FadeBlock {
        Vec3<int> pos;
        float alpha;
        FadeBlock(Vec3<int> p, float a) : pos(p), alpha(a) {}
    };
    Vec2<float> currentRot = { 0.f, 0.f };
    bool strict = false;
    std::vector<FadeBlock> fadingBlocks;
public:
    Scaffold();
    void OnTick() override;
    void LevelRenderHook() override;
    void RotationUpdateHook(LocalPlayer* localPlayer) override;
    void SentPacketHook(Packet* packet, bool& cancel) override;
private:
    Vec3<int> getBlockUnderFeet(LocalPlayer* player);
    int findBlockHotbarSlot(LocalPlayer* player);
    bool isBlockPlaceable(LocalPlayer* player, const Vec3<int>& pos);
    void placeBlock(LocalPlayer* player, GameMode* gameMode, const Vec3<int>& pos, int slot);
};