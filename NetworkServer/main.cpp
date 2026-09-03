#include "NetworkServer.h"

using namespace std;

unique_ptr<NetworkServer> Server;

//void SpamThread()
//{
//	while (true)
//	{
//		Server->BroadcastACServers("AntiCheat will require new strings.dll to start.");
//		Sleep(5000);
//		Server->BroadcastACServers("Download the new one from xhalo.tk/anticheat");
//		Sleep(900000);
//		Server->BroadcastACServers("AntiCheat necesitar? nuevas strings.dll para empezar.");
//		Sleep(5000);
//		Server->BroadcastACServers("Descargue el nuevo por xhalo.tk/anticheat");
//		Sleep(900000);
//	}
//}

void CleanUpFunc()
{
	Server->StopServer();
}

void main() try
{
#ifndef DEBUG
	Server = make_unique<NetworkServer>(2501);
#else
	Server = make_unique<NetworkServer>(1337);
#endif
	Server->StartServer();

	printf("Successfully initialised the server.\n");

	SetConsoleCtrlHandler((PHANDLER_ROUTINE)CleanUpFunc, TRUE);

	//std::thread(SpamThread).detach();

	string buff;
	while(true)
	{
		getline(cin, buff);
		if (!_stricmp("reload", buff.c_str()))
		{
			Server->CacheFiles();
		}
		else if (!_stricmp("newsapp", buff.c_str()))
		{
			string msg;
			Server->LockConsole();
			getline(cin, msg);
			Server->NewSapp(msg);
			Server->UnlockConsole();
			Server->CacheFiles();
		}
		else if (!_stricmp("list", buff.c_str()))
		{
			Server->ListServers();
		}
		else if (!_stricmp("flist", buff.c_str()))
		{
			Server->ListServersToFile();
		}
		else if (!_stricmp("mlist", buff.c_str()))
		{
			Server->ListMaps();
		}
		else if (!_stricmp("bc", buff.c_str()))
		{
			string msg;
			Server->LockConsole();
			getline(cin, msg);
			Server->BroadcastServers(msg);
			Server->UnlockConsole();
		}
		else if (!_stricmp("bcacs", buff.c_str()))
		{
			string msg;
			Server->LockConsole();
			getline(cin, msg);
			Server->BroadcastACServers(msg);
			Server->UnlockConsole();
		}
		else if (!_stricmp("bcac", buff.c_str()))
		{
			string msg;
			Server->LockConsole();
			getline(cin, msg);
			Server->BroadcastAC(msg);
			Server->UnlockConsole();
		}
		else if (!_stricmp("quit", buff.c_str()))
			break;
	}

	CleanUpFunc();
}
catch (exception& e) {
	cerr << e.what() << endl;
}