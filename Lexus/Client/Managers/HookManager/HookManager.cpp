#include "HookManager.h"
#include "../ModuleManager/ModuleManager.h"
#include "../../Client.h"
#include <kiero.h>
#include "../ModuleManager/Modules/Category/Combat/Criticals.h"

class TextPacketHook : public FuncHook {
private:
	using func_t = void(__fastcall*)(const float*, const float*, const float*, std::shared_ptr<Packet>);
	static inline func_t oFunc;

	static void callback(const float* a1, const float* a2, const float* a3, std::shared_ptr<Packet> packet) {
		bool cancel = false;
		ModuleManager::onRecievePacket(packet.get(), cancel);
		if (!cancel) oFunc(a1, a2, a3, packet);
	}
public:
	TextPacketHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&callback;
	}
};

class LevelEventPacketHook : public FuncHook {
private:
	using func_t = void(__fastcall*)(const float*, const float*, const float*, std::shared_ptr<Packet>);
	static inline func_t oFunc;

	static void callback(const float* a1, const float* a2, const float* a3, std::shared_ptr<Packet> packet) {
		bool cancel = false;
		ModuleManager::onRecievePacket(packet.get(), cancel);
		if (!cancel) oFunc(a1, a2, a3, packet);
	}
public:
	LevelEventPacketHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&callback;
	}
};


class LevelSoundEventPacketHook : public FuncHook {
private:
	using func_t = void(__fastcall*)(const float*, const float*, const float*, std::shared_ptr<Packet>);
	static inline func_t oFunc;

	static void callback(const float* a1, const float* a2, const float* a3, std::shared_ptr<Packet> packet) {
		bool cancel = false;
		ModuleManager::onRecievePacket(packet.get(), cancel);
		if (!cancel) oFunc(a1, a2, a3, packet);
	}
public:
	LevelSoundEventPacketHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&callback;
	}
};


class ActorEventPacketHook : public FuncHook {
private:
	using func_t = void(__fastcall*)(const float*, const float*, const float*, std::shared_ptr<Packet>);
	static inline func_t oFunc;

	static void callback(const float* a1, const float* a2, const float* a3, std::shared_ptr<Packet> packet) {
		bool cancel = false;
		ModuleManager::onRecievePacket(packet.get(), cancel);
		if (!cancel) oFunc(a1, a2, a3, packet);
	}
public:
	ActorEventPacketHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&callback;
	}
};

class RemoveActorPacketHook : public FuncHook {
private:
	using func_t = void(__fastcall*)(const float*, const float*, const float*, std::shared_ptr<Packet>);
	static inline func_t oFunc;

	static void callback(const float* a1, const float* a2, const float* a3, std::shared_ptr<Packet> packet) {
		bool cancel = false;
		ModuleManager::onRecievePacket(packet.get(), cancel);
		if (!cancel) oFunc(a1, a2, a3, packet);
	}
public:
	RemoveActorPacketHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&callback;
	}
};

class SetScorePacketHook : public FuncHook {
private:
	using func_t = void(__fastcall*)(const float*, const float*, const float*, std::shared_ptr<Packet>);
	static inline func_t oFunc;

	static void callback(const float* a1, const float* a2, const float* a3, std::shared_ptr<Packet> packet) {
		bool cancel = false;
		ModuleManager::onRecievePacket(packet.get(), cancel);
		if (!cancel) oFunc(a1, a2, a3, packet);
	}
public:
	SetScorePacketHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&callback;
	}
};

ID3D11Device* g_Device = nullptr;
ID3D11DeviceContext* g_Context = nullptr;
HWND g_hWnd = nullptr;

BlockPos g_LastStartDestroyPos = BlockPos(0, 0, 0);
uint8_t g_LastStartDestroyFace = 255;

class ResizeBuffersHook : public FuncHook {
private:
	using reiszeBuffers_t = HRESULT(__thiscall*)(IDXGISwapChain*, int, int, int, DXGI_FORMAT, int);
	static inline reiszeBuffers_t oResizeBuffers;

	static HRESULT resizeBuffersCallback(IDXGISwapChain* swapChain, int bufferCount, int width, int height, DXGI_FORMAT newFormat, int swapChainFlags) {
		InterfaceUtils::Clean();
		return oResizeBuffers(swapChain, bufferCount, width, height, newFormat, swapChainFlags);
	}
public:
	ResizeBuffersHook() {
		OriginFunc = (void*)&oResizeBuffers;
		func = (void*)&resizeBuffersCallback;
	}
};

class PresentHook : public FuncHook {
private:
	using present_t = HRESULT(__thiscall*)(IDXGISwapChain3*, UINT, UINT);
	static inline present_t oPresent;

	static HRESULT presentCallback(IDXGISwapChain3* swapChain, UINT syncInterval, UINT flags) {

		if (!Client::isInitialized())
			return oPresent(swapChain, syncInterval, flags);

		// Initial and Update DeltaTime
		{
			static bool initDeltaTime = false;
			static std::chrono::steady_clock::time_point lastTime;
			static std::chrono::steady_clock::time_point currentTime;

			currentTime = std::chrono::steady_clock::now();

			if (initDeltaTime)
				InterfaceUtils::deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count() / 1000.0f;

			lastTime = currentTime;
			initDeltaTime = true;
		}

		static HWND window = (HWND)FindWindowA(nullptr, (LPCSTR)"Minecraft");

		ID3D12Device* d3d12Device = nullptr;
		ID3D11Device* d3d11Device = nullptr;

		if (SUCCEEDED(swapChain->GetDevice(IID_PPV_ARGS(&d3d12Device)))) {
			static_cast<ID3D12Device5*>(d3d12Device)->RemoveDevice();
			return oPresent(swapChain, syncInterval, flags);
		}
		else if (SUCCEEDED(swapChain->GetDevice(IID_PPV_ARGS(&d3d11Device)))) {

			InterfaceUtils::NewFrame(swapChain, d3d11Device, (float)GetDpiForWindow(window));
			InterfaceUtils::Render();
			InterfaceUtils::EndFrame();

			d3d11Device->Release();
		}

		return oPresent(swapChain, syncInterval, flags);
	}
public:
	PresentHook() {
		OriginFunc = (void*)&oPresent;
		func = (void*)&presentCallback;
	}
};

