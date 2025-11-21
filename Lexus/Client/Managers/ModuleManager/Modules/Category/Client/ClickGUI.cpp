#include "ClickGUI.h"
#include "../../../ModuleManager.h"
#include "../../../../../../Libs/json.hpp"

std::unordered_map<Module*, float> moduleScrollMap;
float categoryScrollOffset = 0.f;
UIColor windowsideborder = (64, 64, 64, 255);
UIColor windowbgcolor = (32, 32, 32, 100);
std::string searchQuery = "";
bool isSearching = false;

ClickGUI::ClickGUI() : Module(Category::Client, "ClickGUI", "Display all modules", VK_INSERT) {
    addSetting(new SliderSetting<float>("Blur", "Background blur intensity", &blurStrength, 4.f, 0.f, 20.f));
    addSetting(new BoolSetting("Description", "Show Description", &showDescription, true));
    addSetting(new ColorSetting("MainColor", "Main color", &mainColor, UIColor(44, 134, 148, 195)));
    addSetting(new ColorSetting("SecondaryColor", "Secondary color", &secondaryColor, UIColor(9, 14, 13, 255)));
    addSetting(new ColorSetting("Window Side Line", "Secondary color", &windowbgcolor, UIColor(44, 134, 148, 195)));
    addSetting(new ColorSetting("Window Side", "Secondary color", &windowsideborder, UIColor(0, 14, 16, 255)));
    addSetting(new ColorSetting("TextColor", "Text color", &textColor, UIColor(255, 255, 255, 255)));
}

ClickGUI::~ClickGUI() {
    for (auto& window : windowList) {
        delete window;
    }
    windowList.clear();
}

ClickGUI::ClickWindow::ClickWindow(std::string windowName, Category c) {
    this->name = windowName;
    this->category = c;
    this->extended = true;

    for (auto& mod : ModuleManager::moduleList) {
        if (mod->getCategory() == c) {
            this->moduleList.push_back(mod);
        }
    }

    std::sort(this->moduleList.begin(), this->moduleList.end(), [](Module* lhs, Module* rhs) {
        return lhs->getModuleName() < rhs->getModuleName();
        });
}

void ClickGUI::onDisable() {
    g_Data.clientInstance->grabMouse();
    isLeftClickDown = false;
    isRightClickDown = false;
    isHoldingLeftClick = false;
    isHoldingRightClick = false;
    draggingWindowPtr = nullptr;
    capturingKbSettingPtr = nullptr;
    draggingSliderSettingPtr = nullptr;
    openAnim = 0.0f;
    currentModule = nullptr;
    isSearching = false;
    searchQuery = "";
    categoryScrollOffset = 0.f;
}

void ClickGUI::onEnable() {
    g_Data.clientInstance->releasebMouse();
    openAnim = 0.0f;
    currentModule = nullptr;
    isSearching = false;
    searchQuery = "";
    categoryScrollOffset = 0.f;
}

bool ClickGUI::isVisible() {
    return false;
}

void ClickGUI::OnKeyUpdateHook(int key, bool isDown) {
    if (!isEnabled()) {
        if (key == getKeybind() && isDown) {
            setEnabled(true);
        }
    }
    else {
        if (isDown) {
            if (key < 192) {
                if (capturingKbSettingPtr != nullptr) {
                    if (key != VK_ESCAPE)
                        *capturingKbSettingPtr->value = key;
                    capturingKbSettingPtr = nullptr;
                    return;
                }
                else if (isSearching && key == VK_BACK) {
                    if (!searchQuery.empty())
                        searchQuery.pop_back();
                }
                else if (isSearching && key != VK_ESCAPE && key != VK_RETURN && key != VK_TAB) {
                    searchQuery += (char)key;
                }
            }
            if (key == VK_ESCAPE) {
                if (isSearching) {
                    isSearching = false;
                    searchQuery = "";
                }
                else {
                    setEnabled(false);
                }
            }
            else if (key == VK_RETURN && isSearching) {
                isSearching = false;
                if (!searchQuery.empty()) {
                    for (auto& window : windowList) {
                        for (auto& mod : window->moduleList) {
                            if (mod->getModuleName().find(searchQuery) != std::string::npos) {
                                currentModule = mod;
                                break;
                            }
                        }
                        if (currentModule) break;
                    }
                }
                searchQuery = "";
            }
            else if (key == getKeybind()) {
                setEnabled(false);
            }
        }
    }
}

void ClickGUI::onMouseUpdate(Vec2<float> mousePosA, char mouseButton, char isDown) {
    switch (mouseButton) {
    case 0:
        mousePos = mousePosA;
        break;
    case 1:
        isLeftClickDown = isDown;
        isHoldingLeftClick = isDown;
        break;
    case 2:
        isRightClickDown = isDown;
        isHoldingRightClick = isDown;
        break;
    case 4:
        if (isDown) {
            float moveVec = (isDown < 0) ? -15.f : 15.f;
            Vec2<float> screenSize = g_Data.clientInstance->guiData->windowSizeReal;
            float windowWidth = 1000.f;
            float windowHeight = 600.f;
            Vec2<float> windowPos = Vec2<float>((screenSize.x - windowWidth) / 2.f, (screenSize.y - windowHeight) / 2.f);
            float tabWidth = 150.f;

            if (mousePos.x >= windowPos.x && mousePos.x <= windowPos.x + tabWidth &&
                mousePos.y >= windowPos.y && mousePos.y <= windowPos.y + windowHeight) {
                categoryScrollOffset += moveVec;
                float maxScroll = std::max(0.f, (float)(windowList.size() * (35.f) - windowHeight + 100.f));
                categoryScrollOffset = std::clamp(categoryScrollOffset, 0.f, maxScroll);
            }
            else if (currentModule != nullptr) {
                moduleScrollMap[currentModule] += moveVec;
            }
        }
        break;
    }

    if (capturingKbSettingPtr != nullptr) {
        if (isRightClickDown) {
            *capturingKbSettingPtr->value = 0;
            capturingKbSettingPtr = nullptr;
            isRightClickDown = false;
        }
    }

    if (draggingSliderSettingPtr != nullptr) {
        if (!isHoldingLeftClick)
            draggingSliderSettingPtr = nullptr;
    }
}

