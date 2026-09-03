#pragma once

#include <memory>

#include "Events\NetworkEventPackets.h"

void BroadcastPacket(wchar_t* data, size_t size);

template<class _Objty, class... _Types>
void NetworkEvent(_Types&&... _Args)
{
	auto Packet = std::make_unique<_Objty>(std::forward<_Types>(_Args)...);
	BroadcastPacket(reinterpret_cast<wchar_t*>(Packet.get()), (Packet->size() + 1) / 2);
}

void SendPacket(int PlayerIndex, wchar_t* data, size_t size);

template<class _Objty, class... _Types>
void NetworkEventPlayer(int PlayerIndex, _Types&&... _Args)
{
	auto Packet = std::make_unique<_Objty>(std::forward<_Types>(_Args)...);
	SendPacket(PlayerIndex, reinterpret_cast<wchar_t*>(Packet.get()), (Packet->size() + 1) / 2);
}

void InstallBuildChatPacketHook();

void HandleNetworkEvents(int PlayerIndex, const NetworkEventBase* data);