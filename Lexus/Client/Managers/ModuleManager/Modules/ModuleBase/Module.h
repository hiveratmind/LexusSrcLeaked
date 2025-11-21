#pragma once
#include <string>
#include <vector>
#include "../../../../../SDK/Game.h"
#include "../../../../../Renderer/D2D.h"
#include "../../../../../Renderer/MCR.h"
#include "../../../../../Utils/TimerUtil.h"
#include "../../../../../Utils/Minecraft/InventoryUtil.h"
#include "../../../../../Utils/Minecraft/PlayerUtil.h"
#include "../../../../../Utils/Minecraft/TargetUtil.h"
#include "../../../../../Utils/ActorUtils.h"
#include "../../../../../Utils/Minecraft/WorldUtil.h"
#include "../../../../../Libs/json.hpp"
#include "../../../../../Renderer/ImGuiUtils.h"
#include "../ModuleUtils.h/GetPingUtil.h"
#include "../ModuleUtils.h/QuickMath.h"

#include "Settings/Setting.h"
#include "Settings/BoolSetting.h"
#include "Settings/ColorSetting.h"
#include "Settings/EnumSetting.h"
#include "Settings/KeybindSetting.h"
#include "Settings/PageSetting.h"
#include "Settings/SliderSetting.h"

enum class Category {
	Combat = 0,
	Movement = 1,
	Render = 2,
	Player = 3,
	World = 4,
	Misc = 5,
	Client = 6,
	Unused = 7, //DO NOT USE THIS FOR MODULES!!
	Unused2 = 8 //DO NOT USE!!
};

class Module {
private:
	std::string name;
	std::string description;
	Category category;
	bool enabled = false;
	bool visible = true;
	int keybind = 0x0;
	int toggleMode = 0;

	std::vector<Setting*> settings;
public:
	//Arraylist stuff
	float arraylistAnim = 0.f;

	//ClickGUI stuff
	float selectedAnim = 0.f;
	bool extended = false;
	float toggleAnim = 0.0f;
protected:
	inline Setting* addSetting(Setting* setting) {
		this->settings.push_back(setting);
		return setting;
	}
public:
	Module(Category c, std::string moduleName, std::string des, int k = 0x0);
	~Module();

	inline std::string getModuleName() {
		return this->name;
	}

	inline std::string getDescription() {
		return this->description;
	}

	inline Category getCategory() {
		return this->category;
	}

	inline std::vector<Setting*>& getSettingList() {
		return this->settings;
	}
	int* modulePagePtr = nullptr;
	int modulePage = 0;
public:
	virtual std::string getModeText();
	virtual bool isEnabled();
	virtual bool isVisible();
	virtual bool isHoldMode();
	virtual int getKeybind();
	virtual void setKeybind(int key);
	virtual bool runOnBackground();
	virtual void setEnabled(bool enable);
	virtual void toggle();
	virtual void onDisable();
	virtual void onEnable();
	virtual void OnKeyUpdateHook(int key, bool isDown);
	virtual void OnLocalTick();
	virtual void OnTick();
	virtual void WorldTickHook(Level* level);
	virtual void RotationUpdateHook(LocalPlayer* localPlayer);
	virtual void SentPacketHook(Packet* packet, bool& cancel);
	virtual void InterfaceUtilsHook();
	virtual void MCRHook(MinecraftUIRenderContext* renderCtx);
	virtual void LevelRenderHook();
	virtual void GLMHandRenderHook(glm::mat4* matrix);
	virtual void OnLoadConfig(void* conf);
	virtual void BuildingHook(const BlockPos& blockPos, const uint8_t face, bool& cancel);
	virtual void OnSaveConfig(void* conf);
	virtual void onAttack(Actor* actor, bool& cancel);
	virtual void OnContainerScreenTick(ContainerScreenController* controller);
	virtual void ImGuiHook();
	virtual void onReceivePacket(Packet* packet, bool& cancel);
};
