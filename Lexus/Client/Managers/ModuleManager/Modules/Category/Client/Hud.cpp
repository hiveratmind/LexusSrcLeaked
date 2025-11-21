#include "HUD.h"
#include "../../../ModuleManager.h"

HUD::HUD() : Module(Category::Client, "HUD", "Displays arraylist, coordinates, and player list") {
    addSetting(new SliderSetting<float>("Size Modifier", "Scale of text", &sizeModifier, 1.f, 0.5f, 3.f));
    addSetting(new ColorSetting("Color", "Main color for text", &color, UIColor(255, 255, 255, 255), true));
    addSetting(new BoolSetting("Femboy", "Use gay colors instead of main color", &rainbow, false));
    addSetting(new SliderSetting<float>("Background Opacity", "Opacity of background rectangles", &backgroundOpacity, 0.5f, 0.f, 1.f));
    addSetting(new BoolSetting("ArrayList", "Show enabled modules list", &showArrayList, true));
    addSetting(new BoolSetting("Position", "Show player coordinates", &showPosition, true));
    addSetting(new BoolSetting("Show Ping", "Show ping", &showPing, true));
    addSetting(new BoolSetting("Show Server IP", "Show server IP", &showServerIP, true));
}

static float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

void HUD::InterfaceUtilsHook() {
    auto player = g_Data.getLocalPlayer();
    if (!player || !player->stateVector) return;

    auto screenSize = g_Data.getClientInstance()->getguiData()->windowSizeReal;

    const float screenPadding = 9.f;
    const float textPadding = 3.f;
    float baseTextSize = sizeModifier;
    float drawTextSize = baseTextSize;
    float lineThickness = std::max(1.f, baseTextSize * 2.0f);
    constexpr float lerpSpeed = 0.07f;
    constexpr float fadeSpeed = 0.05f;
    float bgAlpha = backgroundOpacity * 255.f;

    if (showArrayList || !moduleStates.empty()) {
        auto enabledModules = ModuleManager::getEnabledModules();
        enabledModules.erase(std::remove_if(enabledModules.begin(), enabledModules.end(),
            [](Module* m) { return !m->isVisible(); }), enabledModules.end());

        std::sort(enabledModules.begin(), enabledModules.end(), [=](Module* a, Module* b) {
            auto getTotalWidth = [&](Module* m) {
                std::string mode = m->getModeText();
                if (mode.empty() || mode == "NULL" || mode == "null") mode = "";
                float wName = InterfaceUtils::GetTextWidth(m->getModuleName(), drawTextSize);
                float wMode = mode.empty() ? 0.f : InterfaceUtils::GetTextWidth(mode, drawTextSize);
                float space = InterfaceUtils::GetTextWidth(" ", drawTextSize);
                return wName + (mode.empty() ? 0.f : wMode + space);
                };
            return getTotalWidth(a) > getTotalWidth(b);
            });

        std::unordered_map<Module*, ModuleState> newStates;
        std::unordered_map<Module*, bool> stillVisible;

        float yPos = screenPadding;
        int total = (int)enabledModules.size();
        int index = 0;
        float deltaTime = InterfaceUtils::deltaTime;

        for (auto mod : enabledModules) {
            std::string name = mod->getModuleName();
            std::string modeText = mod->getModeText();
            if (modeText.empty() || modeText == "NULL" || modeText == "null") modeText = "";

            float nameWidth = InterfaceUtils::GetTextWidth(name, drawTextSize);
            float modeWidth = modeText.empty() ? 0.f : InterfaceUtils::GetTextWidth(modeText, drawTextSize);
            float spaceWidth = modeText.empty() ? 0.f : InterfaceUtils::GetTextWidth(" ", drawTextSize);
            float rectWidth = nameWidth + modeWidth + spaceWidth + textPadding * 2;
            float rectHeight = InterfaceUtils::GetTextHeight("X", drawTextSize) + textPadding * 2;

            ModuleState& state = moduleStates[mod];
            if (state.alpha < 255.f) state.alpha = std::min(255.f, state.alpha + deltaTime * 510.f);
            state.targetY = yPos;
            state.currentY = InterfaceUtils::Lerp(state.currentY, state.targetY, deltaTime * 20.f);
            state.alpha = InterfaceUtils::Lerp(state.alpha, 255.f, deltaTime * 20.f);
            state.width = InterfaceUtils::Lerp(state.width, rectWidth, deltaTime * 20.f);

            UIColor mainColor = rainbow ? ColorUtil::astolfoRainbow(index * 4, total, color.a) : color;
            mainColor.a = (int)state.alpha;
            UIColor grayColor = UIColor(128, 128, 128, (int)state.alpha);
            UIColor bgColor = UIColor(0, 0, 0, (int)(backgroundOpacity * state.alpha));

            Vec4<float> bgRect = {
                screenSize.x - screenPadding - state.width,
                state.currentY,
                screenSize.x - screenPadding,
                state.currentY + rectHeight
            };

           // InterfaceUtils::PushAxisAlignedClip(bgRect, false);

            InterfaceUtils::addBlur(bgRect, 8.f, false, InterfaceUtils::CornerRoundType::Left, 8.f);
            InterfaceUtils::FillRect(bgRect, bgColor, 8.f, InterfaceUtils::CornerRoundType::Left);

            Vec4<float> sideBar = {
                bgRect.z,
                bgRect.y + 0.05f * rectHeight,
                bgRect.z + lineThickness,
                bgRect.w - 0.05f * rectHeight
            };
            InterfaceUtils::FillRect(sideBar, mainColor, 4.f, InterfaceUtils::CornerRoundType::Right);

         //  InterfaceUtils::PopAxisAlignedClip();

            InterfaceUtils::RenderText({ bgRect.x + textPadding, bgRect.y + textPadding }, name, mainColor, drawTextSize);
            if (!modeText.empty())
                InterfaceUtils::RenderText({ bgRect.x + textPadding + nameWidth + spaceWidth, bgRect.y + textPadding }, modeText, grayColor, drawTextSize);

            yPos += rectHeight;
            ++index;
            newStates[mod] = state;
            stillVisible[mod] = true;
        }

        for (auto& [mod, state] : moduleStates) {
            if (stillVisible.find(mod) != stillVisible.end()) continue;

            state.alpha = InterfaceUtils::Lerp(state.alpha, 0.f, deltaTime * 20.f);
            state.currentY = InterfaceUtils::Lerp(state.currentY, state.targetY, deltaTime * 20.f);
            state.width = InterfaceUtils::Lerp(state.width, 0.f, deltaTime * 20.f);

            if (state.alpha <= 0.1f) continue;

            UIColor mainColor = rainbow ? ColorUtil::astolfoRainbow(0, 1, color.a) : color;
            mainColor.a = (int)state.alpha;
            UIColor grayColor = UIColor(128, 128, 128, (int)state.alpha);
            UIColor bgColor = UIColor(0, 0, 0, (int)(backgroundOpacity * state.alpha));

            float rectHeight = InterfaceUtils::GetTextHeight("X", drawTextSize) + textPadding * 2;
            Vec4<float> bgRect = {
                screenSize.x - screenPadding - state.width,
                state.currentY,
                screenSize.x - screenPadding,
                state.currentY + rectHeight
            };

            InterfaceUtils::addBlur(bgRect, 8.f, false, InterfaceUtils::CornerRoundType::Left, 8.f);
            InterfaceUtils::FillRect(bgRect, bgColor, 8.f, InterfaceUtils::CornerRoundType::Left);

            Vec4<float> sideBar = {
                bgRect.z,
                bgRect.y + 0.05f * rectHeight,
                bgRect.z + lineThickness,
                bgRect.w - 0.05f * rectHeight
            };
            InterfaceUtils::FillRect(sideBar, mainColor, 4.f, InterfaceUtils::CornerRoundType::Right);

            newStates[mod] = state;
        }

        moduleStates = std::move(newStates);
    }

    float infoYOffset = screenSize.y - screenPadding;

    if (showPosition) {
        Vec3<float> pos = player->getPos();
        char coords[64];
        snprintf(coords, sizeof(coords), "%d %d %d", (int)pos.x, (int)pos.y, (int)pos.z);

        float rectWidth = InterfaceUtils::GetTextWidth(coords, drawTextSize) + textPadding * 2;
        float rectHeight = InterfaceUtils::GetTextHeight("X", drawTextSize) + textPadding * 2;
        infoYOffset -= rectHeight;

        UIColor drawColor = rainbow ? ColorUtil::astolfoRainbow(0, 1, color.a) : color;
        drawColor.a = color.a;

        Vec4<float> bgRect = {
            screenPadding,
            infoYOffset,
            screenPadding + rectWidth,
            infoYOffset + rectHeight
        };

        InterfaceUtils::addBlur(bgRect, 8.f, false, InterfaceUtils::CornerRoundType::Right, 8.f);
        InterfaceUtils::FillRect(bgRect, UIColor(0, 0, 0, (int)bgAlpha), 8.f, InterfaceUtils::CornerRoundType::Right);
        InterfaceUtils::FillRect({ bgRect.x - lineThickness, bgRect.y, bgRect.x, bgRect.w }, drawColor, 8.f, InterfaceUtils::CornerRoundType::Left);
        InterfaceUtils::RenderText({ bgRect.x + textPadding, bgRect.y + textPadding }, coords, drawColor, drawTextSize);
    }

    if (showPing || showServerIP) {
        auto player = g_Data.getLocalPlayer();
        if (player) {
            RakPeer* peer = player->level->getPacketSender()->networkSystem->RemoteConnectorComposite->RakNetConnector->getpeer();

            if (peer != nullptr) {
                float infoYOffsetLocal = infoYOffset;

                if (showPing) {
                    int ping = RakPeerUtil::GetCurrentPing(peer);
                    std::string pingText = ping >= 0 ? "Ping: " + std::to_string(ping) + "ms" : "Ping: N/A";

                    float rectWidth = InterfaceUtils::GetTextWidth(pingText, drawTextSize) + textPadding * 2;
                    float rectHeight = InterfaceUtils::GetTextHeight("X", drawTextSize) + textPadding * 2;
                    infoYOffsetLocal -= rectHeight;

                    UIColor drawColor = rainbow ? ColorUtil::astolfoRainbow(0, 1, color.a) : color;
                    drawColor.a = color.a;

                    Vec4<float> bgRect = {
                        screenPadding,
                        infoYOffsetLocal,
                        screenPadding + rectWidth,
                        infoYOffsetLocal + rectHeight
                    };
                    InterfaceUtils::addBlur(bgRect, 8.f, false, InterfaceUtils::CornerRoundType::Right, 8.f);
                    InterfaceUtils::FillRect(bgRect, UIColor(0, 0, 0, (int)bgAlpha), 8.f, InterfaceUtils::CornerRoundType::Right);
                    InterfaceUtils::FillRect({ bgRect.x - lineThickness, bgRect.y, bgRect.x, bgRect.w }, drawColor, 8.f, InterfaceUtils::CornerRoundType::Left);
                    InterfaceUtils::RenderText({ bgRect.x + textPadding, bgRect.y + textPadding }, pingText, drawColor, drawTextSize);
                }

                if (showServerIP) {
                    std::string ipText = "Server: N/A";

                    networkSystem* netSys = player->level->getPacketSender()->networkSystem;
                    if (netSys) {
                        SocialGameConnectionInfo* connInfo = netSys->getConnectionInfo();
                        if (connInfo) {
                            ipText = "Server: " + connInfo->serverIp;
                        }
                    }

                    float rectWidth = InterfaceUtils::GetTextWidth(ipText, drawTextSize) + textPadding * 2;
                    float rectHeight = InterfaceUtils::GetTextHeight("X", drawTextSize) + textPadding * 2;
                    infoYOffsetLocal -= rectHeight;

                    UIColor drawColor = rainbow ? ColorUtil::astolfoRainbow(0, 1, color.a) : color;
                    drawColor.a = color.a;

                    Vec4<float> bgRect = {
                        screenPadding,
                        infoYOffsetLocal,
                        screenPadding + rectWidth,
                        infoYOffsetLocal + rectHeight
                    };

                    InterfaceUtils::addBlur(bgRect, 8.f, false, InterfaceUtils::CornerRoundType::Right, 8.f);
                    InterfaceUtils::FillRect(bgRect, UIColor(0, 0, 0, (int)bgAlpha), 8.f, InterfaceUtils::CornerRoundType::Right);
                    InterfaceUtils::FillRect({ bgRect.x - lineThickness, bgRect.y, bgRect.x, bgRect.w }, drawColor, 8.f, InterfaceUtils::CornerRoundType::Left);
                    InterfaceUtils::RenderText({ bgRect.x + textPadding, bgRect.y + textPadding }, ipText, drawColor, drawTextSize);
                }

                infoYOffset = infoYOffsetLocal;
            }
        }
    }

    std::string watermark = "Lexus";
    float watermarkWidth = InterfaceUtils::GetTextWidth(watermark, drawTextSize) + textPadding * 2;
    float watermarkHeight = InterfaceUtils::GetTextHeight("X", drawTextSize) + textPadding * 2;
    UIColor watermarkColor = rainbow ? ColorUtil::astolfoRainbow(0, 1, color.a) : color;
    watermarkColor.a = color.a;

    Vec4<float> watermarkRect = {
        screenPadding,
        screenPadding,
        screenPadding + watermarkWidth,
        screenPadding + watermarkHeight
    };

    InterfaceUtils::addBlur(watermarkRect, 8.f, false, InterfaceUtils::CornerRoundType::Right, 8.f);
    InterfaceUtils::FillRect(watermarkRect, UIColor(0, 0, 0, (int)bgAlpha), 8.f, InterfaceUtils::CornerRoundType::Right);
    InterfaceUtils::FillRect({ watermarkRect.x - lineThickness, watermarkRect.y, watermarkRect.x, watermarkRect.w }, watermarkColor, 8.f, InterfaceUtils::CornerRoundType::Left);
    InterfaceUtils::RenderText({ watermarkRect.x + textPadding, watermarkRect.y + textPadding }, watermark, watermarkColor, drawTextSize);
}
