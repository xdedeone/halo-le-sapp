#pragma once

#include "SuperHeader.h" 

extern vect3f PrevCamera[16];
extern vect3f PrevHeadPos[16];

extern bool PrevLock[16];
extern DWORD Locked[16];

extern DWORD PendingScores[16];
extern DWORD AimbotScores[16];

extern DWORD LastBulletKill[16];
extern long LastTeleport[16];

void LoadAimbotScan();
void AimbotScan();