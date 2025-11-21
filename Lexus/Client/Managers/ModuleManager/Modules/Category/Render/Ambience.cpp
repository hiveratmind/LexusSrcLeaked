#include "Ambience.h"
#include <chrono>

Ambience::Ambience() : Module(Category::Render, "Ambience", "Change The g_Data Ambience.") {
    addSetting(new BoolSetting("Rain", "", &radulasso1, radulasso1));
    addSetting(new BoolSetting("Fog", "", &radulasso2, radulasso2));
    addSetting(new SliderSetting<float>("Rain Level", "", &radulasso_rain_level, radulasso_rain_level, 0.f, 1.f, 0.f));
    addSetting(new SliderSetting<float>("Fog Level", "", &radulasso_fog_level, radulasso_fog_level, 0.f, 2.f, 0.f));
}

Ambience::~Ambience() {
}

void Ambience::OnLocalTick() {
    if (g_Data.getLocalPlayer() == nullptr) {
        return;
    }

    if (radulasso1) {
        g_Data.getClientInstance()->getRegion()->getDimension()->getWeather()->rainLevel = radulasso_rain_level;
    }

    if (radulasso2) {
        g_Data.getClientInstance()->getRegion()->getDimension()->getWeather()->fogLevel = radulasso_fog_level;
    }
}

void Ambience::onDisable() {
    if (g_Data.getLocalPlayer() == nullptr) {
        return;
    }

    g_Data.getClientInstance()->getRegion()->getDimension()->getWeather()->rainLevel = 0;
    g_Data.getClientInstance()->getRegion()->getDimension()->getWeather()->fogLevel = 0;
}

void Ambience::onEnable() {
    if (g_Data.getLocalPlayer() == nullptr) {
        return;
    }
}

