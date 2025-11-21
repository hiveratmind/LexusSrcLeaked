#pragma once
#include "../../ModuleBase/Module.h"

class AutoTotem : public Module {
private:
	int delay = 0;
	long long lastPlaceTime = 0;
public:
	AutoTotem();
	void OnTick() override;
	std::string getModeText() override;
};
