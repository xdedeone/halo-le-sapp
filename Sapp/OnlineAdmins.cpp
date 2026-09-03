#include "OnlineAdmins.h"

void OnlineAdminList::SetAdminLevel(size_t PlayerIndex, int lev)
{
	OnlineAdmins[PlayerIndex] = lev;
}
int OnlineAdminList::GetAdminLevel(size_t PlayerIndex)
{
	return OnlineAdmins[PlayerIndex];
}
void OnlineAdminList::DeleteAdmin(size_t PlayerIndex)
{
	OnlineAdmins[PlayerIndex] = -1;
}
void OnlineAdminList::Clear()
{
	for (size_t i = 0; i < 16; i++)
		OnlineAdmins[i] = -1;
}

OnlineAdminList GlobalOnlineAdminList;