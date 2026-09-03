#include <Windows.h>

void main()
{
	system("taskkill /im NetworkServer.exe");
	Sleep(60000);
	STARTUPINFO sinfo = {};
	PROCESS_INFORMATION info;
	CreateProcess("NetworkServer.exe", NULL, NULL, NULL, NULL, NULL, NULL, NULL, &sinfo, &info);
}