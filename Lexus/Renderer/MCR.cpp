#include "MCR.h"
#include "../Client/Client.h"
#include "../SDK/Game.h"

BaseActorRenderContext RenderUtils::baseActorRenderCtx;
MinecraftUIRenderContext* RenderUtils::renderCtx = nullptr;
ScreenContext* RenderUtils::screenCtx = nullptr;
ShaderColor* RenderUtils::currentColor = nullptr;
Tessellator* RenderUtils::tessellator = nullptr;
mce::MaterialPtr* RenderUtils::uiMaterial = nullptr;
mce::MaterialPtr* RenderUtils::blendMaterial = nullptr;
Font* RenderUtils::mcFont = nullptr;
Vec3<float> RenderUtils::origin;
float RenderUtils::deltaTime = 0.016f;
std::vector<RenderUtils::Render3dData> RenderUtils::quad3dRenderList;
std::vector<RenderUtils::Render3dData> RenderUtils::line3dRenderList;

void RenderUtils::onRenderScreen(MinecraftUIRenderContext* ctx) {
	baseActorRenderCtx = BaseActorRenderContext(ctx->screenContext, ctx->clientInstance, ctx->clientInstance->mcGame);
	renderCtx = ctx;
	screenCtx = ctx->screenContext;
	currentColor = ctx->screenContext->currentShaderColor;
	tessellator = ctx->screenContext->tessellator;

	if (uiMaterial == nullptr)
		uiMaterial = mce::MaterialPtr::createMaterial(HashedString("ui_textured_and_glcolor"));

	if (blendMaterial == nullptr)
		blendMaterial = mce::MaterialPtr::createMaterial(HashedString("fullscreen_cube_overlay_blend"));

	mcFont = ctx->clientInstance->mcGame->mcFont;

	ModuleManager::MCRHook(renderCtx);
	RenderUtils::renderCtx->flushText(0.f);
}

void RenderUtils::onRenderWorld(LevelRenderer* levelRenderer, ScreenContext* screenContext) {

	quad3dRenderList.clear();
	line3dRenderList.clear();

	origin = levelRenderer->levelRendererPlayer->realOrigin;
	ModuleManager::LevelRenderHook();

	SetColor(mce::Color(1.f, 1.f, 1.f, 1.f));
	if (!quad3dRenderList.empty()) {
		tessellator->begin(VertextFormat::QUAD);
		for (auto& quad3dRenderData : quad3dRenderList) {
			mce::Color mcColor = quad3dRenderData.color.toMCColor();
			tessellator->color(mcColor.r, mcColor.g, mcColor.b, mcColor.a);
			tessellator->vertex(quad3dRenderData.pos.x, quad3dRenderData.pos.y, quad3dRenderData.pos.z);
		}
		MeshHelpers::renderMeshImmediately(screenCtx, tessellator, blendMaterial);
		quad3dRenderList.clear();
	}

	if (!line3dRenderList.empty()) {
		tessellator->begin(VertextFormat::LINE_LIST);
		for (auto& line3dRenderData : line3dRenderList) {
			mce::Color mcColor = line3dRenderData.color.toMCColor();
			tessellator->color(mcColor.r, mcColor.g, mcColor.b, mcColor.a);
			tessellator->vertex(line3dRenderData.pos.x, line3dRenderData.pos.y, line3dRenderData.pos.z);
		}
		MeshHelpers::renderMeshImmediately(screenCtx, tessellator, blendMaterial);
		line3dRenderList.clear();
	}
}

void RenderUtils::SetColor(const mce::Color& color) {
	currentColor->color = color;
	currentColor->dirty = true;
}

void RenderUtils::SetColor(const UIColor& color) {
	currentColor->color = color.toMCColor();
	currentColor->dirty = true;
}

