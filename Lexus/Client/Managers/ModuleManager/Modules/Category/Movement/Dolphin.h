#pragma once
#include "../../ModuleBase/Module.h"

class Dolphin : public Module {
public:
    Dolphin();
    void OnTick() override;
    std::string getModeText() override;
};
