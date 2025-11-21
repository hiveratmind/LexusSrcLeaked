#pragma once
#include "../../ModuleBase/Module.h"

class Criticals : public Module {
private:
    PlayerAuthInputPacket lastInputPacket{};

    void sendPacketAsync(const PlayerAuthInputPacket& pkt);
    void setPrevAuthInputPacket(const PlayerAuthInputPacket& pkt);

public:
    Criticals();
    bool lb = false;
    void SentPacketHook(Packet* packet, bool& cancel) override;
    void onAttack(Actor* target, bool& cancel) override;
};