void RenderUtils::RenderText(const Vec2<float>& textPos, const std::string& textStr, const UIColor& color, float textSize) {
	std::string text = textStr;
	mce::Color mcColor = color.toMCColor();

	RectangleArea rect = {};
	rect._x0 = textPos.x;
	rect._x1 = textPos.x;
	rect._y0 = textPos.y;
	rect._y1 = textPos.y;

	TextMeasureData textMeasureData = {};
	textMeasureData.fontSize = textSize;
	textMeasureData.linePadding = 0.f;
	textMeasureData.renderShadow = true;
	textMeasureData.showColorSymbol = false;
	textMeasureData.hideHyphen = false;

	static CaretMeasureData caretMeasureData = {};
	caretMeasureData.position = -1.f;
	caretMeasureData.shouldRender = false;

	renderCtx->RenderText(mcFont, &rect, &text, &mcColor, mcColor.a, 0, &textMeasureData, &caretMeasureData);
}

float RenderUtils::GetTextWidth(const std::string& textStr, float textSize) {
	std::string text = textStr;
	return renderCtx->getLineLength(mcFont, &text, textSize, false);
}

float RenderUtils::GetTextHeight(float textSize) {
	return (9.f * textSize);
}

void RenderUtils::DrawQuad(const Vec2<float>& p1, const Vec2<float>& p2, const Vec2<float>& p3, const Vec2<float>& p4, const UIColor& color) {
	SetColor(color);
	tessellator->begin(VertextFormat::QUAD, 4);

	tessellator->vertex(p1.x, p1.y, 0.f);
	tessellator->vertex(p2.x, p2.y, 0.f);
	tessellator->vertex(p3.x, p3.y, 0.f);
	tessellator->vertex(p4.x, p4.y, 0.f);

	MeshHelpers::renderMeshImmediately(screenCtx, tessellator, uiMaterial);
}

void RenderUtils::DrawRect(const Vec4<float>& rect, const UIColor& color, float thickness) {
	float left = rect.x;
	float top = rect.y;
	float right = rect.z;
	float bottom = rect.w;

	thickness = std::max(1.0f, thickness);

	// Top border (a thin horizontal rect)
	FillRect(Vec4<float>(left, top, right, top + thickness), color);

	// Bottom border
	FillRect(Vec4<float>(left, bottom - thickness, right, bottom), color);

	// Left border (a thin vertical rect)
	FillRect(Vec4<float>(left, top + thickness, left + thickness, bottom - thickness), color);

	// Right border
	FillRect(Vec4<float>(right - thickness, top + thickness, right, bottom - thickness), color);
}

void RenderUtils::FillRect(const Vec4<float>& rect, const UIColor& color) {
	DrawQuad(Vec2<float>(rect.x, rect.w), Vec2<float>(rect.z, rect.w), Vec2<float>(rect.z, rect.y), Vec2<float>(rect.x, rect.y), color);
}

static __forceinline void drawLine3(const Vec2<float>& start, const Vec2<float>& end, const UIColor& color, float lineWidth) {
	// The lineWidth parameter can be ignored or handled later if your MCR pipeline doesn't support line thickness
	// Convert 2D screen positions into 3D render queue positions at z = 0 or a fixed depth

	// We can push the vertices into the MCR line3dRenderList for rendering later
	RenderUtils::line3dRenderList.push_back(RenderUtils::Render3dData(Vec3<float>{start.x, start.y, 0.f}, color));
	RenderUtils::line3dRenderList.push_back(RenderUtils::Render3dData(Vec3<float>{end.x, end.y, 0.f}, color));
}

