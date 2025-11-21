#pragma once

#include "../../ModuleBase/Module.h"

class TestModule : public Module {
public:
    TestModule();

    void onEnable() override;
    void OnTick() override;
};
