#include "NetworkClient.h"

#include "Windows.h"

#include "json\json.h"

#include <iostream>

using namespace std;

enum RemoteConsoleOpcode { RC_LOGIN = 1, RC_QUERY, RC_QUERY_STATS, RC_QUERY_POS, RC_CIN, RC_COUT, RC_CHAT, RC_PJOIN, RC_PLEAVE, RC_TEAMCHANGE, RC_NEWGAME };

void main()
{
	Connect("localhost");

	string out;

	Json::Value root;
	root["opcode"] = RC_LOGIN;
	root["username"] = "lol";
	root["password"] = "81dc9bdb52d04dc20036dbd8313ed055";

	Json::FastWriter writer;
	out += writer.write(root);

	root.clear();
	root["opcode"] = RC_CIN;
	root["command"] = "info";

	out += writer.write(root);

	root["command"] = "sv_players";

	out += writer.write(root);

	root.clear();
	root["opcode"] = RC_QUERY;

	out += writer.write(root);

	SendData(out);

	cout << out << endl;

	root.clear();
	root["opcode"] = RC_QUERY_POS;

	out = writer.write(root);

	cout << out << endl;

	while (1)
	{
		SendData(out);
		Sleep(1000);
	}
}