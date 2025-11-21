#pragma once
#include "../../ModuleBase/Module.h"
#include "PacketMine.h"

class BedMiner : public Module {
private:
    float horizontalRange = 6.f;

public:
    BedMiner();
    void OnTick() override;
};
