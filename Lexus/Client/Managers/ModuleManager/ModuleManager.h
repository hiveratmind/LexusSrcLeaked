#pragma once

#include "Modules/Category/Client/Console.h" //IMPORT BIG BOY STUFF

#include "Modules/ModuleBase/Module.h"
#include "Modules/Category/Player/PacketMine.h"
#include "Modules/Category/Render/FakeBlock.h"
#include "Modules/Category/Player/BedMiner.h"
#include "Modules/Category/Player/NoFall.h"
#include "Modules/Category/Combat/Criticals.h"
#include "Modules/Category/Player/Regen.h"
#include "Modules/Category/Client/Hud.h"
#include "Modules/Category/Render/ESP.h"
#include "Modules/Category/Combat/KillAura.h"
#include "Modules/Category/Combat/AutoTotem.h"
#include "Modules/Category/Movement/AutoSprint.h"
#include "Modules/Category/Movement/Jesus.h"
#include "Modules/Category/Movement/NoSlowDown.h"
#include "Modules/Category/Movement/Velocity.h"
#include "Modules/Category/Movement/AntiVoid.h"
#include "Modules/Category/Render/CustomChat.h"
#include "Modules/Category/Render/Fullbright.h"
#include "Modules/Category/Render/NoHurtCam.h"
#include "Modules/Category/Render/NoRender.h"
#include "Modules/Category/Render/Swing.h"
#include "Modules/Category/Render/ChestESP.h"
#include "Modules/Category/Render/BedESP.h"
#include "Modules/Category/Render/Ambience.h"
#include "Modules/Category/Render/TimeChanger.h"
#include "Modules/Category/Player/AntiBot.h"
#include "Modules/Category/World/Disabler.h"
#include "Modules/Category/Misc/NoPacket.h"
#include "Modules/Category/Misc/Timer.h"
#include "Modules/Category/Client/ClickGUI.h"
#include "Modules/Category/Client/CustomFont.h"
#include "Modules/Category/Combat/FastBow.h"
#include "Modules/Category/Movement/Fly.h"
#include "Modules/Category/Movement/Dolphin.h"
#include "Modules/Category/Movement/Jetpack.h"
#include "Modules/Category/Render/ViewClip.h"
#include "Modules/Category/Render/GlintColor.h"
#include "Modules/Category/Player/Scaffold.h"
#include "Modules/Category/Misc/HackerDetector.h"
#include "Modules/Category/Movement/BunnyHop.h"
#include "Modules/Category/Movement/AirJump.h"
#include "Modules/Category/Combat/Reach.h"
#include "Modules/Category/Combat/Hitbox.h"

class ModuleManager {
public:
	static inline std::vector<Module*> moduleList;

	static void init();
	static void shutdown();

	template <typename TRet>
	static TRet* getModule() {
		for (Module* mod : moduleList) {
			TRet* result = dynamic_cast<TRet*>(mod);
			if (result == nullptr)
				continue;
			return result;
		}
		return nullptr;
	}
	static std::vector<Module*> getEnabledModules() {
		std::vector<Module*> enabledModules;
		for (Module* mod : moduleList) {
			if (mod->isEnabled())
				enabledModules.push_back(mod);
		}
		return enabledModules;
	}
	template <typename T>
	static void registerModule(std::vector<Module*>& list) {
		list.push_back(new T());
	}
	static void onRecievePacket(Packet* packet, bool& cancel);
	static void BuildingHook(const BlockPos& blockPos, const uint8_t face, bool& cancel);
	static void OnKeyUpdateHook(int key, bool isDown);
	static void OnLocalTick();
	static void OnTick();
	static void WorldTickHook(Level* level);
	static void GLMHandRenderHook(glm::mat4* matrix);
	static void RotationUpdateHook(LocalPlayer* localPlayer);
	static void SentPacketHook(Packet* packet, bool& cancel);
	static void InterfaceUtilsHook();
	static void MCRHook(MinecraftUIRenderContext* renderCtx);
	static void LevelRenderHook();
	static void OnLoadConfig(void* conf);
	static void OnSaveConfig(void* conf);
	static void OnContainerScreenTick(ContainerScreenController* controller);
	static void onAttack(Actor* actor, bool& cancel);
	static void ImGuiHook();
};
