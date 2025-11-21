#include "AutoSprint.h"

AutoSprint::AutoSprint() : Module(Category::Movement, "AutoSprint", "Automatically sprint without holding the key")
{
	modeEnum = (EnumSetting*)addSetting(new EnumSetting("Mode", "NULL", { "Legit", "Omni" }, &mode, 0));
}

std::string AutoSprint::getModeText() {
	return modeEnum->enumList[mode];
}

void AutoSprint::OnTick() {
	LocalPlayer* localPlayer = g_Data.getLocalPlayer();
	if (!g_Data.canUseMoveKeys())
		return;

	bool isSprinting = localPlayer->getStatusFlag(ActorFlags::Sprinting);
	if (isSprinting)
		return;

	if (mode == 0) {
		if (g_Data.isKeyDown('W'))
			localPlayer->setSprinting(true);
	}
	else {
		if (g_Data.isKeyDown('W') || g_Data.isKeyDown('A') || g_Data.isKeyDown('S') || g_Data.isKeyDown('D'))
			localPlayer->setSprinting(true);
	}
}
