#include "Client.h"
#include <Windows.h>
#include <wincrypt.h>

#include "../Utils/FileUtil.h"
#include "../Utils/NetworkUtil.h"
#include "../Utils/Logger.h"
#include "winrt/Windows.UI.Core.h"
#include "winrt/windows.system.h"
#include <winrt/Windows.UI.Notifications.h>
#include <winrt/Windows.Data.Xml.Dom.h>
#include "winrt/windows.applicationmodel.core.h"
#include "winrt/Windows.UI.ViewManagement.h"
#include "winrt/Windows.Foundation.h"

void AddFont(const std::string& fontPath) {
    std::wstring temp = std::wstring(fontPath.begin(), fontPath.end());
    AddFontResource(temp.c_str());
}

void Client::DisplayClientMessage(const char* fmt, ...) {
    LocalPlayer* localPlayer = g_Data.getLocalPlayer();
    if (localPlayer == nullptr)
        return;

    va_list arg;
    va_start(arg, fmt);
    char message[300];
    vsprintf_s(message, 300, fmt, arg);
    va_end(arg);

    static std::string headerMessage;
    if (headerMessage.empty()) {
        char headerCStr[50];
        sprintf_s(headerCStr, 50, "%sLexus %s ", MCTF::LIGHT_PURPLE, MCTF::WHITE);
        headerMessage = std::string(headerCStr);
    }

    std::string messageStr = headerMessage + std::string(message);
    localPlayer->displayClientMessage(messageStr);
}

void Client::init() {
    std::string ClientPath = FileUtil::getClientPath();
    if (!FileUtil::doesFilePathExist(ClientPath)) {
        FileUtil::createPath(ClientPath);
    }

    Logger::init();
    ConfigManager::init();

    Addresses::init();
    MCTF::initMCTF();

    ModuleManager::init();
    CommandManager::init();
    HookManager::init();

    if (g_Data.clientInstance == nullptr)
        return;

    initialized = true;

    Beep(50, 255);

    if (!ConfigManager::doesConfigExist("default"))
        ConfigManager::createNewConfig("default");

    ConfigManager::loadConfig("default");
}

void Client::shutdown() {
    ConfigManager::saveConfig();
    initialized = false;
}