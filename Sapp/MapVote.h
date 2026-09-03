#pragma once

#include <sstream>
#include <string>
#include <vector>

class MapVoteItem
{
	std::wstring Map;
	std::wstring Mode;
	std::wstring Desc;
	size_t MinPlayers;
	size_t MaxPlayers;
	size_t Votes;

public:
	MapVoteItem(std::wstring &&map, std::wstring &&mode, std::wstring &&desc, size_t min = 0, size_t max = 16)
		: Map(std::move(map)), Mode(std::move(mode)), Desc(std::move(desc)), MinPlayers(min), MaxPlayers(max), Votes(0) {}

	const std::wstring& GetMap() const;
	const std::wstring& GetMode() const;
	const std::wstring& GetDesc() const;
	size_t GetMinPlayers() const;
	size_t GetMaxPlayers() const;
	size_t GetVotes() const;

	void AddVote();
	void ResetVotes();

	bool PlayersFit(size_t player_number) const;

	friend std::wostream& operator <<(std::wostream &os, const MapVoteItem &CList);
};

class MapVotes
{
	std::vector <MapVoteItem> MapVoteList;
	std::wstring mapvotes_file;
	bool PLVoted[16] = {};
	size_t NextVote = 0;
	size_t Winner = -1;

public:	
	bool MapVoteAdd(std::wstring &&map, std::wstring &&mode, std::wstring &&desc, size_t min, size_t max, size_t id);
	bool MapVoteDel(size_t Index);
	void Vote(int PlayerIndex, size_t vote);
	void ClearVotes();
	void List();
	bool Launch();
	void LoadMapVotes();
	void SaveMapVotes();
	void Clear();
	size_t size();
	std::wostringstream StreamOut();
};

extern MapVotes GlobalMapVotes;