void RenderUtils::drawRing3d(const Vec3<float>& worldPos, float innerRadius, float outerRadius,
	const UIColor& color, const UIColor& lineColor,
	float scale, const Vec3<float>& rotationAngles,
	int segments) {

	Vec3<float> center = worldPos.sub(RenderUtils::origin);
	glm::mat4 transform = glm::mat4(1.0f);
	transform = glm::rotate(transform, rotationAngles.x, glm::vec3(1, 0, 0));
	transform = glm::rotate(transform, rotationAngles.y, glm::vec3(0, 1, 0));
	transform = glm::rotate(transform, rotationAngles.z, glm::vec3(0, 0, 1));
	transform = glm::scale(transform, glm::vec3(scale));

	std::vector<Vec3<float>> outerRing, innerRing;
	outerRing.reserve(segments + 1);
	innerRing.reserve(segments + 1);

	for (int i = 0; i <= segments; ++i) {
		float angle = 2.0f * PI * i / segments;
		float cosA = cos(angle);
		float sinA = sin(angle);

		glm::vec4 outer = transform * glm::vec4(outerRadius * cosA, 0, outerRadius * sinA, 1.0f);
		glm::vec4 inner = transform * glm::vec4(innerRadius * cosA, 0, innerRadius * sinA, 1.0f);

		outerRing.emplace_back(center.x + outer.x, center.y + outer.y, center.z + outer.z);
		innerRing.emplace_back(center.x + inner.x, center.y + inner.y, center.z + inner.z);
	}

	if (color.a > 0) {
		for (int i = 0; i < segments; ++i) {
			quad3dRenderList.push_back(Render3dData(innerRing[i], color));
			quad3dRenderList.push_back(Render3dData(outerRing[i + 1], color));
			quad3dRenderList.push_back(Render3dData(outerRing[i], color));

			quad3dRenderList.push_back(Render3dData(innerRing[i], color));
			quad3dRenderList.push_back(Render3dData(innerRing[i + 1], color));
			quad3dRenderList.push_back(Render3dData(outerRing[i + 1], color));
		}
	}

	if (lineColor.a > 0) {
		for (int i = 0; i < segments; ++i) {
			line3dRenderList.push_back(Render3dData(innerRing[i], lineColor));
			line3dRenderList.push_back(Render3dData(innerRing[i + 1], lineColor));

			line3dRenderList.push_back(Render3dData(outerRing[i], lineColor));
			line3dRenderList.push_back(Render3dData(outerRing[i + 1], lineColor));
		}
	}
}


/*void RenderUtils::RenderItem(const Vec2<float>& pos, ItemStack* itemStack, float scale, bool showDurabilityBar) {
	baseActorRenderCtx.itemRenderer->renderGuiItemNew(&baseActorRenderCtx, itemStack, 0, pos.x, pos.y, 1.f, scale, false);

	if (itemStack->item.get()->isGlint(itemStack)) {
		SetColor(mce::Color(1.f, 1.f, 1.f, 1.f));
		baseActorRenderCtx.itemRenderer->renderGuiItemNew(&baseActorRenderCtx, itemStack, 0, pos.x, pos.y, 1.f, scale, true);
	}

	if (showDurabilityBar) {
		Item* item = itemStack->item.get();
		short maxDamage = item->getMaxDamage();
		short damageValue = item->getDamageValue(itemStack->mUserData);
		short durability = maxDamage - damageValue;

		if (maxDamage != 0 && durability != maxDamage) {
			float percent = (float)durability / (float)maxDamage;
			UIColor color = ColorUtil::lerp(UIColor(255, 0, 0), UIColor(0, 255, 0), percent);
			Vec4<float> barRect = Vec4<float>(pos.x + (2.f * scale), pos.y + (13.5f * scale), pos.x + (14.f * scale), pos.y + (15.f * scale));
			Vec4<float> durabilityRect = Vec4<float>(barRect.x, barRect.y, barRect.x + ((barRect.z - barRect.x) * percent), barRect.y + (1.f * scale));

			FillRect(barRect, UIColor(0, 0, 0, 255));
			FillRect(durabilityRect, color);
		}
	}
}*/
void RenderUtils::RenderItem(ItemStack* item, Vec2<float> pos, float opacity, float scale) {
	if (g_Data.getLocalPlayer() == nullptr) return;
	baseActorRenderCtx.itemRenderer->renderGuiItemNew(&baseActorRenderCtx, item, 0, pos.x, pos.y, opacity, scale, false);
	ItemStackBase* itemBase = (ItemStackBase*)item;
	if (item->item->isGlint(itemBase)) {
		RenderUtils::SetColor(mce::Color(1.f, 1.f, 1.f, opacity));
		baseActorRenderCtx.itemRenderer->renderGuiItemNew(&baseActorRenderCtx, item, 0, pos.x, pos.y, opacity, scale, true);
	}
}

