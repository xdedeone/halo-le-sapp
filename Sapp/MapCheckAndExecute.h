#pragma once

extern bool MapExecuted;
extern bool MapVoteCanceled;

bool CheckMap(const char* map, const char* mode);
bool RunMap(const char* Map, const char* Mode);

void Install_sv_map_Handler();