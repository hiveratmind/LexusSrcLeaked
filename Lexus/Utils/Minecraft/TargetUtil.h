#pragma once
#include "../../SDK/World/LocalPlayer.h"

namespace TargetUtil {
	bool ValidCheck(Actor* target, bool isMob = false, float rangeCheck = 999999.f, bool teamcheck = false);
	bool isFriendValid(Actor* target, bool isMob = false, float rangeCheck = 999999.f);
	bool sortByDist(Actor* a1, Actor* a2);
}