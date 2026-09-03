#pragma once

#include "GenericCommand.h"
#include "HaloUtilities.h"
#include "ObjectManagement.h"

class CommandBalanceTeams final : public GenericCommand
{
public:
	CommandBalanceTeams() : GenericCommand(L"balance_teams", L"", 2, ServerOnly::yes) {}

	bool Execute(const CommandEnviroment& env) override
	{
		if(!*TeamGame)
		{
			env_Print("There is no team game in progress.");
			return false;
		}

		int RealPlayerNumber = 0;
		size_t TakenSlotIndexes[16];

		GetPlayers(RealPlayerNumber, TakenSlotIndexes);

		if(RealPlayerNumber == 0)
		{
			env_Print("There are no players in the server.");
			return true;
		}

		float Scores[16];
		float reds_score = 0, blues_score = 0;
		float tmp_score;
		size_t tmp_index;

		for(int i = 0; i < RealPlayerNumber; i++)
			Scores[i] = (float)PlayerTable[TakenSlotIndexes[i]].kills + (float)PlayerTable[TakenSlotIndexes[i]].assists*0.5f - ((float)PlayerTable[TakenSlotIndexes[i]].deaths + (float)PlayerTable[TakenSlotIndexes[i]].tk_number*2.0f);

		for(int i = 0; i < RealPlayerNumber-1; i++)
		{
			for(int j = i + 1; j < RealPlayerNumber; j++)
			{
				if(Scores[i] < Scores[j])
				{
					tmp_score = Scores[i];
					tmp_index = TakenSlotIndexes[i];
					Scores[i] = Scores[j];
					TakenSlotIndexes[i] = TakenSlotIndexes[j];
					Scores[j] = tmp_score;
					TakenSlotIndexes[j] = tmp_index;
				}
			}
		}

		for(int i = 0; i < RealPlayerNumber; i++)
			Scores[i] = sqrt((Scores[i] + abs(Scores[RealPlayerNumber-1]) + 1) * (((float)(PlayerTable[TakenSlotIndexes[i]].kills + 1)) / ((float)(PlayerTable[TakenSlotIndexes[i]].deaths + 1))));

		int n = 0;
		size_t team = 0;		
		size_t changes = 0;

		while(n != RealPlayerNumber)
		{
			if(team == 0)
			{
				while(reds_score <= blues_score && n < RealPlayerNumber)
				{
					if(PlayerTable[TakenSlotIndexes[n]].Team == 1)
						changes++;

					reds_score += Scores[n++];
				}
			}	
			else
			{
				while(blues_score <= reds_score && n < RealPlayerNumber)
				{
					if(PlayerTable[TakenSlotIndexes[n]].Team == 0)
						changes++;

					blues_score += Scores[n++];
				}
			}
			team = !team;
		}

		if(changes > (size_t)(RealPlayerNumber / 2))
			team = 1;
		else
			team = 0;

		n = 0;
		reds_score = 0;
		blues_score = 0;

		while(n != RealPlayerNumber)
		{
			if(team == 0)
			{
				while(reds_score <= blues_score && n < RealPlayerNumber)
				{
					if(PlayerTable[TakenSlotIndexes[n]].Team == 1)
					{
						Server->PlayerData[TakenSlotIndexes[n]].team = 0;
						PlayerTable[TakenSlotIndexes[n]].Team = 0;
						if (PlayerAliveOnly(TakenSlotIndexes[n]))
						{
							KillPlayer(TakenSlotIndexes[n]);
							PlayerTable[TakenSlotIndexes[n]].deaths--;
						}
					}
					reds_score += Scores[n++];
				}
			}		
			else
			{
				while(blues_score <= reds_score && n < RealPlayerNumber)
				{
					if(PlayerTable[TakenSlotIndexes[n]].Team == 0)
					{
						Server->PlayerData[TakenSlotIndexes[n]].team = 1;
						PlayerTable[TakenSlotIndexes[n]].Team = 1;
						if (PlayerAliveOnly(TakenSlotIndexes[n]))
						{
							KillPlayer(TakenSlotIndexes[n]);
							PlayerTable[TakenSlotIndexes[n]].deaths--;
						}
					}
					blues_score += Scores[n++];
				}
			}
			team = !team;
		}

		env_Print("The teams were balanced.");
		return true;
	}
};