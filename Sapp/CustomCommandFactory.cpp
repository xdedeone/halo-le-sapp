#include "CustomCommandFactory.h"

#include "Addresses.h"
#include "ArgContainer.h"
#include "CommandFactory.h"
#include "CustomCommand.h"
#include "HaloUtilities.h"
#include "TextUtilities.h"

static std::wstring custom_commands_file;

void LoadCustomCommands()
{
	custom_commands_file = FormatWArgs(L"%hs\\sapp\\commands.txt", GametypesFolder);
	crFile(custom_commands_file);

	FILE *CommandsFile;
	_wfopen_s(&CommandsFile, custom_commands_file.c_str(), L"r, ccs=UTF-16LE");

	if (!CommandsFile)
	{
		PrintConsole(12, "Failed to open Custom Commands file!");
		return;
	}

	wchar_t buff[4096];

	size_t i = 0;
	while (fgetws(buff, 4096, CommandsFile))
	{
		size_t n;
		std::wstring line = buff;

		if ((n = line.find_first_not_of(L" \t")) != std::wstring::npos)
		{
			if (line[n] == ';')
				continue;
		}

		TrimSpaces(line);

		ArgContainer args(line);
		if (args.argc > 1)
		{
			std::vector <std::wstring> Args;

			size_t param = 1;
			while (args.argc > param - 1 && args[param][0] == '#')
			{
				Args.emplace_back(args[param]);
				param++;
			}

			int level;
			if (!ToNumber(args[param + 1], level) || level < -1 || level > 4)
				level = 4;

			if (!GlobalCommandFactory.Exists(args[0]))
			{
				GlobalCommandFactory.AddCommand(std::make_unique<CustomCommand>(std::move(args[0]), level, args[param], Args));
				i++;
			}
		}
	}

	fclose(CommandsFile);

	if (i != 0)
	{
		PrintConsole(15, FormatArgs("%u custom commands were loaded.", i).c_str());
	}
}

void SaveCustomCommand(const std::wstring& name, const std::wstring& cmd, const std::vector <std::wstring>& args, int level)
{
	FILE *CommandsFile;
	_wfopen_s(&CommandsFile, custom_commands_file.c_str(), L"a+, ccs=UTF-16LE");

	if (!CommandsFile)
	{
		PrintConsole(12, "Failed to open Custom Commands file!");
	}
	else
	{
		fwprintf_s(CommandsFile, L"\n%ls", name.c_str());
		for (size_t j = 0; j < args.size(); j++)
			fwprintf_s(CommandsFile, L" %ls", args[j].c_str());

		fwprintf_s(CommandsFile, L" \'%ls\' %d", cmd.c_str(), level);

		fclose(CommandsFile);
	}
}