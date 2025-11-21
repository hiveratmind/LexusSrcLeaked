#include "NoFall.h"
#include "../../../ModuleManager.h"

NoFall::NoFall() : Module(Category::Player, "NoFall", "Take No Fall Damage") {
    addSetting(new EnumSetting("Type", "NoFall mode", { "Vanilla", "NoGround", "Nukkit", "LifeBoat" }, &NoFallType, 0));
}

NoFall::~NoFall() {
}

std::string NoFall::getModeText() {
    switch (NoFallType) {
    case 0: return "Vanilla";
    case 1: return "NoGround";
    case 2: return "Nukkit";
    case 3: return "LifeBoat";
    default: return "Unknown";
    }
}

void NoFall::sendPacketAsync(const PlayerAuthInputPacket& pkt) {
    if (!g_Data.canUseMoveKeys()) return;

    auto packet = MinecraftPackets::createPacket(PacketID::PlayerAuthInput);
    auto* newPkt = reinterpret_cast<PlayerAuthInputPacket*>(packet.get());

    *newPkt = pkt;

    LocalPlayer* localPlayer = g_Data.getLocalPlayer();
    if (!localPlayer || !localPlayer->level || !localPlayer->level->getPacketSender())
        return;

    localPlayer->level->getPacketSender()->sendToServer(newPkt);
}

void NoFall::setPrevAuthInputPacket(const PlayerAuthInputPacket& pkt) {
    if (!g_Data.canUseMoveKeys()) return;
    lastInputPacket = pkt;
}

void NoFall::SentPacketHook(Packet* packet, bool& cancel) {
    if (NoFallType == 1) {
        if (g_Data.getLocalPlayer() && g_Data.getLocalPlayer()->getFallDistance() > 3.f) {
            if (packet->getId() == PacketID::MovePlayer) {
                MovePlayerPacket* movePacket = reinterpret_cast<MovePlayerPacket*>(packet);
                movePacket->onGround = true;
            }
        }
    }
    else if (NoFallType == 3) {
        if (packet->getId() == PacketID::PlayerAuthInput) {
            auto& authPkt = *reinterpret_cast<PlayerAuthInputPacket*>(packet);
            setPrevAuthInputPacket(authPkt);
        }
    }
}

void NoFall::OnTick() {
    auto plr = g_Data.getLocalPlayer();
    if (!plr) return;

    bool ShouldNoFall = !plr->isOnGround();

    if (NoFallType == 3 && ShouldNoFall) {
        tickCounter++;
        if (tickCounter >= 1) {
            tickCounter = 0;

            if (!g_Data.canUseMoveKeys()) return;
            if (lastInputPacket.position.y == 0.f) return; 

            PlayerAuthInputPacket pktToSend = lastInputPacket;
            pktToSend.position.y += 0.01f;

            sendPacketAsync(pktToSend);
        }
        return; 
    }

    if (ShouldNoFall && NoFallType != 1) {
        auto packetShared = MinecraftPackets::createPacket(PacketID::PlayerAction);
        auto* p = reinterpret_cast<PlayerActionPacket*>(packetShared.get());

        p->mAction = (NoFallType == 0) ? PlayerActionPacket::PlayerActionType::Respawn : PlayerActionPacket::PlayerActionType::StartGliding;
        p->mRuntimeId = g_Data.getLocalPlayer()->getRuntimeIDComponent()->runtimeId.id;

        auto localPlayer = g_Data.getLocalPlayer();
        if (!localPlayer || !localPlayer->level) return;

        auto packetSender = localPlayer->level->getPacketSender();
        if (!packetSender) return;

        packetSender->sendToServer(p);
    }
}