class LevelTickHook : public FuncHook {
private:
	using func_t = void(__thiscall*)(Level*);
	static inline func_t oFunc;

	static void LevelTickCallback(Level* _this) {
		if (g_Data.getLocalPlayer() != nullptr && g_Data.getLocalPlayer()->level == _this) {
			ModuleManager::WorldTickHook(_this);
		}
		return oFunc(_this);
	}
public:
	LevelTickHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&LevelTickCallback;
	}
};

class GetPickRangeHook : public FuncHook {
private:
	using func_t = float(__thiscall*)(GameMode*, InputMode const&, bool);
	static inline func_t oFunc;

	static float GameModeGetPickRangeCallback(GameMode* _this, InputMode const& currentInputMode, bool isVR) {
		return oFunc(_this, currentInputMode, isVR);
	}

public:
	GetPickRangeHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&GameModeGetPickRangeCallback;
	}
};

class StopDestroyBlockHook : public FuncHook {
private:
	using func_t = void(__thiscall*)(GameMode*, BlockPos*);
	static inline func_t oFunc;

	static void StopDestroyBlockCallback(GameMode* _this, BlockPos* pos) {
		if (ModuleManager::getModule<PacketMine>()->isEnabled()) return;
		oFunc(_this, pos);
	}
public:
	StopDestroyBlockHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&StopDestroyBlockCallback;
	}
};

class StartDestroyBlockHook : public FuncHook {
private:
	using func_t = bool(__thiscall*)(GameMode*, BlockPos*, uint8_t, bool*);
	static inline func_t oFunc;

	static bool StartDestroyBlockCallback(GameMode* _this, BlockPos* pos, uint8_t face, bool* hasDestroyedBlock) {
		g_LastStartDestroyPos = *pos;
		g_LastStartDestroyFace = face;

		auto packetMine = ModuleManager::getModule<PacketMine>();
		if (packetMine && packetMine->isEnabled()) {
			packetMine->mine(*pos, face);
		}
		return oFunc(_this, pos, face, hasDestroyedBlock);
	}

public:
	StartDestroyBlockHook() {
		OriginFunc = (void**)&oFunc;
		func = (void*)&StartDestroyBlockCallback;
	}
};


class PlayerSlowDownHook : public FuncHook {
private:
	using func_t = void(__thiscall*)(__int64, __int64, __int64);
	static inline func_t oFunc;

	static void PlayerSlowDownCallback(__int64 a1, __int64 a2, __int64 a3) {
		static NoSlowDown* noSlowMod = ModuleManager::getModule<NoSlowDown>();
		if (noSlowMod->isEnabled())
			return;

		oFunc(a1, a2, a3);
	}
public:
	PlayerSlowDownHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&PlayerSlowDownCallback;
	}
	// Search float value "0.34999999" -> xrefs first Function.
};

class MobSwingDurationHook : public FuncHook {
private:
	using func_t = uint32_t(__thiscall*)(Mob*);
	static inline func_t oFunc;

	static uint32_t MobGetCurrentSwingDurationCallback(Mob* _this) {
		static Swing* swingMod = ModuleManager::getModule<Swing>();
		if (swingMod->isEnabled() && g_Data.getLocalPlayer() == _this) {
			return swingMod->swingSpeed;
		}
		static FakeBlock* fb = ModuleManager::getModule<FakeBlock>();
		if (fb && fb->isEnabled() && fb->isBlocking) {
			return 10.f;
		}

		return oFunc(_this);
	}
public:
	MobSwingDurationHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&MobGetCurrentSwingDurationCallback;
	}
};

class ActorSetRotHook : public FuncHook {
private:
	using func_t = float* (__fastcall*)(__int64, __int64, Actor*);
	static inline func_t oFunc;

	static float* ActorSetRotCallback(__int64 a1, __int64 a2, Actor* a3) {
		LocalPlayer* localPlayer = g_Data.getLocalPlayer();
		if (localPlayer == a3) {
			float* result = oFunc(a1, a2, a3);
			ModuleManager::RotationUpdateHook(localPlayer);
			return result;
		}
		return oFunc(a1, a2, a3);
	}
public:
	ActorSetRotHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&ActorSetRotCallback;
	}
};

class ActorSwingHook : public FuncHook {
private:
	using func_t = void(__thiscall*)(Actor*);
	static inline func_t oFunc;

	static void ActorSwingCallback(Actor* _this) {
		oFunc(_this);
	}
public:
	ActorSwingHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&ActorSwingCallback;
	}
};

class ActorIsOnFireHook : public FuncHook {
private:
	using func_t = bool(__thiscall*)(Actor*);
	static inline func_t oFunc;

	static bool ActorIsOnFireCallback(Actor* _this) {
		static NoRender* noRenderMod = ModuleManager::getModule<NoRender>();
		if (_this == g_Data.getLocalPlayer() && noRenderMod->isEnabled() && noRenderMod->noFire)
			return false;

		return oFunc(_this);
	}
public:
	ActorIsOnFireHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&ActorIsOnFireCallback;
	}
};

