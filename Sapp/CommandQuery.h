#pragma once

#include "GenericCommand.h"
#include "GSQueryPatch.h"

class CommandQueryAdd final : public GenericCommand
{
	std::string key;
	std::string value;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 3)
		{
			key = WToStr(args[1]);
			value = WToStr(args[2]);
			return true;
		}
		return false;
	}

public:
	CommandQueryAdd() : GenericCommand(L"query_add", L"<key> <value>") {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (key.find('\\') == std::string::npos && value.find('\\') == std::string::npos)
		{
			ExtraQueryItems[key] = value;
			env_Print(FormatWArgs(L"Added %ls - %ls argument pair to the query string!", key.c_str(), value.c_str()).c_str());
			FormatQueryString();
		}
		else
		{
			env_Print("Error: The arguments can't contain the \"\\\" character!");
		}
		return true;
	}
};

class CommandQueryDel final : public GenericCommand
{
	std::string query_name;
	size_t query_index;

	bool ParseArgs(ArgContainer& args) override
	{
		if (args.argc == 2)
		{
			if (ToNumber(args[1], query_index))
			{
				query_name.clear();
			}
			else
			{
				query_name = WToStr(args[1]);
			}
			return true;
		}
		return false;
	}

public:
	CommandQueryDel() : GenericCommand(L"query_del", L"<query name>\n<query index>") {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (!query_name.empty())
		{
			auto& it = ExtraQueryItems.find(query_name);
			if (it != ExtraQueryItems.end())
			{
				ExtraQueryItems.erase(it);
				env_Print(FormatWArgs(L"Successfully deleted query %ls!", query_name.c_str()).c_str());
				FormatQueryString();
			}
			else
			{
				env_Print("No query found with this name!");
			}
		}
		else
		{
			if (query_index < ExtraQueryItems.size())
			{
				size_t i = 0;
				for (auto& it = ExtraQueryItems.begin(); it != ExtraQueryItems.end(); ++it)
				{
					if (query_index == i++)
					{
						ExtraQueryItems.erase(it);
						env_Print(FormatArgs("Successfully deleted query with ID '%u'!", query_index).c_str());
						FormatQueryString();
						break;
					}
				}
			}
			else
			{
				env_Print("Invalid query index!");
			}
		}

		return true;
	}
};

class CommandQueryList final : public GenericCommand
{
public:
	CommandQueryList() : GenericCommand(L"query_list") {}

	bool Execute(const CommandEnviroment& env) override
	{
		size_t i = 0;
		for (auto& it : ExtraQueryItems)
		{
			env_Print(FormatArgs("%2u  %hs - %hs", i++, it.first.c_str(), it.second.c_str()).c_str());
		}
		return true;
	}
};