#pragma once
#include "../../ModuleBase/Module.h"

class Reach : public Module {
private:

	float* SurvivalreachPtr = nullptr;
public:
	float reachValue = 4.f;
	Reach();

	virtual void onDisable() override;
	virtual void OnTick() override;
	std::string getModeText() override;
};