#pragma once
#include "CommandBase.h"

class PanicCommand : public CommandBase {
public:
	PanicCommand();
	bool execute(const std::vector<std::string>& args) override;
};
