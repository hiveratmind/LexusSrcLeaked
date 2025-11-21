#pragma once
#include "../../ModuleBase/Module.h"

class NameTags : public Module {
private:
	float opacity = 0.4f;
public:
	NameTags();
	virtual void InterfaceUtilsHook() override;
};