#pragma once
#include "../../ModuleBase/Module.h"

class NoFall : public Module {
private:
    PlayerAuthInputPacket lastInputPacket{};

    void sendPacketAsync(const PlayerAuthInputPacket& pkt);
    void setPrevAuthInputPacket(const PlayerAuthInputPacket& pkt);

public:
    NoFall();
    ~NoFall();

    int NoFallType = 0;
    int tickCounter = 0;
    virtual std::string getModeText() override;
    virtual void OnTick() override;
    virtual void SentPacketHook(Packet* packet, bool& cancel) override;
};
