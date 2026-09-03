#include "MapVote.h"

#include "Addresses.h"
#include "HaloUtilities.h"
#include "MapCheckAndExecute.h"
#include "MiscellaneousFunctions.h"
#include "Prefs.h"
#include "TextUtilities.h"

using namespace std;

const wstring& MapVoteItem::GetMap() const
{
	return Map;
}

const wstring& MapVoteItem::GetMode() const
{
	return Mode;
}

const wstring& MapVoteItem::GetDesc() const
{
	return Desc;
}

size_t MapVoteItem::GetMinPlayers() const
{
	return MinPlayers;
}

size_t MapVoteItem::GetMaxPlayers() const
{
	return MaxPlayers;
}

size_t MapVoteItem::GetVotes() const
{
	return Votes;
}

void MapVoteItem::AddVote()
{
	Votes++;
}
void MapVoteItem::ResetVotes()
{
	Votes = 0;
}

bool MapVoteItem::PlayersFit(size_t player_number) const
{
	return (player_number >= MinPlayers && player_number <= MaxPlayers);
}

wostream& operator <<(wostream &os, const MapVoteItem &MItem)
{
	os.width(16);
	os << left << MItem.Map;
	os.width(16);
	os << MItem.Mode;
	os.width(32);
	os << MItem.Desc;
	os << right << MItem.MinPlayers << "-" << MItem.MaxPlayers;
	return os;
}


bool MapVotes::MapVoteAdd(wstring &&map, wstring &&mode, wstring &&desc, size_t min, size_t max, size_t id)
{
	if (id == -1)
		id = MapVoteList.size();

	if (id > MapVoteList.size())
		return false;

	MapVoteList.emplace(MapVoteList.begin() + id, move(map), move(mode), move(desc), min, max);
	return true;
	
}
bool MapVotes::MapVoteDel(size_t Index)
{
	if(Index >= MapVoteList.size())
		return false;

	MapVoteList.erase(MapVoteList.begin() + Index);

	if (GlobalPrefs->MapVote && MapVoteList.size() < 2)
	{
		GlobalPrefs->MapVote = false;
	}

	return true;
}
void MapVotes::Vote(int PlayerIndex, size_t vote)
{
	if(GlobalPrefs->MapVote && !MapVoteCanceled && MapVoteList.size())
	{
		if (!PLVoted[PlayerIndex])
		{
			vote--;
			if (vote < MapVoteList.size() && vote < GlobalPrefs->MaxVotes)
			{
				if (MapVoteList.size() > GlobalPrefs->MaxVotes)
					vote = (vote + NextVote) % MapVoteList.size();

				if (MapVoteList[vote].PlayersFit(GetPlayerNumber()))
				{
					PLVoted[PlayerIndex] = true;

					MapVoteList[vote].AddVote();

					bool won = true;
					for (size_t i = 0; i < MapVoteList.size(); i++)
					{
						if (i != vote && MapVoteList[i].GetVotes() >= MapVoteList[vote].GetVotes())
						{
							won = false;
							break;
						}
					}

					if (won)
					{
						Winner = vote;
						RunMap(WToStr(MapVoteList[vote].GetMap()).c_str(), WToStr(MapVoteList[vote].GetMode()).c_str());
					}

					MsgSend(PlayerIndex, FormatWArgs(L"You voted for %ls!", MapVoteList[vote].GetDesc().c_str()).c_str());
				}
				else
					MsgSend(PlayerIndex, "This option is unavailable with the current player number!");
			}
			else
				MsgSend(PlayerIndex, "Incorrect vote number!");
		}
		else
			MsgSend(PlayerIndex, "You already voted for a map!");
	}
	else
		MsgSend(PlayerIndex, "Map voting is disabled!");
}
void MapVotes::ClearVotes()
{
	for(size_t i = 0; i < MapVoteList.size(); i++)
		MapVoteList[i].ResetVotes();

	memset(PLVoted, 0, 16);

	Winner = -1;
}

