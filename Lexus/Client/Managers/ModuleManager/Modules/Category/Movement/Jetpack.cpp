#include "Jetpack.h"
#include "../../Client/Client.h"

Jetpack::Jetpack() : Module(Category::Movement, "Jetpack", "Launches you forward in the direction you're looking") {}

void Jetpack::OnTick() {
    auto* player = g_Data.getLocalPlayer();
    if (!player || !player->stateVector || !player->rotation) return;

	float calcYaw = (player->rotation->presentRot.y + 90.f) * (PI / 180.f);
	float calcPitch = (player->rotation->prevRot.x) * -(PI / 180.f);

	Vec3<float> moveVec;
	moveVec.x = cos(calcYaw) * cos(calcPitch) * 1.f;
	moveVec.y = sin(calcPitch) * 1.f;
	moveVec.z = sin(calcYaw) * cos(calcPitch) * 1.f;

	player->lerpMotion(moveVec);
}

std::string Jetpack::getModeText() {
	return "Velocity";
}
