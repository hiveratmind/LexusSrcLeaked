#include "ModuleManager.h"
#include "../../Client.h"
#include "Modules/Category/Render/TargetHud.h"
#include "Modules/Category/Combat/TestModule.h"
#include "Modules/Category/Combat/Aimbot.h"
#include "Modules/Category/Combat/PerfectHits.h"
#include "Modules/Category/Render/NameTags.h"
#include "Modules/Category/Render/Ambient.h"
#include "Modules/Category/Render/JumpCircles.h"

void ModuleManager::init() {

	// Combat
	registerModule<KillAura>(moduleList);
	registerModule<Hitbox>(moduleList);
	registerModule<TestModule>(moduleList);
	registerModule<Reach>(moduleList);
	registerModule<Aimbot>(moduleList);
	registerModule<AutoTotem>(moduleList);
	registerModule<Criticals>(moduleList);
	registerModule<PerfectHits>(moduleList);
	registerModule<FastBow>(moduleList);

	// Movement
	registerModule<AutoSprint>(moduleList);
	registerModule<Jesus>(moduleList);
	registerModule<NoSlowDown>(moduleList);
	registerModule<Fly>(moduleList);
	registerModule<Dolphin>(moduleList);
	registerModule<AirJump>(moduleList);
	registerModule<BunnyHop>(moduleList);
	registerModule<Jetpack>(moduleList);
	registerModule<Velocity>(moduleList);
	registerModule<AntiVoid>(moduleList);

	// Render
	registerModule<StorageESP>(moduleList);
	registerModule<BedESP>(moduleList);
	registerModule<Ambient>(moduleList);
	registerModule<NameTags>(moduleList);
	registerModule<JumpCircle>(moduleList);
	registerModule<Fullbright>(moduleList);
	registerModule<NoHurtCam>(moduleList);
	registerModule<NoRender>(moduleList);
	registerModule<ESP>(moduleList);
	registerModule<Ambience>(moduleList);
	registerModule<FakeBlock>(moduleList);
	registerModule<ViewClip>(moduleList);
	registerModule<GlintColor>(moduleList);
	registerModule<TimeChanger>(moduleList);
	registerModule<Swing>(moduleList);

	// Player
	registerModule<AntiBot>(moduleList);
	registerModule<BedMiner>(moduleList);
	registerModule<Scaffold>(moduleList);
	registerModule<NoFall>(moduleList);
	registerModule<Regen>(moduleList);
	registerModule<PacketMine>(moduleList);

	// World
	registerModule<Disabler>(moduleList);

	// Misc
	registerModule<NoPacket>(moduleList);
	registerModule<HackerDetector>(moduleList);
	registerModule<Timer>(moduleList);

	// Client
	registerModule<ClickGUI>(moduleList);
	registerModule<CustomFont>(moduleList);
	registerModule<HUD>(moduleList);
	registerModule<Console>(moduleList);

	std::sort(moduleList.begin(), moduleList.end(), [](Module* lhs, Module* rhs) {
		return lhs->getModuleName() < rhs->getModuleName();
		});

	getModule<ClickGUI>()->InitClickGUI();
}

void ModuleManager::onRecievePacket(Packet* packet, bool& cancel) {
	if (!Client::isInitialized())
		return;

	for (auto& mod : moduleList) {
		if (mod->isEnabled() || mod->runOnBackground()) {
			mod->onReceivePacket(packet, cancel);
		}
		if (cancel) break;
	}
}

void ModuleManager::shutdown() {
	for (auto& mod : moduleList) {
		mod->setEnabled(false);
	}
	for (auto& mod : moduleList) {
		delete mod;
		mod = nullptr;
	}
	moduleList.clear();
}

void ModuleManager::OnKeyUpdateHook(int key, bool isDown) {
	if (!Client::isInitialized())
		return;

	for (auto& mod : moduleList) {
		mod->OnKeyUpdateHook(key, isDown);
	}
}