void RenderUtils::RenderItemDurability(ItemStack* itemStack, Vec2<float> itemPos, float scale, float opacity, int mode) {
	if (g_Data.getLocalPlayer() == nullptr) return;
	Item* item = itemStack->item.get();
	float maxDamage = item->getMaxDamage();
	float damageValue = item->getDamageValue(itemStack->mUserData);
	float currentDamage = maxDamage - damageValue;
	if (currentDamage == maxDamage) return;
	float percentage = currentDamage / maxDamage * 100;
	float fraction = percentage / 100;
	UIColor durabilityColor = ColorUtil::lerp(UIColor(255, 0, 0), UIColor(0, 255, 0), fraction);
	if (mode == 0 || mode == 2) {
		Vec4<float> barPos = Vec4<float>(itemPos.x + 1.5f * scale, itemPos.y + 14.f * scale, itemPos.x + 15.f * scale, itemPos.y + 16.f * scale);
		Vec4<float> rect = Vec4<float>(barPos.x, barPos.y, barPos.x + ((barPos.z - barPos.x) / 100 * (int)percentage), barPos.w - 1.f * scale);
		RenderUtils::FillRect(barPos, UIColor(0.f, 0.f, 0.f));
		RenderUtils::FillRect(rect, durabilityColor);
	}
	if (mode == 1 || mode == 2) {
		std::string duraText = std::to_string((int)percentage);
		RenderUtils::RenderText(Vec2<float>(itemPos.x + 7.5f, itemPos.y - 6.5f), duraText, durabilityColor, scale);
	}
}

void RenderUtils::DrawLine2D(const Vec2<float>& from, const Vec2<float>& to, float thickness, const UIColor& color) {
	tessellator->begin(VertextFormat::LINE_LIST);
	tessellator->color(color.r, color.g, color.b, color.a);
	tessellator->vertex(from.x, from.y, 0.0f);
	tessellator->vertex(to.x, to.y, 0.0f);
}

