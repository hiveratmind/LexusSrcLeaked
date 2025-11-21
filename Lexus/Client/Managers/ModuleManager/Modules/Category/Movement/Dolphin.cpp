#include "Dolphin.h"

Dolphin::Dolphin() : Module(Category::Movement, "Dolphin", "Boosts you upward in water") {}

void Dolphin::OnTick() {
    auto* player = g_Data.getLocalPlayer();
    if (!player || !player->stateVector) return;

    if (player->isInWater()) {
        player->stateVector->velocity.y += 0.04f;
    }
}

std::string Dolphin::getModeText() {
    return "Java"; //this is a java module from a 1.8 tutorial, thus return java
}