void ModuleManager::OnLocalTick() {
	if (!Client::isInitialized())
		return;

	for (auto& mod : moduleList) {
		if (mod->isEnabled() || mod->runOnBackground()) {
			mod->OnLocalTick();
		}
	}
}

void ModuleManager::OnTick() {
	if (!Client::isInitialized())
		return;

	for (auto& mod : moduleList) {
		if (mod->isEnabled() || mod->runOnBackground()) {
			mod->OnTick();
		}
	}
}

void ModuleManager::OnContainerScreenTick(ContainerScreenController* controller) {
	if (!Client::isInitialized())
		return;

	for (auto& mod : moduleList) {
		if (mod->isEnabled() || mod->runOnBackground()) {
			mod->OnContainerScreenTick(controller);
		}
	}
}

void ModuleManager::WorldTickHook(Level* level) {
	if (!Client::isInitialized())
		return;

	for (auto& mod : moduleList) {
		if (mod->isEnabled() || mod->runOnBackground()) {
			mod->WorldTickHook(level);
		}
	}
}

void ModuleManager::ImGuiHook() {
	if (!Client::isInitialized())
		return;

	for (auto& mod : moduleList) {
		if (mod->isEnabled() || mod->runOnBackground()) {
			mod->ImGuiHook();
		}
	}
}

void ModuleManager::GLMHandRenderHook(glm::mat4* matrix) {
	if (!Client::isInitialized())
		return;

	for (auto& mod : moduleList) {
		if (mod->isEnabled() || mod->runOnBackground()) {
			mod->GLMHandRenderHook(matrix);
		}
	}
}

void ModuleManager::RotationUpdateHook(LocalPlayer* localPlayer) {
	if (!Client::isInitialized())
		return;

	for (auto& mod : moduleList) {
		if (mod->isEnabled() || mod->runOnBackground()) {
			mod->RotationUpdateHook(localPlayer);
		}
	}
}

void ModuleManager::SentPacketHook(Packet* packet, bool& cancel) {
	if (!Client::isInitialized())
		return;

	for (auto& mod : moduleList) {
		if ((mod->isEnabled() || mod->runOnBackground()) && !cancel) {
			mod->SentPacketHook(packet, cancel);
		}
	}
}


void ModuleManager::InterfaceUtilsHook() {
	if (!Client::isInitialized())
		return;

	for (auto& mod : moduleList) {
		if (mod->isEnabled() || mod->runOnBackground()) {
			mod->InterfaceUtilsHook();
		}
	}
}

void ModuleManager::MCRHook(MinecraftUIRenderContext* renderCtx) {
	if (!Client::isInitialized())
		return;

	for (auto& mod : moduleList) {
		if (mod->isEnabled() || mod->runOnBackground()) {
			mod->MCRHook(renderCtx);
		}
	}
}

void ModuleManager::LevelRenderHook() {
	if (!Client::isInitialized())
		return;

	for (auto& mod : moduleList) {
		if (mod->isEnabled() || mod->runOnBackground()) {
			mod->LevelRenderHook();
		}
	}
}

void ModuleManager::OnLoadConfig(void* conf) {
	if (!Client::isInitialized())
		return;

	for (auto& mod : moduleList) {
		mod->OnLoadConfig(conf);
	}
}

void ModuleManager::OnSaveConfig(void* conf) {
	if (!Client::isInitialized())
		return;

	for (auto& mod : moduleList) {
		mod->OnSaveConfig(conf);
	}
}

void ModuleManager::BuildingHook(const BlockPos& blockPos, const uint8_t face, bool& cancel) {
	if (!Client::isInitialized()) return;
	for (auto& mod : moduleList) {
		if (mod->isEnabled() || mod->runOnBackground()) {
			mod->BuildingHook(blockPos, face, cancel);
		}
		if (cancel) break;
	}
}

void ModuleManager::onAttack(Actor* actor, bool& cancel) {
	if (!Client::isInitialized()) return;
	for (auto& mod : moduleList) {
		if (mod->isEnabled() || mod->runOnBackground()) {
			mod->onAttack(actor, cancel);
		}
		if (cancel) break;
	}
}
