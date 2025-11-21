#pragma once
#include "../../ModuleBase/Module.h"
#include "../../../../../../Utils/Minecraft/Intenvoru.h"

inline void writeBytes(uintptr_t address, std::string bytes, int length) {
    DWORD oldProtect;
    VirtualProtect((LPVOID)address, length, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy((LPVOID)address, bytes.c_str(), length);
    VirtualProtect((LPVOID)address, length, oldProtect, &oldProtect);
}

inline void readBytes(void* address, void* buffer, size_t size) {
    DWORD oldProtect;
    VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(buffer, address, size);
    VirtualProtect(address, size, oldProtect, &oldProtect);
}

class FakeBlock : public Module {
public:
    int swingType = 0;
    int blockAnim = 0;
    int blockTimer = 0;
    float rotate1 = 1.0f;
    float rotate2 = 1.0f;
    float rotate3 = 1.0f;
    float pos1 = 1.0f;
    float pos2 = 1.0f;
    float pos3 = 1.0f;
    bool isBlocking = false;
    bool onlyblockflux = true;
    void* fluxSwingAddr = (void*)(MemoryUtil::findSignature("E8 ? ? ? ? F3 0F ? ? ? ? ? ? F3 0F ? ? ? ? ? ? 48 8B ? F3 0F ? ? 48 8B"));
    char ogBytes[5] = { 0 };
    int blocktmr = 100;

    FakeBlock() : Module(Category::Render, "FakeBlock", "Swing and blocking effects.") {
        addSetting(new EnumSetting("Swing Type", "", { "Flux", "Vanilla" }, &swingType, 0));
        addSetting(new EnumSetting("Block Type", "", { "Flux", "Test", "None" }, &blockAnim, 0));
        addSetting(new BoolSetting("Only Block", "Only set flux swing if we are blocking", &onlyblockflux, true));
        addSetting(new SliderSetting<float>("Rotate1", "", &rotate1, 1.f, 0.f, 2.f));
        addSetting(new SliderSetting<float>("Rotate2", "", &rotate2, 1.f, 0.f, 1.1f));
        addSetting(new SliderSetting<float>("Rotate3", "", &rotate3, 1.f, 0.f, 1.1f));
        addSetting(new SliderSetting<float>("Pos1", "", &pos1, 1.f, 0.f, 4.f));
        addSetting(new SliderSetting<float>("Pos2", "", &pos2, 1.f, 0.f, 4.f));
        addSetting(new SliderSetting<float>("Pos3", "", &pos3, 1.f, 0.f, 4.f));

        addSetting(new SliderSetting<int>("block timer", "", &blocktmr, 100, 10, 200));
    }

    bool isSwordEquipped() {
        int selectedItem = InventoryUtils::getSelectedItemId();
        return selectedItem == 314 || selectedItem == 318 || selectedItem == 313 || selectedItem == 329 || selectedItem == 322 || selectedItem == 622;
    }

    void onEnable() override {
        if (fluxSwingAddr)
            readBytes(fluxSwingAddr, ogBytes, 5);
        isBlocking = false;
    }

    void onDisable() override {
        if (fluxSwingAddr && swingType == 0)
            writeBytes((uintptr_t)fluxSwingAddr, std::string(ogBytes, 5), 5);
        isBlocking = false;
    }

    void onAttack(Actor* actor, bool& cancel) override {
        blockTimer = blocktmr;
    }

    void GLMHandRenderHook(glm::mat4* matrix) override {
        glm::mat4& mat = *matrix;

        bool blockingNow = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) && g_Data.canUseMoveKeys() && isSwordEquipped();
        blockingNow = blockingNow || (blockTimer > 0 && g_Data.canUseMoveKeys() && isSwordEquipped());
        isBlocking = blockingNow;

        if (swingType == 0) {
            if (!onlyblockflux || (onlyblockflux && isBlocking))
                writeBytes((uintptr_t)fluxSwingAddr, "\x90\x90\x90\x90\x90", 5);
            else
                writeBytes((uintptr_t)fluxSwingAddr, std::string(ogBytes, 5), 5);
        }
        else {
            writeBytes((uintptr_t)fluxSwingAddr, std::string(ogBytes, 5), 5);
        }

        if (isBlocking) {
            if (blockAnim == 0) {
                mat = glm::translate<float>(mat, glm::vec3(0.42222223281f * pos1, 0.0f, -0.16666666269302368f * pos1));
                mat = glm::translate<float>(mat, glm::vec3(-0.1f * pos2, 0.15f * pos2, -0.2f * pos2));
                mat = glm::translate<float>(mat, glm::vec3(-0.24f * pos3, 0.25f * pos3, -0.20f * pos3));
                mat = glm::rotate<float>(mat, -1.98f * rotate1, glm::vec3(0.0f, 1.0f, 0.0f));
                mat = glm::rotate<float>(mat, 1.30f * rotate2, glm::vec3(4.0f, 0.0f, 0.0f));
                mat = glm::rotate<float>(mat, 60.0f * rotate3, glm::vec3(0.0f, 1.0f, 0.0f));
            }
            else if (blockAnim == 1) {
                mat = glm::mat4(1.0f);
                mat = glm::translate<float>(mat, glm::vec3(0.42222223281f, 0.0f, -0.16666666269302368f));
                mat = glm::translate<float>(mat, glm::vec3(-0.1f, 0.15f, -0.2f));
                mat = glm::translate<float>(mat, glm::vec3(-0.24f, 0.25f, -0.20f));
                mat = glm::rotate<float>(mat, -1.98f, glm::vec3(0.0f, 1.0f, 0.0f));
                mat = glm::rotate<float>(mat, 1.30f, glm::vec3(4.0f, 0.0f, 0.0f));
                mat = glm::rotate<float>(mat, 60.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            }
        }

        if (blockTimer > 0)
            blockTimer--;
    }
};
