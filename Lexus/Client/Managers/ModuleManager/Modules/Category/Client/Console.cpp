#include "Console.h"
#include <algorithm>

Console::Console() : Module(Category::Client, "Console", "Debug console for module output") {}

Console* Console::getInstance() {
    static Console instance;
    return &instance;
}

void Console::log(const std::string& text) {
    std::lock_guard<std::mutex> lock(logMutex);
    logLines.emplace_back(std::chrono::steady_clock::now(), text);
    float totalHeight = lineHeight * logLines.size();
    maxScroll = std::max(0.f, totalHeight - windowSize.y + textPadding * 2.f);
}

void Console::clampScroll() {
    scrollOffset = std::clamp(scrollOffset, 0.f, maxScroll);
}

void Console::pruneOldLogs() {
    auto now = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> lock(logMutex);

    std::unordered_set<std::string> seenMessages;

    logLines.erase(
        std::remove_if(logLines.begin(), logLines.end(),
            [&](const auto& entry) {
                bool tooOld = std::chrono::duration_cast<std::chrono::seconds>(now - entry.first).count() > 2;
                bool isDuplicate = !seenMessages.insert(entry.second).second;
                return tooOld || isDuplicate;
            }),
        logLines.end());

    float totalHeight = lineHeight * logLines.size();
    maxScroll = std::max(0.f, totalHeight - windowSize.y + textPadding * 2.f);
}

bool Console::isMouseOverTitleBar(Vec2<float> mousePos) {
    Vec4<float> titleBarRect = {
        windowPos.x, windowPos.y,
        windowPos.x + windowSize.x,
        windowPos.y + 28.f
    };
    return titleBarRect.contains(mousePos);
}

void Console::onMouseUpdate(Vec2<float> mousePos, char mouseButton, char isDown, short mouseWheelDelta) {
    currentMousePos = mousePos;

    switch (mouseButton) {
    case 1:
        isLeftClickDown = isDown;
        isHoldingLeftClick = isDown;
        break;
    }

    if (isMouseOverTitleBar(mousePos)) {
        if (isLeftClickDown && !dragging) {
            dragging = true;
            dragOffset = mousePos.sub(windowPos);
        }
    }

    if (!isHoldingLeftClick) {
        dragging = false;
    }

    if (dragging) {
        windowPos = mousePos.sub(dragOffset);
    }

    Vec4<float> contentRect = {
        windowPos.x + 2.f, windowPos.y + 28.f,
        windowPos.x + windowSize.x - 2.f,
        windowPos.y + windowSize.y - 2.f
    };

    if (contentRect.contains(mousePos) && mouseWheelDelta != 0) {
        scrollOffset -= mouseWheelDelta * 20.f;
        clampScroll();
    }
}

void Console::render() {
    if (!isEnabled()) return;

    pruneOldLogs();

    Vec4<float> consoleRect = {
        windowPos.x, windowPos.y,
        windowPos.x + windowSize.x,
        windowPos.y + windowSize.y
    };

    InterfaceUtils::FillRect(consoleRect, UIColor(0, 0, 0, 160), 6.f, InterfaceUtils::CornerRoundType::Full);

    Vec4<float> titleBarRect = {
        windowPos.x, windowPos.y,
        windowPos.x + windowSize.x,
        windowPos.y + 28.f
    };
    InterfaceUtils::FillRect(titleBarRect, UIColor(30, 30, 30, 200), 6.f, InterfaceUtils::CornerRoundType::TopOnly);
    InterfaceUtils::RenderText({ windowPos.x + 10, windowPos.y + 8 }, "Debug Console", UIColor(255, 255, 255), 1.1f);

    Vec4<float> contentRect = {
        windowPos.x + 2.f, windowPos.y + 28.f,
        windowPos.x + windowSize.x - 2.f,
        windowPos.y + windowSize.y - 2.f
    };
    InterfaceUtils::PushAxisAlignedClip(contentRect, false);

    std::lock_guard<std::mutex> lock(logMutex);
    float yCursor = windowPos.y + 28.f - scrollOffset;

    for (const auto& [timestamp, line] : logLines) {
        if (yCursor + lineHeight < windowPos.y + 28.f) {
            yCursor += lineHeight;
            continue;
        }
        if (yCursor > windowPos.y + windowSize.y)
            break;

        UIColor color(220, 220, 220);
        if (line.find("Warning:") != std::string::npos) {
            color = UIColor(255, 255, 0);
        }
        else if (line.find("Critical:") != std::string::npos) {
            color = UIColor(255, 80, 80);
        }

        InterfaceUtils::RenderText({ windowPos.x + 10.f, yCursor }, line, color, 1.0f);
        yCursor += lineHeight;
    }

    InterfaceUtils::PopAxisAlignedClip();
}