#pragma once
#include "../../ModuleBase/Module.h"

class Fly : public Module {
public:
    Fly();
    void OnTick() override;
    std::string getModeText() override;
    float speed = 1.0f;
    float glide = 0.f;
};
