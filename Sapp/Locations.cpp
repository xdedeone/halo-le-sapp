#include "Locations.h"

#include "Addresses.h"
#include "Event.h"
#include "EventScan.h"
#include "HaloUtilities.h"
#include "TextUtilities.h"

using namespace std;

wstring LocationItem::ToString() const
{
	return FormatWArgs(L"%hs:%ls:%4.2f %4.2f %4.2f", mapName.c_str(), name.c_str(), location.x, location.y, location.z);
}
size_t Locations::Find(const wstring &name)
{
	for(size_t i = 0; i < locationList.size(); i++)
		if(!_stricmp(locationList[i].mapName.c_str(), CurrentMap.c_str()) && !_wcsicmp(locationList[i].name.c_str(), name.c_str()))
			return i;

	return -1;
}
void Locations::Add(const wstring &name, const vect3f &loc)
{
	LoadLocations();
	locationList.emplace_back(CurrentMap, name, loc);
	SaveLoc();
}
void Locations::Del(size_t i)
{	
	locationList.erase(locationList.begin()+i);
	SaveLoc();
}
vect3f Locations::Get(size_t i)
{			
	return locationList[i].location;
}
void Locations::Clear()
{
	locationList.clear();
}
wostringstream Locations::StreamOut()
{
	wostringstream os;
	size_t n = 0;
	for (size_t i = 0; i < locationList.size(); i++)
	{
		if (!_stricmp(locationList[i].mapName.c_str(), CurrentMap.c_str()))
		{
			os << locationList[i].ToString() << endl;
			n++;
		}
	}

	if (n == 0)
		os << FormatWArgs(L"There are no locations for %ls.", CurrentMapW.c_str()) << endl;

	return os;
}
wostringstream Locations::StreamOutAll()
{
	wostringstream os;
	if (locationList.empty())
	{
		os << L"There are no locations." << endl;
	}
	else
	{
		for (size_t i = 0; i < locationList.size(); i++)
		{
			os << locationList[i].ToString() << endl;
		}
	}
	return os;
}
void Locations::SaveLoc()
{
	FILE *LocationsFile;
	_wfopen_s(&LocationsFile, locations_file.c_str(), L"w, ccs=UTF-16LE");

	if(!LocationsFile)
	{
		PrintConsole(12, "Failed to open Locations file!");
		return;
	}

	for(size_t i = 0; i < locationList.size(); i++)
	{
		fwprintf_s(LocationsFile, L"%ls\n", locationList[i].ToString().c_str());
	}

	fclose(LocationsFile);
}
void Locations::LoadLocations()
{
	locations_file = L"sapp\\locations.txt";
	crFile(locations_file);

	FILE *LocationsFile;
	_wfopen_s(&LocationsFile, locations_file.c_str(), L"r, ccs=UTF-16LE");

	if(!LocationsFile)
	{
		PrintConsole(12, "Failed to open Locations file!");
		return;
	}

	wchar_t buff[4096];

	while(fgetws(buff, 4096, LocationsFile))
	{
		wstring line = buff;

		vector <wstring> lineItems;
		Split(lineItems, line, L':');

		for(size_t i = 0; i < lineItems.size(); i++)
			TrimSpaces(lineItems[i]);

		if(lineItems.size() >= 3)
		{
			bool found = false;
			for(size_t i = 0; i < locationList.size(); i++)
			{					
				if(!_stricmp(locationList[i].mapName.c_str(), WToStr(lineItems[0]).c_str()) && !_wcsicmp(locationList[i].name.c_str(), lineItems[1].c_str()))
				{
					found = true;
					break;
				}
			}
			if(!found)
			{
				vect3f vect;
				if (TryParseVect3f(lineItems[2], vect))
				{
					locationList.emplace_back(WToStr(lineItems[0]).c_str(), lineItems[1].c_str(), vect);
				}
			}
		}
	}

	fclose(LocationsFile);
}

Locations GlobalLocations;