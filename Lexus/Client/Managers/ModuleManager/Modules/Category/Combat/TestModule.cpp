#include "TestModule.h"
#include "../../Client/Client.h"

TestModule::TestModule()
    : Module(Category::Player, "TestModule", "Calls resetRot() on the local player") {
}

void TestModule::onEnable() {
    auto player = g_Data.getLocalPlayer();
    if (player) {
    }
}

void TestModule::OnTick() {
    auto player = g_Data.getLocalPlayer();
    if (!player || !player->level) return;

    float maxDistance = 6.0f;

    auto actors = ActorUtils::getActorList(false);

    for (auto* target : actors) {
        if (!target) continue;
        if (target == player) continue;
        if (!target->isAlive()) continue;
        if (!TargetUtil::ValidCheck(target, true)) continue;

        float dist = player->getPos().dist(target->getPos());
        if (dist <= maxDistance) {
            player->_crit(*target); 
            player->_crit(*player);
            player->getgameMode()->attack(player);
        }
    }
}
