#include "TimeChanger.h"

TimeChanger::TimeChanger() : Module(Category::Render, "TimeChanger", "Change The Time.") {
    addSetting(new SliderSetting<float>("Time", "", &FurryWare, 0.f, 0.f, 1.f));
}

TimeChanger::~TimeChanger() {
}
