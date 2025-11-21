#include "ToggleCommand.h"
#include "../../../Client.h"

ToggleCommand::ToggleCommand() : CommandBase("toggle", "Toggles a module on or off", "<module>", { "t" }) {
}

bool ToggleCommand::execute(const std::vector<std::string>& args) {
	if (args.size() < 2)
		return false;

	std::string moduleName = args[1];
	std::transform(moduleName.begin(), moduleName.end(), moduleName.begin(), ::tolower);

	for (auto& mod : ModuleManager::moduleList) {
		std::string modName = mod->getModuleName();
		std::transform(modName.begin(), modName.end(), modName.begin(), ::tolower);

		if (modName == moduleName) {
			mod->toggle();
			Client::DisplayClientMessage("Module %s%s %s", MCTF::GRAY, mod->getModuleName().c_str(), mod->isEnabled() ? "enabled." : "disabled.");
			return true;
		}
	}

	Client::DisplayClientMessage("%sCouldn't find module with name: %s", MCTF::RED, moduleName.c_str());
	return true;
}
