#pragma once
#include "../../ModuleBase/Module.h"

class AntiBot : public Module {
public:
    bool symbolCheck = false;
    int maxLength = 20;
    bool keyWords = false;
    bool invalid = false;
	AntiBot();
};
