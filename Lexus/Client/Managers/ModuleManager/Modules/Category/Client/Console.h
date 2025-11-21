#pragma once
#include "../../ModuleBase/Module.h"

class Console : public Module {
public:
    Console();

    static Console* getInstance();

    void log(const std::string& text);

    void render();
    void onMouseUpdate(Vec2<float> mousePos, char mouseButton, char isDown, short mouseWheelDelta);
    
private:
    std::vector<std::pair<std::chrono::steady_clock::time_point, std::string>> logLines;
    std::mutex logMutex;

    float scrollOffset = 0.0f;
    float maxScroll = 0.0f;
    const float lineHeight = 14.0f;
    const float textPadding = 6.0f;

    Vec2<float> windowPos = { 50.f, 300.f };
    Vec2<float> windowSize = { 400.f, 250.f };

    bool dragging = false;
    Vec2<float> dragOffset;
    bool isLeftClickDown = false;
    bool isHoldingLeftClick = false;
    Vec2<float> currentMousePos;

    void clampScroll();
    void pruneOldLogs();
    bool isMouseOverTitleBar(Vec2<float> mousePos);
};