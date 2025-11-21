#pragma once
#include "../../ModuleBase/Module.h"

class Ambient : public Module {
public:
    Ambient();
    void OnTick() override;
    void InterfaceUtilsHook() override;

private:
    struct Letter {
        Vec3<float> pos;
        float lifetime;
    };
    std::vector<Letter> letters;
    float spawnRadius;
    float spawnInterval;
    float timer;
    static const std::string IMAGE_URL;
};