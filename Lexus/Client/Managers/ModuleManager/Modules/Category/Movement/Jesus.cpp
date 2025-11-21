#include "Jesus.h"

Jesus::Jesus() : Module(Category::Movement, "Jesus", "Walk over water, like Jesus.")
{
}

Jesus::~Jesus() {
}
void Jesus::OnTick() {
	LocalPlayer* gm = g_Data.getLocalPlayer();
	if (gm->getMoveInputHandler()->isSneakDown) return;

	if (g_Data.getClientInstance()->getRegion()->getBlock(gm->getPos().toInt())->blockLegacy->blockName.find("water") != std::string::npos) {
		gm->stateVector->velocity.y = 0.06f;
		gm->setIsOnGround(true);
		wasInWater = true;
	}
	else if (gm->isInWater() || gm->isInLava()) {
		gm->stateVector->velocity.y = 0.1f;
		gm->setIsOnGround(true);
		wasInWater = true;
	}
	else {
		if (wasInWater) {
			wasInWater = false;
			gm->stateVector->velocity.x *= 1.2f;
			gm->stateVector->velocity.x *= 1.2f;
		}
	}
}

std::string Jesus::getModeText() {
	return "Vanilla";
}
