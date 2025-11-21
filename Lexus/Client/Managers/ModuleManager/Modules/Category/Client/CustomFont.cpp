#include "CustomFont.h"

CustomFont::CustomFont() : Module(Category::Client, "CustomFont", "Font of Client"){
	addSetting(new SliderSetting<int>("FontSize", "NULL", &fontSize, 16, 10, 40));
	addSetting(new BoolSetting("Italic", "NULL", &italic, false));
	addSetting(new BoolSetting("Shadow", "NULL", &shadow, true));
    fontEnumSetting = (EnumSetting*)addSetting(new EnumSetting("Font", "NULL", {
        "Direct2D",
        "Arial",
        "Calibri",
        "Verdana",
        "Tahoma"
        }, &fontMode, 3));

}

bool CustomFont::isEnabled() {
	return true;
}

bool CustomFont::isVisible() {
	return false;
}

std::string CustomFont::getSelectedFont() {
	return fontEnumSetting->enumList[fontMode];
}