#include "Hitbox.h"

Hitbox::Hitbox() : Module(Category::Combat, "Hitbox", "Increase hitbox size of entities") {
	addSetting(new SliderSetting<float>("Height", "NULL", &height, height, 1.8f, 5.f));
	addSetting(new SliderSetting<float>("Width", "NULL", &width, width, 0.6f, 5.f));
	addSetting(new BoolSetting("Mobs", "NULL", &includeMobs, includeMobs));
}

void Hitbox::OnTick() {
	LocalPlayer* localPlayer = g_Data.getLocalPlayer();
	if (!localPlayer) return;

	Level* level = localPlayer->level;
	if (level == nullptr)
		return;

	for (auto& entity : level->getRuntimeActorList()) {
		if (TargetUtil::ValidCheck(entity, includeMobs)) {
			AABB& entityAABB = entity->aabbShape->aabb;

			Vec3<float> center = entityAABB.getCenter();

			float halfWidth = width / 2.0f;
			float halfHeight = height / 2.0f;

			entityAABB.lower = Vec3<float>(center.x - halfWidth, center.y - halfHeight, center.z - halfWidth);
			entityAABB.upper = Vec3<float>(center.x + halfWidth, center.y + halfHeight, center.z + halfWidth);
		}
	}
}

std::string Hitbox::getModeText() {
	return std::string(std::to_string(static_cast<int>(width)) + " " + std::to_string(static_cast<int>(height)));
}