#pragma once
#include "../../ModuleBase/Module.h"

class ViewClip : public Module {
private:
    void* targetAddress = nullptr;
    uint8_t ogBytes[5]{};
    bool nopedBytes = false;

public:
    ViewClip() : Module(Category::Render, "ViewClip", "Allows the camera to clip through blocks") {
        if (Addresses::CameraClipCheck != 0) {
            targetAddress = reinterpret_cast<void*>(Addresses::CameraClipCheck);
            MemoryUtil::copyBytes(targetAddress, ogBytes, 5);
        }
    }

    void onDisable() override {
        if (targetAddress && nopedBytes) {
            MemoryUtil::patchBytes(targetAddress, ogBytes, 5);
            nopedBytes = false;
        }
    }

    void onEnable() override {
        if (targetAddress && !nopedBytes) {
            MemoryUtil::nopBytes(targetAddress, 5);
            nopedBytes = true;
        }
    }
};