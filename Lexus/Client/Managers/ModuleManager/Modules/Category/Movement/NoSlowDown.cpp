#include "NoSlowDown.h"

NoSlowDown::NoSlowDown() : Module(Category::Movement, "NoSlowDown", "Don't get slowed down when eating/using item")
{
};

std::string NoSlowDown::getModeText() {
	return "Vanilla";
}
