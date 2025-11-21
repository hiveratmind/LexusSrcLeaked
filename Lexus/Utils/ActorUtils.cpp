#include "ActorUtils.h"

#include "../../SDK/Game.h"
#include "../../SDK/World/Actor.h"

class ActorOwnerComponent
{
public:
    class Actor* mActor;
};

struct ActorUniqueIDComponent {
    uint64_t mUniqueID;
};

std::vector<Actor*> ActorUtils::getActorList(bool playerOnly) {
    std::vector<Actor*> result;

    Actor* localPlayer = g_Data.getClientInstance()->getLocalPlayer();
    if (!localPlayer) return result;

    auto* context = localPlayer->getEntityContext();
    auto& registry = context->getRegistry();

    auto view = registry.view<ActorOwnerComponent, ActorTypeComponent>();

    for (auto entity : view) {
        auto& owner = view.get<ActorOwnerComponent>(entity);
        auto& type = view.get<ActorTypeComponent>(entity);

        if (!owner.mActor) continue;
        if (playerOnly && type.id != static_cast<int>(ActorType::Player)) continue;

        result.push_back(owner.mActor);
    }

    return result;
}

std::vector<Actor*> ActorUtils::getActorsOfType(ActorType type) {
    std::vector<Actor*> result;

    Actor* localPlayer = g_Data.getClientInstance()->getLocalPlayer();
    if (!localPlayer) return result;

    auto* context = localPlayer->getEntityContext();
    auto& registry = context->getRegistry();

    auto view = registry.view<ActorOwnerComponent, ActorTypeComponent>();

    for (auto entity : view) {
        auto& owner = view.get<ActorOwnerComponent>(entity);
        auto& typeComponent = view.get<ActorTypeComponent>(entity);

        if (!owner.mActor) continue;
        if (typeComponent.id != static_cast<int>(type)) continue;

        result.push_back(owner.mActor);
    }

    return result;
}
