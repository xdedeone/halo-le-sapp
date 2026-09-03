#pragma once

class OnlineAdminList
{
	int OnlineAdmins[16];

public:
	OnlineAdminList() { Clear(); }
	void SetAdminLevel(size_t PlayerIndex, int lev);
	int GetAdminLevel(size_t PlayerIndex);
	void DeleteAdmin(size_t PlayerIndex);
	void Clear();
};

extern OnlineAdminList GlobalOnlineAdminList;