#pragma once
#include "../../ModuleBase/Module.h"
#include <unordered_map>

struct PlayerInfo {
    Vec3<float> lastPos;
    Vec2<float> lastRot;
};

class HackerDetector : public Module {
public:
    HackerDetector();

    // Lifecycle
    virtual void onEnable() override;
    virtual void onDisable() override;
    virtual void OnTick() override;

private:
    std::unordered_map<Actor*, PlayerInfo> trackedPlayers;

    float getDownVelocity(Actor* player);
    bool isFastRotating(Actor* player, const PlayerInfo& info);
    bool isSpeeding(Actor* player, const PlayerInfo& info);
    bool hasInvalidPitch(Actor* player);
};
