#pragma once
#include "../../ModuleBase/Module.h"

class AirJump : public Module {
public:
    AirJump();
    void OnTick() override;
    std::string getModeText() override;
private:
    bool legacyMode = false;
    bool jumpQueued = false;
};