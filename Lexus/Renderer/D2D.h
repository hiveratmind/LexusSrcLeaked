#pragma once
#include <d3d11.h>
#include <d3d12.h>
#include <d3d11on12.h>
#include <d2d1_3.h>
#include <dxgi1_4.h>
#include <dwrite_3.h>

#include <string>

#include "../Utils/Maths.h"
#include "../Utils/ColorUtil.h"

namespace InterfaceUtils {
    extern float deltaTime;
    enum class CornerRoundType {
        None,
        BottomOnly,
        TopOnly,
        Full,
        SidesOnly,
        Left,
        Right
    };
    inline float Lerp(float a, float b, float t) {
        return a + (b - a) * std::clamp(t, 0.f, 1.f);
    }
    void NewFrame(IDXGISwapChain3* swapChain, ID3D11Device* d3d11Device, float fxdpi);
    void EndFrame();
    void Render();
    void Clean();
    void Flush();
    void drawVerticalGradient(const Vec4<float>& rect, const UIColor& startColor, const UIColor& endColor);
    Vec2<float> getWindowSize();
    void RenderText(const Vec2<float>& textPos, const std::string& textStr, const UIColor& color, float textSize = 1.f, bool storeTextLayout = true);
    float GetTextWidth(const std::string& textStr, float textSize = 1.f, bool storeTextLayout = true);
    float GetTextHeight(const std::string& textStr, float textSize = 1.f, bool storeTextLayout = true);
    void drawLine(const Vec2<float>& startPos, const Vec2<float>& endPos, const UIColor& color, float width = 1.f);
    void DrawRect(const Vec4<float>& rect, const UIColor& color, float width, float rounding = 0.0f, CornerRoundType roundType = CornerRoundType::None);
    void FillRect(const Vec4<float>& rect, const UIColor& color, float rounding = 0.0f, CornerRoundType roundType = CornerRoundType::None);
    void fillRoundingRectangle(const Vec4<float>& rect, const UIColor& color, float radius);
    void drawBox(const AABB& box, const UIColor& fillColor, const UIColor& outlineColor, float thickness, bool fill, bool outline);
    void drawBoxTest(const Vec3<float>& lower, const Vec3<float>& upper, float lineWidth, const UIColor& fillColor, const UIColor& outlineColor, bool fill, bool outline, float rotationAngle, const Vec3<float>& rotationAxis);
    void drawBoxOutline(const Vec3<float>& lower, const Vec3<float>& upper, float lineWidth, const UIColor& outlineColor, float rotationAngle, const Vec3<float>& rotationAxis);
    void fillGlowRectangle(const Vec4<float>& rect, const UIColor& color, float glowStrength);
    void drawCircle(const Vec2<float>& centerPos, const UIColor& color, float radius, float width = 1.f);
    void fillCircle(const Vec2<float>& centerPos, const UIColor& color, float radius);
    void addBlur(const Vec4<float>& rect, float strength, bool flush = false, CornerRoundType roundType = CornerRoundType::None, float rounding = 0.0f);
    void drawColorSB(const Vec4<float>& rect, float hue);
    void drawColorHueBar(const Vec4<float>& rect);
    void lookAt(float viewMatrix[4][4], const Vec3<float>& eye, const Vec3<float>& center, const Vec3<float>& up);
    void perspective(float projectionMatrix[4][4], float fov, float aspect, float zNear, float zFar);
    void FillGradientRect(const Vec4<float>& rect, const UIColor& color1, const UIColor& color2, float rounding, CornerRoundType roundType);
    void DrawColorWheel(Vec2<float> center, float radius);
    Vec3<float> GetHueFromMouse(Vec2<float> center, float radius, Vec2<float> mouse);
    Vec3<uint8_t> HSVtoRGB(float h, float s, float v);
    void fillTriangle(const Vec2<float>& p1, const Vec2<float>& p2, const Vec2<float>& p3, const UIColor& color);
    void drawTriangle(const Vec2<float>& p1, const Vec2<float>& p2, const Vec2<float>& p3, const UIColor& color, float strokeWidth = 1.0f);
    void PushAxisAlignedClip(const Vec4<float>& rect, bool aliased);
    void PopAxisAlignedClip();
    void InitWICFactory();
    void CleanupWICFactory();
};