#include "AimbotScan.h"

#include "Addresses.h"
#include "bans.h"
#include "Event.h"
#include "HaloUtilities.h"
#include "LogThread.h"
#include "MiscellaneousFunctions.h"
#include "NoLead.h"
#include "ObjectManagement.h"
#include "OnlineAdmins.h"
#include "Patcher.h"
#include "Prefs.h"
#include "TextUtilities.h"

const float TO_RAD = 57.29577951308232f; // 180/PI
const float QUARTER_PI = 0.785398163397448309616f;

DWORD LastBulletKill[16] = {};
long LastTeleport[16] = {};
DWORD WaitForCameraC[16] = {};

vect3f PrevCamera[16];
vect3f PrevHeadPos[16];
vect3f PrevLockedHeadPos[16];

DWORD Expected[16];

bool PrevLock[16] = {};
DWORD Locked[16] = {};

DWORD PendingScores[16] = {};
DWORD AimbotScores[16] = {};

void LoadAimbotScan()
{
	for(int i = 0; i < 16; i++) 
	{
		Expected[i] = i;

		if (ServerRunning() && PlayerAlive(i))
		{
			Dynamic_Player* Player = ObjectTable[PlayerTable[i].ObjectIndexNum].Offset;
			PrevCamera[i] = Player->Camera;
			PrevHeadPos[i] = Player->PlayerVectors.Pos;
		}
	}
}

