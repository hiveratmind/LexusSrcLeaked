#include "Module.h"
#include "../../../../../Libs/json.hpp"
#include "../../Client/Client.h"

#include "../../../NotificationManager/NotificationManager.h"
#include "../../../../../Utils/FileUtil.h"

using json = nlohmann::json;

Module::Module(Category c, std::string moduleName, std::string des, int k) {
	this->name = moduleName;
	this->description = des;
	this->category = c;
	this->keybind = k;

	addSetting(new BoolSetting("Visible", "Visible on arraylist", &visible, true));
	addSetting(new KeybindSetting("Keybind", "Keybind of module", &keybind, k));
	addSetting(new EnumSetting("Toggle", "How module should be toggled", { "Press", "Hold" }, &toggleMode, 0));
}

Module::~Module() {
	for (auto& setting : settings) {
		delete setting;
		setting = nullptr;
	}
	settings.clear();
}

std::string Module::getModeText() {
	return "NULL";
}

bool Module::isEnabled() {
	return enabled;
}

bool Module::isVisible() {
	return visible;
}

bool Module::isHoldMode() {
	return toggleMode;
}

int Module::getKeybind() {
	return keybind;
}

void Module::setKeybind(int key) {
	this->keybind = key;
}

bool Module::runOnBackground() {
	return false;
}

void Module::setEnabled(bool enable) {
	if (this->enabled != enable) {
		this->enabled = enable;
		if (enable) {
			this->onEnable();
		}
		else {
			this->onDisable();
		}
	}
}

void Module::toggle() {
	setEnabled(!enabled);
}

void Module::BuildingHook(const BlockPos& blockPos, const uint8_t face, bool& cancel) {}

void Module::onDisable() {
	/*if (ModuleManager::getModule<ToggleSounds>()->isEnabled()) {
		FileUtil::PlaySoundFromUrl("https://yiffing.zone/sounds/notify_off.wav");
	}*/
}

void Module::onEnable() {
	/*if (ModuleManager::getModule<ToggleSounds>()->isEnabled()) {
		FileUtil::PlaySoundFromUrl("https://yiffing.zone/sounds/notify_on.wav");
	}*/
}

void Module::onAttack(Actor* actor, bool& cancel) {

}

void Module::OnKeyUpdateHook(int key, bool isDown) {
	if (getKeybind() == key) {
		if (isHoldMode()) {
			setEnabled(isDown);
		}
		else {
			if (isDown) {
				toggle();
			}
		}
	}
}

void Module::OnLocalTick() {
}

void Module::OnTick() {
}

void Module::OnContainerScreenTick(ContainerScreenController* controller) {
}

void Module::WorldTickHook(Level* level) {
}

void Module::ImGuiHook() {

}

void Module::RotationUpdateHook(LocalPlayer* localPlayer) {
}

void Module::SentPacketHook(Packet* packet, bool& cancel) {
}

void Module::InterfaceUtilsHook() {
}

void Module::GLMHandRenderHook(glm::mat4 *matrix) {
}

void Module::MCRHook(MinecraftUIRenderContext* renderCtx) {
}

void Module::LevelRenderHook() {
}

void Module::onReceivePacket(Packet* packet, bool& cancel) {

}

void Module::OnLoadConfig(void* confVoid) {
	json* conf = reinterpret_cast<json*>(confVoid);
	std::string modName = this->getModuleName();

	if (conf->contains(modName)) {
		json obj = conf->at(modName);
		if (obj.is_null())
			return;

		if (obj.contains("enabled")) {
			this->setEnabled(obj.at("enabled").get<bool>());
		}

		for (auto& setting : settings) {
			std::string settingName = setting->name;

			if (obj.contains(settingName)) {
				json confValue = obj.at(settingName);
				if (confValue.is_null())
					continue;

				switch (setting->type) {
				case SettingType::BOOL_S: {
					BoolSetting* boolSetting = static_cast<BoolSetting*>(setting);
					(*boolSetting->value) = confValue.get<bool>();
					break;
				}
				case SettingType::KEYBIND_S: {
					KeybindSetting* keybindSetting = static_cast<KeybindSetting*>(setting);
					(*keybindSetting->value) = confValue.get<int>();
					break;
				}
				case SettingType::ENUM_S: {
					EnumSetting* enumSetting = static_cast<EnumSetting*>(setting);
					(*enumSetting->value) = confValue.get<int>();
					break;
				}
				case SettingType::COLOR_S: {
					ColorSetting* colorSetting = static_cast<ColorSetting*>(setting);
					(*colorSetting->colorPtr) = ColorUtil::HexStringToColor(confValue.get<std::string>());
					break;
				}
				case SettingType::PAGE_S: {
					PageSetting* pageSetting = static_cast<PageSetting*>(setting);
					*pageSetting->valuePtr = obj[pageSetting->name].get<int>();
					break;
				}
				case SettingType::SLIDER_S: {
					SliderSettingBase* sliderSettingBase = static_cast<SliderSettingBase*>(setting);
					if (sliderSettingBase->valueType == ValueType::INT_T) {
						SliderSetting<int>* intSlider = static_cast<SliderSetting<int>*>(sliderSettingBase);
						(*intSlider->valuePtr) = confValue.get<int>();
					}
					else if (sliderSettingBase->valueType == ValueType::FLOAT_T) {
						SliderSetting<float>* floatSlider = static_cast<SliderSetting<float>*>(sliderSettingBase);
						(*floatSlider->valuePtr) = confValue.get<float>();
					}
					break;
				}
				}
			}
		}
	}
}

void Module::OnSaveConfig(void* confVoid) {
	json* conf = reinterpret_cast<json*>(confVoid);
	std::string modName = this->getModuleName();
	json obj = (*conf)[modName];

	obj["enabled"] = this->isEnabled();

	for (auto& setting : settings) {
		std::string settingName = setting->name;

		switch (setting->type) {
		case SettingType::BOOL_S: {
			BoolSetting* boolSetting = static_cast<BoolSetting*>(setting);
			obj[settingName] = (*boolSetting->value);
			break;
		}
		case SettingType::KEYBIND_S: {
			KeybindSetting* keybindSetting = static_cast<KeybindSetting*>(setting);
			obj[settingName] = (*keybindSetting->value);
			break;
		}
		case SettingType::ENUM_S: {
			EnumSetting* enumSetting = static_cast<EnumSetting*>(setting);
			obj[settingName] = (*enumSetting->value);
			break;
		}
		case SettingType::COLOR_S: {
			ColorSetting* colorSetting = static_cast<ColorSetting*>(setting);
			obj[settingName] = ColorUtil::ColorToHexString((*colorSetting->colorPtr));
			break;
		}
		case SettingType::PAGE_S: {
			PageSetting* pageSetting = static_cast<PageSetting*>(setting);
			obj[settingName] = (*pageSetting->valuePtr); 
			break;
		}
		case SettingType::SLIDER_S: {
			SliderSettingBase* sliderSettingBase = static_cast<SliderSettingBase*>(setting);
			if (sliderSettingBase->valueType == ValueType::INT_T) {
				SliderSetting<int>* intSlider = static_cast<SliderSetting<int>*>(sliderSettingBase);
				obj[settingName] = (*intSlider->valuePtr);
			}
			else if (sliderSettingBase->valueType == ValueType::FLOAT_T) {
				SliderSetting<float>* floatSlider = static_cast<SliderSetting<float>*>(sliderSettingBase);
				obj[settingName] = (*floatSlider->valuePtr);
			}
			break;
		}
		}
	}

	(*conf)[modName] = obj;
}
