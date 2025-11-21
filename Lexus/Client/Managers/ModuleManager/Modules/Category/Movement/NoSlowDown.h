#pragma once
#include "../../ModuleBase/Module.h"

class NoSlowDown : public Module {
public:
	NoSlowDown();
	std::string getModeText() override;
};
