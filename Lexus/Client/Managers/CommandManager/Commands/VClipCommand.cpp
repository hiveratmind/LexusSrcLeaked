#include "VClipCommand.h"
#include "../../../Client.h"
#include "../../../../Utils/Logger.h"

VClipCommand::VClipCommand() : CommandBase("vclip", "Vertically clip the player by <distance>", "<distance>", {}) {
}

bool VClipCommand::execute(const std::vector<std::string>& args) {
	if (args.size() != 2)
		return false;

	LocalPlayer* player = g_Data.getLocalPlayer();
	if (!player) {
		Client::DisplayClientMessage("%sFailed to find local player.", MCTF::RED);
		return true;
	}

	try {
		float distance = std::stof(args[1]);
		Vec3<float> pos = player->getPos();
		pos.y += distance;
		player->setPos(pos);
		Client::DisplayClientMessage("%sVClipped %s%.2f %sblocks.", MCTF::GRAY, MCTF::WHITE, distance, MCTF::GRAY);
	}
	catch (...) {
		Client::DisplayClientMessage("%sInvalid distance!", MCTF::RED);
	}

	return true;
}