void RenderUtils::Draw3dBox(const AABB& aabb, const UIColor& color, const UIColor& lineColor, float scale, BoxRotation rotation) {
	Vec3<float> diff = aabb.upper.sub(aabb.lower);
	Vec3<float> newLower = aabb.lower.sub(RenderUtils::origin);

	Vec3<float> vertices[8] = {
		{newLower.x, newLower.y, newLower.z},
		{newLower.x + diff.x, newLower.y, newLower.z},
		{newLower.x, newLower.y, newLower.z + diff.z},
		{newLower.x + diff.x, newLower.y, newLower.z + diff.z},

		{newLower.x, newLower.y + diff.y, newLower.z},
		{newLower.x + diff.x, newLower.y + diff.y, newLower.z},
		{newLower.x, newLower.y + diff.y, newLower.z + diff.z},
		{newLower.x + diff.x, newLower.y + diff.y, newLower.z + diff.z}
	};

	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(scale), rotation.angle, rotation.axis.toGlm());
	Vec3<float> newLowerReal = newLower.add(Vec3<float>(0.5f, 0.5f, 0.5f));

	for (int i = 0; i < 8; i++) {
		glm::vec4 rotatedVertex = rotationMatrix * glm::vec4(vertices[i].x - newLowerReal.x, vertices[i].y - newLowerReal.y, vertices[i].z - newLowerReal.z, 0.0f);
		vertices[i] = Vec3<float>(rotatedVertex.x + newLowerReal.x, rotatedVertex.y + newLowerReal.y, rotatedVertex.z + newLowerReal.z);
	}

	if (color.a > 0) {
		static uint8_t v[48] = { 5, 7, 6, 4, 4, 6, 7, 5, 1, 3, 2, 0, 0, 2, 3, 1, 4, 5, 1, 0, 0, 1, 5, 4, 6, 7, 3, 2, 2, 3, 7, 6, 4, 6, 2, 0, 0, 2, 6, 4, 5, 7, 3, 1, 1, 3, 7, 5 };
		for (int i = 0; i < 48; i++)
			quad3dRenderList.push_back(Render3dData(vertices[v[i]], color));
	}

	if (lineColor.a > 0) {
		static auto drawLine = [](const Vec3<float>& start, const Vec3<float>& end, const UIColor& _color) {
			line3dRenderList.push_back(Render3dData(start, _color));
			line3dRenderList.push_back(Render3dData(end, _color));
			};

		// Top square
		drawLine(vertices[4], vertices[5], lineColor);
		drawLine(vertices[5], vertices[7], lineColor);
		drawLine(vertices[7], vertices[6], lineColor);
		drawLine(vertices[6], vertices[4], lineColor);

		// Bottom Square
		drawLine(vertices[0], vertices[1], lineColor);
		drawLine(vertices[1], vertices[3], lineColor);
		drawLine(vertices[3], vertices[2], lineColor);
		drawLine(vertices[2], vertices[0], lineColor);

		// Sides
		drawLine(vertices[0], vertices[4], lineColor);
		drawLine(vertices[1], vertices[5], lineColor);
		drawLine(vertices[2], vertices[6], lineColor);
		drawLine(vertices[3], vertices[7], lineColor);
	}
}

bool RenderUtils::WorldToScreen(const Vec3<float>& pos, Vec2<float>& out) {

	ClientHMDState* hmdState = g_Data.clientInstance->getHMDState();

	Vec2<float> screenSize = g_Data.clientInstance->guiData->windowSizeScaled;
	Vec3<float> relativePos = pos.sub(origin);

	glm::mat4 mvpMatrix = hmdState->mLastLevelProjMatrix * hmdState->mLastLevelViewMatrix;
	glm::vec4 clipSpacePosition = mvpMatrix * glm::vec4(relativePos.x, relativePos.y, relativePos.z, 1.0f);

	if (clipSpacePosition.w <= 0.0f)
		return false;

	clipSpacePosition /= clipSpacePosition.w;

	out = Vec2<float>(
		(clipSpacePosition.x + 1.0f) * (0.5f * screenSize.x),
		(1.0f - clipSpacePosition.y) * (0.5f * screenSize.y)
	);

	return true;
}

