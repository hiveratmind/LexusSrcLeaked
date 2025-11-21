#pragma once
#include "../../ModuleBase/Module.h"

enum class VelocityMode {
    Regular,
    Lag
};

class Velocity : public Module {
public:
    float horizontal = 0.f;
    float vertical = 0.f;
    int lagDelay = 5;
    VelocityMode mode = VelocityMode::Regular;

    Velocity();

    std::string getModeText() override;

    void OnTick() override;
    void SentPacketHook(Packet* packet, bool& cancel) override;

private:
    std::deque<PlayerAuthInputPacket> delayedPackets;
    int tickCounter = 0;
    int latestTicksAlive = 0;
    void sendDelayedPacket();
};
