#include "NameTags.h"

NameTags::NameTags() : Module(Category::Render, "NameTags", "Better nametags.") {
	addSetting(new SliderSetting<float>("Opacity", "NULL", &opacity, 1.f, 0.f, 1.f));
}

static bool envaledchar(char c) {
	return !(c >= 0 && *reinterpret_cast<unsigned char*>(&c) < 128);
}

std::string sanitizex(const std::string& text) {
	std::string out;
	bool wasValid = true;
	for (char c : text) {
		bool isValid = !envaledchar(c);
		if (wasValid) {
			if (!isValid) {
				wasValid = false;
			}
			else {
				out += c;
			}
		}
		else {
			wasValid = isValid;
		}
	}
	return out;
}

bool isDevName(const std::string& name) {
	static std::vector<std::string> devNames = {
		"luv avaaa",
		"JapaneseJSDF",
		"xStrayings",
		"SakuraUwU3916",
		"Rusher4039",
		"ChinesePLA69",
		"baizegg",
		"X59c",
		"LanGame DSY", //yes this is me
		"DayXXBoomerXX"
	};
	for (const auto& n : devNames) {
		if (_stricmp(name.c_str(), n.c_str()) == 0) return true;
	}
	return false;
}

void NameTags::InterfaceUtilsHook() {
	LocalPlayer* lp = g_Data.getLocalPlayer();
	if (lp == nullptr) return;
	for (Actor* actor : ActorUtils::getActorList()) {
		if (TargetUtil::ValidCheck(actor) || (actor == lp)) {
			Vec2<float> pos;
			if (g_Data.clientInstance->WorldToScreen(actor->getEyePos().add2(0.f, 0.75f, 0.f), pos)) {
				std::string name = *actor->getNameTag();
				std::string sanitizedMessage = sanitizex(name);

				float nameWidth = InterfaceUtils::GetTextWidth(sanitizedMessage, 1.5f);
				float devWidth = isDevName(sanitizedMessage) ? InterfaceUtils::GetTextWidth(" DEV", 1.5f) : 0.f;
				float totalWidth = nameWidth + devWidth;

				float textHeight = InterfaceUtils::GetTextHeight(sanitizedMessage, 1.5f);
				float textPadding = 1.5f;
				Vec2<float> textPos = Vec2<float>(pos.x - totalWidth / 2.f, pos.y - textHeight / 2.f);
				Vec4<float> rectPos = Vec4<float>(
					textPos.x - textPadding * 3.0f,
					textPos.y - textPadding,
					textPos.x + totalWidth + textPadding * 3.0f,
					textPos.y + textHeight + textPadding
				);

				InterfaceUtils::addBlur(rectPos, 8.f, false, InterfaceUtils::CornerRoundType::Full, 8.f);
				InterfaceUtils::FillRect(rectPos, UIColor(0, 0, 0, (int)(255 * opacity)), 8.f, InterfaceUtils::CornerRoundType::Full);

				InterfaceUtils::RenderText(textPos, sanitizedMessage, UIColor(255, 255, 255, 255), 1.5f, true);

				if (isDevName(sanitizedMessage)) {
					Vec2<float> devPos = Vec2<float>(textPos.x + nameWidth, textPos.y);
					InterfaceUtils::RenderText(devPos, " DEV", UIColor(0, 255, 0, 255), 1.5f, true);
				}
			}
		}
	}
}
