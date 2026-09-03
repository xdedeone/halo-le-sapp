#pragma once

#include "Events\NetworkEventPackets.h"

void SendPacket(void* data, size_t size);

template<class _Objty, class... _Types>
void NetworkEvent(_Types&&... _Args)
{
	_Objty* Packet = new _Objty(std::forward<_Types>(_Args)...);
	SendPacket(Packet, (sizeof(_Objty) + 1) / 2);
	delete Packet;
}

void InstallNetworkeventsHook();