class BobHurtHook : public FuncHook {
private:
	using func_t = uint32_t(__thiscall*)(void*, glm::mat4*);
	static inline func_t oFunc;

	static uint32_t BobHurtCallBack(void* _this, glm::mat4* matrix) {
		ModuleManager::GLMHandRenderHook(matrix);

		return oFunc(_this, matrix);
	}
public:
	BobHurtHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&BobHurtCallBack;
	}
};

class ActorNormalTickHook : public FuncHook {
private:
	using func_t = void(__thiscall*)(Actor*);
	static inline func_t oFunc;

	static void ActorNormalTickCallback(Actor* _this) {
		LocalPlayer* localPlayer = g_Data.getLocalPlayer();
		if (localPlayer == _this) {
			ModuleManager::OnTick();
		}

		oFunc(_this);
	}
public:
	ActorNormalTickHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&ActorNormalTickCallback;
	}
};

class ActorLerpMotionHook : public FuncHook {
private:
	using func_t = void(__thiscall*)(Actor*, Vec3<float>*);
	static inline func_t oFunc;

	static void ActorLerpMotionCallback(Actor* _this, Vec3<float>* delta) {

		static Velocity* velocityMod = ModuleManager::getModule<Velocity>();
		if (velocityMod->isEnabled() && g_Data.getLocalPlayer() == _this) {
			static void* networkSender = reinterpret_cast<void*>(MemoryUtil::findSignature("48 8D 57 38 48 8B CE 48") + 0x14);
			if (_ReturnAddress() == networkSender) {
				float hPercent = velocityMod->horizontal / 100.f;
				float vPercent = velocityMod->vertical / 100.f;
				*delta = _this->stateVector->velocity.lerp(*delta, hPercent, vPercent, hPercent);
			}
		}

		return oFunc(_this, delta);
	}
public:
	ActorLerpMotionHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&ActorLerpMotionCallback;
	}
};

class AttackHook : public FuncHook {
private:
	using func_t = bool(__thiscall*)(GameMode*, Actor*);
	static inline func_t oFunc;

	static bool callback(GameMode* _this, Actor* _actor) {
		static PacketMine* packetMine = ModuleManager::getModule<PacketMine>();
		bool cancel = false;
		ModuleManager::onAttack(_actor, cancel);
		if (cancel) return false;
		return oFunc(_this, _actor);
	}
public:
	AttackHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&callback;
	}
};

class GetBlockHook : public FuncHook {
private:
	using func_t = Block * (__thiscall*)(BlockSource*, const BlockPos&);
	static inline func_t oFunc;

	static Block* BlockSourceGetBlockCallback(BlockSource* _this, const BlockPos& blockPos) {
		Block* result = oFunc(_this, blockPos);
		BedESP* bedChams = ModuleManager::getModule<BedESP>();
		if (bedChams && bedChams->isEnabled() && result && result->blockLegacy) {
			std::string blockName = result->blockLegacy->blockName;

			if (!blockName.empty() && blockName.find("bed") != std::string::npos) {
				if (!blockName.empty() && blockName.find("bedrock") == std::string::npos) {
					bedChams->blockList[blockPos] = bedChams->fillColor;
				}
			}
		}

		StorageESP* storageESP = ModuleManager::getModule<StorageESP>();
		if (storageESP && storageESP->isEnabled() && result && result->blockLegacy) {
			std::string blockName = result->blockLegacy->blockName;
			std::transform(blockName.begin(), blockName.end(), blockName.begin(), ::tolower);

			if (blockName.find("chest") != std::string::npos && blockName.find("ender") == std::string::npos) {
				storageESP->blockList[blockPos] = { storageESP->chestFillColor, storageESP->chestLineColor };
			}
			else if (blockName.find("shulker") != std::string::npos) {
				storageESP->blockList[blockPos] = { storageESP->shulkerFillColor, storageESP->shulkerLineColor };
			}
			else if (storageESP->showHopper && (result->blockLegacy->blockId == 154 || blockName.find("hopper") != std::string::npos)) {
				storageESP->blockList[blockPos] = { storageESP->hopperFillColor, storageESP->hopperLineColor };
			}
			else if (storageESP->showDropper && blockName.find("dropper") != std::string::npos) {
				storageESP->blockList[blockPos] = { storageESP->dropperFillColor, storageESP->dropperLineColor };
			}
			else if (storageESP->showDispenser && (result->blockLegacy->blockId == 23 || blockName.find("dispenser") != std::string::npos)) {
				storageESP->blockList[blockPos] = { storageESP->dispenserFillColor, storageESP->dispenserLineColor };
			}
		}
		return result;
	}
public:
	GetBlockHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&BlockSourceGetBlockCallback;
	}
};

class SendPacketHook : public FuncHook {
private:
	using func_t = void(__thiscall*)(PacketSender*, Packet*);
	static inline func_t oFunc;

	static void SendPacketCallback(PacketSender* _this, Packet* packet) {
		bool cancel = false;

		ModuleManager::SentPacketHook(packet, cancel);

		if (!cancel) {
			oFunc(_this, packet);
		}
	}

public:
	SendPacketHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&SendPacketCallback;
	}
};

class DrawImageHook : public FuncHook {
private:
	using func_t = __int64(__fastcall*)(MinecraftUIRenderContext*, TexturePtr*, Vec2<float>&, Vec2<float>&, Vec2<float>&, Vec2<float>&);
	static inline func_t oFunc;

