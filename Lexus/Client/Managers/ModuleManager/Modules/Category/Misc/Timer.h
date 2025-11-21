#pragma once
#include "../../ModuleBase/Module.h"

class Timer : public Module {
private:
	float timerValue = 21.0f;
public:
	Timer();

	void onDisable() override;
	void OnLocalTick() override;
	std::string getModeText() override;
};