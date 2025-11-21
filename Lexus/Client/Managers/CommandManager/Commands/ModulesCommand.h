#pragma once
#include "CommandBase.h"

class ModulesCommand : public CommandBase {
public:
	ModulesCommand();
	bool execute(const std::vector<std::string>& args) override;
};
