#pragma once
#include "CommandBase.h"

class BindListCommand : public CommandBase {
public:
	BindListCommand();
	bool execute(const std::vector<std::string>& args) override;
};
