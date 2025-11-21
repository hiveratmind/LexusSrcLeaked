#pragma once

namespace RakPeerUtil {
    inline int GetCurrentPing(RakPeer* peer) {
        if (!peer || peer->NumberOfConnections() == 0)
            return -1;
        return peer->getPing();
    }

    inline int GetAveragePing(RakPeer* peer) {
        if (!peer || peer->NumberOfConnections() == 0)
            return -1;
        return peer->getAvgPing();
    }

    inline const char* GetLocalIP(RakPeer* peer) {
        if (!peer || peer->GetNumberOfAddresses() == 0)
            return nullptr;
        return peer->GetLocalIP(0);
    }

    inline const char* GetServerIP(RakPeer* peer) {
        if (!peer || peer->NumberOfConnections() == 0)
            return nullptr;

        SystemAddress addr = peer->GetSystemAddressFromIndex(0);
        return reinterpret_cast<const char*>(&addr);
    }
}