#pragma once
#include "../../ModuleBase/Module.h"

class Disabler : public Module {
public:
    Disabler();

    std::string getModeText() override;
    void SentPacketHook(Packet* packet, bool& cancel) override;
    void OnTick() override;
        
private:
    int mode = 0; // 0 = Lifeboat, 1 = Prediction

    Vec3<float> lastPos = Vec3<float>(0.f, 0.f, 0.f);
    Vec3<float> velocity = Vec3<float>(0.f, 0.f, 0.f);
};
