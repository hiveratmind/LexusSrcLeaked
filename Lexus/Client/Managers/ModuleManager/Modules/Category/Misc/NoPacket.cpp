#include "NoPacket.h"

NoPacket::NoPacket()
	: Module(Category::Misc, "NoPacket", "Stop sending packets to servers\n(This module commonly used for bug/dupe on server)") {
}

void NoPacket::SentPacketHook(Packet* packet, bool& cancel) {
	cancel = true;
}
