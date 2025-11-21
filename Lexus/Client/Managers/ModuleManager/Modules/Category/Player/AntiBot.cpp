#include "AntiBot.h"

AntiBot::AntiBot() : Module(Category::Player, "AntiBot", "Stop people from being with Bots(Anti-bot).") {
    addSetting(new BoolSetting("Symbol Check", "Check symbols to detect bots", &symbolCheck, false));
    addSetting(new SliderSetting<int>("Max Length", "Maximum allowed name length", &maxLength, 20, 16, 64));
    addSetting(new BoolSetting("Key Words", "Check for suspicious keywords", &keyWords, false));
    addSetting(new BoolSetting("Invalid", "Mark invalid players as bots", &invalid, false));
}