#include "NetworkClient.h"
#include <Windows.h>

void main()
{
	WSInit();

	NetworkClient* Client = new NetworkClient();
	if(Client->Connect())
	{
		printf("Connected to the Global Server.\n");

		Client->Ping();
		Client->GetIP();
		Client->VersionCheck();
		Client->GetClientNumber();
		Client->ListClients();
	}
	else
		printf("Failed to connect to the Global Server.\n");

	char buff[256];	
	while(true)
	{
		gets_s(buff, 255);
		if(!strcmp(buff, "reboot"))
		{
			Client->RebootServer();
			Sleep(10000);
		}
		else if(!strcmp(buff, "quit"))
			break;
	}

	Client->Disconnect();

	WSShutDown();
}