UIColor getColorFromCode(char code) {
	switch (code) {
	case '0': return UIColor(0, 0, 0);           // BLACK
	case '1': return UIColor(0, 0, 170);         // DARK_BLUE
	case '2': return UIColor(0, 170, 0);         // DARK_GREEN
	case '3': return UIColor(0, 170, 170);       // DARK_AQUA
	case '4': return UIColor(170, 0, 0);         // DARK_RED
	case '5': return UIColor(170, 0, 170);       // DARK_PURPLE
	case '6': return UIColor(255, 170, 0);       // GOLD
	case '7': return UIColor(170, 170, 170);     // GRAY
	case '8': return UIColor(85, 85, 85);        // DARK_GRAY
	case '9': return UIColor(85, 85, 255);       // BLUE
	case 'a': return UIColor(85, 255, 85);       // GREEN
	case 'b': return UIColor(85, 255, 255);      // AQUA
	case 'c': return UIColor(255, 85, 85);       // RED
	case 'd': return UIColor(255, 85, 255);      // LIGHT_PURPLE
	case 'e': return UIColor(255, 255, 85);      // YELLOW
	case 'f': return UIColor(255, 255, 255);     // WHITE
	default:  return UIColor(255, 255, 255);     // Default white if unknown
	}
}
void RenderUtils::drawNameTags(Actor* ent, float textSize, bool drawHealth, bool useUnicodeFont) {
	/*
	if (!ent || !ent->getNameTag()) return;

	Vec2<float> screenPos;
	Vec3<float> nameTagPos = ent->getEyePos().add2(0.f, 0.5f, 0.f);

	if (!RenderUtils::WorldToScreen(nameTagPos, screenPos)) return;

	std::string text = *ent->getNameTag();

	float textWidth = RenderUtils::GetTextWidth(text, textSize);
	float textHeight = RenderUtils::GetTextHeight(textSize);

	float padding = 1.5f * textSize;

	Vec4<float> background = {
		screenPos.x - textWidth / 2.f - padding,
		screenPos.y - padding,
		screenPos.x + textWidth / 2.f + padding,
		screenPos.y + textHeight + padding
	};

	static NameTags* nameTagsMod = ModuleManager::getModule<NameTags>();
	if (!nameTagsMod) return;
	static ClickGUI* ihatemylife = ModuleManager::getModule<ClickGUI>();
	if (!ihatemylife) return;

	UIColor nametagsBackgroundColor(12, 12, 12);
	UIColor nametagsUnderlineColor = ColorUtil::getRainbowColor(3.0f, 1.f, 1.0f, 50);

	int bgAlpha = static_cast<int>(nameTagsMod->opacity * 200);
	if (bgAlpha > 255) bgAlpha = 255;
	if (bgAlpha < 0) bgAlpha = 0;

	UIColor bgColor = nametagsBackgroundColor;
	bgColor.a = bgAlpha;

	RenderUtils::FillRect(background, bgColor);

	if (nameTagsMod->underline) {
		Vec4<float> underline = background;
		underline.y = underline.w - 1.0f * textSize;

		UIColor underlineColor = nametagsUnderlineColor;
		for (size_t i = 0; i + 1 < text.size(); ++i) {
			if (text[i] == '§') {
				underlineColor = getColorFromCode(text[i + 1]);
				break;
			}
		}

		underlineColor.a = static_cast<int>(0.9f * 255);
		RenderUtils::FillRect(underline, underlineColor);
	}
	Vec2<float> textPos(screenPos.x - textWidth / 2.f, screenPos.y);
	RenderUtils::RenderText(textPos, text, UIColor(255, 255, 255), textSize);

	if (ent->getActorTypeComponent() && ent->getActorTypeComponent()->id == 319 && nameTagsMod->displayArmor) {
		auto* player = reinterpret_cast<Player*>(ent);
		float scale = textSize * 0.7f;
		float spacing = 17.f * scale;
		float x = background.x + 1.f * textSize;
		float y = background.y - 18.f * scale;

		for (int i = 3; i >= 0; i--) {
			ItemStack* stack = player->getArmor(i);
			if (stack && stack->item) {
				RenderUtils::RenderItem(stack, Vec2<float>(x, y), 1.f, scale);
				RenderUtils::RenderItemDurability(stack, Vec2<float>(x, y), scale, 1.f, 2);
				x += spacing;
			}
		}

		ItemStack* held = player->getCarriedItem();
		if (held && held->item) {
			float itemX = background.z - (15.f * scale) - 1.5f * textSize;
			RenderUtils::RenderItem(held, Vec2<float>(itemX, y), 1.f, scale);
			RenderUtils::RenderItemDurability(held, Vec2<float>(itemX, y), scale, 1.f, 2);
		}
	}
	*/
}

