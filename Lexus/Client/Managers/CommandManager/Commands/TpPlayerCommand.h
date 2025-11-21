#pragma once
#include "CommandBase.h"

class TpPlayerCommand : public CommandBase {
public:
	TpPlayerCommand();
	bool execute(const std::vector<std::string>& args) override;
};
