#include "BunnyHop.h"
#include "../../../ModuleManager.h"

BunnyHop::BunnyHop()
    : Module(Category::Movement, "BunnyHop", "Auto hop with full directional control") {
    addSetting(new SliderSetting<float>("DownVelocity", "Y-velocity when airborne", &downVelocity, 0.0f, 0.0f, 1.0f));
    addSetting(new SliderSetting<float>("Speed", "Base movement speed", &speed, 0.1f, 0.1f, 1.5f));
    addSetting(new SliderSetting<float>("Friction", "Friction when grounded and moving", &friction, 0.5f, 0.1f, 1.0f));

    addSetting(new BoolSetting("Only KillAura", "Only bunnyhop while killaura has a target", &killauradependency, false));
}

void BunnyHop::OnLocalTick() {
    auto* player = g_Data.getLocalPlayer();
    if (!player || !player->stateVector || !player->rotation) return;

    if (killauradependency) {
        auto aura = ModuleManager::getModule<KillAura>();
        if (aura && aura->targetList.empty()) return;
    }

    auto& velocity = player->stateVector->velocity;
    bool onGround = player->isOnGround();

    bool forward = g_Data.isKeyDown('W');
    bool left = g_Data.isKeyDown('A');
    bool back = g_Data.isKeyDown('S');
    bool right = g_Data.isKeyDown('D');

    int xInput = (right ? 1 : 0) - (left ? 1 : 0);
    int zInput = (forward ? 1 : 0) - (back ? 1 : 0);
    bool isMoving = xInput != 0 || zInput != 0;


    if (onGround) {
        if (isMoving) {
            player->jumpFromGround();
            velocity.x *= friction;
            velocity.z *= friction;
        }
    }
    else {
        velocity.y -= downVelocity;
    }

    if (isMoving) {
        float yaw = player->rotation->presentRot.y;
        float moveAngle = atan2f((float)xInput, (float)zInput);
        float finalYaw = yaw + moveAngle * (180.f / 3.14159265f);

        float rad = (finalYaw + 90.f) * (3.14159265f / 180.f);
        velocity.x = cosf(rad) * speed;
        velocity.z = sinf(rad) * speed;
    }

    if (!isMoving) {
        velocity.x = 0;
        velocity.z = 0;
    }

    player->lerpMotion(velocity);
}

std::string BunnyHop::getModeText() {
    return std::to_string(speed);
}