	static __int64 DrawImageCallback(MinecraftUIRenderContext* ctx, TexturePtr* texture, Vec2<float>& pos, Vec2<float>& size, Vec2<float>& uvPos, Vec2<float>& uvSize) {
		if (!texture || !texture->texture)
			return oFunc(ctx, texture, pos, size, uvPos, uvSize);
		const char* texPath = texture->texture->filePath.empty() ? nullptr : texture->texture->filePath.c_str();
		if (texPath && strcmp(texPath, "textures/ui/title") == 0) {
			if (!RenderUtils::renderCtx) return 0;

			static const std::string mainText = "Lexus";
			static const std::string subText = "beta";

			float mainScale;
			if (g_Data.getLocalPlayer()) {
				mainScale = 1.0f;
			}
			else {
				mainScale = 7.5f;
			}

			float subScale = mainScale * 0.75f;

			float mainTextWidth = RenderUtils::GetTextWidth(mainText, mainScale);
			float mainTextHeight = RenderUtils::GetTextHeight(mainScale);

			float subTextHeight = RenderUtils::GetTextHeight(subScale);

			float textX = pos.x + (size.x / 2.f) - (mainTextWidth / 2.f);

			float mainTextY;
			if (g_Data.getLocalPlayer()) {
				mainTextY = pos.y + 1.f;
			}
			else {
				mainTextY = pos.y;
			}

			Vec2<float> mainTextPos = { textX, mainTextY };
			RenderUtils::RenderText(mainTextPos, mainText, mce::Color(255, 100, 100), mainScale);

			Vec2<float> subTextPos = { textX, mainTextY + mainTextHeight };
			RenderUtils::RenderText(subTextPos, subText, mce::Color(255, 100, 100), subScale);

			RenderUtils::renderCtx->flushText(1.0f);
			return 0;
		}
		return oFunc(ctx, texture, pos, size, uvPos, uvSize);
	}

public:
	DrawImageHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&DrawImageCallback;
	}
};

class SetUpAndRenderHook : public FuncHook {
private:
	using func_t = void(__thiscall*)(ScreenView*, MinecraftUIRenderContext*);
	static inline func_t oFunc;

	static void ScreenView_SetUpAndRenderCallback(ScreenView* _this, MinecraftUIRenderContext* renderCtx) {
		static bool initialized = false;
		if (!initialized && renderCtx) {
			static DrawImageHook drawImageHook;
			void** ctxVtable = *(void***)renderCtx;
			drawImageHook.HookVirtual(ctxVtable, 7, "DrawImage");
			initialized = true;
		}

		oFunc(_this, renderCtx);

		uintptr_t* visualTree = *(uintptr_t**)((uintptr_t)(_this)+0x48);
		std::string rootControlName = *(std::string*)((uintptr_t)(visualTree)+0x28);

		static std::string debugScreen = "debug_screen.debug_screen";
		if (strcmp(rootControlName.c_str(), debugScreen.c_str()) == 0) {
			RenderUtils::onRenderScreen(renderCtx);
			RenderUtils::deltaTime = _this->deltaTime;
		}
	}

public:
	SetUpAndRenderHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&ScreenView_SetUpAndRenderCallback;
	}
};
Vec2<float> hotbarPos = Vec2<float>(0, 0);

Vec2<float> Lerp(Vec2<float> a, Vec2<float> b, float t) {
	return Vec2<float>(a.x + t * (b.x - a.x), a.y + t * (b.y - a.y));
}

class RenderLevelHook : public FuncHook {
private:
	using func_t = __int64(__thiscall*)(LevelRenderer*, ScreenContext*, __int64);
	static inline func_t oFunc;

	static __int64 LevelRenderer_renderLevelCallback(LevelRenderer* _this, ScreenContext* a2, __int64 a3) {
		__int64 result = oFunc(_this, a2, a3);
		if (g_Data.getLocalPlayer() != nullptr && _this->levelRendererPlayer != nullptr && RenderUtils::blendMaterial != nullptr) {
			RenderUtils::onRenderWorld(_this, a2);
		}
		return result;
		//return oFunc(a1, a2, a3);
	}
public:
	RenderLevelHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&LevelRenderer_renderLevelCallback;
	}
	// search for "_effect" -> find function that only have move, movzx -> xref first Function -> function access a3 is LevelRenderer::renderLevel
};

class RenderHitSelectHook : public FuncHook {
private:
	using func_t = void(__thiscall*)(LevelRendererPlayer*, BaseActorRenderContext*, BlockSource*, BlockPos*, bool);
	static inline func_t oFunc;

	static void RenderHitSelectCallback(LevelRendererPlayer* _this, BaseActorRenderContext* renderContext, BlockSource* region, BlockPos* pos, bool fancyGraphics) {
		oFunc(_this, renderContext, region, pos, fancyGraphics);
	}
public:
	RenderHitSelectHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&RenderHitSelectCallback;
	}
};

class HurtCamHook : public FuncHook {
private:
	using func_t = __int64(__thiscall*)(Actor*, char, uint32_t);
	static inline func_t oFunc;

	static __int64 CauseHurtCamCallback(Actor* a1, char a2, uint32_t a3) {
		__int64 result = oFunc(a1, a2, a3);

		static NoHurtCam* noHurtCamMod = ModuleManager::getModule<NoHurtCam>();

		if (noHurtCamMod->isEnabled() && a2 == 2 && a1 == g_Data.getLocalPlayer()) {
			uint32_t* a4 = *(uint32_t**)((uintptr_t)a1 + 0x5A8); // MobAnimation::decrementHurtTime
			if (a4 != nullptr) {
				*a4 = 0;
			}
		}

		return result;
	}
public:
	HurtCamHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&CauseHurtCamCallback;
	}
	// search for "minecraft:totem_particle" lel
};

