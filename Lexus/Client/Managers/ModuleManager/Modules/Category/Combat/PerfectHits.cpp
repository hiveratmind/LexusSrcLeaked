#include "PerfectHits.h"
#include "../../Client/Client.h"

PerfectHits::PerfectHits() : Module(Category::Combat, "PerfectHits", "First hit uses slow timer for perfect combo timing") {}

void PerfectHits::onAttack(Actor* actor, bool& cancel) {
    if (!actor) return;

    auto player = g_Data.getLocalPlayer();
    if (!player) return;

    slowStartTime = std::chrono::steady_clock::now();
    slowActive = true;
}

void PerfectHits::OnLocalTick() {
    Minecraft* mc = g_Data.clientInstance->minecraft;
    if (!mc) return;

    if (slowActive) {
        auto now = std::chrono::steady_clock::now();
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - slowStartTime).count();

        if (elapsedMs < 40) {
            *mc->minecraftTimer = 0.5f;
            *mc->minecraftRenderTimer = 0.5f;
        }
        else {
            *mc->minecraftTimer = 20.0f;
            *mc->minecraftRenderTimer = 20.0f;
            slowActive = false;
        }
    }
    else {
        *mc->minecraftTimer = 20.0f;
        *mc->minecraftRenderTimer = 20.0f;
    }

    auto player = g_Data.getLocalPlayer();
    if (!player) return;
}


void PerfectHits::onDisable() {
    Minecraft* mc = g_Data.clientInstance->minecraft;
    if (mc) {
        *mc->minecraftTimer = 20.0f;
        *mc->minecraftRenderTimer = 20.0f;
    }
    slowActive = false;
}