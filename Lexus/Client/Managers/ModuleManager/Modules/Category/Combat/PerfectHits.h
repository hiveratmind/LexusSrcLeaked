#pragma once
#include "../../ModuleBase/Module.h"
#include <unordered_set>

class PerfectHits : public Module {
public:
    PerfectHits();
    void onAttack(Actor* actor, bool& cancel) override;
    void OnLocalTick() override;
    void onDisable() override;

private:
    bool slowActive = false;
    std::chrono::steady_clock::time_point slowStartTime;
    std::chrono::steady_clock::time_point lastActivationTime;
    bool canActivate = true;
};