class GetViewPerspectiveHook : public FuncHook {
private:
	using func_t = int(__thiscall*)(__int64);
	static inline func_t oFunc;

	static int GetViewPerspectiveCallback(__int64 a1) {
		int result = oFunc(a1);
		g_Data.viewPerspectiveMode = result;
		return result;
	}
public:
	GetViewPerspectiveHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&GetViewPerspectiveCallback;
	}
};

class GetGammaHook : public FuncHook {
private:
	using func_t = float(__thiscall*)(__int64);
	static inline func_t oFunc;

	static float GetGammaCallback(__int64 a1) {

		static bool initPtr = false;
		if (!initPtr) {
			uintptr_t** list = (uintptr_t**)a1;
			NoRender* noRenderMod = ModuleManager::getModule<NoRender>();

			int obtainedSettings = 0;
			for (uint16_t i = 3; i < 450; i++) {
				if (list[i] == nullptr) continue;
				uintptr_t* info = *(uintptr_t**)((uintptr_t)list[i] + 0x8);
				if (info == nullptr) continue;

				std::string* translateName = (std::string*)((uintptr_t)info + 0x158);
				bool* boolSetting = (bool*)((uintptr_t)list[i] + 16);
				if (strcmp(translateName->c_str(), "options.dev_disableRenderWeather") == 0) {
					noRenderMod->noWeatherBoolPtr = boolSetting;
					obtainedSettings++;
				}
				else if (strcmp(translateName->c_str(), "options.dev_disableRenderEntities") == 0) {
					noRenderMod->noEntitiesBoolPtr = boolSetting;
					obtainedSettings++;
				}
				else if (strcmp(translateName->c_str(), "options.dev_disableRenderBlockEntities") == 0) {
					noRenderMod->noBlockEntitiesBoolPtr = boolSetting;
					obtainedSettings++;
				}
				else if (strcmp(translateName->c_str(), "options.dev_disableRenderParticles") == 0) {
					noRenderMod->noParticlesBoolPtr = boolSetting;
					obtainedSettings++;
				}

				if (obtainedSettings == 4)
					break;
			}
			initPtr = true;
		}

		static Fullbright* fullBrightMod = ModuleManager::getModule<Fullbright>();
		if (fullBrightMod->isEnabled())
			return 12.f;

		return oFunc(a1);
	}
public:
	GetGammaHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&GetGammaCallback;
	}
};

class GetFovHook : public FuncHook {
private:
	using func_t = float(__thiscall*)(LevelRendererPlayer*, float, bool);
	static inline func_t oFunc;

	static float GetFovCallback(LevelRendererPlayer* _this, float a, bool applyEffects) {

		static void* getFovForCameraReceive = (void*)MemoryUtil::findSignature("F3 0F 59 05 ?? ?? ?? ?? 48 8B C7 48 8B 5C 24");
		if (_ReturnAddress() == getFovForCameraReceive) {
		}

		return oFunc(_this, a, applyEffects);
	}
public:
	GetFovHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&GetFovCallback;
	}
};

class GetCameraComponentHook : public FuncHook {
private:
	using func_t = void* (__thiscall*)(__int64, __int64);
	static inline func_t oFunc;

	static void* GetCameraComponentCallback(__int64 a1, __int64 a2) {
		void* result = oFunc(a1, a2);
		static void* cameraDistanceCall = reinterpret_cast<void*>(MemoryUtil::findSignature("48 85 C0 74 06 F3 44 0F"));
		if (_ReturnAddress() == cameraDistanceCall && result != nullptr) {
			float& cameraDistance = hat::member_at<float>(result, 0x14);
			cameraDistance = 4.f;
		}
		return result;
	}
public:
	GetCameraComponentHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&GetCameraComponentCallback;
	}
};

class KeyMouseHook : public FuncHook {
private:
	using func_t = void(__fastcall*)(__int64, char, char, __int16, __int16, __int16, __int16, char);
	static inline func_t oFunc;

	static void mouseInputCallback(__int64 a1, char mouseButton, char isDown, __int16 mouseX, __int16 mouseY, __int16 relativeMovementX, __int16 relativeMovementY, char a8) {

		g_Data.keyMousePtr = (void*)(a1 + 0x10);

		static ClickGUI* clickGuiMod = ModuleManager::getModule<ClickGUI>();
		static Console* consoleMod = ModuleManager::getModule<Console>();

		Vec2<float> mousePosF((float)mouseX, (float)mouseY);
		short mouseWheelDelta = 0;

		if (mouseButton == 4) {
			mouseWheelDelta = relativeMovementY;
		}

		if (clickGuiMod && clickGuiMod->isEnabled()) {
			clickGuiMod->onMouseUpdate(mousePosF, mouseButton, isDown);
			return;
		}

		if (!g_Data.canUseMoveKeys()) {
			if (consoleMod && consoleMod->isEnabled()) {
				consoleMod->onMouseUpdate(mousePosF, mouseButton, isDown, mouseWheelDelta);
			}
		}

		oFunc(a1, mouseButton, isDown, mouseX, mouseY, relativeMovementX, relativeMovementY, a8);
	}
public:
	KeyMouseHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&mouseInputCallback;
	}
};

class KeyMapHook : public FuncHook {
private:
	using func_t = __int64(__fastcall*)(uint64_t, bool);
	static inline func_t oFunc;

	static __int64 KeyInputCallback(uint64_t key, bool isDown) {

		static ClickGUI* clickGuiMod = ModuleManager::getModule<ClickGUI>();
		if (clickGuiMod->isEnabled()) {
			clickGuiMod->OnKeyUpdateHook((int)key, isDown);
			return 0;
		}

		if (g_Data.canUseMoveKeys())
			ModuleManager::OnKeyUpdateHook((int)key, isDown);

		return oFunc(key, isDown);
	}
public:
	static void createKeyPress(uint64_t key, bool down) {
		oFunc(key, down);
	}

	KeyMapHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&KeyInputCallback;
	}

	void onHookRequest() override {
		uintptr_t keyMapOffset = 0x0;
		uintptr_t sigOffset = this->address + 7;
		int offset = *reinterpret_cast<int*>((sigOffset + 3));
		keyMapOffset = sigOffset + offset + 7;
		g_Data.keyMapPtr = (void*)keyMapOffset;
	}
};

class SendChatMessageHook : public FuncHook {
private:
	using func_t = bool(__thiscall*)(__int64, const std::string&);
	static inline func_t oFunc;

	static bool SendChatMessageCallback(__int64 _this, const std::string& message) {
		if (message.size() > 0) {
			char firstChar = message[0];
			if (firstChar == CommandManager::prefix) {
				CommandManager::execute(message);
				return false;
			}
		}
		return oFunc(_this, message);
	}
public:
	SendChatMessageHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&SendChatMessageCallback;
	}
};

class ClientInstanceUpdateHook : public FuncHook {
private:
	using func_t = __int64(__thiscall*)(ClientInstance*, bool);
	static inline func_t oFunc;

	static __int64 ClientInstance_UpdateCallback(ClientInstance* _this, bool a2) {
		g_Data.clientInstance = _this;
		return oFunc(_this, a2);
	}
public:
	ClientInstanceUpdateHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&ClientInstance_UpdateCallback;
	}
};

class BuildBlockHook : public FuncHook {
private:
	using func_t = bool(__thiscall*)(GameMode*, BlockPos const&, uint8_t, bool);
	static inline func_t oFunc;

	static bool callback(GameMode* _this, BlockPos const& blockPos, uint8_t face, bool a4) {
	//	static AntiDesync* antiDesync = ModuleManager::getModule<AntiDesync>();
	//	if (antiDesync->isEnabled() && g_Data.getLocalPlayer()->getItemUseDuration() > 0) return false; //this is smart
		bool cancel = false;
		ModuleManager::BuildingHook(blockPos, face, cancel);
		if (cancel) return false;
		return oFunc(_this, blockPos, face, a4);
	}
public:
	BuildBlockHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&callback;
	}
};

class ActorShouldRenderHook : public FuncHook {
private:
	using func_t = bool(__thiscall*)(Actor*);
	static inline func_t oFunc;

	static bool callback(Actor* _this) {
		return oFunc(_this);
	}
public:
	ActorShouldRenderHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&callback;
	}
};

class GetTimeOfDayHook : public FuncHook {
private:
	using func_t = float(__fastcall*)(Dimension*, int, float);
	static inline func_t oFunc;

	static float GetTimeOfDayCallback(Dimension* _this, int a2, float a3) {
		static TimeChanger* timeChanger = ModuleManager::getModule<TimeChanger>();

		if (timeChanger && timeChanger->isEnabled()) {
			return timeChanger->FurryWare;
		}

		return oFunc(_this, a2, a3);
	}

public:
	GetTimeOfDayHook() {
		OriginFunc = (void**)&oFunc;
		func = (void*)&GetTimeOfDayCallback;
		// Signature scanning and Create() are assumed to be handled externally
	}
};

class SplashTextRendererHook : public FuncHook {
private:
	using func_t = void(__fastcall*)(__int64, __int64, __int64, __int64, int, float*);
	static inline func_t oFunc;

	static void callback(__int64 a1, __int64 a2, __int64 a3, __int64 a4, int a5, float* a6) {
		if ((uintptr_t)a1) {
			float v37 = (float)sin(*(double*)((uintptr_t)a1 + 0x18) * 7.225663154526103);
			textSize = (float)((((v37 * v37) * (v37 * v37)) * 0.059999999f) + 1.3f);
		}
		oFunc(a1, a2, a3, a4, a5, a6);
	}
public:
	static inline float textSize = 0.f;

	SplashTextRendererHook() {
		OriginFunc = (void**)&oFunc;
		func = (void*)&callback;
	}
};

class NameTagRendererHook : public FuncHook {
private:
	using func_t = void(__fastcall*)(__int64, __int64, __int64, __int64, int, float*);
	static inline func_t oFunc;

	static void callback(__int64 a1, __int64 a2, __int64 a3, __int64 a4, int a5, float* a6) {
		nametag = *(std::string*)(a1 + 0x10);
		oFunc(a1, a2, a3, a4, a5, a6);
	}
public:
	static inline std::string nametag = "";
	static inline float textSize = 0.f;

	NameTagRendererHook() {
		OriginFunc = (void**)&oFunc;
		func = (void*)&callback;
	}
};

class DrawTransformedHook : public FuncHook {
private:
	using func_t = void(__fastcall*)(__int64*, __int64*, std::string*, float, float, UIColor*, float, float, bool, float);
	static inline func_t oFunc;

