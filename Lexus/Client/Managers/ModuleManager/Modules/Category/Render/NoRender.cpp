#include "NoRender.h"

NoRender::NoRender() : Module(Category::Render, "NoRender", "Don't render thing")
{
	addSetting(new BoolSetting("Fire", "NULL", &noFire, noFire));
	addSetting(new BoolSetting("Weather", "Disable render weather", &noWeather, noWeather));
	addSetting(new BoolSetting("Entities", "Disable render entities", &noEntities, noEntities));
	addSetting(new BoolSetting("Block Entities", "Disable render block entities", &noBlockEntities, noBlockEntities));
	addSetting(new BoolSetting("Particles", "Disable render particles", &noParticles, noParticles));
}

void NoRender::onDisable() {
	if (noWeatherBoolPtr != nullptr)
		*noWeatherBoolPtr = false;

	if (noEntitiesBoolPtr != nullptr)
		*noEntitiesBoolPtr = false;

	if (noBlockEntitiesBoolPtr != nullptr)
		*noBlockEntitiesBoolPtr = false;

	if (noParticlesBoolPtr != nullptr)
		*noParticlesBoolPtr = false;
}

void NoRender::OnLocalTick() {
	if (noWeatherBoolPtr != nullptr)
		*noWeatherBoolPtr = noWeather;

	if (noEntitiesBoolPtr != nullptr)
		*noEntitiesBoolPtr = noEntities;

	if (noBlockEntitiesBoolPtr != nullptr)
		*noBlockEntitiesBoolPtr = noBlockEntities;

	if (noParticlesBoolPtr != nullptr)
		*noParticlesBoolPtr = noParticles;
}