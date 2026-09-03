#pragma once

#include <sstream>
#include <vector>

#include "Vector3f.h"

class LocationItem
{
	friend class Locations;
protected:
	std::string mapName;
	std::wstring name;
	vect3f location;
public:
	LocationItem(const std::string& mapName, const std::wstring& name, const vect3f &location) : mapName(mapName), name(name), location(location) {}
	std::wstring ToString() const;
};

class Locations {
	std::vector <LocationItem> locationList;
	std::wstring locations_file;
public:
	size_t Find(const std::wstring &name);
	void Add(const std::wstring &name, const vect3f &loc);
	void Del(size_t i);
	vect3f Get(size_t i);
	void Clear();
	std::wostringstream StreamOut();
	std::wostringstream StreamOutAll();
	void SaveLoc();
	void LoadLocations();
};

extern Locations GlobalLocations;