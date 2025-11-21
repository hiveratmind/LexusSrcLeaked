#include "Fly.h"

Fly::Fly() : Module(Category::Movement, "Fly", "Fly using movement keys") {
    addSetting(new SliderSetting<float>("Speed", "Speed of fly", &speed, 1.f, 0.f, 5.f));
    addSetting(new SliderSetting<float>("Glide", "Glide value", &glide, 0.f, -0.5f, 0.f));
}

void Fly::OnTick() {
    auto* player = g_Data.getLocalPlayer();
    if (!player || !player->stateVector || !player->rotation) return;

    auto& velocity = player->stateVector->velocity;
    velocity = { 0.f, glide, 0.f };

    bool forward = g_Data.isKeyDown('W');
    bool left = g_Data.isKeyDown('A');
    bool backward = g_Data.isKeyDown('S');
    bool right = g_Data.isKeyDown('D');
    bool up = g_Data.isKeyDown(VK_SPACE);
    bool down = g_Data.isKeyDown(VK_SHIFT);

    if (up) velocity.y += speed;
    if (down) velocity.y -= speed;

    int x = (right ? 1 : 0) - (left ? 1 : 0);
    int z = (forward ? 1 : 0) - (backward ? 1 : 0);

    if (x != 0 || z != 0) {
        float yaw = player->rotation->presentRot.y;
        float moveAngle = atan2f((float)x, (float)z);
        float finalYaw = yaw + moveAngle * (180.f / 3.14159265f);

        float rad = (finalYaw + 90.f) * (3.14159265f / 180.f);
        velocity.x += cosf(rad) * speed;
        velocity.z += sinf(rad) * speed;
    }

    player->lerpMotion(velocity);
}

std::string Fly::getModeText() {
    return "Velocity";
}
