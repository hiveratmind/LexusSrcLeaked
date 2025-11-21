#pragma once

#include <vector>
#include <memory>
#include <string>

class Actor;
enum class ActorType : int;
class InventoryTransactionPacket;

class ActorUtils {
public:
    static std::vector<Actor*> getActorList(bool playerOnly = true);
    static std::vector<Actor*> getActorsOfType(ActorType type);

    template <typename T = Actor>
    static std::vector<T*> getActorsTyped(ActorType type) {
        const std::vector<Actor*> actors = getActorsOfType(type);
        std::vector<T*> result;
        for (auto actor : actors) {
            result.push_back(static_cast<T*>(actor));
        }
        return result;
    }
};
