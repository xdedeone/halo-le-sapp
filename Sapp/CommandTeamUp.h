#pragma once

#include "GenericCommand.h"
#include "ObjectManagement.h"

class CommandTeamUp final : public GenericCommand
{
public:
	CommandTeamUp() : GenericCommand(L"teamup", L"", 2, ServerOnly::yes) {}

	bool Execute(const CommandEnviroment& env) override
	{
		if (!*TeamGame)
		{
			env_Print("There is no team game in progress.");
			return false;
		}

		int RealPlayerNumber = 0;
		size_t TakenSlotIndexes[16];

		GetPlayers(RealPlayerNumber, TakenSlotIndexes);

		if (RealPlayerNumber < 2)
		{
			env_Print("At least 2 players are required to use this function.");
			return false;
		}

		wchar_t names[16][12] = {};
		int MachMatrix[16][16] = {};
		int PairMatrix[16][16] = {};
		int teams[16];

		for (int i = 0; i < RealPlayerNumber; i++)
		{
			for (int j = 0, k = 0; j < 12; j++)
			{
				wchar_t c = PlayerTable[TakenSlotIndexes[i]].name[j];
				if (c == 0)
					break;
				if (c != ' ')
				{
					if (c >= 'A' && c <= 'Z')
						c += 32;
					names[i][k++] = c;
				}	
			}
		}

		for (int i = 0; i < RealPlayerNumber; i++)
		{
			for (int j = 0; j < RealPlayerNumber; j++)
			{
				if (i != j)
				{
					wchar_t c01 = names[i][0];
					wchar_t c02 = names[j][0];

					if (!iswalnum(c01) && !iswalnum(c02))
					{
						int found_a = 0;
						int found_b = 0;
						for (int k = 1; k < 6; k++)
						{
							wchar_t c1 = names[i][k];
							wchar_t c2 = names[j][k];
							if (c1 == 0 || c2 == 0)
								break;
							if (!iswalnum(c1) && abs(c01 - c1) < 3)
								found_a = k;
							if (!iswalnum(c2) && abs(c02 - c2) < 3)
								found_b = k;
							if (found_a || found_b)
								break;
						}
						if (found_a > 1 && found_b > 1)
						{
							for (int k = 1; k < found_a; k++)
							{
								wchar_t c1 = names[i][k];
								wchar_t c2 = names[j][k];

								if (c1 == c2)
								{
									MachMatrix[i][j] += 12;
								}
								else
								{
									MachMatrix[i][j] -= 6;
								}
							}
							if (MachMatrix[i][j] < 0)
								MachMatrix[i][j] = 0;

							continue;
						}
						else if (c01 == c02 && !found_a && !found_b)
						{
							MachMatrix[i][j] += 12;
						}
					}
					for (int k = 0; k < 6; k++)
					{
						wchar_t c1 = names[i][k];
						wchar_t c2 = names[j][k];
						if (c1 == 0 || c2 == 0)
							break;

						if (c1 == c2)
						{
							MachMatrix[i][j] += 12 - k;
						}
						else if (MachMatrix[i][j] > 0)
							MachMatrix[i][j]--;
					}
				}
			}
		}

		for (int i = 0; i < RealPlayerNumber; i++)
		{
			for (int j = 0; j < RealPlayerNumber; j++)
			{
				if (i != j)
				{
					for (int k = 0; k < RealPlayerNumber; k++)
					{
						if ((k == i || MachMatrix[k][i] > 0) && (k == j || MachMatrix[k][j] > 0))
						{
							PairMatrix[i][j]++;
						}
					}
				}
			}
		}

		for (int i = 0; i < RealPlayerNumber; i++)
		{
			for (int j = 0; j < RealPlayerNumber; j++)
			{
				MachMatrix[i][j] *= PairMatrix[i][j];
			}
		}

		for (int i = 0; i < RealPlayerNumber; i++)
		{
			MachMatrix[i][i] = -1;
			int avg = 0;
			for (int j = 0; j < RealPlayerNumber; j++)
			{
				if (MachMatrix[i][j] > 0)
				{
					avg += MachMatrix[i][j];
				}
			}

			avg /= RealPlayerNumber;

			for (int n = 0; n < RealPlayerNumber; n++)
			{
				int max = i;
				for (int j = 0; j < RealPlayerNumber; j++)
				{
					if (MachMatrix[i][j] > MachMatrix[i][max])
						max = j;
				}

				if (MachMatrix[i][max] <= avg)
					break;
				else
					MachMatrix[i][max] = -1;
			}
			for (int j = 0; j < RealPlayerNumber; j++)
			{
				if (MachMatrix[i][j] > -1)
					MachMatrix[i][j] = -2;
			}
		}
		
		for (int i = 0; i < RealPlayerNumber; i++)
		{
			teams[i] = 1;
			for (int j = i + 1; j < RealPlayerNumber; j++)
			{
				bool ok = true;
				for (int n = 0; n < RealPlayerNumber; n++)
				{
					if (MachMatrix[i][n] != MachMatrix[j][n])
					{
						ok = false;
						break;
					}
				}
				if (ok)
				{
					teams[i]++;
				}
				else
				{
					ok = true;
					for (int n = 0; n < RealPlayerNumber; n++)
					{
						if (MachMatrix[i][n] == MachMatrix[j][n])
						{
							ok = false;
							break;
						}
					}
					if (ok)
					{
						teams[i]++;
					}
				}
			}
		}

		int max = 0;
		for (int i = 0; i < RealPlayerNumber; i++)
		{
			if (teams[i] > teams[max])
				max = i;
		}

		for (int i = 0; i < RealPlayerNumber; i++)
		{
			if (MachMatrix[max][i] == -1)
			{
				if (PlayerTable[TakenSlotIndexes[i]].Team == 1)
				{
					Server->PlayerData[TakenSlotIndexes[i]].team = 0;
					PlayerTable[TakenSlotIndexes[i]].Team = 0;
					if (PlayerAliveOnly(TakenSlotIndexes[i]))
					{
						KillPlayer(TakenSlotIndexes[i]);
						PlayerTable[TakenSlotIndexes[i]].deaths--;
					}
				}
			}
			else
			{
				if (PlayerTable[TakenSlotIndexes[i]].Team == 0)
				{
					Server->PlayerData[TakenSlotIndexes[i]].team = 1;
					PlayerTable[TakenSlotIndexes[i]].Team = 1;
					if (PlayerAliveOnly(TakenSlotIndexes[i]))
					{
						KillPlayer(TakenSlotIndexes[i]);
						PlayerTable[TakenSlotIndexes[i]].deaths--;
					}
				}
			}
		}

		env_Print("Team-up done!");

		return true;
	}
};