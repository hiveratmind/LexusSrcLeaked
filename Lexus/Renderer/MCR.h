#pragma once
#include "../SDK/Render/BaseActorRenderContext.h"
#include "../SDK/Render/MinecraftUIRenderContext.h"
#include "../SDK/Render/ScreenContext.h"
#include "../SDK/Render/Tessellator.h"
#include "../SDK/Render/MeshHelpers.h"

#include "../Utils/Maths.h"
#include "../Utils/ColorUtil.h"

namespace RenderUtils {

	struct Render3dData {
		Vec3<float> pos;
		UIColor color;

		bool ignoreDepth = false;
		float lineWidth = 1.0f;

		Render3dData(const Vec3<float>& pos, const UIColor& col)
			: pos(pos), color(col) {
		}

		Render3dData(const Vec3<float>& pos, const UIColor& col, bool ignoreDepth, float width)
			: pos(pos), color(col), ignoreDepth(ignoreDepth), lineWidth(width) {
		}
	};

	struct BoxRotation {
		Vec3<float> axis;
		float angle;
		BoxRotation(const Vec3<float>& _axis, float _angle) {
			axis = _axis;
			angle = _angle;
		}
	};
	void drawRing3d(const Vec3<float>& worldPos, float innerRadius, float outerRadius,
		const UIColor& color, const UIColor& lineColor,
		float scale, const Vec3<float>& rotationAngles,
		int segments);
	void DrawLine2D(const Vec2<float>& from, const Vec2<float>& to, float thickness, const UIColor& color);
	extern BaseActorRenderContext baseActorRenderCtx;
	extern MinecraftUIRenderContext* renderCtx;
	extern ScreenContext* screenCtx;
	void drawNameTags(Actor* ent, float textSize, bool drawHealth, bool useUnicodeFont);
	extern ShaderColor* currentColor;
	extern Tessellator* tessellator;
	extern mce::MaterialPtr* uiMaterial;
	extern mce::MaterialPtr* blendMaterial;
	extern Font* mcFont;
	extern Vec3<float> origin;
	extern float deltaTime;
	extern std::vector<Render3dData> quad3dRenderList;
	extern std::vector<Render3dData> line3dRenderList;

	void onRenderScreen(MinecraftUIRenderContext* ctx);
	void onRenderWorld(LevelRenderer* levelRenderer, ScreenContext* screenContext);
	void SetColor(const mce::Color& color);
	void SetColor(const UIColor& color);
	void RenderText(const Vec2<float>& textPos, const std::string& textStr, const UIColor& color, float textSize = 1.f);
	float GetTextWidth(const std::string& textStr, float textSize = 1.f);
	float GetTextHeight(float textSize = 1.f);
	void DrawQuad(const Vec2<float>& p1, const Vec2<float>& p2, const Vec2<float>& p3, const Vec2<float>& p4, const UIColor& color);
	void DrawRect(const Vec4<float>& rect, const UIColor& color, float thickness = 1.0f);
	void FillRect(const Vec4<float>& rect, const UIColor& color);
	void RenderItem(ItemStack* item, Vec2<float> pos, float opacity, float scale);
	void RenderItemDurability(ItemStack* item, Vec2<float> itemPos, float scale, float opacity, int mode);
	void Draw3dBox(const AABB& aabb, const UIColor& color, const UIColor& lineColor, float scale = 1.0f, BoxRotation rotation = BoxRotation(Vec3<float>(1.f, 1.f, 1.f), 0.f));
	void DrawBox(const Vec3<float>& lower, const Vec3<float>& upper, float lineWidth, bool fill, int mode, bool enableLerp, float lerpDuration, float elapsedLerpTime);
	bool WorldToScreen(const Vec3<float>& pos, Vec2<float>& out);
	void DrawLine(const Vec2<float>& start, const Vec2<float>& end, float thickness, const UIColor& color);
	void DrawLine3D(const Vec3<float>& start, const Vec3<float>& end, const UIColor& color);
}