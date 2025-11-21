#pragma once
#include "../../ModuleBase/Module.h"
#include <vector>

class Circle {
public:
    Vec3<float> pos;
    float scale;
    float alpha;

    Circle(Vec3<float> pos, float scale, float alpha)
        : pos(pos), scale(scale), alpha(alpha) {
    }

    void render(float circleScale, float speed, float alphaSpeed, float test);
    bool shouldRemove() const;
};

class JumpCircle : public Module {
public:
    JumpCircle();

    void OnTick() override;
    void LevelRenderHook() override;
    UIColor mainColor = { 255, 255, 255, 255 };
    std::vector<Circle> circles;
    bool prevStatus = false;
    int groundTicks = 0;
    bool affectOthers = false;
    float scale = 1.0f;
    float test = 30.0f;
    float speed = 1.0f;
    float fade = 2.0f;
};
