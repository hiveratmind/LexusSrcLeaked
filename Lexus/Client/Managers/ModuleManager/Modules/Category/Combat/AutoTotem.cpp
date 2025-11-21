#include "AutoTotem.h"
#include <chrono>

AutoTotem::AutoTotem() : Module(Category::Combat, "AutoTotem", "Automatically puts a totem into offhand") {
	addSetting(new SliderSetting<int>("Delay", "Delay after placing (ms)", &delay, 0, 0, 1000));
}

void AutoTotem::OnTick() {
	LocalPlayer* localPlayer = g_Data.getLocalPlayer();
	auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now().time_since_epoch()
	).count();

	if (now - lastPlaceTime < delay)
		return;

	ItemStack* offhandStack = localPlayer->getOffhandSlot();
	if (offhandStack != nullptr && offhandStack->item.get() != nullptr)
		return;

	Container* inv = localPlayer->playerInventory->container;
	for (int i = 0; i < 36; i++) {
		ItemStack* stack = inv->getItem(i);
		Item* item = stack->item.get();
		if (item && item->itemId == 584) {
			InventoryAction action1(i, stack, ItemStack::getEmptyItem());
			InventoryAction action2(0, ItemStack::getEmptyItem(), stack, ContainerID::Offhand);

			auto tx = std::make_unique<ComplexInventoryTransaction>();
			tx->type = ComplexInventoryTransaction::Type::NormalTransaction;
			tx->data.addAction(action1);
			tx->data.addAction(action2);

			InventoryTransactionPacket packet(std::move(tx));
			localPlayer->level->getPacketSender()->send(&packet);


			lastPlaceTime = now;
			break;
		}
	}
}

std::string AutoTotem::getModeText() {
	return std::to_string(delay);
}