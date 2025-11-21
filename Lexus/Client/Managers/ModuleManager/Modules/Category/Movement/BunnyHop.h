#pragma once
#include "../../ModuleBase/Module.h"

class BunnyHop : public Module {
public:
    BunnyHop();
    void OnLocalTick() override;
    std::string getModeText() override;
private:
    float downVelocity = 0.1f;
    float speed = 0.4f;
    float friction = 0.91f;

    bool killauradependency = false;
};
