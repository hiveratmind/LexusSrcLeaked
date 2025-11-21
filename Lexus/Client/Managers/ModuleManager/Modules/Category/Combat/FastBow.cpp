#include "FastBow.h"

FastBow::FastBow() : Module(Category::Combat, "FastBow", "Fires bow almost instantly") {}

void FastBow::OnTick() {
    auto player = g_Data.getLocalPlayer();
    if (!player || !player->getCarriedItem())
        return;

    auto heldItem = player->getCarriedItem();
    if (!heldItem->item)
        return;

    if (heldItem->item->itemId != 306)
        return;

    if (player->gameMode && player->getStatusFlag(ActorFlags::Usingitem) &&
        heldItem && heldItem->item && heldItem->item->itemId == 306) {
        player->gameMode->releaseUsingItem();
    }
}