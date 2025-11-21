#pragma once
#include "../../ModuleBase/Module.h"
#include "PacketMine.h"

class Regen : public Module {
public:
    Regen();

    float horizontalRange = 6.f;

    std::string currentMode = "";
    std::string getModeText() override;

    void onEnable() override;
    void onDisable() override;
    void OnTick() override;
};