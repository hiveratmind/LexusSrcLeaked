#include "AirJump.h"

AirJump::AirJump() : Module(Category::Movement, "AirJump", "Jump in the air by pressing space") {
    addSetting(new BoolSetting("Legacy Mode", "Continuously jump when holding space", &legacyMode, false));
}

void AirJump::OnTick() {
    auto* player = g_Data.getLocalPlayer();
    if (!player) return;

    if (g_Data.isKeyDown(VK_SPACE)) {
        if (legacyMode || !jumpQueued) {
            player->jumpFromGround();
            jumpQueued = true;
        }
    }
    else {
        jumpQueued = false;
    }
}

std::string AirJump::getModeText() {
    return legacyMode ? "Regular" : "Legacy";
}
