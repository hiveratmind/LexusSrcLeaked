#include "TeleportCommand.h"
#include "../../../Client.h"
#include "../../../../Utils/Logger.h"

TeleportCommand::TeleportCommand() : CommandBase("teleport", "Teleport to given XYZ coords", "<x> <y> <z>", { "tp" }) {
}

bool TeleportCommand::execute(const std::vector<std::string>& args) {
	if (args.size() != 4)
		return false;

	LocalPlayer* player = g_Data.getLocalPlayer();
	if (!player) {
		Client::DisplayClientMessage("%sFailed to find local player.", MCTF::RED);
		return true;
	}

	try {
		float x = std::stof(args[1]);
		float y = std::stof(args[2]);
		float z = std::stof(args[3]);

		player->setPos(Vec3<float>(x, y, z));

		Client::DisplayClientMessage("%sTeleported to %s%.2f %.2f %.2f", MCTF::GRAY, MCTF::WHITE, x, y, z);
	}
	catch (...) {
		Client::DisplayClientMessage("%sInvalid coordinates!", MCTF::RED);
	}

	return true;
}
