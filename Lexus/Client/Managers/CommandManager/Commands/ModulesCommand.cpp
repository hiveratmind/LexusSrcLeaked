#include "ModulesCommand.h"
#include "../../../Client.h"

ModulesCommand::ModulesCommand() : CommandBase("modules", "Lists all modules and their status", "", {}) {
}

bool ModulesCommand::execute(const std::vector<std::string>& args) {
	Client::DisplayClientMessage("%sModules List:", MCTF::BOLD);

	for (auto& mod : ModuleManager::moduleList) {
		Client::DisplayClientMessage("  %s%s%s : %s",
			MCTF::GRAY, mod->getModuleName().c_str(),
			MCTF::RESET, mod->isEnabled() ? "Enabled" : "Disabled");
	}

	return true;
}
