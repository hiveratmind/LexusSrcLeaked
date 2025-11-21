#include "Swing.h"

Swing::Swing() : Module(Category::Render, "Swing", "Swing animation")
{
	addSetting(new SliderSetting<int>("Speed", "Swing speed", &swingSpeed, swingSpeed, 1, 20));
}