void RenderUtils::DrawLine(const Vec2<float>& start, const Vec2<float>& end, float thickness, const UIColor& color) {
	SetColor(color);
	tessellator->begin(VertextFormat::LINE_LIST);

	for (float i = -thickness / 2; i <= thickness / 2; i += 0.5f) {
		tessellator->vertex(start.x + i, start.y + i, 0.0f);
		tessellator->vertex(end.x + i, end.y + i, 0.0f);
	}

	MeshHelpers::renderMeshImmediately(screenCtx, tessellator, uiMaterial);
}

void RenderUtils::DrawLine3D(const Vec3<float>& start, const Vec3<float>& end, const UIColor& color) {
	Vec3<float> relStart = start.sub(origin);
	Vec3<float> relEnd = end.sub(origin);

	line3dRenderList.push_back(Render3dData(relStart, color));
	line3dRenderList.push_back(Render3dData(relEnd, color));
}

void RenderUtils::DrawBox(const Vec3<float>& lower, const Vec3<float>& upper, float lineWidth, bool fill, int mode, bool enableLerp, float lerpDuration, float elapsedLerpTime) {
	// Calculate lerp progress
	float lerpProgress = elapsedLerpTime / lerpDuration;

	Vec3<float> lerpedLower;
	Vec3<float> lerpedUpper;

	if (enableLerp && lerpProgress > 0.f && lerpProgress <= 1.f) {
		////lerpedLower = lower.lerp(upper, lerpProgress);
		//lerpedUpper = upper.lerp(lower, lerpProgress);
		lerpedLower = lower;
		lerpedUpper = upper;
	}
	else {
		lerpedLower = lower;
		lerpedUpper = upper;
	}

	// Calculate box dimensions
	Vec3<float> diff = lerpedUpper.sub(lerpedLower);

	// Box vertices
	Vec3<float> vertices[8] = {
		Vec3<float>(lerpedLower.x, lerpedLower.y, lerpedLower.z),
		Vec3<float>(lerpedLower.x + diff.x, lerpedLower.y, lerpedLower.z),
		Vec3<float>(lerpedLower.x, lerpedLower.y + diff.y, lerpedLower.z),
		Vec3<float>(lerpedLower.x + diff.x, lerpedLower.y + diff.y, lerpedLower.z),
		Vec3<float>(lerpedLower.x, lerpedLower.y, lerpedLower.z + diff.z),
		Vec3<float>(lerpedLower.x + diff.x, lerpedLower.y, lerpedLower.z + diff.z),
		Vec3<float>(lerpedLower.x, lerpedLower.y + diff.y, lerpedLower.z + diff.z),
		Vec3<float>(lerpedLower.x + diff.x, lerpedLower.y + diff.y, lerpedLower.z + diff.z)
	};

	// Project vertices to screen space
	std::vector<Vec2<float>> screenCoords;
	for (int i = 0; i < 8; i++) {
		Vec2<float> screenPos;
		if (RenderUtils::WorldToScreen(vertices[i], screenPos)) {
			screenCoords.push_back(screenPos);
		}
	}

	if (fill) {
		// Need all 8 points to fill box faces
		if (screenCoords.size() < 8) return;

		// Indices for each face (same order as original)
		std::vector<std::tuple<int, int, int, int>> faces = {
			{0, 1, 3, 2},  // Bottom
			{4, 5, 7, 6},  // Top
			{0, 1, 5, 4},  // Front
			{2, 3, 7, 6},  // Back
			{1, 3, 7, 5},  // Right
			{0, 2, 6, 4}   // Left
		};

		for (auto& face : faces) {
			RenderUtils::DrawQuad(
				screenCoords[std::get<0>(face)],
				screenCoords[std::get<1>(face)],
				screenCoords[std::get<2>(face)],
				screenCoords[std::get<3>(face)],
				UIColor(255, 255, 255, 80) // Optional fill color with alpha
			);
			// Optionally draw reversed quad for double-sided fill:
			RenderUtils::DrawQuad(
				screenCoords[std::get<3>(face)],
				screenCoords[std::get<2>(face)],
				screenCoords[std::get<1>(face)],
				screenCoords[std::get<0>(face)],
				UIColor(255, 255, 255, 80)
			);
		}
	}

	if (mode == 1 || mode == 2) {
		// Prepare indices and projected coords for line drawing
		// Here we preserve original vertex indices for logic
		std::vector<std::pair<int, Vec2<float>>> indexedScreenCoords;
		for (int i = 0; i < 8; i++) {
			Vec2<float> screenPos;
			if (RenderUtils::WorldToScreen(vertices[i], screenPos)) {
				int index = (mode == 2) ? (int)indexedScreenCoords.size() : i;
				indexedScreenCoords.emplace_back(index, screenPos);
			}
		}

		if (indexedScreenCoords.size() < 2) return;

		if (mode == 1) {
			// Draw lines between all pairs of vertices along the box edges
			for (auto& fromPair : indexedScreenCoords) {
				int fromIdx = fromPair.first;
				Vec3<float> fromOrig = vertices[fromIdx];

				for (auto& toPair : indexedScreenCoords) {
					int toIdx = toPair.first;
					Vec3<float> toOrig = vertices[toIdx];

					bool shouldDraw = false;
					// Draw line if vertices differ by one axis and are aligned on others
					shouldDraw |= (fromOrig.y == toOrig.y && fromOrig.z == toOrig.z && fromOrig.x < toOrig.x);
					shouldDraw |= (fromOrig.x == toOrig.x && fromOrig.z == toOrig.z && fromOrig.y < toOrig.y);
					shouldDraw |= (fromOrig.x == toOrig.x && fromOrig.y == toOrig.y && fromOrig.z < toOrig.z);

					if (shouldDraw)
						RenderUtils::DrawLine(fromPair.second, toPair.second, lineWidth, UIColor(255, 255, 255, 255));
				}
			}
			return;
		}
		else if (mode == 2) {
			// Find start vertex (leftmost screen x)
			auto startIt = std::min_element(
				indexedScreenCoords.begin(),
				indexedScreenCoords.end(),
				[](const auto& a, const auto& b) { return a.second.x < b.second.x; }
			);

			std::vector<int> indices;
			auto current = *startIt;
			indices.push_back(current.first);
			Vec2<float> lastDir(0.f, -1.f);

			do {
				float smallestAngle = 2.f * 3.14159265359f;
				Vec2<float> smallestDir;
				std::pair<int, Vec2<float>> smallestE = current;

				float lastDirAtan2 = atan2(lastDir.y, lastDir.x);

				for (auto& candidate : indexedScreenCoords) {
					if (candidate.first == current.first)
						continue;

					Vec2<float> dir = candidate.second.sub(current.second);
					float angle = atan2(dir.y, dir.x) - lastDirAtan2;
					if (angle > 3.14159265359f)
						angle -= 2.f * 3.14159265359f;
					else if (angle <= -3.14159265359f)
						angle += 2.f * 3.14159265359f;

					if (angle >= 0.f && angle < smallestAngle) {
						smallestAngle = angle;
						smallestDir = dir;
						smallestE = candidate;
					}
				}

				indices.push_back(smallestE.first);
				lastDir = smallestDir;
				current = smallestE;

			} while (current.first != startIt->first && indices.size() < 8);

			// Draw connected outline lines
			bool hasLast = false;
			Vec2<float> lastVertex;

			for (int idx : indices) {
				Vec2<float> curVertex = indexedScreenCoords[idx].second;
				if (!hasLast) {
					hasLast = true;
					lastVertex = curVertex;
					continue;
				}
				RenderUtils::DrawLine(lastVertex, curVertex, lineWidth, UIColor(255, 255, 255, 255));
				lastVertex = curVertex;
			}
		}
	}
}