	static void callback(__int64* _this, __int64* screenContext, std::string* str, float x, float y, UIColor* color, float angle, float scale, bool centered, float maxWidth) {
		std::string pre = *str;

		std::string logged_as = MCTF::GRAY + std::string("Logged as ") + MCTF::DARK_GRAY + "[" + MCTF::WHITE + NameTagRendererHook::nametag + MCTF::DARK_GRAY + "]";

		using getLineLength = float(__thiscall*)(__int64*, std::string*, float, bool);
		using getTextHeight = float(__thiscall*)(__int64*, std::string*, float);

		static getLineLength getWidthTextFunc = reinterpret_cast<getLineLength>(MemoryUtil::findSignature("48 8B C4 48 81 EC A8 00 00 00 0F"));
		static getTextHeight getTextHeightFunc = reinterpret_cast<getTextHeight>(MemoryUtil::findSignature("48 89 5C 24 ?? 57 48 83 EC ?? 48 8B 01 48 8B FA 0F 29"));

		float textSize = SplashTextRendererHook::textSize * 0.7f;
		std::string what = MCTF::DARK_GRAY + std::string("Welcome to Lexus.");

		oFunc(_this, screenContext, &what, x, y, color, angle, textSize, centered, maxWidth);

		float textHeight = getTextHeightFunc(_this, &logged_as, textSize);
		float textWidth = getWidthTextFunc(_this, &logged_as, textSize, false);
		oFunc(_this, screenContext, &logged_as, x, y + textHeight, color, angle, textSize, true, textWidth);
	}
public:
	DrawTransformedHook() {
		OriginFunc = (void**)&oFunc;
		func = (void*)&callback;
	}

	static inline std::string fontoffset = "";
};

class ContainerScreenTickHook : public FuncHook { //no longer a broken piece of mess!
private:
	using func_t = __int64(__fastcall*)(ContainerScreenController*);
	static inline func_t oFunc;
	static __int64 callback(ContainerScreenController* _this) {
		if (g_Data.getLocalPlayer()) {
			ModuleManager::OnContainerScreenTick(_this);
		}
		return oFunc(_this);
	}
public:
	ContainerScreenTickHook() {
		OriginFunc = (void**)&oFunc;
		func = (void*)&callback;
	}
};

#include <string>

class ConnectionRequestCreateHook : public FuncHook {
private:
	using func_t = void(__fastcall*)(
		__int64*, __int64, __int64, __int64, __int64, __int64, __int64,
		__int64, __int64, __int64, std::string*, int, int, int, __int64,
		char, char, __int64, int, std::string*, std::string*, bool,
		__int64, __int64, __int64, char);

	static inline func_t oFunc;

