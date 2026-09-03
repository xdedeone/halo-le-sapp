#include "Patches.h"

#include "Assist.h"
#include "BlockCheatSpawns.h"
#include "CDKeyCheck.h"
#include "chat.h"
#include "ClientUpdateFixes.h"
#include "ComboFix.h"
#include "ConsolePatch.h"
#include "ConsolePrintHook.h"
#include "DamageApplicationHook.h"
#include "EventScan.h"
#include "GameStart-End.h"
#include "GSQueryPatch.h"
#include "HaloThreadHook.h"
#include "HillTimer.h"
#include "Killed.h"
#include "MapCheckAndExecute.h"
#include "MapLoader.h"
#include "MapReset.h"
#include "MotdHook.h"
#include "mtv.h"
#include "NetworkEvents.h"
#include "NetworkHook.h"
#include "ObjectCreation.h"
#include "ObjectInteractions.h"
#include "ObjectSyncCleanup.h"
#include "Patcher.h"
#include "PingUpdate.h"
#include "PlayerJoin-Leave.h"
#include "PlayerQuitKillFix.h"
#include "rcon.h"
#include "Score.h"
#include "Spawn.h"
#include "Stick.h"
#include "TabCompletion.h"
#include "TeamBalance.h"
#include "TeamChange.h"
#include "Teleport.h"
#include "ThreadCycleFix.h"
#include "UnloadHook.h"
#include "VehicleEnter-Exit.h"
#include "VehicleRespawnHandler.h"
#include "WeaponPickup-Drop.h"

void InstallCCs()
{
	InstallMapTableHandler();
	InstallNetworkHook();
	InstallConsolePrintHook();
	InstallRconHandler();
	InstallChatHandler();
	InstallHaloThreadHook();
	InstallGameStartHandler();
	InstallGameEndHandler();
	InstallMapResetHandler();
	InstallMJoinHandler();
	InstallMLeaveHandler();
	InstallCleanMachinesHandler();
	InstallPJoinHandler();
	InstallEventSpawnHandler();
	InstallEventScoreHandler();
	InstallEventKilledHandler();	
	InstallTeamChangeHandler();
	InstallWeaponPickupHandler();
	InstallNadePickupHandler();
	InstallWeaponDropHandler();
	InstallVehicleEnterHandler();
	InstallVehicleExitHandler();
	InstallObjectInteractionHandler();
	InstallEventAssistHandler();
	InstallTeleportHandler();
	InstallCDKeyCheckHandler();
	InstallTBHandler();
	InstallVehicleRespawnHandler();
	InstallPlayerQuitKillHandler();
	InstallDamageApplicationHook();
	InstallStickHook();
	InstallClientUpdateFixes();
	InstallBuildChatPacketHook();
	InstallBlockCheatSpawnHooks();
	InstallGSQueryFixHandler();
	InstallComboFix();
	InstallObjectSyncCleanupHandler();
#ifndef HPC
	InstallMotdHook();
#endif
	InstallPingUpdateHook();
	InstallTabCompletionHook();
	InstallTCycleFix();	
	Install_sv_map_Handler();
	InstallObjectCreationHandler();
	InstallHillTimerHook();
	//InstallLagFixHook();
	InstallUnloadHook();
}

void ClearPatches()
{
	UninstallMapTableHandler();
	Patches.RemoveAll();
}