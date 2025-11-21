#pragma once
#include "../../ModuleBase/Module.h"

class KillAura : public Module {
private:
    bool render = false;
    UIColor renderColor = UIColor(255, 255, 255);

    int minCPS = 8;
    int maxCPS = 12;
    int currentCPS = 10;
    long long lastAttackTime = 0;

    float minReach = 3.f;
    float maxReach = 6.f;
    float currentReach = 5.f;

    int rotationMode = 0;
    int rotationMinStep = 5;
    int rotationMaxStep = 10;

    Vec2<float> currentRot;
    bool shouldRotate = false;

    int attackMode = 0;
    bool swing = true;
    int switchMode = 0;

    static bool sortByDistance(Actor* a, Actor* b);
    int selectBestWeapon(Actor* target);
    void performAttack(Actor* target);
    bool spoofSwitch(int slot);
    void randomizeSettings();

public:
    KillAura();
    std::vector<Actor*> targetList;
    std::string getModeText() override;
    void onEnable() override;
    void onDisable() override;
    void OnTick() override;
    void RotationUpdateHook(LocalPlayer* localPlayer) override;
    void SentPacketHook(Packet* packet, bool& cancel) override;
    void LevelRenderHook() override;
};