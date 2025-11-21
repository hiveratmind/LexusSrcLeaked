#pragma once
#include "../../ModuleBase/Module.h"

class ClickGUI : public Module {
private:
	struct ClickWindow {
		std::string name;
		Category category;
		Vec2<float> pos;
		bool extended = true;

		// Animation
		float selectedAnim = 0.f;

		std::vector<Module*> moduleList;

		ClickWindow(std::string windowName, Category c);
	};
	Module* currentModule;
	UIColor secondaryColor = (40, 40, 45, 255);
	UIColor mainColor = (115, 135, 255, 255);
	UIColor textColor = (255, 255, 255, 255);

	std::unordered_map<ClickWindow*, float> categoryScrollMap;
	std::vector<ClickWindow*> windowList;
	float openAnim = 0.0f;
	bool initialized = false;

	Vec2<float> mousePos = Vec2<float>(0.f, 0.f);
	Vec2<float> mouseDelta = Vec2<float>(0.f, 0.f);

	bool isLeftClickDown = false;
	bool isHoldingLeftClick = false;
	bool isRightClickDown = false;
	bool isHoldingRightClick = false;

	ClickWindow* draggingWindowPtr = nullptr;

	KeybindSetting* capturingKbSettingPtr = nullptr;
	SliderSettingBase* draggingSliderSettingPtr = nullptr;

	void updateSelectedAnimRect(Vec4<float>& rect, float& anim);
private:
	float blurStrength = 5.f;
	bool showDescription = true;
public:
	ClickGUI();
	~ClickGUI();

	void onDisable() override;
	void onEnable() override;
	bool isVisible() override;
	void OnKeyUpdateHook(int key, bool isDown) override;
	void onMouseUpdate(Vec2<float> mousePos, char mouseButton, char isDown);	// Don't need override

	void InitClickGUI();
	void Render();

	void OnLoadConfig(void* conf) override;
	void OnSaveConfig(void* conf) override;
};