#include "Disabler.h"
#include "../../Client/Client.h"

Disabler::Disabler() : Module(Category::World, "Disabler", "Disable some anticheats") {
    addSetting(new EnumSetting("Mode", "Disabler mode", { "Lifeboat", "Prediction" }, &mode, 0));
}

std::string Disabler::getModeText() {
    switch (mode) {
    case 0: return "Lifeboat";
    case 1: return "Prediction";
    default: return "Unknown";
    }
}

void Disabler::OnTick() {
    if (mode == 1) {
        auto* player = g_Data.getLocalPlayer();
        if (!player) return;

        Vec3<float> currentPos = player->getPos();
        velocity = currentPos.sub(lastPos);
        lastPos = currentPos;
    }
}

void Disabler::SentPacketHook(Packet* packet, bool& cancel) {
    auto* player = g_Data.getLocalPlayer();
    if (!player) return;

    if (mode == 0) {
        if (packet->getId() == PacketID::PlayerAuthInput) {
            auto* authPkt = reinterpret_cast<PlayerAuthInputPacket*>(packet);
            if (authPkt) {
                authPkt->TicksAlive = 0;
            }
        }
        else if (packet->getId() == PacketID::MovePlayer) {
            auto* movePkt = reinterpret_cast<MovePlayerPacket*>(packet);
            if (movePkt) {
                movePkt->mRuntimeId = 0;
                movePkt->mTeleportTick = 0;
            }
        }
    }
    else if (mode == 1) {
        if (packet->getId() == PacketID::MovePlayer) {
            auto* movePkt = reinterpret_cast<MovePlayerPacket*>(packet);
            if (movePkt) {
                Vec3<float> predictedPos = movePkt->mPosition.add(velocity);
                movePkt->mPosition = predictedPos;
            }
        }
        if (packet->getId() == PacketID::PlayerAuthInput) {
            auto* authPkt = reinterpret_cast<PlayerAuthInputPacket*>(packet);
            if (authPkt) {
                Vec3<float> predictedPos = authPkt->position.add(velocity);
                authPkt->position = predictedPos;
            }
        }
    }
}
