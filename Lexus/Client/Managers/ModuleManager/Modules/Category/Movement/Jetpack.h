#pragma once
#include "../../ModuleBase/Module.h"

class Jetpack : public Module {
public:
    Jetpack();
    void OnTick() override;
    std::string getModeText() override;
};
