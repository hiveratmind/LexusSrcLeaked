#pragma once
#include "../../ModuleBase/Module.h"

class HUD : public Module {
public:
    HUD();

    void InterfaceUtilsHook() override;

private:
    struct ModuleState {
        float currentY = 0.f;
        float targetY = 0.f;
        float width = 0.f;
        float alpha = 0.f;
    };

    std::unordered_map<Module*, ModuleState> moduleStates;

    float sizeModifier = 1.f;
    UIColor color = UIColor(255, 255, 255, 255);
    bool rainbow = false;
    float backgroundOpacity = 0.5f;
    bool showArrayList = true;
    bool showPosition = true;
    bool showPing = true;
    bool showServerIP = false;

    std::vector<class Actor*> targetList;
};