	static std::string GenerateRandomDeviceID() {
		static const char charset[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";

		const size_t maxSize = 102 * 102;
		std::string result;
		result.reserve(maxSize);

		for (size_t i = 0; i < maxSize; ++i) {
			result += charset[rand() % (sizeof(charset) - 1)];
		}

		return result;
	}

	static void callback(
		__int64* a1, __int64 a2, __int64 a3, __int64 a4, __int64 a5,
		__int64 a6, __int64 a7, __int64 a8, __int64 a9, __int64 a10,
		std::string* deviceId, int a12, int a13, int a14, __int64 a15,
		char a16, char a17, __int64 a18, int a19,
		std::string* platformUserId, std::string* thirdPartyName,
		bool thirdPartyNameOnly, __int64 a23, __int64 a24,
		__int64 a25, char a26)
	{
		if (deviceId) {
			*deviceId = GenerateRandomDeviceID();
		}

		if (g_Data.getLocalPlayer()) {
			networkSystem* netSys = g_Data.getLocalPlayer()->level->getPacketSender()->networkSystem;
			if (netSys) {
				SocialGameConnectionInfo* info = netSys->getConnectionInfo();
				if (info) {
					std::string serverIp = info->serverIp;
					if (!serverIp.empty()) {
						std::string notificationText = "Connecting to server: " + serverIp;
						NotificationManager::addNotification(notificationText, 5.f);
					}
				}
			}
		}

		oFunc(
			a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
			deviceId, a12, a13, a14, a15, a16, a17, a18, a19,
			platformUserId, thirdPartyName, thirdPartyNameOnly,
			a23, a24, a25, a26);
	}

public:
	ConnectionRequestCreateHook() {
		OriginFunc = (void**)&oFunc;
		func = (void*)&callback;
	}
};

void HookManager::init() {
	MH_Initialize();

	RequestHook<ClientInstanceUpdateHook>(Addresses::ClientInstance_update);
	RequestHook<SendChatMessageHook>(Addresses::ClientInstanceScreenModel_sendChatMessage);
	RequestHook<KeyMapHook>(Addresses::KeyPressFunc);
	RequestHook<KeyMouseHook>(Addresses::KeyMouseFunc);
	RequestHook<GetCameraComponentHook>(Addresses::GetCameraComponent);
	RequestHook<GetFovHook>(Addresses::LevelRendererPlayer_getFov);
	RequestHook<GetGammaHook>(Addresses::Options_getGamma);
	RequestHook<GetViewPerspectiveHook>(Addresses::Options_getViewPerspective);
	RequestHook<HurtCamHook>(Addresses::CauseHurtCamFunc);
	RequestHook<RenderHitSelectHook>(Addresses::LevelRendererPlayer_renderHitSelect);
	RequestHook<RenderLevelHook>(Addresses::LevelRenderer_renderLevel);
	RequestHook<SetUpAndRenderHook>(Addresses::ScreenView_setUpAndRender);
	RequestHook<BobHurtHook>("40 53 56 48 83 EC 78 ? ? 7C");
	uintptr_t splashRenderAddr = MemoryUtil::findSignature("48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D A8 D8 F0");
	RequestHook<SplashTextRendererHook>(splashRenderAddr); //W
	uintptr_t drawTransformedAddr = MemoryUtil::findSignature("48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41 54 41 56 41 57 48 81 EC 00");
	RequestHook<DrawTransformedHook>(drawTransformedAddr);
	uintptr_t nameTagRenderAddr = MemoryUtil::findSignature("48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D A8 88 FE FF FF 48 81 EC ?? ?? ?? ?? 0F 29 70 A8");
	RequestHook<NameTagRendererHook>(nameTagRenderAddr);
	uintptr_t containerScreenTickAddr = MemoryUtil::findSignature("48 8B C4 48 89 58 ?? 48 89 68 ?? 48 89 70 ?? 57 41 56 41 57 48 81 EC ?? ?? ?? ?? 0F 29 70 D8 4C");
	RequestHook<ContainerScreenTickHook>(containerScreenTickAddr);
	uintptr_t connReqCreateAddr = MemoryUtil::findSignature("40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 B4 24 ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 49 8B D9 48 89 55");
	RequestHook<ConnectionRequestCreateHook>(connReqCreateAddr);


	{
		// Network Hook
		uintptr_t** PacketSenderVTable = (uintptr_t**)Addresses::PacketSenderVTable;
		std::shared_ptr<Packet> textPacket = MinecraftPackets::createPacket(PacketID::Text);
		std::shared_ptr<Packet> levelEventPacket = MinecraftPackets::createPacket(PacketID::LevelEvent);
		std::shared_ptr<Packet> levelSoundEventPacket = MinecraftPackets::createPacket(PacketID::LevelSoundEvent);
		std::shared_ptr<Packet> actorEventPacket = MinecraftPackets::createPacket(PacketID::ActorEvent);
		std::shared_ptr<Packet> removeActorPacket = MinecraftPackets::createPacket(PacketID::RemoveActor);
		std::shared_ptr<Packet> setScorePacket = MinecraftPackets::createPacket(PacketID::SetScore);
		RequestHook<SendPacketHook>(PacketSenderVTable, 1);
		RequestHook<TextPacketHook>(textPacket->packetHandler->vTable, 1);
		RequestHook<LevelEventPacketHook>(levelEventPacket->packetHandler->vTable, 1);
		RequestHook<ActorEventPacketHook>(actorEventPacket->packetHandler->vTable, 1);
		RequestHook<RemoveActorPacketHook>(removeActorPacket->packetHandler->vTable, 1);
		RequestHook<SetScorePacketHook>(setScorePacket->packetHandler->vTable, 1);
		RequestHook<LevelSoundEventPacketHook>(levelSoundEventPacket->packetHandler->vTable, 1);
	}

	{
		// Block Hook
		uintptr_t** BlockSourceVTable = (uintptr_t**)Addresses::BlockSourceVTable;
		RequestHook<GetBlockHook>(BlockSourceVTable, 2);
	}

	{
		// Entity Hook
		uintptr_t** PlayerVTable = (uintptr_t**)Addresses::PlayerVTable;
		RequestHook<ActorLerpMotionHook>(PlayerVTable, 24);
		RequestHook<ActorNormalTickHook>(PlayerVTable, 26);
		RequestHook<ActorIsOnFireHook>(PlayerVTable, 51);
		RequestHook<ActorSwingHook>(PlayerVTable, 117);
		RequestHook<ActorSetRotHook>("4C 8B DC 49 89 73 10 49 89 7B 20 55 49 8D 6B A1 48 81 EC ? ? ? ? 48 8B");
		RequestHook<MobSwingDurationHook>("48 89 5C 24 ? 57 48 83 EC 20 48 8B 15 ? ? ? ? 48 8B F9 33 DB");
		RequestHook<PlayerSlowDownHook>("40 53 48 83 EC 30 48 8B 05 ? ? ? ? 48 33 C4 48 89 44 24 ? 48 8B DA 4D 85 C0 74 5D");
		RequestHook<ActorShouldRenderHook>(PlayerVTable, 66);
	}

	{
		// GameMode Hook
		uintptr_t** GameModeVTable = (uintptr_t**)Addresses::GameModeVTable;
		RequestHook<StartDestroyBlockHook>(GameModeVTable, 1);
		RequestHook<StopDestroyBlockHook>(GameModeVTable, 4);
		RequestHook<GetPickRangeHook>(GameModeVTable, 10);
		RequestHook<BuildBlockHook>(GameModeVTable, 6);
		RequestHook<AttackHook>(GameModeVTable, 14);
	}

	{
		// Level Hook
		// Search for "Client%d camera ticking systems" -> xref first function -> What access to a1
		uintptr_t** LevelVTable = (uintptr_t**)Addresses::LevelVTable;
		RequestHook<LevelTickHook>(LevelVTable, 99);
		RequestHook<GetTimeOfDayHook>(MemoryUtil::findSignature("44 8B C2 B8 ? ? ? ? F7 EA"));
	}

	if (kiero::init(kiero::RenderType::D3D12) == kiero::Status::Success) {
		uintptr_t** methodsTable = (uintptr_t**)kiero::getMethodsTable();
		RequestHook<PresentHook>(methodsTable, 140);
		RequestHook<ResizeBuffersHook>(methodsTable, 145);
	}
	else if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {
		uintptr_t** methodsTable = (uintptr_t**)kiero::getMethodsTable();
		RequestHook<PresentHook>(methodsTable, 8);
		RequestHook<ResizeBuffersHook>(methodsTable, 13);
	}
	else {
		return;
	}

	if (!getHook<ClientInstanceUpdateHook>()->enableHook()) {
		return;
	}

	while (g_Data.clientInstance == nullptr) {
		Sleep(250);
	}

	for (auto& funcHook : hooksCache) {
		funcHook->enableHook();
	}

	// We don't this anymore so we can delete it
	for (auto& funcHook : hooksCache) {
		delete funcHook;
	}
	hooksCache.clear();
}

void HookManager::shutdown() {
	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);
	MH_Uninitialize();
	kiero::shutdown();

	for (auto& funcHook : hooksCache) {
		delete funcHook;
	}
	hooksCache.clear();
}