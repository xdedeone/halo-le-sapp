#pragma once

#include <memory>
#include <sstream>
#include <vector>

#include "Vector3f.h"

class AreaBase
{
	friend class Areas;
protected:
	std::string mapName;
	std::wstring name;
	std::wstring type;
	vect3f center;
	bool IsPlayerInside[16];
public:
	AreaBase(const std::string& mapName, const std::wstring& name, const std::wstring& type, const vect3f &center) : mapName(mapName), name(name), type(type), center(center)
	{
		memset(IsPlayerInside, 0, 16);
	}
	virtual bool IsInside(const vect3f &p) const = 0;
	virtual std::wstring ToString() const = 0;
	virtual ~AreaBase() {}
};

class AreaSphere : public AreaBase
{
	float radious;
public:
	AreaSphere(const std::string& mapName, const std::wstring& name, const std::wstring& type, const vect3f &c, float r) : AreaBase(mapName, name, type, c), radious(r) {}
	bool IsInside(const vect3f &p) const;
	std::wstring ToString() const;
};

class AreaCuboid : public AreaBase
{
	vect3f lengths;
	vect3f a, b;
public:
	AreaCuboid(const std::string& mapName, const std::wstring& name, const std::wstring& type, const vect3f &a, const vect3f &b) : AreaBase(mapName, name, type, (a+b)), lengths(a-b), a(a), b(b)
	{
		center /= 2;
		lengths /= 2;
		lengths.Abs();		
	}
	bool IsInside(const vect3f &p) const;
	std::wstring ToString() const;
};


class Areas {
	std::vector <std::shared_ptr<AreaBase>> areaList;
	std::vector <std::shared_ptr<AreaBase>> areaCache;
	std::wstring areas_file;
public:
	size_t Find(const std::wstring &name);
	void RaiseEvents();
	void DeletePlayer(int PlayerIndex);
	void AddSphere(const std::wstring &name, const vect3f &pos, float radious);
	void AddCuboid(const std::wstring &name, const vect3f &pos, const vect3f &lengths);
	void CacheAreas();
	void Del(size_t i);
	void Clear();	
	std::wostringstream StreamOut();
	std::wostringstream StreamOutAll();
	void SaveAreas();
	void LoadAreas();
};

extern Areas GlobalAreas;