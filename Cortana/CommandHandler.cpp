#include "CommandHandler.h"

#include "Addresses.h"
#include "ArgContainer.h"
#include "FoVChange.h"
#include "NetworkClient.h"

bool __stdcall CommandHandler(const char* buff)
{
	std::string command(buff);
	ArgContainer arg(command);

	if (!_stricmp(arg[0].c_str(), "fov"))
	{
		float f = (float)atof(arg[1].c_str());
		if (f > 0 && f < 180)
		{
			NewFoV = f * 0.0174532925f;
		}
		return true;
	}
	else if (!_stricmp(arg[0].c_str(), "alias"))
	{
		if (!SappServer && arg[1].size() && arg[1].size() < 32)
		{
			myClient->SendBigEvent(event_alias, const_cast<char*>(arg[1].c_str()), arg[1].size());
			return true;
		}
	}
	return false;
}