void ClickGUI::InitClickGUI() {
    setEnabled(false);
    windowList.push_back(new ClickWindow("Search", Category::Unused));
    windowList.push_back(new ClickWindow("Combat", Category::Combat));
    windowList.push_back(new ClickWindow("Movement", Category::Movement));
    windowList.push_back(new ClickWindow("Render", Category::Render));
    windowList.push_back(new ClickWindow("Player", Category::Player));
    windowList.push_back(new ClickWindow("World", Category::World));
    windowList.push_back(new ClickWindow("Misc", Category::Misc));
    windowList.push_back(new ClickWindow("Client", Category::Client));
    initialized = true;
}

void ClickGUI::Render() {
    if (!initialized)
        return;

    if (g_Data.canUseMoveKeys())
        g_Data.clientInstance->releasebMouse();

    static Vec2<float> oldMousePos = mousePos;
    mouseDelta = mousePos.sub(oldMousePos);
    oldMousePos = mousePos;

    Vec2<float> screenSize = g_Data.clientInstance->guiData->windowSizeReal;
    float deltaTime = InterfaceUtils::deltaTime;

    // Smooth animation
    openAnim += deltaTime * 3.5f;
    if (openAnim > 1.f)
        openAnim = 1.f;

    // Modern background with subtle gradient
    if (blurStrength > 0.1f)
        InterfaceUtils::addBlur(Vec4<float>(0.f, 0.f, screenSize.x, screenSize.y), blurStrength * openAnim * 0.7f);

    // Semi-transparent dark overlay with gradient
    InterfaceUtils::FillGradientRect(
        Vec4<float>(0.f, 0.f, screenSize.x, screenSize.y),
        UIColor(10, 10, 20, (int)(180 * openAnim)),
        UIColor(20, 20, 30, (int)(150 * openAnim)),
        0.f,
        InterfaceUtils::CornerRoundType::None
    );

    // Main window dimensions and position
    float windowWidth = 1100.f;
    float windowHeight = 650.f;
    Vec2<float> windowPos = Vec2<float>((screenSize.x - windowWidth) / 2.f, (screenSize.y - windowHeight) / 2.f);

    // Modern window with subtle shadow effect
    InterfaceUtils::FillRect(
        Vec4<float>(windowPos.x, windowPos.y, windowPos.x + windowWidth, windowPos.y + windowHeight),
        UIColor(28, 28, 38, 240),
        12.f,
        InterfaceUtils::CornerRoundType::Full
    );

    // Add subtle inner glow
    InterfaceUtils::DrawRect(
        Vec4<float>(windowPos.x, windowPos.y, windowPos.x + windowWidth, windowPos.y + windowHeight),
        UIColor(80, 80, 100, 80),
        1.5f,
        12.f,
        InterfaceUtils::CornerRoundType::Full
    );

    // Sidebar with modern styling
    float tabWidth = 180.f;
    float contentWidth = windowWidth - tabWidth;
    float contentHeight = windowHeight;

    // Sidebar background with subtle gradient
    InterfaceUtils::FillGradientRect(
        Vec4<float>(windowPos.x, windowPos.y, windowPos.x + tabWidth, windowPos.y + windowHeight),
        UIColor(24, 24, 34, 240),
        UIColor(20, 20, 30, 240),
        12.f,
        InterfaceUtils::CornerRoundType::Left
    );

    // Sidebar separator line
    InterfaceUtils::FillRect(
        Vec4<float>(windowPos.x + tabWidth - 1, windowPos.y + 15, windowPos.x + tabWidth + 1, windowPos.y + windowHeight - 15),
        UIColor(60, 60, 80, 120),
        0.f,
        InterfaceUtils::CornerRoundType::None
    );

    // Modern watermark with version
    float watermarkTextSize = 1.1f;
    std::string watermarkText = "LEXUS | v1.1";
    Vec2<float> watermarkPos(windowPos.x + 20.f, windowPos.y + 20.f);
    InterfaceUtils::RenderText(
        watermarkPos,
        watermarkText,
        UIColor(180, 180, 220, 220),
        watermarkTextSize
    );

    // Add a subtle divider under watermark
    InterfaceUtils::FillRect(
        Vec4<float>(windowPos.x + 20, windowPos.y + 45, windowPos.x + tabWidth - 20, windowPos.y + 47),
        UIColor(70, 70, 90, 120),
        0.f,
        InterfaceUtils::CornerRoundType::None
    );

    // Modern tab system
    float textSize = 1.0f;
    float textHeight = InterfaceUtils::GetTextHeight("", textSize);
    float tabHeight = textHeight * 2.0f;
    float yOffset = windowPos.y + 60.f - categoryScrollOffset;

    static int selectedTab = 0;
    std::string descriptionText = "NULL";

    Vec4<float> categoryClipRect(windowPos.x, windowPos.y + 60.f, windowPos.x + tabWidth, windowPos.y + windowHeight - 80.f);
    InterfaceUtils::PushAxisAlignedClip(categoryClipRect, false);

    for (int i = 0; i < windowList.size(); i++) {
        auto& window = windowList[i];
        Vec4<float> tabRect(windowPos.x + 15.f, yOffset, windowPos.x + tabWidth - 15.f, yOffset + tabHeight);

        if (tabRect.contains(mousePos)) {
            if (isLeftClickDown) {
                selectedTab = i;
                currentModule = nullptr;
                isLeftClickDown = false;
                if (i == 0) isSearching = true;
                else isSearching = false;
            }
            descriptionText = window->name + " Tab";
        }

        // Modern tab styling
        UIColor tabColor = (i == selectedTab) ? mainColor : UIColor(40, 40, 60, 180);
        InterfaceUtils::FillRect(tabRect, tabColor, 8.f, InterfaceUtils::CornerRoundType::Full);

        // Selected tab indicator
        if (i == selectedTab) {
            InterfaceUtils::FillRect(
                Vec4<float>(tabRect.x, tabRect.y, tabRect.x + 4, tabRect.w),
                UIColor(255, 255, 255, 180),
                0.f,
                InterfaceUtils::CornerRoundType::None
            );
        }

        // Tab hover effect
        if (tabRect.contains(mousePos)) {
            InterfaceUtils::DrawRect(
                tabRect,
                UIColor(255, 255, 255, 80),
                1.2f,
                8.f,
                InterfaceUtils::CornerRoundType::Full
            );
        }

        Vec2<float> textPos(
            tabRect.x + 15.f,
            tabRect.y + (tabRect.w - tabRect.y - textHeight) / 2.f
        );

        InterfaceUtils::RenderText(textPos, window->name, textColor, textSize);
        yOffset += tabHeight + 8.f;
    }

    InterfaceUtils::PopAxisAlignedClip();

    // Content area
    if (selectedTab >= 0 && selectedTab < windowList.size()) {
        auto& window = windowList[selectedTab];
        float moduleSpace = 8.f;
        float contentX = windowPos.x + tabWidth + 15.f;
        float contentY = windowPos.y + 15.f;
        float contentMaxWidth = contentWidth - 30.f;
        float contentMaxHeight = contentHeight - 30.f;

        // Search functionality
        if (isSearching) {
            // Modern search bar
            Vec4<float> searchRect(contentX, contentY, contentX + contentMaxWidth, contentY + textHeight * 2.0f);
            InterfaceUtils::FillRect(searchRect, UIColor(40, 40, 60, 200), 8.f, InterfaceUtils::CornerRoundType::Full);

            // Search icon
            InterfaceUtils::RenderText(
                Vec2<float>(searchRect.x + 15.f, searchRect.y + (searchRect.w - searchRect.y - textHeight) / 2.f),
                "",
                UIColor(150, 150, 180),
                textSize
            );

            std::string displayText = searchQuery.empty() ? "Search modules..." : searchQuery;
            UIColor displayColor = searchQuery.empty() ? UIColor(120, 120, 150) : textColor;

            InterfaceUtils::RenderText(
                Vec2<float>(searchRect.x + 40.f, searchRect.y + (searchRect.w - searchRect.y - textHeight) / 2.f),
                displayText,
                displayColor,
                textSize
            );

            // Search results
            std::vector<Module*> searchResults;
            if (!searchQuery.empty()) {
                for (auto& win : windowList) {
                    for (auto& mod : win->moduleList) {
                        std::string modName = mod->getModuleName();
                        std::transform(modName.begin(), modName.end(), modName.begin(), ::tolower);
                        std::string query = searchQuery;
                        std::transform(query.begin(), query.end(), query.begin(), ::tolower);
                        if (modName.find(query) != std::string::npos) {
                            searchResults.push_back(mod);
                        }
                    }
                }
            }

            float yOffsetContent = contentY + textHeight * 2.0f + 15.f;
            float moduleWidth = contentMaxWidth;
            float moduleHeight = textHeight * 2.0f;

            Vec4<float> clipRect(contentX, contentY + textHeight * 2.0f + 15.f, contentX + contentMaxWidth, contentY + contentMaxHeight);
            InterfaceUtils::PushAxisAlignedClip(clipRect, false);

            float& scrollOffset = moduleScrollMap[currentModule];
            yOffsetContent += scrollOffset;

            for (auto& mod : searchResults) {
                Vec4<float> modRect(contentX, yOffsetContent, contentX + moduleWidth, yOffsetContent + moduleHeight);

                // Modern toggle switch
                float toggleHeight = moduleHeight * 0.6f;
                float toggleWidth = toggleHeight * 1.8f;
                float toggleRadius = toggleHeight / 2.f;
                float knobRadius = toggleRadius - 1.5f;

                Vec4<float> toggleRect(
                    modRect.z - toggleWidth - 15.f,
                    modRect.y + (moduleHeight - toggleHeight) / 2.f,
                    modRect.z - 15.f,
                    modRect.y + (moduleHeight + toggleHeight) / 2.f
                );

                if (modRect.contains(mousePos)) {
                    descriptionText = mod->getDescription();
                    if (isLeftClickDown && !toggleRect.contains(mousePos)) {
                        currentModule = mod;
                        isLeftClickDown = false;
                        isSearching = false;
                    }
                }

                if (toggleRect.contains(mousePos) && isLeftClickDown) {
                    mod->setEnabled(!mod->isEnabled());
                    isLeftClickDown = false;
                }

                // Modern module card
                InterfaceUtils::FillRect(modRect, UIColor(35, 35, 50, 200), 8.f, InterfaceUtils::CornerRoundType::Full);

                // Hover effect
                if (modRect.contains(mousePos)) {
                    InterfaceUtils::DrawRect(
                        modRect,
                        UIColor(80, 80, 120, 120),
                        1.2f,
                        8.f,
                        InterfaceUtils::CornerRoundType::Full
                    );
                }

                // Module name with icon
                InterfaceUtils::RenderText(
                    Vec2<float>(modRect.x + 15.f, modRect.y + (modRect.w - modRect.y - textHeight) / 2.f),
                    mod->getModuleName(),
                    textColor,
                    textSize * 1.1f
                );

                // Modern toggle animation
                UIColor offColor = UIColor(50, 50, 70, 255);
                float targetAnim = mod->isEnabled() ? 1.f : 0.f;
                mod->toggleAnim += (targetAnim - mod->toggleAnim) * std::clamp(deltaTime * 12.f, 0.f, 1.f);
                float anim = mod->toggleAnim;

                UIColor toggleBg = ColorUtil::lerp(offColor, mainColor, anim);
                InterfaceUtils::FillRect(toggleRect, toggleBg, toggleRadius, InterfaceUtils::CornerRoundType::Full);

                // Toggle knob with shadow effect
                float knobX = toggleRect.x + knobRadius + (toggleWidth - knobRadius * 2.f) * anim;
                Vec2<float> knobCenter(knobX, toggleRect.y + toggleHeight / 2.f);
                InterfaceUtils::fillCircle(knobCenter, UIColor(255, 255, 255), knobRadius);
                InterfaceUtils::drawCircle(knobCenter, UIColor(0, 0, 0, 60), knobRadius, 1.f);

                yOffsetContent += moduleHeight + 10.f;
            }

            InterfaceUtils::PopAxisAlignedClip();
        }
        else if (currentModule == nullptr) {
            // Category view with modern grid layout
            float moduleWidth = (contentMaxWidth - 10.f) / 2.f;
            float moduleHeight = textHeight * 3.0f;
            float columnGap = 15.f;

            Vec4<float> clipRect(contentX, contentY, contentX + contentMaxWidth, contentY + contentMaxHeight);
            InterfaceUtils::PushAxisAlignedClip(clipRect, false);

            float& scrollOffset = categoryScrollMap[window];
            float yOffsetContent = contentY + scrollOffset;

            int column = 0;
            float yOffsetLeft = yOffsetContent;
            float yOffsetRight = yOffsetContent;

            for (size_t i = 0; i < window->moduleList.size(); ++i) {
                auto& mod = window->moduleList[i];
                float xPos = column == 0 ? contentX : contentX + moduleWidth + columnGap;
                float yPos = column == 0 ? yOffsetLeft : yOffsetRight;

                Vec4<float> modRect(xPos, yPos, xPos + moduleWidth, yPos + moduleHeight);

                // Modern toggle switch
                float toggleHeight = moduleHeight * 0.5f;
                float toggleWidth = toggleHeight * 1.8f;
                float toggleRadius = toggleHeight / 2.f;
                float knobRadius = toggleRadius - 1.5f;

                Vec4<float> toggleRect(
                    modRect.z - toggleWidth - 15.f,
                    modRect.y + (moduleHeight - toggleHeight) / 2.f,
                    modRect.z - 15.f,
                    modRect.y + (moduleHeight + toggleHeight) / 2.f
                );

                if (modRect.contains(mousePos)) {
                    descriptionText = mod->getDescription();
                    if (isLeftClickDown && !toggleRect.contains(mousePos)) {
                        currentModule = mod;
                        isLeftClickDown = false;
                    }
                }

                if (toggleRect.contains(mousePos) && isLeftClickDown) {
                    mod->setEnabled(!mod->isEnabled());
                    isLeftClickDown = false;
                }

                // Modern module card with subtle shadow
                InterfaceUtils::FillRect(modRect, UIColor(35, 35, 50, 200), 8.f, InterfaceUtils::CornerRoundType::Full);

                // Hover effect
                if (modRect.contains(mousePos)) {
                    InterfaceUtils::DrawRect(
                        modRect,
                        UIColor(80, 80, 120, 120),
                        1.5f,
                        8.f,
                        InterfaceUtils::CornerRoundType::Full
                    );
                }

                // Module name with larger text
                float nameHeight = InterfaceUtils::GetTextHeight("X", 1.3f);
                InterfaceUtils::RenderText(
                    Vec2<float>(modRect.x + 15.f, modRect.y + (modRect.w - modRect.y - nameHeight) / 2.f),
                    mod->getModuleName(),
                    textColor,
                    textSize * 1.3f
                );

                // Modern toggle animation
                UIColor offColor = UIColor(50, 50, 70, 255);
                float targetAnim = mod->isEnabled() ? 1.f : 0.f;
                mod->toggleAnim += (targetAnim - mod->toggleAnim) * std::clamp(deltaTime * 12.f, 0.f, 1.f);
                float anim = mod->toggleAnim;

                UIColor toggleBg = ColorUtil::lerp(offColor, mainColor, anim);
                InterfaceUtils::FillRect(toggleRect, toggleBg, toggleRadius, InterfaceUtils::CornerRoundType::Full);

                // Toggle knob with shadow
                float knobX = toggleRect.x + knobRadius + (toggleWidth - knobRadius * 2.f) * anim;
                Vec2<float> knobCenter(knobX, toggleRect.y + toggleHeight / 2.f);
                InterfaceUtils::fillCircle(knobCenter, UIColor(255, 255, 255), knobRadius);
                InterfaceUtils::drawCircle(knobCenter, UIColor(0, 0, 0, 60), knobRadius, 1.f);

                if (column == 0)
                    yOffsetLeft += moduleHeight + 12.f;
                else
                    yOffsetRight += moduleHeight + 12.f;

                column = 1 - column;
            }

            InterfaceUtils::PopAxisAlignedClip();
        }
        else {
            // Module settings view
            float headerHeight = textHeight * 2.0f;
            Vec4<float> clipRect(contentX, contentY + headerHeight + 5.f, contentX + contentMaxWidth, contentY + contentMaxHeight);

            // Modern module header
            Vec4<float> headerRect(contentX, contentY, contentX + contentMaxWidth, contentY + headerHeight);
            InterfaceUtils::FillRect(headerRect, UIColor(40, 40, 60, 220), 8.f, InterfaceUtils::CornerRoundType::TopOnly);

            // Header accent
            InterfaceUtils::FillRect(
                Vec4<float>(headerRect.x, headerRect.y, headerRect.x + 4, headerRect.w),
                mainColor,
                0.f,
                InterfaceUtils::CornerRoundType::None
            );

            // Modern toggle switch in header
            float toggleHeight = headerHeight * 0.5f;
            float toggleWidth = toggleHeight * 1.8f;
            float toggleRadius = toggleHeight / 2.f;
            float knobRadius = toggleRadius - 1.5f;

            Vec4<float> toggleRect(
                headerRect.z - toggleWidth - 15.f,
                headerRect.y + (headerHeight - toggleHeight) / 2.f,
                headerRect.z - 15.f,
                headerRect.y + (headerHeight + toggleHeight) / 2.f
            );

            if (toggleRect.contains(mousePos) && isLeftClickDown) {
                currentModule->setEnabled(!currentModule->isEnabled());
                isLeftClickDown = false;
            }

            float targetAnim = currentModule->isEnabled() ? 1.f : 0.f;
            currentModule->toggleAnim += (targetAnim - currentModule->toggleAnim) * std::clamp(deltaTime * 12.f, 0.f, 1.f);
            float anim = currentModule->toggleAnim;

            UIColor offColor = UIColor(50, 50, 70, 255);
            UIColor toggleBg = ColorUtil::lerp(offColor, mainColor, anim);
            InterfaceUtils::FillRect(toggleRect, toggleBg, toggleRadius, InterfaceUtils::CornerRoundType::Full);

            // Toggle knob with shadow
            float knobX = toggleRect.x + knobRadius + (toggleWidth - knobRadius * 2.f) * anim;
            Vec2<float> knobCenter(knobX, toggleRect.y + toggleHeight / 2.f);
            InterfaceUtils::fillCircle(knobCenter, UIColor(255, 255, 255), knobRadius);
            InterfaceUtils::drawCircle(knobCenter, UIColor(0, 0, 0, 60), knobRadius, 1.f);

            // Back button functionality
            bool shouldClearModule = headerRect.contains(mousePos) && isRightClickDown;
            if (shouldClearModule) {
                currentModule = nullptr;
                isRightClickDown = false;
                return;
            }

            // Module name in header
            if (currentModule != nullptr) {
                InterfaceUtils::RenderText(
                    Vec2<float>(headerRect.x + 15.f, headerRect.y + (headerRect.w - headerRect.y - textHeight) / 2.f),
                    currentModule->getModuleName(),
                    textColor,
                    textSize * 1.2f
                );
            }

            InterfaceUtils::PushAxisAlignedClip(clipRect, false);

            float& scrollOffset = moduleScrollMap[currentModule];
            float yOffsetContent = contentY + headerHeight + 15.f + scrollOffset;
            float settingSpace = 8.f;

            for (auto& setting : currentModule->getSettingList()) {
                int currentPage = (currentModule->modulePagePtr != nullptr) ? *currentModule->modulePagePtr : -1;

                if (setting->type != SettingType::PAGE_S &&
                    setting->name != "Visible" &&
                    setting->name != "Keybind" &&
                    setting->name != "Toggle" &&
                    (currentModule->modulePagePtr != nullptr && setting->settingPage != currentPage))
                    continue;

                Vec4<float> settingRect(contentX, yOffsetContent, contentX + contentMaxWidth, yOffsetContent + textHeight * 1.2f);

                if (settingRect.w > contentY + contentMaxHeight) break;

                if (settingRect.contains(mousePos)) {
                    descriptionText = setting->description;
                }

                switch (setting->type) {
                case SettingType::COLOR_S: {
                    auto* colorSetting = static_cast<ColorSetting*>(setting);

                    if (settingRect.contains(mousePos) && isRightClickDown) {
                        colorSetting->extended = !colorSetting->extended;
                        isRightClickDown = false;
                    }

                    // Modern color preview
                    float radius = textHeight / 2.5f;
                    Vec2<float> center(settingRect.z - radius - 15.f, settingRect.y + (settingRect.w - settingRect.y) / 2.f);

                    InterfaceUtils::RenderText(
                        Vec2<float>(settingRect.x + 15.f, settingRect.y + (settingRect.w - settingRect.y - textHeight) / 2.f),
                        setting->name + ":",
                        textColor,
                        textSize
                    );

                    // Color circle with border
                    InterfaceUtils::fillCircle(center, *colorSetting->colorPtr, radius);
                    InterfaceUtils::drawCircle(center, UIColor(80, 80, 100), radius, 1.5f);

                    yOffsetContent += textHeight * 1.2f;

                    if (colorSetting->extended) {
                        yOffsetContent += settingSpace;
                        float startY = yOffsetContent;

                        for (auto& slider : colorSetting->colorSliders) {
                            Vec4<float> rect(settingRect.x, yOffsetContent, settingRect.z, yOffsetContent + textHeight * 1.0f);
                            Vec2<float> labelPos(rect.x + 15.f, rect.y + (rect.w - rect.y - textHeight) / 2.f);

                            if (rect.contains(mousePos) && isLeftClickDown) {
                                draggingSliderSettingPtr = slider;
                                isLeftClickDown = false;
                            }

                            uint8_t& val = *slider->valuePtr;
                            float valPerc = (val - slider->minValue) / (float)(slider->maxValue - slider->minValue);
                            valPerc = std::clamp(valPerc, 0.f, 1.f);

                            if (draggingSliderSettingPtr == slider) {
                                float perc = (mousePos.x - rect.x) / (rect.z - rect.x);
                                perc = std::clamp(perc, 0.f, 1.f);
                                val = (uint8_t)(slider->minValue + (slider->maxValue - slider->minValue) * perc);
                            }

                            Vec4<float> filled(rect.x, rect.y, rect.x + (rect.z - rect.x) * valPerc, rect.w);
                            char valStr[8]; sprintf_s(valStr, "%i", val);
                            std::string valTxt(valStr);
                            Vec2<float> valPos(rect.z - InterfaceUtils::GetTextWidth(valTxt, textSize) - 15.f, labelPos.y);

                            // Modern slider track
                            InterfaceUtils::FillRect(
                                Vec4<float>(rect.x + 15.f, rect.y + textHeight + 5.f, rect.z - 15.f, rect.y + textHeight + 9.f),
                                UIColor(60, 60, 80),
                                2.f,
                                InterfaceUtils::CornerRoundType::Full
                            );

                            // Filled portion
                            InterfaceUtils::FillRect(
                                Vec4<float>(rect.x + 15.f, rect.y + textHeight + 5.f, rect.x + 15.f + (rect.z - rect.x - 30.f) * valPerc, rect.y + textHeight + 9.f),
                                mainColor,
                                2.f,
                                InterfaceUtils::CornerRoundType::Full
                            );

                            // Slider knob
                            Vec2<float> knobPos(
                                rect.x + 15.f + (rect.z - rect.x - 30.f) * valPerc,
                                rect.y + textHeight + 7.f
                            );
                            InterfaceUtils::fillCircle(knobPos, UIColor(255, 255, 255), 6.f);
                            InterfaceUtils::drawCircle(knobPos, UIColor(0, 0, 0, 60), 6.f, 1.f);

                            InterfaceUtils::RenderText(labelPos, slider->name + ":", textColor, textSize);
                            InterfaceUtils::RenderText(valPos, valTxt, textColor, textSize);

                            yOffsetContent += textHeight * 1.0f + 15.f;
                        }

                        yOffsetContent -= settingSpace;
                    }
                    break;
                }
                case SettingType::BOOL_S: {
                    auto* boolSetting = static_cast<BoolSetting*>(setting);
                    bool& value = *boolSetting->value;

                    static std::unordered_map<BoolSetting*, float> toggleAnim;
                    float& anim = toggleAnim[boolSetting];
                    anim += ((value ? 1.f : 0.f) - anim) * 0.2f;
                    if (std::abs(anim - (value ? 1.f : 0.f)) < 0.01f) anim = (value ? 1.f : 0.f);

                    if (settingRect.contains(mousePos) && isLeftClickDown) {
                        value = !value;
                        isLeftClickDown = false;
                    }

                    InterfaceUtils::RenderText(
                        Vec2<float>(settingRect.x + 15.f, settingRect.y + (settingRect.w - settingRect.y - textHeight) / 2.f),
                        setting->name,
                        textColor,
                        textSize
                    );

                    // Modern toggle switch
                    const float height = textHeight * 0.9f;
                    const float width = height * 1.8f;
                    const float y = settingRect.y + (settingRect.w - settingRect.y - height) / 2.f;
                    const float x = settingRect.z - width - 15.f;
                    Vec4<float> toggleRect(x, y, x + width, y + height);

                    UIColor offColor = UIColor(50, 50, 70, 255);
                    UIColor toggleBg = ColorUtil::lerp(offColor, mainColor, anim);
                    InterfaceUtils::FillRect(toggleRect, toggleBg, height / 2.f, InterfaceUtils::CornerRoundType::Full);

                    // Toggle knob with shadow
                    float knobRadius = height / 2.f - 1.5f;
                    float knobX = toggleRect.x + knobRadius + (width - knobRadius * 2.f) * anim;
                    Vec2<float> knobCenter(knobX, y + height / 2.f);
                    InterfaceUtils::fillCircle(knobCenter, UIColor(255, 255, 255), knobRadius);
                    InterfaceUtils::drawCircle(knobCenter, UIColor(0, 0, 0, 60), knobRadius, 1.f);

                    yOffsetContent += textHeight * 1.2f;
                    break;
                }
                case SettingType::KEYBIND_S: {
                    auto* keybindSetting = static_cast<KeybindSetting*>(setting);
                    int& value = *keybindSetting->value;

                    if (settingRect.contains(mousePos) && isLeftClickDown) {
                        capturingKbSettingPtr = (capturingKbSettingPtr == keybindSetting ? nullptr : keybindSetting);
                        isLeftClickDown = false;
                    }

                    std::string displayText = (setting == capturingKbSettingPtr) ? "Press a key..." : (value ? KeyNames[value] : "None");

                    // Modern keybind button
                    float btnWidth = InterfaceUtils::GetTextWidth(displayText, textSize) + 20.f;
                    Vec4<float> keybindBtnRect(
                        settingRect.z - btnWidth - 15.f,
                        settingRect.y + (settingRect.w - settingRect.y - textHeight * 1.2f) / 2.f,
                        settingRect.z - 15.f,
                        settingRect.y + (settingRect.w - settingRect.y + textHeight * 1.2f) / 2.f
                    );

                    InterfaceUtils::FillRect(
                        keybindBtnRect,
                        UIColor(50, 50, 70, 180),
                        6.f,
                        InterfaceUtils::CornerRoundType::Full
                    );

                    // Hover effect
                    if (settingRect.contains(mousePos)) {
                        InterfaceUtils::DrawRect(
                            keybindBtnRect,
                            UIColor(80, 80, 120, 120),
                            1.2f,
                            6.f,
                            InterfaceUtils::CornerRoundType::Full
                        );
                    }

                    Vec2<float> textPos(
                        keybindBtnRect.x + (keybindBtnRect.z - keybindBtnRect.x - InterfaceUtils::GetTextWidth(displayText, textSize)) / 2.f,
                        settingRect.y + (settingRect.w - settingRect.y - textHeight) / 2.f
                    );

                    InterfaceUtils::RenderText(
                        Vec2<float>(settingRect.x + 15.f, settingRect.y + (settingRect.w - settingRect.y - textHeight) / 2.f),
                        setting->name + ":",
                        textColor,
                        textSize
                    );
                    InterfaceUtils::RenderText(textPos, displayText, textColor, textSize);

                    yOffsetContent += textHeight * 1.2f;
                    break;
                }
                case SettingType::ENUM_S: {
                    auto* enumSetting = static_cast<EnumSetting*>(setting);
                    int& value = *enumSetting->value;

                    if (settingRect.contains(mousePos)) {
                        if (isLeftClickDown) {
                            value = (value + 1) % enumSetting->enumList.size();
                            isLeftClickDown = false;
                        }
                        else if (isRightClickDown) {
                            value = (value - 1 + enumSetting->enumList.size()) % enumSetting->enumList.size();
                            isRightClickDown = false;
                        }
                    }

                    std::string displayText = enumSetting->enumList[value];

                    // Modern enum selector
                    float btnWidth = InterfaceUtils::GetTextWidth(displayText, textSize) + 30.f;
                    Vec4<float> enumBtnRect(
                        settingRect.z - btnWidth - 15.f,
                        settingRect.y + (settingRect.w - settingRect.y - textHeight * 1.2f) / 2.f,
                        settingRect.z - 15.f,
                        settingRect.y + (settingRect.w - settingRect.y + textHeight * 1.2f) / 2.f
                    );

                    InterfaceUtils::FillRect(
                        enumBtnRect,
                        UIColor(50, 50, 70, 180),
                        6.f,
                        InterfaceUtils::CornerRoundType::Full
                    );

                    // Hover effect
                    if (settingRect.contains(mousePos)) {
                        InterfaceUtils::DrawRect(
                            enumBtnRect,
                            UIColor(80, 80, 120, 120),
                            1.2f,
                            6.f,
                            InterfaceUtils::CornerRoundType::Full
                        );
                    }

                    // Arrows
                    float arrowSize = 6.f;
                    Vec2<float> leftArrowPos(
                        enumBtnRect.x + 8.f,
                        enumBtnRect.y + (enumBtnRect.w - enumBtnRect.y) / 2.f
                    );
                    Vec2<float> rightArrowPos(
                        enumBtnRect.z - 8.f,
                        enumBtnRect.y + (enumBtnRect.w - enumBtnRect.y) / 2.f
                    );

                    InterfaceUtils::fillTriangle(
                        Vec2<float>(leftArrowPos.x, leftArrowPos.y - arrowSize),
                        Vec2<float>(leftArrowPos.x + arrowSize, leftArrowPos.y),
                        Vec2<float>(leftArrowPos.x, leftArrowPos.y + arrowSize),
                        textColor
                    );

                    InterfaceUtils::fillTriangle(
                        Vec2<float>(rightArrowPos.x, rightArrowPos.y - arrowSize),
                        Vec2<float>(rightArrowPos.x - arrowSize, rightArrowPos.y),
                        Vec2<float>(rightArrowPos.x, rightArrowPos.y + arrowSize),
                        textColor
                    );

                    Vec2<float> textPos(
                        enumBtnRect.x + (enumBtnRect.z - enumBtnRect.x - InterfaceUtils::GetTextWidth(displayText, textSize)) / 2.f,
                        settingRect.y + (settingRect.w - settingRect.y - textHeight) / 2.f
                    );

                    InterfaceUtils::RenderText(
                        Vec2<float>(settingRect.x + 15.f, settingRect.y + (settingRect.w - settingRect.y - textHeight) / 2.f),
                        setting->name + ":",
                        textColor,
                        textSize
                    );
                    InterfaceUtils::RenderText(textPos, displayText, textColor, textSize);

                    yOffsetContent += textHeight * 1.2f;
                    break;
                }
                case SettingType::SLIDER_S: {
                    auto* sliderBase = static_cast<SliderSettingBase*>(setting);

                    static std::unordered_map<SliderSettingBase*, float> animMap;
                    float& anim = animMap[sliderBase];

                    float value = 0.f, minVal = 0.f, maxVal = 1.f;

                    if (sliderBase->valueType == ValueType::INT_T) {
                        auto* s = static_cast<SliderSetting<int>*>(sliderBase);
                        value = static_cast<float>(*s->valuePtr);
                        minVal = static_cast<float>(s->minValue);
                        maxVal = static_cast<float>(s->maxValue);
                    }
                    else {
                        auto* s = static_cast<SliderSetting<float>*>(sliderBase);
                        value = *s->valuePtr;
                        minVal = s->minValue;
                        maxVal = s->maxValue;
                    }

                    float perc = std::clamp((value - minVal) / (maxVal - minVal), 0.f, 1.f);

                    Vec2<float> labelPos(
                        settingRect.x + 15.f,
                        settingRect.y + (settingRect.w - settingRect.y - textHeight) / 2.f
                    );
                    InterfaceUtils::RenderText(labelPos, setting->name + ":", textColor, textSize);

                    char valStr[16];
                    if (sliderBase->valueType == ValueType::INT_T) {
                        sprintf_s(valStr, "%i", static_cast<int>(value));
                    }
                    else {
                        sprintf_s(valStr, "%.2f", value);
                    }
                    std::string valTxt(valStr);

                    Vec2<float> valPos(
                        settingRect.z - InterfaceUtils::GetTextWidth(valTxt, textSize) - 15.f,
                        labelPos.y
                    );
                    InterfaceUtils::RenderText(valPos, valTxt, textColor, textSize);

                    if (settingRect.contains(mousePos) && isLeftClickDown) {
                        draggingSliderSettingPtr = sliderBase;
                        isLeftClickDown = false;
                    }

                    if (draggingSliderSettingPtr == sliderBase) {
                        float newPerc = (mousePos.x - settingRect.x) / (settingRect.z - settingRect.x);
                        newPerc = std::clamp(newPerc, 0.f, 1.f);
                        float newVal = minVal + (maxVal - minVal) * newPerc;

                        if (sliderBase->valueType == ValueType::INT_T)
                            *static_cast<SliderSetting<int>*>(sliderBase)->valuePtr = static_cast<int>(std::round(newVal));
                        else
                            *static_cast<SliderSetting<float>*>(sliderBase)->valuePtr = newVal;

                        perc = newPerc;
                    }

                    anim += (perc - anim) * 0.25f;
                    if (std::abs(anim - perc) < 0.005f) anim = perc;

                    // Modern slider
                    float sliderY = settingRect.y + textHeight + 10.f;
                    float sliderH = 6.f;

                    Vec4<float> track(
                        settingRect.x + 15.f,
                        sliderY,
                        settingRect.z - 15.f,
                        sliderY + sliderH
                    );

                    Vec4<float> fill(
                        track.x,
                        track.y,
                        track.x + (track.z - track.x) * anim,
                        track.w
                    );

                    // Track background
                    InterfaceUtils::FillRect(
                        track,
                        UIColor(60, 60, 80),
                        sliderH / 2.f,
                        InterfaceUtils::CornerRoundType::Full
                    );

                    // Filled portion
                    InterfaceUtils::FillRect(
                        fill,
                        mainColor,
                        sliderH / 2.f,
                        InterfaceUtils::CornerRoundType::Full
                    );

                    // Slider knob
                    Vec2<float> knob(
                        track.x + (track.z - track.x) * anim,
                        sliderY + sliderH / 2.f
                    );
                    InterfaceUtils::fillCircle(knob, UIColor(255, 255, 255), 8.f);
                    InterfaceUtils::drawCircle(knob, UIColor(0, 0, 0, 60), 8.f, 1.f);

                    yOffsetContent += textHeight * 1.2f + 20.f;
                    break;
                }
                }

                yOffsetContent += settingSpace;
            }

            InterfaceUtils::PopAxisAlignedClip();
        }
    }

    /*
    // Modern tooltip
    if (showDescription && descriptionText != "NULL") {
        Vec2<float> mousePadding = Vec2<float>(20.f, 20.f);
        float padding = 10.f;
        float textWidth = InterfaceUtils::GetTextWidth(descriptionText, 0.85f);

        Vec4<float> rectPos = Vec4<float>(
            mousePos.x + mousePadding.x,
            mousePos.y + mousePadding.y,
            mousePos.x + mousePadding.x + textWidth + padding * 2.f,
            mousePos.y + mousePadding.y + InterfaceUtils::GetTextHeight(descriptionText, 0.85f) + padding * 2.f
        );

        // Tooltip background with subtle shadow
        InterfaceUtils::FillRect(
            rectPos,
            UIColor(20, 20, 30, 220),
            6.f,
            InterfaceUtils::CornerRoundType::Full
        );

        // Tooltip border
        InterfaceUtils::DrawRect(
            rectPos,
            UIColor(80, 80, 120, 180),
            1.2f,
            6.f,
            InterfaceUtils::CornerRoundType::Full
        );

        Vec2<float> textPos = Vec2<float>(
            rectPos.x + padding,
            rectPos.y + padding
        );

        InterfaceUtils::RenderText(
            textPos,
            descriptionText,
            UIColor(220, 220, 240),
            0.85f
        );
    }
    */

    isLeftClickDown = false;
    isRightClickDown = false;
}
using json = nlohmann::json;

