#pragma once

#include <sstream>
#include <string>
#include <vector>

class MapcycleItem
{
	std::string Map;
	std::string Mode;
	size_t MinPlayers;
	size_t MaxPlayers;

public:	
	MapcycleItem(std::string &&map, std::string &&mode, size_t min = 0, size_t max = 16)
		: Map(std::move(map)), Mode(std::move(mode)), MinPlayers(min), MaxPlayers(max) {}

	const std::string& GetMap() const;
	const std::string& GetMode() const;
	size_t GetMinPlayers() const;
	size_t GetMaxPlayers() const;

	bool PlayersFit(size_t player_number) const;

	bool operator ==(const MapcycleItem& MItem) const;
	bool operator !=(const MapcycleItem& MItem) const;

	friend std::wostream& operator <<(std::wostream &os, const MapcycleItem &CList);
};


class Mapcycle
{
	std::wstring mapcycle_file;
	
	std::vector<MapcycleItem> MapCycle;
	size_t current = -1;

public:
	void LoadMapcycle();	
	void SaveMapcycle();
	void SetToCurrentMap();
	bool Add(std::string &&map, std::string &&mode, size_t min, size_t max, size_t id);
	bool Del(size_t Index);
	void Clear();

	std::wostringstream StreamOut();

	bool Begin();
	bool Next();
	bool Prev();
	bool Spec(size_t Index);

	const MapcycleItem* WhatsNext();
};

extern Mapcycle GlobalMapcycle;