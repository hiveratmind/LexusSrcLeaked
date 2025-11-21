#include <Windows.h>
#include "Client/Client.h"
#include "Renderer/D2D.h"
#include "Renderer/MCR.h"
#include "winrt/Windows.UI.Core.h"
#include "winrt/windows.system.h"
#include <winrt/Windows.UI.Notifications.h>
#include <winrt/Windows.Data.Xml.Dom.h>
#include "winrt/windows.applicationmodel.core.h"
#include "winrt/Windows.UI.ViewManagement.h"
#include "winrt/Windows.Foundation.h"

DWORD WINAPI initClient(LPVOID lpParameter) {
    Client::init();
    while (Client::isInitialized()) {
        ModuleManager::OnLocalTick();
        Sleep(10);
    }
    Sleep(55);
    FreeLibraryAndExitThread((HMODULE)lpParameter, 1);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)initClient, hModule, 0, 0);
        break;
    case DLL_PROCESS_DETACH:
        HookManager::shutdown();
        ModuleManager::shutdown();
        CommandManager::shutdown();
        InterfaceUtils::Clean();

        Client::DisplayClientMessage("%sEjected", MCTF::RED);
        break;
    }
    return TRUE;
}