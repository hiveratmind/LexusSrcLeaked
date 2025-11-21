#pragma once
#include "CommandBase.h"

class VClipCommand : public CommandBase {
public:
	VClipCommand();
	bool execute(const std::vector<std::string>& args) override;
};
