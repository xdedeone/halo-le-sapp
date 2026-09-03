#pragma once

#include <string>

enum NetworkEventType { NE_Anticheat, NE_DataSync, NE_HUD_Color, NE_NavPoint, NE_Message, NE_Medal };

enum NE_AnticheatType { NE_Anticheat_Session };
enum NE_DataSyncType { NE_DataSync_Handshake, NE_DataSync_Reserved, NE_DataSync_GameSpeed, NE_DataSync_Gravity, NE_DataSync_MTV, NE_DataSync_SJ, NE_DataSync_Zombies, NE_DataSync_TimeLeft, NE_DataSync_TimeElapsed };
enum NE_HUD_ColorType { NE_HUD_Color_Reset, NE_HUD_Color_Reticle, NE_HUD_Color_Primary, NE_HUD_Color_Secondary, NE_HUD_Color_Tertiary };
enum NE_NavPointType { NE_NavPoint_Reset, NE_NavPoint_Static };
enum NE_MessageType { NE_Message_Chat, NE_Message_Rcon, NE_Message_HUD, NE_Message_Sound, NE_Message_Custom_Sound };
enum NE_MedalType { NE_Medal_Kill };

enum AppType { APP_Nothing = 0, APP_SAPP = 1, APP_AC = 2, APP_HAC2 = 4, APP_Reflex = 8 };

#pragma pack(push, 1)

struct ColorRGBA
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
};

struct NetworkEventBase
{
	uint8_t Type;
	uint8_t Opcode;

	size_t size() const { return sizeof(*this); }

	NetworkEventBase(uint8_t type, uint8_t opcode) : Type(type), Opcode(opcode) {}
};

struct ACSession final : public NetworkEventBase
{
	uint32_t SessionKey;

	size_t size() const { return sizeof(*this); }

	ACSession(uint32_t sessionkey) : NetworkEventBase(NE_Anticheat, NE_Anticheat_Session), SessionKey(sessionkey) {}
};

struct DSHandshake final : public NetworkEventBase
{
	uint32_t App;
	uint32_t Version;

	size_t size() const { return sizeof(*this); }

	DSHandshake(uint32_t app, uint32_t version) : NetworkEventBase(NE_DataSync, NE_DataSync_Handshake), App(app), Version(version) {}
};

template <class T>
struct DataSync final : public NetworkEventBase
{
	T Data;

	size_t size() const { return sizeof(*this); }

	DataSync(uint8_t what, T data) : NetworkEventBase(NE_DataSync, what), Data(data) {}
};

struct HUD_Color final : public NetworkEventBase
{
	ColorRGBA color;

	size_t size() const { return sizeof(*this); }

	HUD_Color(NE_HUD_ColorType type, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
		: NetworkEventBase(NE_HUD_Color, type), color{ red, green, blue, alpha } {}
};

struct NavPoint_Static final : public NetworkEventBase
{
	uint8_t primary_type;
	uint8_t secondary_type;
	uint8_t tertiary_type;

	size_t size() const { return sizeof(*this); }

	NavPoint_Static(uint8_t primary, uint8_t secondary, uint8_t tertiary)
		: NetworkEventBase(NE_NavPoint, NE_NavPoint_Static), primary_type(primary), secondary_type(secondary), tertiary_type(tertiary) {}
};

struct EventChatText final : public NetworkEventBase
{
	ColorRGBA color;
	wchar_t message[256];

	size_t size() const { return sizeof(*this) - sizeof(message) + (wcslen(message) + 1) * sizeof(wchar_t); }

	EventChatText(const std::wstring& text, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
		: NetworkEventBase(NE_Message, NE_Message_Chat), color{ red, green, blue, alpha }
	{
		wcscpy_s(message, text.c_str());
	}
};

struct EventRconText final : public NetworkEventBase
{
	ColorRGBA color;
	char message[64];

	size_t size() const { return sizeof(*this) - sizeof(message) + (strlen(message) + 1) * sizeof(char); }

	EventRconText(const char* text, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
		: NetworkEventBase(NE_Message, NE_Message_Rcon), color{ red, green, blue, alpha }
	{
		strcpy_s(message, text);
	}
};

struct EventHUDText final : public NetworkEventBase
{
	ColorRGBA color;
	char message[64];

	size_t size() const { return sizeof(*this) - sizeof(message) + (strlen(message) + 1) * sizeof(char); }

	EventHUDText(const char* text, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
		: NetworkEventBase(NE_Message, NE_Message_HUD), color{ red, green, blue, alpha }
	{
		strcpy_s(message, text);
	}
};

struct Event_Sound final : public NetworkEventBase
{
	uint8_t index;

	size_t size() const { return sizeof(*this); }

	Event_Sound(uint8_t index) : NetworkEventBase(NE_Message, NE_Message_Sound), index(index) {}
};

struct Event_Custom_Sound final : public NetworkEventBase
{
	uint8_t index;

	size_t size() const { return sizeof(*this); }

	Event_Custom_Sound(uint8_t index) : NetworkEventBase(NE_Message, NE_Message_Custom_Sound), index(index) {}
};

struct Medal_Kill final : public NetworkEventBase
{
	uint8_t Killed;
	uint8_t Killer;
	uint16_t Flags;
	uint32_t MetaID;

	size_t size() const { return sizeof(*this); }

	Medal_Kill(uint8_t killed, uint8_t killer, uint8_t flags, uint32_t metaid)
		: NetworkEventBase(NE_Medal, NE_Medal_Kill), Killed(killed), Killer(killer), Flags(flags), MetaID(metaid) {}
};

#pragma pack(pop)