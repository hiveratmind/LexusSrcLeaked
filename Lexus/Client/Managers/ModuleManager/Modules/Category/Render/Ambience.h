#pragma once

#include "../../ModuleBase/Module.h"

class Ambience : public Module {
public:
    bool radulasso1 = false;
    bool radulasso2 = false;
    float radulasso_rain_level = 0.4f;
    float radulasso_fog_level = 0.4f;

    Ambience();
    ~Ambience();

    virtual void OnLocalTick() override;
    void onEnable() override;
    void onDisable() override;
private:
    std::chrono::steady_clock::time_point lastUpdate;
    const std::chrono::milliseconds updateInterval{ 10000 };
};
