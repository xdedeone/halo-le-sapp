#include "Mapcycle.h"

#include <fstream>
#include <iomanip>

#include "Addresses.h"
#include "EventScan.h"
#include "GSQueryPatch.h"
#include "HaloUtilities.h"
#include "MapCheckAndExecute.h"
#include "Prefs.h"
#include "TextUtilities.h"

using namespace std;

const string& MapcycleItem::GetMap() const
{
	return Map;
}
const string& MapcycleItem::GetMode() const
{
	return Mode;
}
size_t MapcycleItem::GetMinPlayers() const
{
	return MinPlayers;
}
size_t MapcycleItem::GetMaxPlayers() const
{
	return MaxPlayers;
}

bool MapcycleItem::PlayersFit(size_t player_number) const
{
	return (player_number >= MinPlayers && player_number <= MaxPlayers);
}

bool MapcycleItem::operator ==(const MapcycleItem& MItem) const
{
	return((_stricmp(Map.c_str(), MItem.Map.c_str()) == 0) && (_stricmp(Mode.c_str(), MItem.Mode.c_str()) == 0));
}
bool MapcycleItem::operator !=(const MapcycleItem& MItem) const
{
	return((_stricmp(Map.c_str(), MItem.Map.c_str()) != 0) || (_stricmp(Mode.c_str(), MItem.Mode.c_str()) != 0));
}

wostream& operator <<(wostream &os, const MapcycleItem &MItem)
{
	os.width(24);	
	os << left << MItem.Map.c_str();
	os.width(24);
	os << MItem.Mode.c_str();
	os.width(2);
	os << right << MItem.MinPlayers << "-" << MItem.MaxPlayers;
	return os;
}


void Mapcycle::LoadMapcycle()
{
	mapcycle_file = FormatWArgs(L"%hs\\sapp\\mapcycle.txt", GametypesFolder);
	fstream MapcycleFile;
	MapcycleFile.open(mapcycle_file, ios::out | ios::app);
	MapcycleFile.close(); // create if not exists

	MapcycleFile.open(mapcycle_file, ios::in);
	if (!MapcycleFile.is_open())
	{
		PrintConsole(12, "Failed to open Mapcycle file!");
		return;
	}

	string line;
	while(getline(MapcycleFile, line))
	{	
		if(line.empty())
			continue;
		
		size_t n;
		if((n = line.find_first_not_of(" \t")) != string::npos)
		{
			if(line[n] == ';')
				continue;
		}

		vector <string> line_items;
		if (!Split(line_items, line, ':') || line_items.size() < 2)
		{
			PrintConsole(12, FormatArgs("Invalid mapcycle entry: \"%hs\"", line.c_str()).c_str());
			continue;
		}

		for(size_t i = 0; i < line_items.size(); i++)
			TrimSpaces(line_items[i]);

		if(CheckMap(line_items[0].c_str(), line_items[1].c_str()))
		{
			size_t minP, maxP;
			if(line_items.size() > 2)
			{
				if(!ToNumber(line_items[2], minP) || minP > 16)
				{
					minP = 0;
				}
				if(line_items.size() > 3)
				{
					if(!ToNumber(line_items[3], maxP) || maxP > 16)
					{
						maxP = 16;
					}
				}
				else
				{
					maxP = 16;
				}
			}
			else
			{
				minP = 0;
				maxP = 16;
			}

			MapCycle.emplace_back(move(line_items[0]), move(line_items[1]), minP, maxP);
		}
		else
		{
			PrintConsole(12, FormatArgs("Invalid mapcycle entry: \"%hs\" \"%hs\"", line_items[0].c_str(), line_items[1].c_str()).c_str());
		}
	}

	MapcycleFile.close();
}

void Mapcycle::SaveMapcycle()
{
	FILE *MapcycleFile;
	_wfopen_s(&MapcycleFile, mapcycle_file.c_str(), L"w");

	if(!MapcycleFile)
	{
		PrintConsole(12, "Failed to open Mapcycle file!");
		return;
	}

	for (size_t i = 0; i < MapCycle.size(); i++)
	{
		fprintf_s(MapcycleFile, "%hs:%hs:%u:%u\n", MapCycle[i].GetMap().c_str(), MapCycle[i].GetMode().c_str(), MapCycle[i].GetMinPlayers(), MapCycle[i].GetMaxPlayers());
	}

	fclose(MapcycleFile);
}

