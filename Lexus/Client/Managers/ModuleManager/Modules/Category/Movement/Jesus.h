#pragma once

#include "../../ModuleBase/Module.h"

class Jesus :
	public Module
{
private:
	bool wasInWater = false;
public:
	Jesus();
	~Jesus();

	std::string getModeText() override;
	virtual void OnTick() override;
};