void ClickGUI::OnLoadConfig(void* confVoid) {
    json* conf = reinterpret_cast<json*>(confVoid);
    std::string modName = this->getModuleName();

    if (conf->contains(modName)) {
        json obj = conf->at(modName);
        if (obj.is_null())
            return;

        if (obj.contains("enabled")) {
            this->setEnabled(obj.at("enabled").get<bool>());
        }

        for (auto& setting : getSettingList()) {
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
                case SettingType::PAGE_S: {
                    PageSetting* pageSetting = static_cast<PageSetting*>(setting);
                    (*pageSetting->valuePtr) = confValue.get<int>();
                    break;
                }
                case SettingType::COLOR_S: {
                    ColorSetting* colorSetting = static_cast<ColorSetting*>(setting);
                    (*colorSetting->colorPtr) = ColorUtil::HexStringToColor(confValue.get<std::string>());
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

        for (auto& window : windowList) {
            std::string windowName = window->name;

            if (obj.contains(windowName)) {
                json confValue = obj.at(windowName);
                if (confValue.is_null())
                    continue;

                if (confValue.contains("isExtended")) {
                    window->extended = confValue["isExtended"].get<bool>();
                }

                if (confValue.contains("pos")) {
                    window->pos.x = confValue["pos"]["x"].get<float>();
                    window->pos.y = confValue["pos"]["y"].get<float>();
                }
            }
        }
    }
}

void ClickGUI::OnSaveConfig(void* confVoid) {
    json* conf = reinterpret_cast<json*>(confVoid);
    std::string modName = this->getModuleName();
    json obj = (*conf)[modName];

    obj["enabled"] = this->isEnabled();

    for (auto& setting : getSettingList()) {
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

    for (auto& window : windowList) {
        obj[window->name]["isExtended"] = window->extended;
        obj[window->name]["pos"]["x"] = window->pos.x;
        obj[window->name]["pos"]["y"] = window->pos.y;
    }

    (*conf)[modName] = obj;
}