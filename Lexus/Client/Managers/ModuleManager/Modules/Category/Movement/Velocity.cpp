#include "Velocity.h"
#include "../../../ModuleManager.h"
#include <deque>

Velocity::Velocity() : Module(Category::Movement, "Velocity", "Prevent you from knockback") {
    addSetting(new EnumSetting("Mode", "Velocity mode", { "Regular", "Lag" }, reinterpret_cast<int*>(&mode), 0));
    addSetting(new SliderSetting<float>("Horizontal", "NULL", &horizontal, horizontal, 0.f, 100.f));
    addSetting(new SliderSetting<float>("Vertical", "NULL", &vertical, vertical, 0.f, 100.f));
    addSetting(new SliderSetting<int>("Lag Delay", "Ticks to delay packets", &lagDelay, lagDelay, 1, 20));
}

std::string Velocity::getModeText() {
    switch (mode) {
    case VelocityMode::Regular: {
        static char textStr[20];
        sprintf_s(textStr, 20, "%i%% %i%%", (int)(std::round(horizontal)), (int)(std::round(vertical)));
        return std::string(textStr);
    }
    case VelocityMode::Lag:
        return "Lag";
    default:
        return "Unknown";
    }
}

void Velocity::sendDelayedPacket() {
    if (delayedPackets.empty()) return;

    PlayerAuthInputPacket pkt = delayedPackets.front();
    delayedPackets.pop_front();

    LocalPlayer* localPlayer = g_Data.getLocalPlayer();
    if (!localPlayer || !localPlayer->level || !localPlayer->level->getPacketSender()) return;

    pkt.TicksAlive = latestTicksAlive;

    localPlayer->level->getPacketSender()->sendToServer(&pkt);
}


void Velocity::OnTick() {
    if (mode == VelocityMode::Lag) {
        tickCounter++;
        if (tickCounter >= lagDelay) {
            tickCounter = 0;
            sendDelayedPacket();
        }
    }
}

void Velocity::SentPacketHook(Packet* packet, bool& cancel) {
    if (mode == VelocityMode::Regular) return;

    if (mode == VelocityMode::Lag && packet->getId() == PacketID::PlayerAuthInput) {
        auto& authPkt = *reinterpret_cast<PlayerAuthInputPacket*>(packet);
        PlayerAuthInputPacket* currentauth = reinterpret_cast<PlayerAuthInputPacket*>(packet);

        latestTicksAlive = currentauth->TicksAlive;

        if (delayedPackets.size() >= static_cast<size_t>(lagDelay)) {
            delayedPackets.pop_front();
        }

        delayedPackets.push_back(authPkt);

        cancel = true;
    }
}