void AimbotScan()
{
	for(size_t i = 0; i < 16; i++)
	{
		if(PlayerAliveOnly(i))
		{
			Dynamic_Player* Player = ObjectTable[PlayerTable[i].ObjectIndexNum].Offset;

			LastBulletKill[i]++;
			LastTeleport[i]++;

			if (LastTeleport[i] > 16)
			{
				vect3f Current = Player->HeadCoord;
				bool found = false;
				for (size_t j = 0; j < 16; j++)
				{
					if (i != j && PlayerAliveOnly(j))
					{
						Dynamic_Player* tPlayer = ObjectTable[PlayerTable[j].ObjectIndexNum].Offset;
						vect3f Target = tPlayer->HeadCoord - tPlayer->PlayerVectors.Mov * (int)PingIndex[i];
						vect3f v1(Target - Current);

						float Deg = GetCameraAngle(Player->Camera, v1);

						if (Deg < QUARTER_PI)
						{
							float Dist = Get3DDistance(Target, Current);

							if (tan(Deg) * Dist < 0.2f)
							{
								found = true;

								bool Wall = false;

								vect3f trans = v1 * 1000.0f;
								vect3f out;
								DWORD unit_hit = 0;
								if (Intersect(PlayerTable[i].ObjectIndex, Current, trans, out, unit_hit))
								{
									if (unit_hit != PlayerTable[j].ObjectIndex)
									{
										Wall = true;
									}
								}
								
								if (PrevLock[i])
								{
									vect3f v0(PrevLockedHeadPos[i] - PrevHeadPos[i]);
									float CameraChange = GetCameraAngle(v1, v0);
									if (CameraChange > (0.05f / Dist) + 0.0005f && Locked[i]++ > 3)
									{
										float Scale = Dist * 5;
										if (Player->Zoomed == 0x00)
											Scale /= 2;
										else if (Player->Zoomed == 0x01)
											Scale /= 8;

										if (Wall)
											Scale *= 2;

										DWORD score = (DWORD)(CameraChange * Dist * Scale * (((float)Locked[i] / 300) + 1));
										if (score > 3)
										{
											AimbotScores[i] += score;
											if (GlobalPrefs->DebugStrings)
												sv_say(FormatWArgs(L"%ls is locking on %ls, score: %u, total score: %u", PlayerTable[i].name, PlayerTable[j].name, score, AimbotScores[i]).c_str());
										}
									}
								}
								
								if (!PendingScores[i])
								{
									float Snap = GetCameraAngle(Player->Camera, PrevCamera[i]);
									if (Snap > (1.0f / Dist) + 0.04f)
									{
										DWORD Score = DWORD(Dist * Snap * TO_RAD);
										if (GlobalPrefs->DebugStrings)
											sv_say(FormatWArgs(L"%ls snapped to %ls - %.1f° from %.1f units", PlayerTable[i].name, PlayerTable[j].name, Snap * TO_RAD, Dist).c_str());

										if (Expected[i] == j)
										{
											if (GlobalPrefs->DebugStrings)
												sv_say(L"- Expected");
											Score *= 2;
										}

										if (LastBulletKill[i] < 16)
										{
											if (GlobalPrefs->DebugStrings)
												sv_say(L"- After a kill");
											Score *= 2;
										}

										if (Wall)
										{
											if (GlobalPrefs->DebugStrings)
												sv_say(L"- Through the wall");
											Score *= 2;
										}

										PendingScores[i] = Score;

										WaitForCameraC[i] = 2;
									}
								}
								else if (!WaitForCameraC[i])
								{
									AimbotScores[i] += PendingScores[i];

									if (GlobalPrefs->DebugStrings)
										sv_say(FormatWArgs(L"Snap Confirmed: +%u score, total score: %u", PendingScores[i], AimbotScores[i]).c_str());

									std::vector <std::pair<std::wstring, std::wstring>> EventArgs;
									EventArgs.emplace_back(L"$snapscore", ToWStr((int)PendingScores[i]));
									GlobalEventHandler.Execute(i, EVENT_SNAP, EventArgs);

									PendingScores[i] = 0;
								}

								PrevLock[i] = true;
								PrevLockedHeadPos[i] = Target;

								if (AimbotScores[i] > GlobalPrefs->AimbotScanScoreLimit)
								{
									AimbotScores[i] = 0;

									if (GlobalPrefs->AimbotScanBanType < 3)
									{
										sv_say(FormatWArgs(L"%ls was banned by the server for aimbotting", PlayerTable[i].name).c_str());

										if (GlobalPrefs->AimbotScanBanType == 1 || GlobalPrefs->AimbotScanBanType == 2)
										{
											WriteLog(L"IPBAN", FormatWArgs(L"%ls was IP banned by the server for aimbotting", PlayerTable[i].name));
											int PlayerNumber = IndexToNumber(i);
											if (PlayerNumber != -1)
											{
												NetInfoStruct* info = **Server->MachineInfo[PlayerNumber].IPAddress;
												if (info != NULL)
												{
													GlobalIPBanlist.LoadIPBans();
													GlobalIPBanlist.BanIP(info->IP, 0xFFFFFFFF, 32, time(0) + GlobalPrefs->AimbotScanBanLength * 60, PlayerTable[i].name, L"AIMBOT");
													GlobalIPBanlist.SaveIPBans();
												}
											}
											if (!GlobalPrefs->FullIPBan)
												sv_kick(i);
										}

										if (GlobalPrefs->AimbotScanBanType == 0 || GlobalPrefs->AimbotScanBanType == 2)
										{
											WriteLog(L"BAN", FormatWArgs(L"%ls was CD-Key banned by the server for aimbotting", PlayerTable[i].name));
											sv_ban(i, FormatArgs("%u", GlobalPrefs->AimbotScanBanLength * 60).c_str());
										}
									}
									else if (GlobalPrefs->AimbotScanBanType == 3)
									{
										std::wstring wstr = FormatWArgs(L"%ls was kicked by the server for aimbotting", PlayerTable[i].name);
										sv_say(wstr.c_str());
										WriteLog(L"KICK", move(wstr));
										sv_kick(i);
									}
								}

								break;
							}
						}
					}
				}
						
				if (!found)
				{
					PendingScores[i] = 0;

					PrevLock[i] = false;

					if (Locked[i] > 0)
						Locked[i]--;	
				}

				if (WaitForCameraC[i])
					WaitForCameraC[i]--;
			}

			PrevCamera[i] = Player->Camera;
			PrevHeadPos[i] = Player->HeadCoord;

			float Deg = 3.15f;
			vect3f Me = Player->HeadCoord;

			for (size_t j = 0; j < 16; j++)
			{
				if (PlayerTable[i].Team != PlayerTable[j].Team && PlayerAliveOnly(j))
				{
					Dynamic_Player* tPlayer = ObjectTable[PlayerTable[j].ObjectIndexNum].Offset;
					vect3f Target = tPlayer->HeadCoord;
					vect3f v(Target - Me);

					float tmpDeg = GetCameraAngle(Player->Camera, v);

					if (tmpDeg < Deg)
					{
						Deg = tmpDeg;
						Expected[i] = j;
					}
				}
			}
		}
	}
}