void Mapcycle::SetToCurrentMap()
{
	if (!MapCycle.empty() && ServerRunning())
	{
		for (size_t i = 0; i < MapCycle.size(); i++)
		{
			if (!_stricmp(MapCycle[i].GetMap().c_str(), CurrentMap.c_str()) && !_stricmp(MapCycle[i].GetMode().c_str(), CurrentMode.c_str()))
			{
				current = i;
				return;
			}
		}

		current = MapCycle.size() - 1;
	}
}


bool Mapcycle::Add(string &&map, string &&mode, size_t min, size_t max, size_t id)
{
	if (id == -1)
		id = MapCycle.size();

	if (id > MapCycle.size())
		return false;

	if (current != -1 && current >= id)
		current++;

	MapCycle.emplace(MapCycle.begin() + id, move(map), move(mode), min, max);
	return true;
}

bool Mapcycle::Del(size_t Index)
{
	if (Index >= MapCycle.size())
		return false;

	MapCycle.erase(MapCycle.begin() + Index);

	if (current != -1 && current >= Index)
	{
		if (current == 0)
			current = MapCycle.size() - 1;
		else
			current--;
	}

	return true;
}

void Mapcycle::Clear()
{
	MapCycle.clear();
	current = -1;
}

bool Mapcycle::Begin()
{
	if(!MapCycle.empty())
	{
		size_t PlayerNumber = GetPlayerNumber();

		size_t begin = 0;
		for (size_t i = 0; i < MapCycle.size(); i++)
		{
			if (MapCycle[i].PlayersFit(PlayerNumber))
			{
				begin = i;
				break;
			}
		}

		if(RunMap(MapCycle[begin].GetMap().c_str(), MapCycle[begin].GetMode().c_str()))
		{
			current = begin;
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}

bool Mapcycle::Next()
{
	if (!MapCycle.empty() && current != -1)
	{
		size_t PlayerNumber = GetPlayerNumber();
		
		if (current == MapCycle.size() - 1)
			current = 0;
		else
			current++;

		size_t next = current;
		do
		{
			if (!MapCycle[next].PlayersFit(PlayerNumber))
			{
				if (next == MapCycle.size() - 1)
					next = 0;
				else
					next++;
			}
			else break;
		} while (next != current);

		if(RunMap(MapCycle[next].GetMap().c_str(), MapCycle[next].GetMode().c_str()))
		{
			current = next;
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}

bool Mapcycle::Prev()
{
	if (!MapCycle.empty() && current != -1)
	{
		size_t PlayerNumber = GetPlayerNumber();

		if (current == 0)
			current = MapCycle.size() - 1;
		else
			current--;

		auto prev = current;
		do
		{
			if (!MapCycle[prev].PlayersFit(PlayerNumber))
			{
				if (prev == 0)
					prev = MapCycle.size() - 1;
				else
					prev--;
			}
			else break;
		} while (prev != current);

		if(RunMap(MapCycle[prev].GetMap().c_str(), MapCycle[prev].GetMode().c_str()))
		{
			current = prev;
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}

bool Mapcycle::Spec(size_t Index)
{
	if(Index < MapCycle.size())
	{
		if(RunMap(MapCycle[Index].GetMap().c_str(), MapCycle[Index].GetMode().c_str()))
		{
			current = Index;
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}

const MapcycleItem* Mapcycle::WhatsNext()
{
	if(!MapCycle.empty() && current != -1)
	{
		size_t next = current + 1;
		if (next == MapCycle.size())
			next = 0;
		return &MapCycle[next];
	}
	else
	{
		return nullptr;
	}
}

wostringstream Mapcycle::StreamOut()
{
	wostringstream os;
	if(MapCycle.empty())
	{
		os << L"There are no mapcycle items." << endl;
	}
	else
	{	
		for (size_t i = 0; i < MapCycle.size(); i++)
		{
			if(i == current)
				os << L">> ";
			else
				os << L"   ";

			os.width(3);
			os << i << L" " << MapCycle[i] << endl;
		}
	}
	return os;
}

Mapcycle GlobalMapcycle;