void MapVotes::List()
{
	if(MapVoteList.size())
	{
		size_t player_number = GetPlayerNumber();
		for(size_t i = 0; i < 16; i++)
		{
			if (PlayerPresent(i) && !PLVoted[i])
			{
				MsgSend(i, "Map voting is enabled, choose a number below:");
				if(MapVoteList.size() > GlobalPrefs->MaxVotes)
				{
					for (size_t j = 0; j < GlobalPrefs->MaxVotes; j++)
					{
						if (MapVoteList[(j + NextVote) % MapVoteList.size()].PlayersFit(player_number))
						{
							MsgSend(i, FormatWArgs(L"%u %ls", j + 1, MapVoteList[(j + NextVote) % MapVoteList.size()].GetDesc().c_str()).c_str());
						}						
					}	
				}
				else
				{
					for (size_t j = 0; j < MapVoteList.size(); j++)
					{
						if (MapVoteList[j].PlayersFit(player_number))
						{
							MsgSend(i, FormatWArgs(L"%u %ls", j + 1, MapVoteList[j].GetDesc().c_str()).c_str());
						}
					}
				}		
			}
		}
	}
}
bool MapVotes::Launch()
{
	if (MapVoteList.size())
	{
		if (Winner == -1)
		{
			if (MapVoteList.size() > GlobalPrefs->MaxVotes)
				Winner = (GetRand(0, GlobalPrefs->MaxVotes) + NextVote) % MapVoteList.size();
			else
				Winner = GetRand(0, MapVoteList.size());

			size_t player_number = GetPlayerNumber();
			size_t orig = Winner;
			do
			{
				if (MapVoteList[Winner].PlayersFit(player_number))
					break;

				Winner = (Winner + 1) % MapVoteList.size();

				if (MapVoteList.size() > GlobalPrefs->MaxVotes)
				{
					size_t limit = (NextVote + GlobalPrefs->MaxVotes) % MapVoteList.size();
					if (limit > NextVote)
					{
						if (Winner >= limit)
						{
							Winner = NextVote;
						}
					}
					else
					{
						if (Winner >= limit && Winner < NextVote)
						{
							Winner = NextVote;
						}
					}
				}
			} while (orig != Winner);
		}

		RunMap(WToStr(MapVoteList[Winner].GetMap()).c_str(), WToStr(MapVoteList[Winner].GetMode()).c_str());

		sv_say(FormatWArgs(L"%ls won!", MapVoteList[Winner].GetDesc().c_str()).c_str());

		if (MapVoteList.size() > GlobalPrefs->MaxVotes)
			NextVote = (NextVote + GlobalPrefs->MaxVotes) % MapVoteList.size();

		return true;
	}
	return false;
}
void MapVotes::LoadMapVotes()
{
	mapvotes_file = FormatWArgs(L"%hs\\sapp\\mapvotes.txt", GametypesFolder);
	crFile(mapvotes_file);

	FILE *MapvotesFile;
	_wfopen_s(&MapvotesFile, mapvotes_file.c_str(), L"r, ccs=UTF-16LE");

	if(!MapvotesFile)
	{
		PrintConsole(12, "Failed to open Mapvotes file!");
		return;
	}

	wchar_t buff[4096];

	while(fgetws(buff, 4096, MapvotesFile))
	{
		size_t n;
		wstring line = buff;

		TrimSpaces(line);

		if (line.empty())
			continue;

		if((n = line.find_first_not_of(L" \t")) != wstring::npos)
		{
			if(line[n] == ';')
				continue;
		}

		vector <wstring> line_items;
		if (!Split(line_items, line, L':') || line_items.size() < 3)
		{
			PrintConsole(12, FormatArgs("Invalid mapvote entry: \"%ls\"", line.c_str()).c_str());
			continue;
		}

		for (size_t i = 0; i < line_items.size(); i++)
			TrimSpaces(line_items[i]);

		if (CheckMap(WToStr(line_items[0]).c_str(), WToStr(line_items[1]).c_str()))
		{
			size_t minP, maxP;
			if (line_items.size() > 3)
			{
				if (!ToNumber(line_items[3], minP) || minP > 16)
				{
					minP = 0;
				}
				if (line_items.size() > 4)
				{
					if (!ToNumber(line_items[4], maxP) || maxP > 16)
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

			MapVoteList.emplace_back(move(line_items[0]), move(line_items[1]), move(line_items[2]), minP, maxP);
		}
		else
		{
			PrintConsole(12, FormatArgs("Invalid mapvote entry: \"%ls\" \"%ls\"", line_items[0].c_str(), line_items[1].c_str()).c_str());
		}
	}

	fclose(MapvotesFile);
}
void MapVotes::SaveMapVotes()
{
	FILE *MapvotesFile;
	_wfopen_s(&MapvotesFile, mapvotes_file.c_str(), L"w, ccs=UTF-16LE");

	if(!MapvotesFile)
	{
		PrintConsole(12, "Failed to open Mapvotes file!");
		return;
	}

	for(size_t i = 0; i < MapVoteList.size(); i++)
	{
		fwprintf_s(MapvotesFile, L"%ls:%ls:%ls:%u:%u\n", MapVoteList[i].GetMap().c_str(), MapVoteList[i].GetMode().c_str(), MapVoteList[i].GetDesc().c_str(), MapVoteList[i].GetMinPlayers(), MapVoteList[i].GetMaxPlayers());
	}

	fclose(MapvotesFile);
}
void MapVotes::Clear()
{
	MapVoteList.clear();
}
size_t MapVotes::size()
{
	return MapVoteList.size();
}
wostringstream MapVotes::StreamOut()
{
	wostringstream os;
	if(MapVoteList.empty())
	{
		os << L"There are no map votes." << endl;
	}
	else
	{
		for(size_t i = 0; i < MapVoteList.size(); i++)
		{
			if (i == Winner)
				os << L">> ";
			else
				os << L"   ";

			os.width(3);
			os << i << L" " << MapVoteList[i] << endl;
		}
	}
	return os;
}

MapVotes GlobalMapVotes;