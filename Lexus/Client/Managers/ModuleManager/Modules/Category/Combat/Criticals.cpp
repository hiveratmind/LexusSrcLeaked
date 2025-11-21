#include "Criticals.h"

Criticals::Criticals() : Module(Category::Combat, "Criticals", "Each hit is a critical hit") {
    addSetting(new BoolSetting("LifeBoat", "Bypasses lifeboat kick", &lb, true));
}

void Criticals::sendPacketAsync(const PlayerAuthInputPacket& pkt) {
    if (!g_Data.canUseMoveKeys()) return;

    auto packet = MinecraftPackets::createPacket(PacketID::PlayerAuthInput);
    auto* newPkt = reinterpret_cast<PlayerAuthInputPacket*>(packet.get());

    *newPkt = pkt;

    if (lb) {
        newPkt->TicksAlive = 0;
    }

    LocalPlayer* localPlayer = g_Data.getLocalPlayer();
    if (!localPlayer || !localPlayer->level || !localPlayer->level->getPacketSender())
        return;

    localPlayer->level->getPacketSender()->sendToServer(newPkt);
}

void Criticals::setPrevAuthInputPacket(const PlayerAuthInputPacket& pkt) {
    if (!g_Data.canUseMoveKeys()) return;
    lastInputPacket = pkt;
}

void Criticals::SentPacketHook(Packet* packet, bool& cancel) {
    if (!g_Data.getLocalPlayer()) return;
    if (!g_Data.canUseMoveKeys()) return;

    if (packet->getId() == PacketID::MovePlayer) {
        auto& movePkt = *reinterpret_cast<MovePlayerPacket*>(packet);
        movePkt.onGround = false;
    }
    else if (packet->getId() == PacketID::PlayerAuthInput) {
        const auto& currAuthPkt = *reinterpret_cast<PlayerAuthInputPacket*>(packet);
        setPrevAuthInputPacket(currAuthPkt);
    }
}

void Criticals::onAttack(Actor* target, bool& cancel) {
    if (!g_Data.canUseMoveKeys()) return;

    if (lastInputPacket.TicksAlive < 5) return;

    PlayerAuthInputPacket pktToModify = lastInputPacket;

    pktToModify.removeMovingInput();

    pktToModify.mInputData |= InputData::Jumping;
    pktToModify.mInputData |= InputData::JumpDown;
    pktToModify.position.y += 1.0f;

    if (lb) {
        pktToModify.TicksAlive = 0;
    }

    sendPacketAsync(pktToModify);

    if (lb) {
        pktToModify.TicksAlive = 0;
    }

    pktToModify.mInputData &= ~InputData::Jumping;
    pktToModify.mInputData &= ~InputData::JumpDown;
    pktToModify.position.y -= 0.5f;

    for (int i = 0; i < 6; i++) {
        sendPacketAsync(pktToModify);
    }
}
