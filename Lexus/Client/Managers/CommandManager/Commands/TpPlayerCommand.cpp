#include "TpPlayerCommand.h"
#include "../../../Client.h"

TpPlayerCommand::TpPlayerCommand() : CommandBase("tpplayer", "Teleport to a player by name", "<player>", { "tpp" }) {
}

bool TpPlayerCommand::execute(const std::vector<std::string>& args) {
	if (args.size() != 2)
		return false;

	LocalPlayer* localPlayer = g_Data.getLocalPlayer();
	if (!localPlayer) {
		Client::DisplayClientMessage("%sFailed to find local player.", MCTF::RED);
		return true;
	}

	std::string targetName = args[1];
	std::transform(targetName.begin(), targetName.end(), targetName.begin(), ::tolower);

	for (auto& entity : localPlayer->level->getRuntimeActorList()) {
		if (!TargetUtil::ValidCheck(entity, false)) 
			continue;

		std::string entityName = entity->getNameTag()->c_str();
		std::transform(entityName.begin(), entityName.end(), entityName.begin(), ::tolower);

		if (entityName == targetName) {
			Vec3<float> pos = entity->getPos();
			localPlayer->setPos(pos);
			Client::DisplayClientMessage("%sTeleported to player %s%s.", MCTF::GRAY, entityName.c_str(), MCTF::RESET);
			return true;
		}
	}

	Client::DisplayClientMessage("%sPlayer '%s' not found.", MCTF::RED, targetName.c_str());
	return true;
}
