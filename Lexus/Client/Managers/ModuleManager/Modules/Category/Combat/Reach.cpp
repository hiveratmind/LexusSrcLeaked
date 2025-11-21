#include "Reach.h"

Reach::Reach() : Module(Category::Combat, "Reach", "Extend reach.") {
	addSetting(new SliderSetting<float>("Reach", "NULL", &reachValue, 7.f, 3.f, 10.f));
}

std::string Reach::getModeText() {
	char buffer[10];
	sprintf_s(buffer, "%.1f", reachValue);
	return std::string(buffer) + "m";
}

void writeToProtectedAddr(float* addr, float value)
{
	DWORD oldProtect;
	VirtualProtect(addr, sizeof(float), PAGE_EXECUTE_READWRITE, &oldProtect);
	*addr = value;
	VirtualProtect(addr, sizeof(float), oldProtect, &oldProtect);
}

void Reach::onDisable() {
	if (SurvivalreachPtr != nullptr) {
		*SurvivalreachPtr = 3.0f;
		DWORD oldProt = 0;
		VirtualProtect(SurvivalreachPtr, sizeof(float), oldProt, &oldProt);
	}
}

void Reach::OnTick() {
	if (!g_Data.getLocalPlayer()) return;

	if (SurvivalreachPtr == nullptr) {
		uintptr_t survivalReachSig = MemoryUtil::findSignature("F3 0F ? ? ? ? ? ? 44 0F ? ? 76 ? C6 44 24 64");
		survivalReachSig += 4;
		int offset = *reinterpret_cast<int*>(survivalReachSig);
		SurvivalreachPtr = reinterpret_cast<float*>(survivalReachSig + offset + 4);

		DWORD oldProt = 0;
		VirtualProtect(SurvivalreachPtr, 4, PAGE_EXECUTE_READWRITE, &oldProt);
	}

	if (SurvivalreachPtr != nullptr) {
		*SurvivalreachPtr = reachValue;
	}
}