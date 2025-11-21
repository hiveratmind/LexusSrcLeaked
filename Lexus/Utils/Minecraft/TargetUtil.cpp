#include "TargetUtil.h"
#include "../../SDK/Game.h"
#include "../../Client/Client.h"

#include <cctype>


bool TargetUtil::ValidCheck(Actor* target, bool isMob, float rangeCheck, bool teamcheck) {
    LocalPlayer* localPlayer = g_Data.getLocalPlayer();
    if (localPlayer == nullptr)
        return false;

    if (target == nullptr)
        return false;

    if (target == localPlayer)
        return false;

    std::string* namePtr = target->getNameTag();
    if (namePtr == nullptr)
        return false;
    const std::string& name = *namePtr;

    if (teamcheck) {
        if (target->isTeammate(g_Data.getLocalPlayer())) return false;
    }

    if (ModuleManager::getModule<AntiBot>()->isEnabled() && !isMob) {
        if (name.length() <= 1 || name.find("game selector") != std::string::npos)
            return false;

        if (target->aabbShape->width >= 0.70f && target->aabbShape->width <= 0.80f &&
            target->aabbShape->height >= 2.0f && target->aabbShape->height <= 2.2f)
            return false;

        AntiBot* antiBot = ModuleManager::getModule<AntiBot>();
        if (!antiBot)
            return false;

        std::string lowerName = name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
            [](unsigned char c) { return std::tolower(c); });

        if (antiBot->symbolCheck) {
            if (lowerName.find('-') != std::string::npos || lowerName.find('|') != std::string::npos || lowerName.find(':') != std::string::npos)
                return false;
        }

        if (lowerName.length() > static_cast<size_t>(antiBot->maxLength))
            return false;

        if (antiBot->keyWords) {
            static const std::vector<std::string> suspiciousWords1 = {
                "shop", "keeper", "click", "buy", "right", "click", "item", "tier", "second", "upgrade", "gen", "I", "spawns"
            };

            int foundCount = 0;
            for (const auto& word : suspiciousWords1) {
                if (lowerName.find(word) != std::string::npos) {
                    foundCount++;
                    if (foundCount >= 3)
                        return false;
                }
            }
        }

        if (antiBot->keyWords) {
            static const std::vector<std::string> suspiciousWords2 = {
                "buy", "click", "to", "tap"
            };

            int foundCount = 0;
            for (const auto& word : suspiciousWords2) {
                if (lowerName.find(word) != std::string::npos) {
                    foundCount++;
                    if (foundCount >= 3)
                        return false;
                }
            }
        }

        if (antiBot->invalid) {
            if (target->getStatusFlag(ActorFlags::Idling))
                return false;
        }
    }

    ActorTypeComponent* actorTypeComponent = target->getActorTypeComponent();
    if (actorTypeComponent == nullptr)
        return false;

    uint32_t entityId = actorTypeComponent->id;

    if (!isMob) {
        if (entityId != 319)
            return false;

        if (name.empty())
            return false;
    }
    else {
        static const std::vector<uint32_t> bannedEntityId = {
            64,      // entity.item.name
            69,      // entity.xp_orb.name
            4194372, // entity.xp_bottle.name
            4194390, // entity.splash_potion.name
            4194405, // entity.lingering_potion.name
            95,      // entity.area_effect_cloud.name
            12582992,// entity.arrow.name
            4194385, // entity.snowball.name
            4194447, // entity.wind_charge_projectile.name
            4194393, // entity.wither_skull.name
            4194395, // entity.wither_skull_dangerous.name
            72,      // entity.fireworks_rocket.name
            4194391, // entity.ender_pearl.name
            77,      // entity.fishing_hook.name
            12582985,// entity.thrown_trident.name
            66,      // entity.falling_block.name
            65,      // entity.tnt.name
        };

        for (uint32_t i : bannedEntityId) {
            if (entityId == i)
                return false;
        }
    }

    if ((localPlayer->getPos().dist(target->getPos())) > rangeCheck)
        return false;

    return true;
}



bool TargetUtil::isFriendValid(Actor* target, bool isMob, float rangeCheck) {
    return false;
}
bool TargetUtil::sortByDist(Actor* a1, Actor* a2) {
	Vec3<float> lpPos = g_Data.getLocalPlayer()->getPos();
	return ((a1->getPos().dist(lpPos)) < (a2->getPos().dist(lpPos)));
}