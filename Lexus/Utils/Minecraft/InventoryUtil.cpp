#include "InventoryUtil.h"

void InventoryUtil::switchSlot(int32_t slot) {
	g_Data.getLocalPlayer()->playerInventory->selectedSlot = slot;
}

ItemStack* InventoryUtil::getItem(int32_t slot) {
	return g_Data.getLocalPlayer()->playerInventory->container->getItem(slot);
}

int32_t InventoryUtil::findItemSlotInHotbar(uint16_t itemId) {
	int32_t result = -1;
	for (int i = 0; i < 9; i++) {
		ItemStack* itemStack = g_Data.getLocalPlayer()->playerInventory->container->getItem(i);
		if (itemStack->isValid() && itemStack->item.get()->itemId == itemId) {
			result = i;
			break;
		}
	}
	return result;
}

int32_t InventoryUtil::findItemSlotInInventory(uint16_t itemId) {
	int32_t result = -1;
	for (int i = 0; i < 36; i++) {
		ItemStack* itemStack = g_Data.getLocalPlayer()->playerInventory->container->getItem(i);
		if (itemStack->isValid() && itemStack->item.get()->itemId == itemId) {
			result = i;
			break;
		}
	}
	return result;
}