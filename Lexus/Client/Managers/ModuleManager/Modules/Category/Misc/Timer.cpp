#include "Timer.h"

Timer::Timer() : Module(Category::Misc, "Timer", "Modify TPS (ticks per second) of game") {
	addSetting(new SliderSetting<float>("TPS", "ticks per second", &timerValue, 21, 1, 50));
}

std::string Timer::getModeText() {
	char buffer[10];
	sprintf_s(buffer, "%.1f", timerValue);
	return std::string(buffer);
}

void Timer::onDisable() {
	Minecraft* mc = g_Data.clientInstance->minecraft;
	if (mc != nullptr) {
		*mc->minecraftTimer = 20.0f;
		*mc->minecraftRenderTimer = 20.0f;
	}
}

void Timer::OnLocalTick() {
	Minecraft* mc = g_Data.clientInstance->minecraft;
	if (mc != nullptr) {
		*mc->minecraftTimer = (float)timerValue;
		*mc->minecraftRenderTimer = (float)timerValue;
	}
}