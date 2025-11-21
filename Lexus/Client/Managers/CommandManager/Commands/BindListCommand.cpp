#include "BindListCommand.h"
#include "../../../Client.h"

BindListCommand::BindListCommand() : CommandBase("bindlist", "Lists all module keybinds", "", {}) {
}

bool BindListCommand::execute(const std::vector<std::string>& args) {
	Client::DisplayClientMessage("%sModule Keybinds:", MCTF::BOLD);

	for (auto& mod : ModuleManager::moduleList) {
		int key = mod->getKeybind();
		if (key == 0) 
			continue;

		std::string keyName = "None";
		if (key > 0 && key < 190) {
			keyName = KeyNames[key];
		}
		Client::DisplayClientMessage("  %s%s%s : %s%s", MCTF::GRAY, mod->getModuleName().c_str(), MCTF::RESET, MCTF::GRAY, keyName.c_str());
	}

	return true;
}
