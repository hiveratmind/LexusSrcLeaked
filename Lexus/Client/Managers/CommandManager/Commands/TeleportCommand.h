#pragma once
#include "CommandBase.h"

class TeleportCommand : public CommandBase {
public:
	TeleportCommand();
	bool execute(const std::vector<std::string>& args) override;
};
