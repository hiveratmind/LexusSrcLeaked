#pragma once
#include "../../ModuleBase/Module.h"

class FastBow : public Module {
public:
    FastBow();
    void OnTick() override;
};
