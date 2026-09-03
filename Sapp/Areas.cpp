#include "Areas.h"

#include "Addresses.h"
#include "Event.h"
#include "EventScan.h"
#include "HaloUtilities.h"
#include "MiscellaneousFunctions.h"
#include "TextUtilities.h"

using namespace std;

wstring AreaSphere::ToString() const
{
	return FormatWArgs(L"%hs:%ls:%ls:%4.2f %4.2f %4.2f:%4.2f", mapName.c_str(), name.c_str(), type.c_str(), center.x, center.y, center.z, radious);
}
wstring AreaCuboid::ToString() const
{
	return FormatWArgs(L"%hs:%ls:%ls:%4.2f %4.2f %4.2f:%4.2f %4.2f %4.2f", mapName.c_str(), name.c_str(), type.c_str(), a.x, a.y, a.z, b.x, b.y, b.z);
}
bool AreaSphere::IsInside(const vect3f &p) const
{
	if(Get3DDistance(center, p) > radious)
		return false;
	else
		return true;
}
bool AreaCuboid::IsInside(const vect3f &p) const
{
	if(abs(center.x - p.x) > lengths.x || abs(center.y - p.y) > lengths.y || abs(center.z - p.z) > lengths.z)
		return false;
	else
		return true;
}
size_t Areas::Find(const wstring &name)
{
	for(size_t i = 0; i < areaList.size(); i++)
		if(!_stricmp(areaList[i]->mapName.c_str(), CurrentMap.c_str()) && !_wcsicmp(areaList[i]->name.c_str(), name.c_str()))
			return i;

	return -1;
}
void Areas::RaiseEvents()
{
	if (!areaCache.empty())
	{
		size_t alive[16];
		vect3f coords[16];

		for (size_t i = 0; i < 16; i++)
		{
			if (PlayerAlive(i))
			{
				alive[i] = 1;
				GetPlayerPosition(i, coords[i]);
			}
			else
			{
				alive[i] = 0;
			}
		}

		for (size_t n = 0; n < areaCache.size(); n++)
		{
			for (size_t i = 0; i < 16; i++)
			{
				if (alive[i])
				{
					if (areaCache[n]->IsInside(coords[i]))
					{
						if (!areaCache[n]->IsPlayerInside[i])
						{
							areaCache[n]->IsPlayerInside[i] = true;
							vector <pair<wstring, wstring>> EventArgs;
							EventArgs.emplace_back(L"$area", areaCache[n]->name);
							GlobalEventHandler.Execute(i, EVENT_AREA_ENTER, EventArgs);
						}
					}
					else
					{
						if (areaCache[n]->IsPlayerInside[i])
						{
							areaCache[n]->IsPlayerInside[i] = false;
							vector <pair<wstring, wstring>> EventArgs;
							EventArgs.emplace_back(L"$area", areaCache[n]->name);
							GlobalEventHandler.Execute(i, EVENT_AREA_EXIT, EventArgs);
						}
					}
				}
			}
		}
	}
}
void Areas::DeletePlayer(int PlayerIndex)
{
	for(size_t n = 0; n < areaList.size(); n++)
	{
		areaList[n]->IsPlayerInside[PlayerIndex] = false;
	}
}
void Areas::AddSphere(const wstring &name, const vect3f &pos, float radious)
{
	LoadAreas();
	areaList.emplace_back(new AreaSphere(CurrentMap, name, L"Sphere", pos, radious));
	SaveAreas();
}
void Areas::AddCuboid(const wstring &name, const vect3f &posA, const vect3f &posB)
{
	LoadAreas();
	areaList.emplace_back(new AreaCuboid(CurrentMap, name, L"Cuboid", posA, posB));
	SaveAreas();
}
void Areas::CacheAreas()
{
	areaCache.clear();

	if (ServerRunning())
	{
		for (size_t n = 0; n < areaList.size(); n++)
		{
			if (!_stricmp(areaList[n]->mapName.c_str(), CurrentMap.c_str()))
			{
				areaCache.emplace_back(areaList[n]);
			}
		}
	}
}
void Areas::Del(size_t i)
{	
	for (size_t n = 0; n < areaCache.size(); n++)
	{
		if (areaCache[n] == areaList[i])
		{
			areaCache.erase(areaCache.begin() + n);
			break;
		}
	}
	areaList.erase(areaList.begin() + i);
	SaveAreas();
}
void Areas::Clear()
{
	areaCache.clear();
	areaList.clear();
}
wostringstream Areas::StreamOut()
{
	wostringstream os;
	size_t n = 0;
	for (size_t i = 0; i < areaList.size(); i++)
	{
		if (!_stricmp(areaList[i]->mapName.c_str(), CurrentMap.c_str()))
		{
			os << areaList[i]->ToString() << endl;
			n++;
		}
	}

	if (n == 0)
		os << FormatWArgs(L"There are no areas for %ls.", CurrentMapW.c_str()) << endl;

	return os;
}
wostringstream Areas::StreamOutAll()
{
	wostringstream os;
	if (areaList.empty())
	{
		os << L"There are no areas." << endl;
	}
	else
	{
		for (size_t i = 0; i < areaList.size(); i++)
		{
			os << areaList[i]->ToString() << endl;
		}
	}
	return os;
}
void Areas::SaveAreas()
{
	FILE *AreasFile;
	_wfopen_s(&AreasFile, areas_file.c_str(), L"w, ccs=UTF-16LE");

	if(!AreasFile)
	{
		PrintConsole(12, "Failed to open Areas file!");
		return;
	}

	for(size_t i = 0; i < areaList.size(); i++)
	{
		fwprintf_s(AreasFile, L"%ls\n", areaList[i]->ToString().c_str());
	}

	fclose(AreasFile);
}
void Areas::LoadAreas()
{
	areas_file = L"sapp\\areas.txt";
	crFile(areas_file);

	FILE *AreasFile;
	_wfopen_s(&AreasFile, areas_file.c_str(), L"r, ccs=UTF-16LE");

	if(!AreasFile)
	{
		PrintConsole(12, "Failed to open Areas file!");
		return;
	}

	wchar_t buff[4096];

	while(fgetws(buff, 4096, AreasFile))
	{
		wstring line = buff;

		vector <wstring> lineItems;
		Split(lineItems, line, L':');

		for(size_t i = 0; i < lineItems.size(); i++)
			TrimSpaces(lineItems[i]);

		if(lineItems.size() >= 5)
		{
			bool found = false;
			for(size_t i = 0; i < areaList.size(); i++)
			{
				if(!_stricmp(areaList[i]->mapName.c_str(), WToStr(lineItems[0]).c_str()) && !_wcsicmp(areaList[i]->name.c_str(), lineItems[1].c_str()))
				{
					found = true;
					break;
				}
			}
			if(!found)
			{
				if(!_wcsicmp(lineItems[2].c_str(), L"sphere"))
				{
					vect3f vect;
					float radius;
					if (TryParseVect3f(lineItems[3], vect) && ToNumber(lineItems[4], radius))
					{
						areaList.emplace_back(new AreaSphere(WToStr(lineItems[0]).c_str(), lineItems[1].c_str(), lineItems[2].c_str(), vect, radius));
					}
				}
				else if(!_wcsicmp(lineItems[2].c_str(), L"cuboid"))
				{
					vect3f posA;
					vect3f posB;
					if (TryParseVect3f(lineItems[3], posA) && TryParseVect3f(lineItems[4], posB))
					{
						areaList.emplace_back(new AreaCuboid(WToStr(lineItems[0]).c_str(), lineItems[1].c_str(), lineItems[2].c_str(), posA, posB));
					}
				}
			}
		}
	}

	fclose(AreasFile);
}

Areas GlobalAreas;