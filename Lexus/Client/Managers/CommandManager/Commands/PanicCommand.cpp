#include "PanicCommand.h"
#include "../../../Client.h"

PanicCommand::PanicCommand() : CommandBase("panic", "Disables all modules and clears all binds", "", {}) {
}

bool PanicCommand::execute(const std::vector<std::string>& args) {
	int disabledCount = 0;
	int unboundCount = 0;

	for (auto& mod : ModuleManager::moduleList) {
		if (mod->isEnabled()) {
			mod->setEnabled(false);
			disabledCount++;
		}

		if (mod->getKeybind() != 0) {
			mod->setKeybind(0);
			unboundCount++;
		}
	}

	Client::DisplayClientMessage("%sDisabled %d modules and cleared %d keybinds.", MCTF::GRAY, disabledCount, unboundCount);
	return true;
}
