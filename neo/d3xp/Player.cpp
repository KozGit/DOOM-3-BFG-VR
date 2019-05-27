/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "precompiled.h"
#pragma hdrstop

#include "Game_local.h"
#include "../framework/Common_local.h"
#include "PredictedValue_impl.h"

#include "vr/Vr.h" // Koz

#define	ANGLE2SHORT(x)			( idMath::Ftoi( (x) * 65536.0f / 360.0f ) & 65535 )

idCVar flashlight_batteryDrainTimeMS( "flashlight_batteryDrainTimeMS", "30000", CVAR_INTEGER, "amount of time (in MS) it takes for full battery to drain (-1 == no battery drain)" );
idCVar flashlight_batteryChargeTimeMS( "flashlight_batteryChargeTimeMS", "3000", CVAR_INTEGER, "amount of time (in MS) it takes to fully recharge battery" );
idCVar flashlight_minActivatePercent( "flashlight_minActivatePercent", ".25", CVAR_FLOAT, "( 0.0 - 1.0 ) minimum amount of battery (%) needed to turn on flashlight" );
idCVar flashlight_batteryFlickerPercent( "flashlight_batteryFlickerPercent", ".1", CVAR_FLOAT, "chance of flickering when battery is low" );

// No longer userinfo, but I don't want to rename the cvar
idCVar ui_showGun( "ui_showGun", "1", CVAR_GAME | CVAR_ARCHIVE | CVAR_BOOL, "show gun" );

// Client-authoritative stuff
idCVar pm_clientAuthoritative_debug( "pm_clientAuthoritative_debug", "0", CVAR_BOOL, "" );
idCVar pm_controllerShake_damageMaxMag( "pm_controllerShake_damageMaxMag", "60.0f", CVAR_FLOAT, "" );
idCVar pm_controllerShake_damageMaxDur( "pm_controllerShake_damageMaxDur", "60.0f", CVAR_FLOAT, "" );

idCVar pm_clientAuthoritative_warnDist( "pm_clientAuthoritative_warnDist", "100.0f", CVAR_FLOAT, "" );
idCVar pm_clientAuthoritative_minDistZ( "pm_clientAuthoritative_minDistZ", "1.0f", CVAR_FLOAT, "" );
idCVar pm_clientAuthoritative_minDist( "pm_clientAuthoritative_minDist", "-1.0f", CVAR_FLOAT, "" );
idCVar pm_clientAuthoritative_Lerp( "pm_clientAuthoritative_Lerp", "0.9f", CVAR_FLOAT, "" );

idCVar pm_clientAuthoritative_Divergence( "pm_clientAuthoritative_Divergence", "200.0f", CVAR_FLOAT, "" );
idCVar pm_clientInterpolation_Divergence( "pm_clientInterpolation_Divergence", "5000.0f", CVAR_FLOAT, "" );

idCVar pm_clientAuthoritative_minSpeedSquared( "pm_clientAuthoritative_minSpeedSquared", "1000.0f", CVAR_FLOAT, "" );

idCVar vr_wipScale( "vr_wipScale", "1.0", CVAR_FLOAT | CVAR_ARCHIVE, "" );

idCVar vr_debugGui( "vr_debugGui", "0", CVAR_BOOL, "" );
idCVar vr_guiFocusPitchAdj( "vr_guiFocusPitchAdj", "7", CVAR_FLOAT | CVAR_ARCHIVE, "View pitch adjust to help activate in game touch screens" );

idCVar vr_bx1( "vr_bx1", "5", CVAR_FLOAT, "");
idCVar vr_bx2( "vr_bx2", "5", CVAR_FLOAT, "" );
idCVar vr_by1( "vr_by1", "1", CVAR_FLOAT, "" );
idCVar vr_by2( "vr_by2", "0", CVAR_FLOAT, "" );
idCVar vr_bz1( "vr_bz1", "0", CVAR_FLOAT, "" );
idCVar vr_bz2( "vr_bz2", "0", CVAR_FLOAT, "" );

idCVar vr_teleportVel( "vr_teleportVel", "650", CVAR_FLOAT,"" );
idCVar vr_teleportDist( "vr_teleportDist", "60", CVAR_FLOAT,"" );
idCVar vr_teleportMaxPoints( "vr_teleportMaxPoints", "24", CVAR_FLOAT, "" );
idCVar vr_teleportMaxDrop( "vr_teleportMaxDrop", "360", CVAR_FLOAT, "" );

idCVar vr_laserSightUseOffset( "vr_laserSightUseOffset", "1", CVAR_BOOL | CVAR_ARCHIVE, " 0 = lasersight emits straight from barrel.\n 1 = use offsets from weapon def" );


// for testing
idCVar ftx( "ftx", "0", CVAR_FLOAT, "" );
idCVar fty( "fty", "0", CVAR_FLOAT, "" );
idCVar ftz( "ftz", "0", CVAR_FLOAT, "" );


extern idCVar g_demoMode;

const idVec3 neckOffset( -3, 0, -6 );
const int waistZ = -22.f;

idCVar vr_slotDebug( "vr_slotDebug", "0", CVAR_BOOL, "slot debug visualation" );
idCVar vr_slotMag( "vr_slotMag", "0.1", CVAR_FLOAT | CVAR_ARCHIVE, "slot vibration magnitude (0 is off)" );
idCVar vr_slotDur( "vr_slotDur", "18", CVAR_INTEGER | CVAR_ARCHIVE, "slot vibration duration in milliseconds" );
idCVar vr_slotDisable( "vr_slotDisable", "0", CVAR_BOOL | CVAR_ARCHIVE, "slot disable" );

slot_t slots[ SLOT_COUNT ] = {
	{ idVec3( 0, 10, -4 ), 9.0f * 9.0f },
	{ idVec3( 0, -10, -4 ), 9.0f * 9.0f },
	{ idVec3( -9, -4, 4 ), 9.0f * 9.0f },
	{ idVec3( -9, -4,-waistZ - neckOffset.z ), 9.0f * 9.0f },
	{ idVec3( 4, 8, -waistZ + 2 ), 9.0f * 9.0f },
	{ idVec3( -neckOffset.x, 0, -waistZ - neckOffset.z + 7 ), 9.0f * 9.0f },
};

idAngles pdaAngle1( 0, -90, 0);
idAngles pdaAngle2( 0, 0, 76.5);
idAngles pdaAngle3( 0, 0, 0);

extern idCVar g_useWeaponDepthHack;
extern idCVar in_independentAim;


/*
===============================================================================

	Player control of the Doom Marine.
	This object handles all player movement and world interaction.

===============================================================================
*/

// distance between ladder rungs (actually is half that distance, but this sounds better)
const int LADDER_RUNG_DISTANCE = 32;

// amount of health per dose from the health station
const int HEALTH_PER_DOSE = 10;

// time before a weapon dropped to the floor disappears
const int WEAPON_DROP_TIME = 20 * 1000;

// time before a next or prev weapon switch happens
const int WEAPON_SWITCH_DELAY = 150;

// how many units to raise spectator above default view height so it's in the head of someone
const int SPECTATE_RAISE = 25;

const int HEALTHPULSE_TIME = 333;

// minimum speed to bob and play run/walk animations at
const float MIN_BOB_SPEED = 5.0f;

// Special team used for spectators that we ONLY store on lobby.  The local team property on player remains as 0 or 1.
const float LOBBY_SPECTATE_TEAM_FOR_VOICE_CHAT = 2;

const idEventDef EV_Player_GetButtons( "getButtons", NULL, 'd' );
const idEventDef EV_Player_GetMove( "getMove", NULL, 'v' );
const idEventDef EV_Player_GetViewAngles( "getViewAngles", NULL, 'v' );
const idEventDef EV_Player_StopFxFov( "stopFxFov" );
const idEventDef EV_Player_EnableWeapon( "enableWeapon" );
const idEventDef EV_Player_DisableWeapon( "disableWeapon" );
const idEventDef EV_Player_GetCurrentWeapon( "getCurrentWeapon", NULL, 's' );
const idEventDef EV_Player_GetPreviousWeapon( "getPreviousWeapon", NULL, 's' );
const idEventDef EV_Player_SelectWeapon( "selectWeapon", "s" );
const idEventDef EV_Player_GetWeaponEntity( "getWeaponEntity", NULL, 'e' );
const idEventDef EV_Player_OpenPDA( "openPDA" );
const idEventDef EV_Player_InPDA( "inPDA", NULL, 'd' );
const idEventDef EV_Player_ExitTeleporter( "exitTeleporter" );
const idEventDef EV_Player_StopAudioLog( "stopAudioLog" );
const idEventDef EV_Player_HideTip( "hideTip" );
const idEventDef EV_Player_LevelTrigger( "levelTrigger" );
const idEventDef EV_SpectatorTouch( "spectatorTouch", "et" );
const idEventDef EV_Player_GiveInventoryItem( "giveInventoryItem", "s" );
const idEventDef EV_Player_RemoveInventoryItem( "removeInventoryItem", "s" );
const idEventDef EV_Player_GetIdealWeapon( "getIdealWeapon", NULL, 's' );
const idEventDef EV_Player_SetPowerupTime( "setPowerupTime", "dd" );
const idEventDef EV_Player_IsPowerupActive( "isPowerupActive", "d", 'd' );
const idEventDef EV_Player_WeaponAvailable( "weaponAvailable", "s", 'd' );
const idEventDef EV_Player_StartWarp( "startWarp" );
const idEventDef EV_Player_StopHelltime( "stopHelltime", "d" );
const idEventDef EV_Player_ToggleBloom( "toggleBloom", "d" );
const idEventDef EV_Player_SetBloomParms( "setBloomParms", "ff" );

// Koz begin - let scripts query which hand does what when using motion controls
const idEventDef EV_Player_GetWeaponHand( "getWeaponHand", NULL, 'd' );
const idEventDef EV_Player_GetFlashHand( "getFlashHand", NULL, 'd' ); // get flashlight hand
const idEventDef EV_Player_GetWeaponHandState( "getWeaponHandState", NULL, 'd' );
const idEventDef EV_Player_GetFlashHandState( "getFlashHandState", NULL, 'd' ); // get flashlight hand state
const idEventDef EV_Player_GetFlashState( "getFlashState", NULL, 'd' ); // get flashlight state

// Koz end

CLASS_DECLARATION( idActor, idPlayer )
EVENT( EV_Player_GetButtons,			idPlayer::Event_GetButtons )
EVENT( EV_Player_GetMove,				idPlayer::Event_GetMove )
EVENT( EV_Player_GetViewAngles,			idPlayer::Event_GetViewAngles )
EVENT( EV_Player_StopFxFov,				idPlayer::Event_StopFxFov )
EVENT( EV_Player_EnableWeapon,			idPlayer::Event_EnableWeapon )
EVENT( EV_Player_DisableWeapon,			idPlayer::Event_DisableWeapon )
EVENT( EV_Player_GetCurrentWeapon,		idPlayer::Event_GetCurrentWeapon )
EVENT( EV_Player_GetPreviousWeapon,		idPlayer::Event_GetPreviousWeapon )
EVENT( EV_Player_SelectWeapon,			idPlayer::Event_SelectWeapon )
EVENT( EV_Player_GetWeaponEntity,		idPlayer::Event_GetWeaponEntity )
EVENT( EV_Player_OpenPDA,				idPlayer::Event_OpenPDA )
EVENT( EV_Player_InPDA,					idPlayer::Event_InPDA )
EVENT( EV_Player_ExitTeleporter,		idPlayer::Event_ExitTeleporter )
EVENT( EV_Player_StopAudioLog,			idPlayer::Event_StopAudioLog )
EVENT( EV_Player_HideTip,				idPlayer::Event_HideTip )
EVENT( EV_Player_LevelTrigger,			idPlayer::Event_LevelTrigger )
EVENT( EV_Gibbed,						idPlayer::Event_Gibbed )
EVENT( EV_Player_GiveInventoryItem,		idPlayer::Event_GiveInventoryItem )
EVENT( EV_Player_RemoveInventoryItem,	idPlayer::Event_RemoveInventoryItem )
EVENT( EV_Player_GetIdealWeapon,		idPlayer::Event_GetIdealWeapon )
EVENT( EV_Player_WeaponAvailable,		idPlayer::Event_WeaponAvailable )
EVENT( EV_Player_SetPowerupTime,		idPlayer::Event_SetPowerupTime )
EVENT( EV_Player_IsPowerupActive,		idPlayer::Event_IsPowerupActive )
EVENT( EV_Player_StartWarp,				idPlayer::Event_StartWarp )
EVENT( EV_Player_StopHelltime,			idPlayer::Event_StopHelltime )
EVENT( EV_Player_ToggleBloom,			idPlayer::Event_ToggleBloom )
EVENT( EV_Player_SetBloomParms,			idPlayer::Event_SetBloomParms )
// Koz begin
EVENT( EV_Player_GetWeaponHand, 		idPlayer::Event_GetWeaponHand )
EVENT( EV_Player_GetFlashHand,			idPlayer::Event_GetFlashHand ) // get flashlight hand
EVENT( EV_Player_GetWeaponHandState,	idPlayer::Event_GetWeaponHandState )
EVENT( EV_Player_GetFlashHandState,		idPlayer::Event_GetFlashHandState ) // get flashlight hand state
EVENT( EV_Player_GetFlashState,			idPlayer::Event_GetFlashState ) // get flashlight state
// Koz end
END_CLASS

const int MAX_RESPAWN_TIME = 10000;
const int RAGDOLL_DEATH_TIME = 3000;
const int MAX_PDAS = 64;
const int MAX_PDA_ITEMS = 128;
const int STEPUP_TIME = 200;
const int MAX_INVENTORY_ITEMS = 20;

/*
==============
idInventory::Clear
==============
*/
void idInventory::Clear()
{
	maxHealth		= 0;
	weapons			= 0;
	duplicateWeapons= 0;
	foundWeapons	= 0;
	powerups		= 0;
	armor			= 0;
	maxarmor		= 0;
	deplete_armor	= 0;
	deplete_rate	= 0.0f;
	deplete_ammount	= 0;
	nextArmorDepleteTime = 0;
	
	for( int i = 0; i < ammo.Num(); ++i )
	{
		ammo[i].Set( 0 );
	}
	
	ClearPowerUps();
	
	// set to -1 so that the gun knows to have a full clip the first time we get it and at the start of the level
	for( int i = 0; i < clip.Num(); ++i )
	{
		clip[i].Set( -1 );
		clipDuplicate[i].Set( -1 );
	}
	
	items.DeleteContents( true );
	memset( pdasViewed, 0, 4 * sizeof( pdasViewed[0] ) );
	pdas.Clear();
	videos.Clear();
	emails.Clear();
	selVideo = 0;
	selEMail = 0;
	selPDA = 0;
	selAudio = 0;
	pdaOpened = false;
	
	levelTriggers.Clear();
	
	nextItemPickup = 0;
	nextItemNum = 1;
	onePickupTime = 0;
	pickupItemNames.Clear();
	objectiveNames.Clear();
	
	ammoPredictTime = 0;
	
	lastGiveTime = 0;
	
	ammoPulse	= false;
	weaponPulse	= false;
	armorPulse	= false;
}

/*
==============
idInventory::GivePowerUp
==============
*/
void idInventory::GivePowerUp( idPlayer* player, int powerup, int msec )
{
	powerups |= 1 << powerup;
	powerupEndTime[ powerup ] = gameLocal.time + msec;
}

/*
==============
idInventory::ClearPowerUps
==============
*/
void idInventory::ClearPowerUps()
{
	int i;
	for( i = 0; i < MAX_POWERUPS; i++ )
	{
		powerupEndTime[ i ] = 0;
	}
	powerups = 0;
}

/*
==============
idInventory::GetPersistantData
==============
*/
void idInventory::GetPersistantData( idDict& dict )
{
	int		i;
	int		num;
	idDict*	item;
	idStr	key;
	const idKeyValue* kv;
	const char* name;
	
	// armor
	dict.SetInt( "armor", armor );
	
	// don't bother with powerups, maxhealth, maxarmor, or the clip
	
	// ammo
	for( i = 0; i < AMMO_NUMTYPES; i++ )
	{
		name = idWeapon::GetAmmoNameForNum( ( ammo_t )i );
		if( name )
		{
			dict.SetInt( name, ammo[ i ].Get() );
		}
	}
	
	//Save the clip data
	for( i = 0; i < MAX_WEAPONS; i++ )
	{
		dict.SetInt( va( "clip%i", i ), clip[ i ].Get() );
		dict.SetInt( va( "clipDuplicate%i", i ), clipDuplicate[ i ].Get() );
	}
	
	// items
	num = 0;
	for( i = 0; i < items.Num(); i++ )
	{
		item = items[ i ];
		
		// copy all keys with "inv_"
		kv = item->MatchPrefix( "inv_" );
		if( kv )
		{
			while( kv )
			{
				sprintf( key, "item_%i %s", num, kv->GetKey().c_str() );
				dict.Set( key, kv->GetValue() );
				kv = item->MatchPrefix( "inv_", kv );
			}
			num++;
		}
	}
	dict.SetInt( "items", num );
	
	// pdas viewed
	for( i = 0; i < 4; i++ )
	{
		dict.SetInt( va( "pdasViewed_%i", i ), pdasViewed[i] );
	}
	
	dict.SetInt( "selPDA", selPDA );
	dict.SetInt( "selVideo", selVideo );
	dict.SetInt( "selEmail", selEMail );
	dict.SetInt( "selAudio", selAudio );
	dict.SetInt( "pdaOpened", pdaOpened );
	
	// pdas
	for( i = 0; i < pdas.Num(); i++ )
	{
		sprintf( key, "pda_%i", i );
		dict.Set( key, pdas[ i ]->GetName() );
	}
	dict.SetInt( "pdas", pdas.Num() );
	
	// video cds
	for( i = 0; i < videos.Num(); i++ )
	{
		sprintf( key, "video_%i", i );
		dict.Set( key, videos[ i ]->GetName() );
	}
	dict.SetInt( "videos", videos.Num() );
	
	// emails
	for( i = 0; i < emails.Num(); i++ )
	{
		sprintf( key, "email_%i", i );
		dict.Set( key, emails[ i ]->GetName() );
	}
	dict.SetInt( "emails", emails.Num() );
	
	// weapons
	dict.SetInt( "weapon_bits", weapons | duplicateWeapons | foundWeapons ); // Carl: magically get back any weapons we dropped
	dict.SetInt( "weapon_duplicates", duplicateWeapons ); // Carl: Dual wielding, bitmask for which weapons you have two of

	dict.SetInt( "levelTriggers", levelTriggers.Num() );
	for( i = 0; i < levelTriggers.Num(); i++ )
	{
		sprintf( key, "levelTrigger_Level_%i", i );
		dict.Set( key, levelTriggers[i].levelName );
		sprintf( key, "levelTrigger_Trigger_%i", i );
		dict.Set( key, levelTriggers[i].triggerName );
	}
}

/*
==============
idInventory::RestoreInventory
==============
*/
void idInventory::RestoreInventory( idPlayer* owner, const idDict& dict )
{
	int			i;
	int			num;
	idDict*		item;
	idStr		key;
	idStr		itemname;
	const idKeyValue* kv;
	const char*	name;
	
	Clear();
	
	// health/armor
	maxHealth		= dict.GetInt( "maxhealth", "100" );
	armor			= dict.GetInt( "armor", "50" );
	maxarmor		= dict.GetInt( "maxarmor", "100" );
	deplete_armor	= dict.GetInt( "deplete_armor", "0" );
	deplete_rate	= dict.GetFloat( "deplete_rate", "2.0" );
	deplete_ammount	= dict.GetInt( "deplete_ammount", "1" );
	
	// the clip and powerups aren't restored
	
	// ammo
	for( i = 0; i < AMMO_NUMTYPES; i++ )
	{
		name = idWeapon::GetAmmoNameForNum( ( ammo_t )i );
		if( name )
		{
			ammo[ i ] = dict.GetInt( name );
		}
	}
	
	//Restore the clip data
	for( i = 0; i < MAX_WEAPONS; i++ )
	{
		clip[i] = dict.GetInt( va( "clip%i", i ), "-1" );
		clipDuplicate[i] = dict.GetInt( va( "clipDuplicate%i", i ), "-1" );
	}
	
	// items
	num = dict.GetInt( "items" );
	items.SetNum( num );
	for( i = 0; i < num; i++ )
	{
		item = new( TAG_ENTITY ) idDict();
		items[ i ] = item;
		sprintf( itemname, "item_%i ", i );
		kv = dict.MatchPrefix( itemname );
		while( kv )
		{
			key = kv->GetKey();
			key.Strip( itemname );
			item->Set( key, kv->GetValue() );
			kv = dict.MatchPrefix( itemname, kv );
		}
	}
	
	// pdas viewed
	for( i = 0; i < 4; i++ )
	{
		pdasViewed[i] = dict.GetInt( va( "pdasViewed_%i", i ) );
	}
	
	selPDA = dict.GetInt( "selPDA" );
	selEMail = dict.GetInt( "selEmail" );
	selVideo = dict.GetInt( "selVideo" );
	selAudio = dict.GetInt( "selAudio" );
	pdaOpened = dict.GetBool( "pdaOpened" );
	
	// pdas
	num = dict.GetInt( "pdas" );
	pdas.SetNum( num );
	for( i = 0; i < num; i++ )
	{
		sprintf( itemname, "pda_%i", i );
		pdas[i] = static_cast<const idDeclPDA*>( declManager->FindType( DECL_PDA, dict.GetString( itemname, "default" ) ) );
	}
	
	// videos
	num = dict.GetInt( "videos" );
	videos.SetNum( num );
	for( i = 0; i < num; i++ )
	{
		sprintf( itemname, "video_%i", i );
		videos[i] = static_cast<const idDeclVideo*>( declManager->FindType( DECL_VIDEO, dict.GetString( itemname, "default" ) ) );
	}
	
	// emails
	num = dict.GetInt( "emails" );
	emails.SetNum( num );
	for( i = 0; i < num; i++ )
	{
		sprintf( itemname, "email_%i", i );
		emails[i] = static_cast<const idDeclEmail*>( declManager->FindType( DECL_EMAIL, dict.GetString( itemname, "default" ) ) );
	}
	
	// weapons are stored as a number for persistant data, but as strings in the entityDef
	weapons	= dict.GetInt( "weapon_bits", "0" );
	foundWeapons = weapons; // Carl: if we drop any of these weapons, we magically get them back at the end of the level
	duplicateWeapons = dict.GetInt( "weapon_duplicates", "0" ); // Carl: Dual wielding, bitmask for which weapons you have two of
	
	if( g_skill.GetInteger() >= 3 || cvarSystem->GetCVarBool( "fs_buildresources" ) )
	{
		Give( owner, dict, "weapon", dict.GetString( "weapon_nightmare" ), NULL, false, ITEM_GIVE_FEEDBACK | ITEM_GIVE_UPDATE_STATE );
	}
	else
	{
		Give( owner, dict, "weapon", dict.GetString( "weapon" ), NULL, false, ITEM_GIVE_FEEDBACK | ITEM_GIVE_UPDATE_STATE );
	}
	
	num = dict.GetInt( "levelTriggers" );
	for( i = 0; i < num; i++ )
	{
		sprintf( itemname, "levelTrigger_Level_%i", i );
		idLevelTriggerInfo lti;
		lti.levelName = dict.GetString( itemname );
		sprintf( itemname, "levelTrigger_Trigger_%i", i );
		lti.triggerName = dict.GetString( itemname );
		levelTriggers.Append( lti );
	}
	
}

/*
==============
idInventory::Save
==============
*/
void idInventory::Save( idSaveGame* savefile ) const
{
	int i;
	
	savefile->WriteInt( maxHealth );
	savefile->WriteInt( weapons | foundWeapons | duplicateWeapons ); // Carl: don't make saves incompatible, we don't save how many of each weapon we have
	savefile->WriteInt( powerups );
	savefile->WriteInt( armor );
	savefile->WriteInt( maxarmor );
	savefile->WriteInt( ammoPredictTime );
	savefile->WriteInt( deplete_armor );
	savefile->WriteFloat( deplete_rate );
	savefile->WriteInt( deplete_ammount );
	savefile->WriteInt( nextArmorDepleteTime );
	
	for( i = 0; i < AMMO_NUMTYPES; i++ )
	{
		savefile->WriteInt( ammo[ i ].Get() );
	}
	for( i = 0; i < MAX_WEAPONS; i++ )
	{
		savefile->WriteInt( clip[ i ].Get() + clipDuplicate[ i ].Get() );
	}
	for( i = 0; i < MAX_POWERUPS; i++ )
	{
		savefile->WriteInt( powerupEndTime[ i ] );
	}
	
	savefile->WriteInt( items.Num() );
	for( i = 0; i < items.Num(); i++ )
	{
		savefile->WriteDict( items[ i ] );
	}
	
	savefile->WriteInt( pdasViewed[0] );
	savefile->WriteInt( pdasViewed[1] );
	savefile->WriteInt( pdasViewed[2] );
	savefile->WriteInt( pdasViewed[3] );
	
	savefile->WriteInt( selPDA );
	savefile->WriteInt( selVideo );
	savefile->WriteInt( selEMail );
	savefile->WriteInt( selAudio );
	savefile->WriteBool( pdaOpened );
	
	savefile->WriteInt( pdas.Num() );
	for( i = 0; i < pdas.Num(); i++ )
	{
		savefile->WriteString( pdas[ i ]->GetName() );
	}
	
	savefile->WriteInt( pdaSecurity.Num() );
	for( i = 0; i < pdaSecurity.Num(); i++ )
	{
		savefile->WriteString( pdaSecurity[ i ] );
	}
	
	savefile->WriteInt( videos.Num() );
	for( i = 0; i < videos.Num(); i++ )
	{
		savefile->WriteString( videos[ i ]->GetName() );
	}
	
	savefile->WriteInt( emails.Num() );
	for( i = 0; i < emails.Num(); i++ )
	{
		savefile->WriteString( emails[ i ]->GetName() );
	}
	
	savefile->WriteInt( nextItemPickup );
	savefile->WriteInt( nextItemNum );
	savefile->WriteInt( onePickupTime );
	
	savefile->WriteInt( pickupItemNames.Num() );
	for( i = 0; i < pickupItemNames.Num(); i++ )
	{
		savefile->WriteString( pickupItemNames[i] );
	}
	
	savefile->WriteInt( objectiveNames.Num() );
	for( i = 0; i < objectiveNames.Num(); i++ )
	{
		savefile->WriteMaterial( objectiveNames[i].screenshot );
		savefile->WriteString( objectiveNames[i].text );
		savefile->WriteString( objectiveNames[i].title );
	}
	
	savefile->WriteInt( levelTriggers.Num() );
	for( i = 0; i < levelTriggers.Num(); i++ )
	{
		savefile->WriteString( levelTriggers[i].levelName );
		savefile->WriteString( levelTriggers[i].triggerName );
	}
	
	savefile->WriteBool( ammoPulse );
	savefile->WriteBool( weaponPulse );
	savefile->WriteBool( armorPulse );
	
	savefile->WriteInt( lastGiveTime );
	
	for( i = 0; i < AMMO_NUMTYPES; i++ )
	{
		savefile->WriteInt( rechargeAmmo[i].ammo );
		savefile->WriteInt( rechargeAmmo[i].rechargeTime );
		savefile->WriteString( rechargeAmmo[i].ammoName );
	}
}

/*
==============
idInventory::Restore
==============
*/
void idInventory::Restore( idRestoreGame* savefile )
{
	int i, num;
	
	savefile->ReadInt( maxHealth );
	savefile->ReadInt( weapons );
	foundWeapons = weapons;
	duplicateWeapons = 0;
	savefile->ReadInt( powerups );
	savefile->ReadInt( armor );
	savefile->ReadInt( maxarmor );
	savefile->ReadInt( ammoPredictTime );
	savefile->ReadInt( deplete_armor );
	savefile->ReadFloat( deplete_rate );
	savefile->ReadInt( deplete_ammount );
	savefile->ReadInt( nextArmorDepleteTime );
	
	for( i = 0; i < AMMO_NUMTYPES; i++ )
	{
		int savedAmmo = 0;
		savefile->ReadInt( savedAmmo );
		ammo[ i ].Set( savedAmmo );
	}
	for( i = 0; i < MAX_WEAPONS; i++ )
	{
		int savedClip = 0;
		savefile->ReadInt( savedClip );
		clip[ i ].Set( savedClip );
		clipDuplicate[ i ].Set( 0 );
	}
	for( i = 0; i < MAX_POWERUPS; i++ )
	{
		savefile->ReadInt( powerupEndTime[ i ] );
	}
	
	savefile->ReadInt( num );
	for( i = 0; i < num; i++ )
	{
		idDict* itemdict = new( TAG_ENTITY ) idDict;
		
		savefile->ReadDict( itemdict );
		items.Append( itemdict );
	}
	
	// pdas
	savefile->ReadInt( pdasViewed[0] );
	savefile->ReadInt( pdasViewed[1] );
	savefile->ReadInt( pdasViewed[2] );
	savefile->ReadInt( pdasViewed[3] );
	
	savefile->ReadInt( selPDA );
	savefile->ReadInt( selVideo );
	savefile->ReadInt( selEMail );
	savefile->ReadInt( selAudio );
	savefile->ReadBool( pdaOpened );
	
	savefile->ReadInt( num );
	for( i = 0; i < num; i++ )
	{
		idStr strPda;
		savefile->ReadString( strPda );
		pdas.Append( static_cast<const idDeclPDA*>( declManager->FindType( DECL_PDA, strPda ) ) );
	}
	
	// pda security clearances
	savefile->ReadInt( num );
	for( i = 0; i < num; i++ )
	{
		idStr invName;
		savefile->ReadString( invName );
		pdaSecurity.Append( invName );
	}
	
	// videos
	savefile->ReadInt( num );
	for( i = 0; i < num; i++ )
	{
		idStr strVideo;
		savefile->ReadString( strVideo );
		videos.Append( static_cast<const idDeclVideo*>( declManager->FindType( DECL_VIDEO, strVideo ) ) );
	}
	
	// email
	savefile->ReadInt( num );
	for( i = 0; i < num; i++ )
	{
		idStr strEmail;
		savefile->ReadString( strEmail );
		emails.Append( static_cast<const idDeclEmail*>( declManager->FindType( DECL_EMAIL, strEmail ) ) );
	}
	
	savefile->ReadInt( nextItemPickup );
	savefile->ReadInt( nextItemNum );
	savefile->ReadInt( onePickupTime );
	savefile->ReadInt( num );
	for( i = 0; i < num; i++ )
	{
		idStr itemName;
		savefile->ReadString( itemName );
		pickupItemNames.Append( itemName );
	}
	
	savefile->ReadInt( num );
	for( i = 0; i < num; i++ )
	{
		idObjectiveInfo obj;
		
		savefile->ReadMaterial( obj.screenshot );
		savefile->ReadString( obj.text );
		savefile->ReadString( obj.title );
		
		objectiveNames.Append( obj );
	}
	
	savefile->ReadInt( num );
	for( i = 0; i < num; i++ )
	{
		idLevelTriggerInfo lti;
		savefile->ReadString( lti.levelName );
		savefile->ReadString( lti.triggerName );
		levelTriggers.Append( lti );
	}
	
	savefile->ReadBool( ammoPulse );
	savefile->ReadBool( weaponPulse );
	savefile->ReadBool( armorPulse );
	
	savefile->ReadInt( lastGiveTime );
	
	for( i = 0; i < AMMO_NUMTYPES; i++ )
	{
		savefile->ReadInt( rechargeAmmo[i].ammo );
		savefile->ReadInt( rechargeAmmo[i].rechargeTime );
		
		idStr name;
		savefile->ReadString( name );
		strcpy( rechargeAmmo[i].ammoName, name );
	}
}

/*
==============
idInventory::AmmoIndexForAmmoClass
==============
*/
ammo_t idInventory::AmmoIndexForAmmoClass( const char* ammo_classname ) const
{
	return idWeapon::GetAmmoNumForName( ammo_classname );
}

/*
==============
idInventory::AmmoIndexForAmmoClass
==============
*/
int idInventory::MaxAmmoForAmmoClass( const idPlayer* owner, const char* ammo_classname ) const
{
	return owner->spawnArgs.GetInt( va( "max_%s", ammo_classname ), "0" );
}

/*
==============
idInventory::AmmoPickupNameForIndex
==============
*/
const char* idInventory::AmmoPickupNameForIndex( ammo_t ammonum ) const
{
	return idWeapon::GetAmmoPickupNameForNum( ammonum );
}

/*
==============
idInventory::WeaponIndexForAmmoClass
mapping could be prepared in the constructor
==============
*/
int idInventory::WeaponIndexForAmmoClass( const idDict& spawnArgs, const char* ammo_classname ) const
{
	int i;
	const char* weapon_classname;
	for( i = 0; i < MAX_WEAPONS; i++ )
	{
		weapon_classname = spawnArgs.GetString( va( "def_weapon%d", i ) );
		if( !weapon_classname )
		{
			continue;
		}
		const idDeclEntityDef* decl = gameLocal.FindEntityDef( weapon_classname, false );
		if( !decl )
		{
			continue;
		}
		if( !idStr::Icmp( ammo_classname, decl->dict.GetString( "ammoType" ) ) )
		{
			return i;
		}
	}
	return -1;
}

/*
==============
idInventory::AmmoIndexForWeaponClass
==============
*/
ammo_t idInventory::AmmoIndexForWeaponClass( const char* weapon_classname, int* ammoRequired )
{
	const idDeclEntityDef* decl = gameLocal.FindEntityDef( weapon_classname, false );
	if( !decl )
	{
		//gameLocal.Error( "Unknown weapon in decl '%s'", weapon_classname );
		return 0;
	}
	if( ammoRequired )
	{
		*ammoRequired = decl->dict.GetInt( "ammoRequired" );
	}
	ammo_t ammo_i = AmmoIndexForAmmoClass( decl->dict.GetString( "ammoType" ) );
	return ammo_i;
}

/*
==============
idInventory::AddPickupName
==============
*/
void idInventory::AddPickupName( const char* name, idPlayer* owner )     //_D3XP
{
	int num = pickupItemNames.Num();
	if( ( num == 0 ) || ( pickupItemNames[ num - 1 ].Icmp( name ) != 0 ) )
	{
		if( idStr::Cmpn( name, STRTABLE_ID, STRTABLE_ID_LENGTH ) == 0 )
		{
			pickupItemNames.Append( idLocalization::GetString( name ) );
		}
		else
		{
			pickupItemNames.Append( name );
		}
	}
}

/*
==============
idInventory::Give
==============
*/
bool idInventory::Give( idPlayer* owner, const idDict& spawnArgs, const char* statname, const char* value,
						idPredictedValue< int >* idealWeapon, bool updateHud, unsigned int giveFlags )
{
	int						i;
	const char*				pos;
	const char*				end;
	int						len;
	idStr					weaponString;
	int						max;
	const idDeclEntityDef*	weaponDecl;
	bool					tookWeapon;
	int						amount;
	const char*				name;
	
	if( !idStr::Icmp( statname, "ammo_bloodstone" ) )
	{
		i = AmmoIndexForAmmoClass( statname );
		max = MaxAmmoForAmmoClass( owner, statname );
		
		if( max <= 0 )
		{
			if( giveFlags & ITEM_GIVE_UPDATE_STATE )
			{
				//No Max
				ammo[ i ] += atoi( value );
			}
		}
		else
		{
			//Already at or above the max so don't allow the give
			if( ammo[ i ].Get() >= max )
			{
				if( giveFlags & ITEM_GIVE_UPDATE_STATE )
				{
					ammo[ i ] = max;
				}
				return false;
			}
			if( giveFlags & ITEM_GIVE_UPDATE_STATE )
			{
				//We were below the max so accept the give but cap it at the max
				ammo[ i ] += atoi( value );
				if( ammo[ i ].Get() > max )
				{
					ammo[ i ] = max;
				}
			}
		}
	}
	else if( !idStr::Icmpn( statname, "ammo_", 5 ) )
	{
		i = AmmoIndexForAmmoClass( statname );
		max = MaxAmmoForAmmoClass( owner, statname );
		if( ammo[ i ].Get() >= max )
		{
			return false;
		}
		// Add ammo for the feedback flag because it's predicted.
		// If it is a misprediction, the client will be corrected in
		// a snapshot.
		if( giveFlags & ITEM_GIVE_FEEDBACK )
		{
			amount = atoi( value );
			if( amount )
			{
				ammo[ i ] += amount;
				if( ( max > 0 ) && ( ammo[ i ].Get() > max ) )
				{
					ammo[ i ] = max;
				}
				ammoPulse = true;
			}
			
			name = AmmoPickupNameForIndex( i );
			if( idStr::Length( name ) )
			{
				AddPickupName( name, owner ); //_D3XP
			}
		}
	}
	else if( !idStr::Icmp( statname, "armor" ) )
	{
		if( armor >= maxarmor )
		{
			return false;	// can't hold any more, so leave the item
		}
		if( giveFlags & ITEM_GIVE_UPDATE_STATE )
		{
			amount = atoi( value );
			if( amount )
			{
				armor += amount;
				if( armor > maxarmor )
				{
					armor = maxarmor;
				}
				nextArmorDepleteTime = 0;
				armorPulse = true;
			}
		}
	}
	else if( idStr::FindText( statname, "inclip_" ) == 0 )
	{
		if( giveFlags & ITEM_GIVE_UPDATE_STATE )
		{
			idStr temp = statname;
			i = atoi( temp.Mid( 7, 2 ) );
			if( i != -1 )
			{
				// set, don't add. not going over the clip size limit.
				SetClipAmmoForWeapon( i, false, atoi( value ) ); // Carl: todo, dual wielding, are we picking up the original or the duplicate?
			}
		}
	}
	else if( !idStr::Icmp( statname, "invulnerability" ) )
	{
		owner->GivePowerUp( INVULNERABILITY, SEC2MS( atof( value ) ), giveFlags );
	}
	else if( !idStr::Icmp( statname, "helltime" ) )
	{
		owner->GivePowerUp( HELLTIME, SEC2MS( atof( value ) ), giveFlags );
	}
	else if( !idStr::Icmp( statname, "envirosuit" ) )
	{
		owner->GivePowerUp( ENVIROSUIT, SEC2MS( atof( value ) ), giveFlags );
		owner->GivePowerUp( ENVIROTIME, SEC2MS( atof( value ) ), giveFlags );
	}
	else if( !idStr::Icmp( statname, "berserk" ) )
	{
		owner->GivePowerUp( BERSERK, SEC2MS( atof( value ) ), giveFlags );
		//} else if ( !idStr::Icmp( statname, "haste" ) ) {
		//	owner->GivePowerUp( HASTE, SEC2MS( atof( value ) ) );
	}
	else if( !idStr::Icmp( statname, "adrenaline" ) )
	{
		if( giveFlags & ITEM_GIVE_UPDATE_STATE )
		{
			GivePowerUp( owner, ADRENALINE, SEC2MS( atof( value ) ) );
		}
	}
	else if( !idStr::Icmp( statname, "mega" ) )
	{
		if( giveFlags & ITEM_GIVE_UPDATE_STATE )
		{
			GivePowerUp( owner, MEGAHEALTH, SEC2MS( atof( value ) ) );
		}
	}
	else if( !idStr::Icmp( statname, "weapon" ) )
	{
		// Carl TODO Dual wielding
		tookWeapon = false;
		for( pos = value; pos != NULL; pos = end )
		{
			end = strchr( pos, ',' );
			if( end )
			{
				len = end - pos;
				end++;
			}
			else
			{
				len = (int)strlen( pos );
			}
			
			idStr weaponName( pos, 0, len );
			
			// find the number of the matching weapon name
			for( i = 0; i < MAX_WEAPONS; i++ )
			{
				if( weaponName == spawnArgs.GetString( va( "def_weapon%d", i ) ) )
				{
					break;
				}
			}
			
			if( i >= MAX_WEAPONS )
			{
				gameLocal.Warning( "Unknown weapon '%s'", weaponName.c_str() );
				continue;
			}
			
			// cache the media for this weapon
			weaponDecl = gameLocal.FindEntityDef( weaponName, false );
			
			// don't pickup "no ammo" weapon types three times
			// not for D3 SP .. there is only one case in the game where you can get a no ammo
			// weapon when you might already have it, in that case it is more conistent to pick it up
			if( common->IsMultiplayer() && ( weapons & ( 1 << i ) ) && ( duplicateWeapons & ( 1 << i ) ) && ( weaponDecl != NULL ) && !weaponDecl->dict.GetInt( "ammoRequired" ) )
			{
				continue;
			}
			
			if( !gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) || ( weaponName == "weapon_fists" ) || ( weaponName == "weapon_soulcube" ) )
			{
				if( ( weapons & ( 1 << i ) ) == 0 || ( duplicateWeapons & ( 1 << i ) ) == 0 || common->IsMultiplayer() )
				{
					tookWeapon = true;
					
					// This is done during "feedback" so that clients can predict the ideal weapon.
					if( giveFlags & ITEM_GIVE_FEEDBACK )
					{
						idLobbyBase& lobby = session->GetActingGameStateLobbyBase();
						lobbyUserID_t& lobbyUserID = gameLocal.lobbyUserIDs[owner->entityNumber];
						if( lobby.GetLobbyUserWeaponAutoSwitch( lobbyUserID ) && idealWeapon != NULL && i != owner->weapon_bloodstone_active1 && i != owner->weapon_bloodstone_active2 && i != owner->weapon_bloodstone_active3 )
						{
							idealWeapon->Set( i );
						}
					}
					
					if( giveFlags & ITEM_GIVE_UPDATE_STATE )
					{
						if( updateHud && lastGiveTime + 1000 < gameLocal.time )
						{
							if( owner->hud )
							{
								owner->hud->GiveWeapon( owner, i );
							}
							lastGiveTime = gameLocal.time;
						}
						
						weaponPulse = true;
						if( ( weapons & ( 1 << i ) ) == 0 )
							weapons |= ( 1 << i );
						else
							duplicateWeapons |= ( 1 << i );
						foundWeapons |= weapons;
												
						if( weaponName != "weapon_pda" )
						{
							for( int index = 0; index < NUM_QUICK_SLOTS; ++index )
							{
								if( owner->GetQuickSlot( index ) == -1 )
								{
									owner->SetQuickSlot( index, i );
									break;
								}
							}
						}
					}
				}
			}
		}
		return tookWeapon;
	}
	else if( !idStr::Icmp( statname, "item" ) || !idStr::Icmp( statname, "icon" ) || !idStr::Icmp( statname, "name" ) || !idStr::Icmp( statname, "desc" ) )
	{
		// ignore these as they're handled elsewhere
		return false;
	}
	else
	{
		// unknown item
		gameLocal.Warning( "Unknown stat '%s' added to player's inventory", statname );
		return false;
	}
	
	return true;
}

/*
===============
idInventoy::Drop
===============
*/
void idInventory::Drop( const idDict& spawnArgs, const char* weapon_classname, int weapon_index )
{
	// remove the weapon bit
	// also remove the ammo associated with the weapon as we pushed it in the item
	assert( weapon_index != -1 || weapon_classname );
	if( weapon_index == -1 )
	{
		for( weapon_index = 0; weapon_index < MAX_WEAPONS; weapon_index++ )
		{
			if( !idStr::Icmp( weapon_classname, spawnArgs.GetString( va( "def_weapon%d", weapon_index ) ) ) )
			{
				break;
			}
		}
		if( weapon_index >= MAX_WEAPONS )
		{
			gameLocal.Error( "Unknown weapon '%s'", weapon_classname );
		}
	}
	else if( !weapon_classname )
	{
		weapon_classname = spawnArgs.GetString( va( "def_weapon%d", weapon_index ) );
	}
	// Carl: Dual wielding
	if( duplicateWeapons & ( 1 << weapon_index ) )
		duplicateWeapons &= ( 0xffffffff ^ ( 1 << weapon_index ) );
	else
		weapons &= ( 0xffffffff ^ ( 1 << weapon_index ) );
	ammo_t ammo_i = AmmoIndexForWeaponClass( weapon_classname, NULL );
	if( ammo_i && ammo_i < AMMO_NUMTYPES )
	{
		//ammo[ ammo_i ] = ammo[ ammo_i ].Get() - clip[ weapon_index ].Get(); // Carl: we don't really want to throw away all our ammo, do we?
		clip[ weapon_index ] = -1;
	}
}

/*
===============
idInventory::HasAmmo
===============
*/
int idInventory::HasAmmo( ammo_t type, int amount )
{
	if( ( type == 0 ) || !amount )
	{
		// always allow weapons that don't use ammo to fire
		return -1;
	}
	
	// check if we have infinite ammo
	if( ammo[ type ].Get() < 0 )
	{
		return -1;
	}
	
	// return how many shots we can fire
	return ammo[ type ].Get() / amount;
	
}

/*
===============
idInventory::HasAmmo
===============
*/
int idInventory::HasAmmo( const char* weapon_classname, bool includeClip, idPlayer* owner )  		//_D3XP
{
	int ammoRequired;
	ammo_t ammo_i = AmmoIndexForWeaponClass( weapon_classname, &ammoRequired );
	
	int ammoCount = HasAmmo( ammo_i, ammoRequired );
	if( includeClip && owner )
	{
		int w = owner->SlotForWeapon( weapon_classname );
		ammoCount += Max( 0, clip[w].Get() );
		ammoCount += Max( 0, clipDuplicate[w].Get() );
	}
	return ammoCount;
	
}

/*
===============
idInventory::HasEmptyClipCannotRefill
===============
*/
bool idInventory::HasEmptyClipCannotRefill( const char* weapon_classname, idPlayer* owner, const bool duplicate )
{

	int clipSize;
	if( duplicate )
		clipSize = clipDuplicate[owner->SlotForWeapon( weapon_classname )].Get();
	else
		clipSize = clip[owner->SlotForWeapon( weapon_classname )].Get();
	if( clipSize )
	{
		return false;
	}
	
	const idDeclEntityDef* decl = gameLocal.FindEntityDef( weapon_classname, false );
	if( decl == NULL )
	{
		gameLocal.Error( "Unknown weapon in decl '%s'", weapon_classname );
		return false;
	}
	int minclip = decl->dict.GetInt( "minclipsize" );
	if( !minclip )
	{
		return false;
	}
	
	ammo_t ammo_i = AmmoIndexForAmmoClass( decl->dict.GetString( "ammoType" ) );
	int ammoRequired = decl->dict.GetInt( "ammoRequired" );
	int ammoCount = HasAmmo( ammo_i, ammoRequired );
	if( ammoCount < minclip )
	{
		return true;
	}
	return false;
}

/*
===============
idInventory::UseAmmo
===============
*/
bool idInventory::UseAmmo( ammo_t type, int amount )
{
	if( g_infiniteAmmo.GetBool() )
	{
		return true;
	}
	
	if( !HasAmmo( type, amount ) )
	{
		return false;
	}
	
	// take an ammo away if not infinite
	if( ammo[ type ].Get() >= 0 )
	{
		const int currentAmmo = GetInventoryAmmoForType( type );
		SetInventoryAmmoForType( type, currentAmmo - amount );
	}
	
	return true;
}

/*
===============
idInventory::UpdateArmor
===============
*/
void idInventory::UpdateArmor()
{
	if( deplete_armor != 0.0f && deplete_armor < armor )
	{
		if( !nextArmorDepleteTime )
		{
			nextArmorDepleteTime = gameLocal.time + deplete_rate * 1000;
		}
		else if( gameLocal.time > nextArmorDepleteTime )
		{
			armor -= deplete_ammount;
			if( armor < deplete_armor )
			{
				armor = deplete_armor;
			}
			nextArmorDepleteTime = gameLocal.time + deplete_rate * 1000;
		}
	}
}

/*
===============
idInventory::InitRechargeAmmo
===============
* Loads any recharge ammo definitions from the ammo_types entity definitions.
*/
void idInventory::InitRechargeAmmo( idPlayer* owner )
{

	memset( rechargeAmmo, 0, sizeof( rechargeAmmo ) );
	
	const idKeyValue* kv = owner->spawnArgs.MatchPrefix( "ammorecharge_" );
	while( kv )
	{
		idStr key = kv->GetKey();
		idStr ammoname = key.Right( key.Length() - (int)strlen( "ammorecharge_" ) );
		int ammoType = AmmoIndexForAmmoClass( ammoname );
		rechargeAmmo[ammoType].ammo = ( atof( kv->GetValue().c_str() ) * 1000 );
		strcpy( rechargeAmmo[ammoType].ammoName, ammoname );
		kv = owner->spawnArgs.MatchPrefix( "ammorecharge_", kv );
	}
}

/*
===============
idInventory::RechargeAmmo
===============
* Called once per frame to update any ammo amount for ammo types that recharge.
*/
void idInventory::RechargeAmmo( idPlayer* owner )
{

	for( int i = 0; i < AMMO_NUMTYPES; i++ )
	{
		if( rechargeAmmo[i].ammo > 0 )
		{
			if( !rechargeAmmo[i].rechargeTime )
			{
				//Initialize the recharge timer.
				rechargeAmmo[i].rechargeTime = gameLocal.time;
			}
			int elapsed = gameLocal.time - rechargeAmmo[i].rechargeTime;
			if( elapsed >= rechargeAmmo[i].ammo )
			{
				int intervals = ( gameLocal.time - rechargeAmmo[i].rechargeTime ) / rechargeAmmo[i].ammo;
				ammo[i] += intervals;
				
				int max = MaxAmmoForAmmoClass( owner, rechargeAmmo[i].ammoName );
				if( max > 0 )
				{
					if( ammo[i].Get() > max )
					{
						ammo[i] = max;
					}
				}
				rechargeAmmo[i].rechargeTime += intervals * rechargeAmmo[i].ammo;
			}
		}
	}
}

/*
===============
idInventory::CanGive
===============
*/
bool idInventory::CanGive( idPlayer* owner, const idDict& spawnArgs, const char* statname, const char* value )
{

	if( !idStr::Icmp( statname, "ammo_bloodstone" ) )
	{
		int max = MaxAmmoForAmmoClass( owner, statname );
		int i = AmmoIndexForAmmoClass( statname );
		
		if( max <= 0 )
		{
			//No Max
			return true;
		}
		else
		{
			//Already at or above the max so don't allow the give
			if( ammo[ i ].Get() >= max )
			{
				ammo[ i ] = max;
				return false;
			}
			return true;
		}
	}
	else if( !idStr::Icmp( statname, "item" ) || !idStr::Icmp( statname, "icon" ) || !idStr::Icmp( statname, "name" ) )
	{
		// ignore these as they're handled elsewhere
		//These items should not be considered as succesful gives because it messes up the max ammo items
		return false;
	}
	return true;
}

/*
===============
idInventory::SetClipAmmoForWeapon

Ammo is predicted on clients. This function ensures the frame the prediction occurs
is stored so the predicted value doesn't get overwritten by snapshots. Of course
the snapshot-reading function must check this value.
===============
*/
void idInventory::SetClipAmmoForWeapon( const int weapon, const bool duplicate, const int amount )
{
	if( duplicate )
		clipDuplicate[weapon] = amount;
	else
		clip[weapon] = amount;
}

/*
===============
idInventory::SetInventoryAmmoForType

Ammo is predicted on clients. This function ensures the frame the prediction occurs
is stored so the predicted value doesn't get overwritten by snapshots. Of course
the snapshot-reading function must check this value.
===============
*/
void idInventory::SetInventoryAmmoForType( int ammoType, const int amount )
{
	ammo[ammoType] = amount;
}

/*
===============
idInventory::GetClipAmmoForWeapon
===============
*/
int idInventory::GetClipAmmoForWeapon( const int weapon, const bool duplicate ) const
{
	if( duplicate )
		return clipDuplicate[weapon].Get();
	else
		return clip[weapon].Get();
}

/*
===============
idInventory::GetInventoryAmmoForType
===============
*/
int	idInventory::GetInventoryAmmoForType( const int ammoType ) const
{
	return ammo[ammoType].Get();
}

/*
===============
idInventory::WriteAmmoToSnapshot
===============
*/
void idInventory::WriteAmmoToSnapshot( idBitMsg& msg ) const
{
	for( int i = 0; i < AMMO_NUMTYPES; i++ )
	{
		msg.WriteBits( ammo[i].Get(), ASYNC_PLAYER_INV_AMMO_BITS );
	}
	for( int i = 0; i < MAX_WEAPONS; i++ )
	{
		msg.WriteBits( clip[i].Get(), ASYNC_PLAYER_INV_CLIP_BITS );
		// Carl: TODO dual wielding
	}
}

/*
===============
idInventory::ReadAmmoFromSnapshot
===============
*/
void idInventory::ReadAmmoFromSnapshot( const idBitMsg& msg, const int ownerEntityNumber )
{
	for( int i = 0; i < ammo.Num(); i++ )
	{
		const int snapshotAmmo = msg.ReadBits( ASYNC_PLAYER_INV_AMMO_BITS );
		ammo[i].UpdateFromSnapshot( snapshotAmmo, ownerEntityNumber );
	}
	for( int i = 0; i < clip.Num(); i++ )
	{
		const int snapshotClip = msg.ReadBits( ASYNC_PLAYER_INV_CLIP_BITS );
		clip[i].UpdateFromSnapshot( snapshotClip, ownerEntityNumber );
		// Carl: TODO dual wielding
	}
}

/*
===============
idInventory::ReadAmmoFromSnapshot

Doesn't really matter what remote client's ammo count is, so just set it to 999.
===============
*/
void idInventory::SetRemoteClientAmmo( const int ownerEntityNumber )
{
	for( int i = 0; i < ammo.Num(); ++i )
	{
		ammo[i].UpdateFromSnapshot( 999, ownerEntityNumber );
	}
}

/*
==============
idWeaponHolder::idWeaponHolder
==============
*/
idWeaponHolder::idWeaponHolder()
{
	owner = NULL;
	currentWeapon = 0;
	isTheDuplicate = false;
}

/*
==============
idWeaponHolder::~idWeaponHolder
==============
*/
idWeaponHolder::~idWeaponHolder()
{
}

/*
==============
idWeaponHolder::Init
==============
*/
void idWeaponHolder::Init( idPlayer* player )
{
	owner = player;
	currentWeapon = owner->weapon_fists;
	isTheDuplicate = false;
}

/*
==============
idWeaponHolder::isEmpty
==============
*/
bool idWeaponHolder::isEmpty()
{
	if( currentWeapon < 0 || !owner )
		return true;
	return currentWeapon == owner->weapon_fists;
}

// Carl: This new holster code isn't implemented or used yet
#if 0

idHolster::idHolster()
{
	owner = NULL;
	currentWeapon = 0;
	isTheDuplicate = false;
	modelDefHandle = -1;
	memset( &renderEntity, 0, sizeof( renderEntity ) );
}

idHolster::~idHolster()
{
	FreeSlot();
}

/*
==============
idHolster::FreeSlot
==============
*/
void idHolster::FreeSlot()
{
	if( modelDefHandle != -1 )
	{
		if( gameRenderWorld )
			gameRenderWorld->FreeEntityDef(modelDefHandle);
		modelDefHandle = -1;
	}
}

/*
==============
idHolster::HolsterModelByName
==============
*/
void idHolster::HolsterModelByName( const char* modelname, idRenderModel* renderModel )
{
	memset( &renderEntity, 0, sizeof( renderEntity ) );

	if( !renderModel && !( renderModel = renderModelManager->FindModel( modelname ) ) )
	{
		// can't find the model
		return;
	}

	renderEntity.hModel = renderModel;
	if (renderEntity.hModel)
	{
		renderEntity.hModel->Reset();
		renderEntity.bounds = renderEntity.hModel->Bounds(&renderEntity);
	}
	renderEntity.shaderParms[SHADERPARM_RED] = 1.0f;
	renderEntity.shaderParms[SHADERPARM_GREEN] = 1.0f;
	renderEntity.shaderParms[SHADERPARM_BLUE] = 1.0f;
	renderEntity.shaderParms[3] = 1.0f;
	renderEntity.shaderParms[SHADERPARM_TIMEOFFSET] = 0.0f;
	renderEntity.shaderParms[5] = 0.0f;
	renderEntity.shaderParms[6] = 0.0f;
	renderEntity.shaderParms[7] = 0.0f;

	if( strcmp( modelname, "models/weapons/pistol/w_pistol.lwo" ) == 0 )
		holsterAxis = idAngles( 90, 0, 0 ).ToMat3() * 0.75f;
	else if( strcmp( modelname, "models/weapons/shotgun/w_shotgun2.lwo" ) == 0 ||
		strcmp( modelname, "models/weapons/bfg/bfg_world.lwo" ) == 0 )
		holsterAxis = idAngles( 0, -90, -90 ).ToMat3();
	else if( strcmp( modelname, "models/weapons/grabber/grabber_world.ase" ) == 0 )
		holsterAxis = idAngles( -90, 180, 0 ).ToMat3() * 0.5f;
	else if( strcmp( modelname, "models/weapons/machinegun/w_machinegun.lwo" ) == 0 )
		holsterAxis = idAngles( 0, 90, 90 ).ToMat3() * 0.75f;
	else if( strcmp( modelname, "models/weapons/plasmagun/plasmagun_world.lwo" ) == 0 )
		holsterAxis = idAngles( 0, 90, 90 ).ToMat3() * 0.75f;
	else if( strcmp( modelname, "models/weapons/chainsaw/w_chainsaw.lwo" ) == 0 )
		holsterAxis = idAngles( 0, 90, 90 ).ToMat3() * 0.9f;
	else if( strcmp( modelname, "models/weapons/chaingun/w_chaingun.lwo" ) == 0 )
		holsterAxis = idAngles( 0, 90, 90 ).ToMat3() * 0.9f;
	else if( strcmp( modelname, "models/items/pda/pda_world.lwo" ) == 0 )
		holsterAxis = ( pdaAngle1.ToMat3() * pdaAngle2.ToMat3() * pdaAngle3.ToMat3() ) * 0.6f;
	else // eg. flashlight
		holsterAxis = idAngles( 0, 90, 90 ).ToMat3();
}

/*
==============
idHolster::HolsterPDA
==============
*/
void idHolster::HolsterPDA()
{
	if( vr_slotDisable.GetBool() || !owner )
	{
		currentWeapon = owner ? owner->weapon_fists : 0;
		return;
	}

	currentWeapon = owner->weapon_fists;
	if( vr_slotDisable.GetBool() )
		return;

	// we will holster the PDA
	HolsterModelByName( "models/items/pda/pda_world.lwo" );
	currentWeapon = owner->weapon_pda;
}

/*
==============
idHolster::EmptyHolster
==============
*/
void idHolster::EmptyHolster()
{
	FreeSlot();
	currentWeapon = owner ? owner->weapon_fists : 0;
}

/*
==============
idHolster::HolsterFlashlight
==============
*/
void idHolster::HolsterFlashlight()
{
	const char * modelname;

	FreeSlot();
	if( vr_slotDisable.GetBool() || !owner )
	{
		currentWeapon = owner ? owner->weapon_fists : 0;
		return;
	}

	// we will holster the flashlight if carrying it
	if( commonVr->currentFlashlightMode == FLASHLIGHT_HAND && owner->flashlight.GetEntity()->IsLinked() && !owner->spectating && owner->weaponEnabled && !owner->hiddenWeapon && !gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) )
	{
		modelname = owner->flashlight->weaponDef->dict.GetString( "model" );
	}
	else modelname = "";
	HolsterModelByName( modelname );
	currentWeapon = owner->weapon_flashlight;
}

/*
==============
idHolster::StashToExtraHolster
==============
*/
void idHolster::StashToExtraHolster()
{
	if( owner )
	{
		// push current contents to extra holster
		owner->extraHolsteredWeapon = currentWeapon;
		if( renderEntity.hModel )
			owner->extraHolsteredWeaponModel = renderEntity.hModel->Name();
		else
			owner->extraHolsteredWeaponModel = NULL;
	}
	EmptyHolster();
}

void idHolster::RestoreFromExtraHolster()
{
	FreeSlot();
	if( vr_slotDisable.GetBool() || !owner )
	{
		currentWeapon = owner ? owner->weapon_fists : 0;
		return;
	}
	currentWeapon = owner->extraHolsteredWeapon;
	owner->extraHolsteredWeapon = owner->weapon_fists;
	const char * modelname = owner->extraHolsteredWeaponModel;
	owner->extraHolsteredWeaponModel = NULL;

	HolsterModelByName( modelname );
}

/*
==============
idPlayer::SetupHolsterSlot

stashed: -1 = switch weapons, 1 = empty holster of stashed weapon, 0 = stash current weapon in holster but don't switch
==============
*/
void idHolster::HolsterCurrentWeapon( int stashed, int hand )
{
	if( !owner )
	{
		EmptyHolster();
		return;
	}
	// if there's nothing to stash because we were already using fists or PDA
	if (stashed == 0 && (owner->hands[hand].currentWeapon == owner->weapon_pda || owner->hands[ hand ].currentWeapon == owner->weapon_fists))
		return;
	// if we were using fists before activating pda, we didn't stash anything in our holster, so don't unstash anything
	if (stashed == 1 && owner->hands[ hand ].previousWeapon == owner->weapon_fists)
		return;
	idWeapon* weapon = owner->GetWeaponInHand( hand );
	// if we want to read or switch the current weapon but it's not ready
	if( ( !weapon || !weapon->IsReady() ) && stashed != 1 )
	{
		return;
	}

	const char * modelname;
	idRenderModel* renderModel;

	// push current contents to extra holster
	if (stashed == 0)
		StashToExtraHolster();
	else
		FreeSlot();
	if (vr_slotDisable.GetBool())
	{
		currentWeapon = owner->weapon_fists;
		return;
	}

	if (stashed == 1)
		modelname = owner->extraHolsteredWeaponModel;
	else
		modelname = weapon->weaponDef->dict.GetString("model");

	// can we holster?
	if( !modelname ||
		strcmp( modelname, "models/weapons/soulcube/w_soulcube.lwo" ) == 0 ||
		strcmp( modelname, "_DEFAULT" ) == 0 ||
		strcmp( modelname, "models/items/grenade_ammo/grenade.lwo" ) == 0 ||
		strcmp( modelname, "models/items/pda/pda_world.lwo" ) == 0 ||
		!( renderModel = renderModelManager->FindModel( modelname ) ) )
	{
		// can't holster, just unholster
		if( currentWeapon != owner->weapon_fists )
		{
			if( stashed < 0 )
				owner->hands[hand].SelectWeapon( currentWeapon, false, true );
			currentWeapon = owner->weapon_fists;
			memset( &renderEntity, 0, sizeof( renderEntity ) );
		}
		return;
	}

	// we can holster! so unholster or change weapons
	if( stashed < 0 )
	{
		int prevWeapon = owner->hands[hand].currentWeapon;
		owner->hands[hand].SelectWeapon( currentWeapon, false, true );
		currentWeapon = prevWeapon;
	}
	else
	{
		if( stashed == 0 ) // stash current weapon, holstered weapon moves to invisible "extra" slot
		{
			currentWeapon = owner->hands[hand].currentWeapon;
		}
		else // unstash holstered weapon, extra weapon moves back to holster
		{
			owner->hands[hand].SelectWeapon( currentWeapon, true, true );
			RestoreFromExtraHolster();
			return;
		}
	}

	HolsterModelByName( modelname, renderModel );
}

/*
==============
idHolster::UpdateSlot
==============
*/
void idHolster::UpdateSlot()
{
	if( vr_slotDisable.GetBool() || !owner )
	{
		FreeSlot();
		currentWeapon = owner ? owner->weapon_fists : 0;
		return;
	}
	if( renderEntity.hModel )
	{
		renderEntity.timeGroup = owner->timeGroup;

		renderEntity.entityNum = ENTITYNUM_NONE;

		renderEntity.axis = holsterAxis * owner->waistAxis;
		idVec3 slotOrigin = origin + idVec3( -5, 0, 0 );
		if( vr_weaponHand.GetInteger() )
			slotOrigin.y *= -1.0f;
		renderEntity.origin = owner->waistOrigin + slotOrigin * owner->waistAxis;

		renderEntity.allowSurfaceInViewID = owner->entityNumber + 1;
		renderEntity.weaponDepthHack = g_useWeaponDepthHack.GetBool();

		if( modelDefHandle == -1 )
		{
			modelDefHandle = gameRenderWorld->AddEntityDef( &renderEntity );
		}
		else
		{
			gameRenderWorld->UpdateEntityDef( modelDefHandle, &renderEntity );
		}
	}
}

#endif

/*
========================
idPlayerHand::SetControllerShake
========================
*/
void idPlayerHand::SetControllerShake( float highMagnitude, int highDuration, float lowMagnitude, int lowDuration )
{
	// the main purpose of having these buffer is so multiple, individual shake events can co-exist with each other,
	// for instance, a constant low rumble from the chainsaw when it's idle and a harsh rumble when it's being used.

	// find active buffer with similar magnitude values
	int activeBufferWithSimilarMags = -1;
	int inactiveBuffer = -1;
	for( int i = 0; i < MAX_SHAKE_BUFFER; i++ )
	{
		if( gameLocal.GetTime() <= controllerShakeHighTime[i] || gameLocal.GetTime() <= controllerShakeLowTime[i] )
		{
			if( idMath::Fabs( highMagnitude - controllerShakeHighMag[i] ) <= 0.1f && idMath::Fabs( lowMagnitude - controllerShakeLowMag[i] ) <= 0.1f )
			{
				activeBufferWithSimilarMags = i;
				break;
			}
		}
		else
		{
			if( inactiveBuffer == -1 )
			{
				inactiveBuffer = i;		// first, inactive buffer..
			}
		}
	}

	if( activeBufferWithSimilarMags > -1 )
	{
		// average the magnitudes and adjust the time
		controllerShakeHighMag[activeBufferWithSimilarMags] += highMagnitude;
		controllerShakeHighMag[activeBufferWithSimilarMags] *= 0.5f;

		controllerShakeLowMag[activeBufferWithSimilarMags] += lowMagnitude;
		controllerShakeLowMag[activeBufferWithSimilarMags] *= 0.5f;

		controllerShakeHighTime[activeBufferWithSimilarMags] = gameLocal.GetTime() + highDuration;
		controllerShakeLowTime[activeBufferWithSimilarMags] = gameLocal.GetTime() + lowDuration;
		controllerShakeTimeGroup = gameLocal.selectedGroup;
		return;
	}

	if( inactiveBuffer == -1 )
	{
		inactiveBuffer = 0;			// FIXME: probably want to use the oldest buffer..
	}

	controllerShakeHighMag[inactiveBuffer] = highMagnitude;
	controllerShakeLowMag[inactiveBuffer] = lowMagnitude;
	controllerShakeHighTime[inactiveBuffer] = gameLocal.GetTime() + highDuration;
	controllerShakeLowTime[inactiveBuffer] = gameLocal.GetTime() + lowDuration;
	controllerShakeTimeGroup = gameLocal.selectedGroup;
}

void idPlayerHand::ResetControllerShake()
{
	for( int i = 0; i < MAX_SHAKE_BUFFER; i++ )
	{
		controllerShakeHighTime[i] = 0;
	}

	for( int i = 0; i < MAX_SHAKE_BUFFER; i++ )
	{
		controllerShakeHighMag[i] = 0.0f;
	}

	for( int i = 0; i < MAX_SHAKE_BUFFER; i++ )
	{
		controllerShakeLowTime[i] = 0;
	}

	for( int i = 0; i < MAX_SHAKE_BUFFER; i++ )
	{
		controllerShakeLowMag[i] = 0.0f;
	}
}

void idPlayerHand::GetControllerShake( int & highMagnitude, int & lowMagnitude ) const
{

	if( Flicksync_InCutscene || gameLocal.inCinematic )
	{
		// no controller shake during cinematics
		highMagnitude = 0;
		lowMagnitude = 0;
		return;
	}

	float lowMag = 0.0f;
	float highMag = 0.0f;

	lowMagnitude = 0;
	highMagnitude = 0;

	// use highest values from active buffers
	for( int i = 0; i < MAX_SHAKE_BUFFER; i++ )
	{
		if( gameLocal.GetTimeGroupTime( controllerShakeTimeGroup ) < controllerShakeLowTime[i] )
		{
			if( controllerShakeLowMag[i] > lowMag )
			{
				lowMag = controllerShakeLowMag[i];
			}
		}
		if( gameLocal.GetTimeGroupTime( controllerShakeTimeGroup ) < controllerShakeHighTime[i] )
		{
			if( controllerShakeHighMag[i] > highMag )
			{
				highMag = controllerShakeHighMag[i];
			}
		}
	}

	lowMagnitude = idMath::Ftoi( lowMag * 65535.0f );
	highMagnitude = idMath::Ftoi( highMag * 65535.0f );
}

/*
==============
idPlayerHand::idPlayerHand
==============
*/
idPlayerHand::idPlayerHand():
idealWeapon( -1 )
{
	weapon = NULL;
	weaponGone = false;
	playerPdaPos = vec3_zero;
	currentWeapon = -1;
	previousWeapon = -1;
	isTheDuplicate = false;
	weaponSwitchTime = 0;

	laserSightHandle = -1;
	memset( &laserSightRenderEntity, 0, sizeof( laserSightRenderEntity ) );

	crosshairHandle = -1;
	memset( &crosshairEntity, 0, sizeof( crosshairEntity ) );

	lastCrosshairMode = -1;

	throwDirection = vec3_zero;
	throwVelocity = 0.0f;
	frameTime[10] = { 0 };
	position[10] = { vec3_zero };
	frameNum = -1;
	curTime = 0;
	timeDelta = 0;
	startFrameNum = 0;

	grabbingWorld = false;
	triggerDown = false;
	thumbDown = false;
	oldGrabbingWorld = false;
	oldTriggerDown = false;
	oldFlashlightTriggerDown = false;
	oldThumbDown = false;

	// Koz begin
	//laserSightActive = vr_weaponSight.GetInteger() == 0;
	// Koz end

}

/*
==============
idPlayerHand::~idPlayerHand()

Release any resources used by the player's hand.
==============
*/
idPlayerHand::~idPlayerHand()
{
	delete weapon.GetEntity();
	weapon = NULL;
}

/*
==============
idPlayerHand::Init()
==============
*/
void idPlayerHand::Init( idPlayer* player, int hand )
{
	if( vr_debugHands.GetBool() )
	{
		common->Printf( "\nBefore Init():\n" );
		debugPrint();
	}
	owner = player;
	whichHand = hand;

	currentWeapon = -1;
	idealWeapon = -1;
	previousWeapon = -1;
	weaponSwitchTime = 0;

	grabbingWorld = false;
	triggerDown = false;
	thumbDown = false;
	oldGrabbingWorld = false;
	oldTriggerDown = false;
	oldFlashlightTriggerDown = false;
	oldThumbDown = false;

	laserSightHandle = -1;
	// laser sight for 3DTV
	memset( &laserSightRenderEntity, 0, sizeof( laserSightRenderEntity ) );
	laserSightRenderEntity.hModel = renderModelManager->FindModel( "_BEAM" );
	laserSightRenderEntity.customShader = declManager->FindMaterial( "stereoRenderLaserSight" );

	crosshairHandle = -1;
	// model to place crosshair or red dot into 3d space
	memset( &crosshairEntity, 0, sizeof( crosshairEntity ) );
	crosshairEntity.hModel = renderModelManager->FindModel( "/models/mapobjects/weaponsight.lwo" );
	crosshairEntity.weaponDepthHack = true;

	lastCrosshairMode = -1;

	throwDirection = vec3_zero;
	throwVelocity = 0.0f;

	PDAfixed = false;
	lastPdaFixed = PDAfixed;
	playerPdaPos = vec3_zero;
	motionPosition = vec3_zero;
	// motionRotation = idQuat_zero;
	wasPDA = false;

	// Koz begin
	//	laserSightActive = vr_weaponSight.GetInteger() == 0;
	// Koz end

	if( vr_debugHands.GetBool() )
	{
		common->Printf( "\nAfter Init():\n" );
		debugPrint();
	}
}

bool idPlayerHand::holdingFlashlight()
{
	// Carl: weapon must be set to fist or empty hand when we're using the flashlight
	if( whichHand == vr_weaponHand.GetInteger() || commonVr->currentFlashlightPosition != FLASHLIGHT_HAND || !owner || !owner->flashlight || !owner->flashlight.GetEntity()->IsLinked()
		|| owner->spectating || !owner->weaponEnabled || owner->hiddenWeapon || gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) )
		return false;
	return true;
}

bool idPlayerHand::holdingWeapon()
{
	if( !controllingWeapon() )
		return false;
	int w = owner->GetCurrentWeaponSlot();
	return w != owner->weapon_fists && w != owner->weapon_soulcube && w != owner->weapon_bloodstone
		&& w != owner->weapon_bloodstone_active1 && w != owner->weapon_bloodstone_active2 && w != owner->weapon_bloodstone_active3;
}

bool idPlayerHand::floatingWeapon()
{
	if( !controllingWeapon() )
		return false;
	int w = owner->GetCurrentWeaponSlot();
	return w == owner->weapon_soulcube || w == owner->weapon_bloodstone || w == owner->weapon_bloodstone_active1 || w == owner->weapon_bloodstone_active2 || w == owner->weapon_bloodstone_active3;
}

bool idPlayerHand::controllingWeapon()
{
	if( currentWeapon < 0 || !owner || !weapon || weapon->IdentifyWeapon() == WEAPON_PDA || holdingFlashlight()
		|| owner->spectating || !owner->weaponEnabled || owner->hiddenWeapon || gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) )
		return false;
	return true;
}

bool idPlayerHand::holdingPDA()
{
	return (weapon.GetEntity() && weapon->IdentifyWeapon() == WEAPON_PDA); // Carl: Is this the only way? Or do I need to check for forced pda?
}

bool idPlayerHand::holdingPhysics()
{
	// Carl todo
	return false;
}

bool idPlayerHand::holdingItem()
{
	// Carl todo
	return false;
}

bool idPlayerHand::holdingSomethingDroppable()
{
	return holdingWeapon() || holdingFlashlight() || holdingPDA() || holdingPhysics() || holdingItem();
}

bool idPlayerHand::isOverMountedFlashlight()
{
	if( !owner || !owner->flashlight.IsValid() || owner->spectating || !owner->weaponEnabled || owner->hiddenWeapon || gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) )
		return false;
	else if( handSlot == SLOT_FLASHLIGHT_HEAD )
		return commonVr->currentFlashlightPosition == FLASHLIGHT_HEAD;
	else if( handSlot == SLOT_FLASHLIGHT_SHOULDER )
		return commonVr->currentFlashlightPosition == FLASHLIGHT_BODY;
	else
		return false;
}

bool idPlayerHand::tooFullToInteract()
{
	return (vr_mustEmptyHands.GetBool() && ( holdingWeapon() || holdingFlashlight() || holdingPDA() || holdingPhysics() || holdingItem() )) || !handExists();
}

bool idPlayerHand::handExists()
{
	return true;
}

bool idPlayerHand::contextToggleVirtualGrab()
{
	return false;
}

bool idPlayerHand::startVirtualGrab()
{
	virtualGrabDown = true;
	return true;
}

bool idPlayerHand::releaseVirtualGrab()
{
	if( holdingWeapon() )
	{
		virtualGrabDown = false;
		if( vr_gripMode.GetInteger() != VR_GRIP_DEAD_AND_BURIED )
			DropWeapon( false );
		else
		{
			// return weapon to holster it was drawn from
			// if that holster is full, move its contents to inventory
			// if its contents can't be stored in the inventory, then store the ammo but drop the weapon
		}
		return true;
	}
	else if( holdingFlashlight() )
	{
		// now dropping the flashlight moves it to your inventory
		commonVr->currentFlashlightMode = FLASHLIGHT_INVENTORY;
		//vr_flashlightMode.SetInteger( FLASHLIGHT_BODY );
		//vr_flashlightMode.SetModified();
		return true;
	}
	else if( holdingPDA() )
	{
		// for now, dropping any PDA will put it away
		return true;
	}
	else if( holdingItem() )
	{
		// either use the item
		// or drop the item
		// depending on the settings and where our hand is
		return true;
	}
	else if( holdingPhysics() )
	{
		// throw the physics object
		return true;
	}
	return false;
}

/*
===============
idPlayerHand::NextWeapon
===============
*/
void idPlayerHand::NextWeapon( int dir )
{
	if( vr_debugHands.GetBool() )
	{
		if ( dir > 0 )
			common->Printf( "\nBefore NextWeapon():\n" );
		else
			common->Printf( "\nBefore PrevWeapon():\n" );
		debugPrint();
	}

	// Koz dont change weapon if in gui
	if( !owner || commonVr->handInGui || !owner->weaponEnabled || owner->spectating || owner->hiddenWeapon || gameLocal.inCinematic || Flicksync_InCutscene || gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) || owner->health < 0 )
	{
		return;
	}

	if( !handExists() )
		return;

	// check if we have any weapons
	if( !owner->inventory.weapons )
	{
		return;
	}

	// Carl: Dual wielding, handle switching from flashlight properly
	if( holdingFlashlight() )
	{
		if( vr_flashlightStrict.GetBool() )
			commonVr->currentFlashlightMode = FLASHLIGHT_INVENTORY;
		else
			commonVr->currentFlashlightMode = FLASHLIGHT_HEAD;
	}

	int w = idealWeapon.Get();
	while( 1 )
	{
		w += dir;
		if( w >= MAX_WEAPONS )
		{
			w = 0;
		}
		else if( w < 0 )
		{
			w = MAX_WEAPONS - 1;
		}
		if( w == idealWeapon.Get() )
		{
			w = owner->weapon_fists;
			break;
		}
		// if we don't have the weapon in our inventory, skip it
		if( ( ( owner->inventory.weapons & ( 1 << w ) ) == 0 && w != owner->weapon_flashlight ) || ( w == owner->weapon_flashlight && !owner->HasHoldableFlashlight() ) )
		{
			continue;
		}
		// Carl: dual wielding
		if( w != owner->weapon_fists )
		{
			int availableWeaponsOfThisType = 1;
			if( owner->inventory.duplicateWeapons & ( 1 << w ) )
				availableWeaponsOfThisType++;
			// Carl: skip weapons in the other hand unless we have a duplicate (dual wielding)
			if( w == owner->hands[1 - whichHand].idealWeapon.Get() )
			{
				availableWeaponsOfThisType--;
				if( availableWeaponsOfThisType <= 0 )
					continue;
			}
			// Carl: optionally skip weapons in the holster unless we have a duplicate
			if( w == owner->holsteredWeapon )
			{
				availableWeaponsOfThisType--;
				if( availableWeaponsOfThisType <= 0 )
				{
					vr_weaponcycle_t c = (vr_weaponcycle_t)vr_weaponCycleMode.GetInteger();
					if( c == VR_WEAPONCYCLE_INCLUDE_HOLSTERED || c == VR_WEAPONCYCLE_HOLSTERED_AND_FLASHLIGHT || c == VR_WEAPONCYCLE_HOLSTERED_AND_FLASHLIGHT_AND_PDA )
					{
						// Carl: draw weapon from holster
						owner->FreeHolsterSlot();
						owner->holsteredWeapon = owner->weapon_fists;
						memset( &owner->holsterRenderEntity, 0, sizeof( owner->holsterRenderEntity ) );
						availableWeaponsOfThisType++;
					}
					else
					{
						// Skip holstered weapon
						continue;
					}
				}
			}
		}
		const char* weap = owner->spawnArgs.GetString( va( "def_weapon%d", w ) );
		if( !weap[0] )
		{
			continue;
		}
		// Carl: skip weapons if we can't automagically empty our other hand, and we can't dual wield this weapon.
		// I'm making it so we can't automagically empty the weapon hand to please the non-weapon hand,
		// so the non-weapon hand will only cycle through the dual-wieldable weapons.
		if( ( vr_mustEmptyHands.GetBool() || whichHand == ( 1 - vr_weaponHand.GetInteger() ) ) && !owner->CanDualWield( w ) && !owner->CanDualWield( owner->hands[1 - whichHand].currentWeapon ) && !owner->CanDualWield( owner->hands[1 - whichHand].idealWeapon.Get() ) )
		{
			continue;
		}

		// Cycle to the flashlight if we're using our flashlight hand and the flashlight is in our inventory, OR if we set to include the flashlight in the weapon cycle.
		// todo: don't let it cycle to the flashlight if it's in the other hand and we're the weapon hand
		if( w == owner->weapon_flashlight && ( vr_weaponCycleMode.GetInteger() == VR_WEAPONCYCLE_INCLUDE_FLASHLIGHT || vr_weaponCycleMode.GetInteger() == VR_WEAPONCYCLE_HOLSTERED_AND_FLASHLIGHT
			|| vr_weaponCycleMode.GetInteger() == VR_WEAPONCYCLE_HOLSTERED_AND_FLASHLIGHT_AND_PDA || ( whichHand == ( 1 - vr_weaponHand.GetInteger() ) && commonVr->currentFlashlightMode == FLASHLIGHT_INVENTORY ) ) )
		{
			break;
		}
		else if( w == owner->weapon_pda && vr_weaponCycleMode.GetInteger() == VR_WEAPONCYCLE_HOLSTERED_AND_FLASHLIGHT_AND_PDA )
		{
			// skip the pda if we haven't been given one yet
			if( owner->weapon_pda >= 0 && owner->inventory.pdas.Num() == 0 )
				continue;
			break;
		}
		else if( !owner->spawnArgs.GetBool( va( "weapon%d_cycle", w ) ) )
		{
			continue;
		}

		if( owner->inventory.HasAmmo( weap, true, owner ) || w == owner->weapon_bloodstone )
		{
			break;
		}

	}

	if( ( w != currentWeapon ) && ( w != idealWeapon.Get() ) )
	{
		// Carl: If we can't dual-wield it?
		if( !owner->CanDualWield( w ) && !owner->CanDualWield( owner->hands[1 - whichHand].currentWeapon ) && !owner->CanDualWield( owner->hands[1 - whichHand].idealWeapon.Get() ) )
		{
			// if we can't automagically empty our other hand, do nothing
			if( vr_mustEmptyHands.GetBool() )
				return;
			// otherwise, first automagically empty our other hand
			if( owner->CanDualWield( owner->weapon_fists ) )
				owner->hands[1 - whichHand].SelectWeapon( owner->weapon_fists, true, true );
			else
				owner->hands[1 - whichHand].SelectWeapon( weapon_empty_hand, true, true );
		}

		// Carl: Dual wielding, handle switching to flashlight properly
		if( w == owner->weapon_flashlight )
		{
			w = owner->weapon_fists;
			commonVr->currentFlashlightMode = FLASHLIGHT_HAND;
			commonVr->currentFlashlightPosition = FLASHLIGHT_HAND;
			vr_weaponHand.SetInteger( 1 - whichHand );
		}

		idealWeapon = w;
		weaponSwitchTime = gameLocal.time + WEAPON_SWITCH_DELAY;
		owner->UpdateHudWeapon( whichHand );
		if( vr_debugHands.GetBool() )
			common->Printf( "Changing weapon\n" );
	}
	if( vr_debugHands.GetBool() )
	{
		if( dir > 0 )
			common->Printf( "After NextWeapon():\n" );
		else
			common->Printf( "After PrevWeapon():\n" );
		debugPrint();
	}
}

/*
===============
idPlayerHand::PrevWeapon
===============
*/
void idPlayerHand::PrevWeapon()
{
	NextWeapon( -1 );
}

void idPlayerHand::NextBestWeapon()
{
	if( vr_debugHands.GetBool() )
	{
		common->Printf( "\nBefore NextBestWeapon():\n" );
		debugPrint();
	}
	const char* weap;
	int w = MAX_WEAPONS;

	if( !owner->weaponEnabled || !handExists() )
	{
		return;
	}

	// Carl: Dual wielding, handle switching from flashlight properly
	if( holdingFlashlight() )
	{
		if( vr_flashlightStrict.GetBool() )
			commonVr->currentFlashlightMode = FLASHLIGHT_INVENTORY;
		else
			commonVr->currentFlashlightMode = FLASHLIGHT_HEAD;
	}

	while( w > 0 )
	{
		w--;
		if( w == owner->weapon_flashlight )
		{
			continue;
		}
		weap = owner->spawnArgs.GetString( va( "def_weapon%d", w ) );
		if( !weap[ 0 ] || ( ( owner->inventory.weapons & ( 1 << w ) ) == 0 ) || ( !owner->inventory.HasAmmo( weap, true, owner ) ) )
		{
			continue;
		}
		if( !owner->spawnArgs.GetBool( va( "weapon%d_best", w ) ) )
		{
			continue;
		}

		//Some weapons will report having ammo but the clip is empty and
		//will not have enough to fill the clip (i.e. Double Barrel Shotgun with 1 round left)
		//We need to skip these weapons because they cannot be used
		if( owner->inventory.HasEmptyClipCannotRefill( weap, owner, isTheDuplicate ) )
		{
			continue;
		}

		// Carl: dual wielding
		if( w != owner->weapon_fists )
		{
			int availableWeaponsOfThisType = 1;
			if( owner->inventory.duplicateWeapons & ( 1 << w ) )
				availableWeaponsOfThisType++;
			// Carl: skip weapons in the holster unless we have a duplicate, TODO make it optional
			if( w == owner->holsteredWeapon )
			{
				availableWeaponsOfThisType--;
				if( availableWeaponsOfThisType <= 0 )
					continue;
			}
			// Carl: skip weapons in the other hand unless we have a duplicate (dual wielding)
			if( w == owner->hands[ 1 - whichHand ].idealWeapon.Get() )
			{
				availableWeaponsOfThisType--;
				if( availableWeaponsOfThisType <= 0 )
					continue;
			}
		}

		// Carl: Because this is an automatic weapon switch, we don't want to change our other hand, so just skip weapons we can't dual wield
		if( !owner->CanDualWield( w ) && !owner->CanDualWield( owner->hands[1 - whichHand].currentWeapon ) && !owner->CanDualWield( owner->hands[1 - whichHand].idealWeapon.Get() ) )
		{
			continue;
		}
		break;
	}
	idealWeapon = w;
	weaponSwitchTime = gameLocal.time + WEAPON_SWITCH_DELAY;
	owner->UpdateHudWeapon( whichHand );
	if( vr_debugHands.GetBool() )
	{
		common->Printf( "After NextBestWeapon():\n" );
		debugPrint();
	}
}

void idPlayer::SelectWeapon( int num, bool force, bool specific )
{
	if( hands[vr_weaponHand.GetInteger()].handExists() )
		hands[vr_weaponHand.GetInteger()].SelectWeapon( num, force, specific );
	else
		hands[1 - vr_weaponHand.GetInteger()].SelectWeapon( num, force, specific );
}

void idPlayerHand::DropWeapon( bool died )
{
	if( vr_debugHands.GetBool() )
	{
		common->Printf( "Before DropWeapon():\n" );
		debugPrint();
	}
	idVec3 forward, up;

	assert( !common->IsClient() );

	if( !owner || owner->spectating || weaponGone || weapon.GetEntity() == NULL )
		return;

	if( handExists() )
		return;

	idWeapon* weap = weapon.GetEntity();

	if( !weap || ( !died && !weap->IsReady() ) || weap->IsReloading() )
		return;

	// ammoavailable is how many shots we can fire
	// inclip is which amount is in clip right now
	int ammoavailable = weap->AmmoAvailable();
	int inclip = weap->AmmoInClip();

	// don't drop a grenade if we have none left
	if( !idStr::Icmp( idWeapon::GetAmmoNameForNum( weap->GetAmmoType() ), "ammo_grenades" ) && ( ammoavailable - inclip <= 0 ) )
	{
		return;
	}

	ammoavailable += inclip;

	// expect an ammo setup that makes sense before doing any dropping
	// ammoavailable is -1 for infinite ammo, and weapons like chainsaw
	// a bad ammo config usually indicates a bad weapon state, so we should not drop
	// used to be an assertion check, but it still happens in edge cases

	if( ( ammoavailable != -1 ) && ( ammoavailable < 0 ) )
	{
		common->DPrintf( "idPlayer::DropWeapon: bad ammo setup\n" );
		return;
	}
	idEntity* item = NULL;
	// Carl: todo throwing a weapon when you let go of it (also dual wielding)
	if( died )
	{
		// ain't gonna throw you no weapon if I'm dead
		item = weap->DropItem( vec3_origin, 0, -1, died );
	}
	else
	{
		// Carl If we drop it straight down, we'll automatically pick it up immediately (if we're alive), so throw it forward a bit.
		owner->viewAngles.ToVectors( &forward, NULL, &up );
		item = weap->DropItem( 150.0f * forward + 50.0f * up, 500, -1, died );
	}
	if( !item )
		return;
	// set the appropriate ammo in the dropped object
	if( !died )
		ammoavailable = inclip; // Carl: TODO work out what these keys do, don't put ALL our ammo in the drop
	const idKeyValue* keyval = item->spawnArgs.MatchPrefix( "inv_ammo_" );
	if( keyval )
	{
		item->spawnArgs.SetInt( keyval->GetKey(), 0 );// , ammoavailable );
		idStr inclipKey = keyval->GetKey();
		inclipKey.Insert( "inclip_", 4 );
		inclipKey.Insert( va( "%.2d", currentWeapon ), 11 );
		item->spawnArgs.SetInt( inclipKey, inclip );
	}
	if( !died )
	{
		bool isDuplicate = false;
		if( !isDuplicate )
		{
			// remove from our local inventory completely
			owner->inventory.Drop( owner->spawnArgs, item->spawnArgs.GetString( "inv_weapon" ), -1 );
			weap->ResetAmmoClip();
			SelectWeapon( owner->weapon_fists, true, true );
			weap->WeaponStolen();
			weaponGone = true;
		}
		else
		{
			// just remove from our hand
		}
	}
	if( vr_debugHands.GetBool() )
	{
		common->Printf( "After DropWeapon():\n" );
		debugPrint();
	}
}

void idPlayerHand::debugPrint()
{
	if( !vr_debugHands.GetBool() )
		return;
	idStr s = "";
	if( vr_weaponHand.GetInteger() == whichHand )
	{
		s = "(Weapon hand) ";
	}
	else if( vr_weaponHand.GetInteger() < 0 || vr_weaponHand.GetInteger() >= 2 )
	{
		s = "(INVALID vr_weaponHand) ";
		common->Printf( "vr_weaponHand = %d\n", vr_weaponHand.GetInteger() );
	}
	else if( commonVr->currentFlashlightPosition == FLASHLIGHT_HAND )
	{
		s = "(Flashlight in hand) ";
	}
	else if( commonVr->currentFlashlightMode == FLASHLIGHT_HAND )
	{
		s = "(Flashlight hand) ";
	}
	if (whichHand == HAND_RIGHT)
		common->Printf( "*** Right Hand %s***\n", s.c_str() );
	else if( whichHand == HAND_LEFT )
		common->Printf( "*** Left Hand %s***\n", s.c_str() );
	else
		common->Printf( "*** Hand %d %s***\n", whichHand, s.c_str() );
	if( !this->owner )
		common->Printf( "  ERROR: No owner!\n" );
	else
		common->Printf( "  owner = '%s'\n", owner->name.c_str() );
	if( !handExists() )
		common->Printf( "  doesn't exist!\n" );

	// gestures
	common->Printf( "  in holster slot %d\n", handSlot );
	s = "";
	if( grabbingWorld )
		s += "grip ";
	if( virtualGrabDown )
		s += "vGrab ";
	if( triggerDown )
		s += "trigger ";
	if( thumbDown )
		s += "thumb ";
	common->Printf( "  buttons = %s", s.c_str() );
	s = "";
	if( oldGrabbingWorld )
		s += "grip ";
	if( oldVirtualGrabDown )
		s += "vGrab ";
	if( oldTriggerDown )
		s += "trigger ";
	if( oldFlashlightTriggerDown )
		s += "flashlight ";
	if( oldThumbDown )
		s += "thumb ";
	common->Printf( ", old = %s\n", s.c_str() );

	// weapon
	s = "";
	if( isTheDuplicate )
		s += " (duplicate copy)";
	if( weaponGone )
		s += ", gone";
	if( currentWeapon == idealWeapon.Get() )
		common->Printf( "  current/ideal weapon = %s = %d%s\n", GetCurrentWeaponString().c_str(), currentWeapon, s.c_str() );
	else
	{
		common->Printf( "  currentWeapon = %s = %d%s\n", GetCurrentWeaponString().c_str(), currentWeapon, s.c_str() );
		common->Printf( "  idealWeapon = %d, weaponSwitchTime (unused?) = %dms\n", idealWeapon.Get(), weaponSwitchTime - gameLocal.time );
	}
	common->Printf( "  previousWeapon = %d\n", previousWeapon );
	idWeapon* weap = weapon.GetEntity();
	if( weap )
	{
		if( weap->weaponDef )
		{
			common->Printf( "  weapon (%s) = %s = %d\n", weap->GetName(), weap->weaponDef->GetName(), weap->IdentifyWeapon() );
		}
		else
		{
			common->Printf( "  weapon (%s): weaponDef = NULL, IdentifyWeapon = %d\n", weap->GetName(), weap->IdentifyWeapon() );
		}
	}
	else
	{
		common->Printf( "  weapon = NULL\n" );
	}
	// sight
	common->Printf( "  laser sight: active=%d, handle=%d\n", laserSightActive, laserSightHandle );
	common->Printf( "  crosshair: lastMode=%d, handle=%d\n", lastCrosshairMode, crosshairHandle );
	// PDA
	common->Printf( "  PDA (was %d): fixed=%d (last=%d), ", PDAfixed, lastPdaFixed, wasPDA );
	// Functions
	common->Printf( "  holdingWeapon=%d, floatingWeapon=%d, controllingWeapon=%d, tooFullToInteract=%d\n", holdingWeapon(), floatingWeapon(), controllingWeapon(), tooFullToInteract() );
	common->Printf( "  holdingFlashlight=%d, isOverMountedFlashlight=%d, holdingPDA=%d, holdingPhysics=%d, holdingItem=%d, holdingSomethingDroppable=%d\n", holdingFlashlight(), isOverMountedFlashlight(), holdingPDA(), holdingPhysics(), holdingItem(), holdingSomethingDroppable() );
}


/*
==============
idPlayer::idPlayer
==============
*/
idPlayer::idPlayer():
	previousViewQuat( 0.0f, 0.0f, 0.0f, 1.0f ),
	nextViewQuat( 0.0f, 0.0f, 0.0f, 1.0f ),
	serverOverridePositionTime( 0 ),
	clientFireCount( 0 )
{
	aas = NULL;
	travelFlags = TFL_WALK | TFL_AIR | TFL_CROUCH | TFL_WALKOFFLEDGE | TFL_BARRIERJUMP | TFL_JUMP | TFL_LADDER | TFL_WATERJUMP | TFL_ELEVATOR | TFL_SPECIAL;
	aimValidForTeleport = false;

	
	teleportAimPoint = vec3_zero;
	teleportPoint = vec3_zero;
	teleportAimPointPitch = 0.0f;

	warpMove				= false;
	warpAim					= false;
	warpVel					= vec3_zero;
	noclip					= false;
	godmode					= false;

	jetMove					= false;
	jetMoveVel				= vec3_zero;

	spawnAnglesSet			= false;
	spawnAngles				= ang_zero;
	viewAngles				= ang_zero;
	cmdAngles				= ang_zero;
	
	// Koz begin : for independent weapon aiming in VR.
	commonVr->independentWeaponYaw = 0.0f;
	commonVr->independentWeaponPitch = 0.0f;
	// Koz end
	
	oldButtons				= 0;
	buttonMask				= 0;
	oldImpulseSequence		= 0;
	
	lastHitTime				= 0;
	lastSndHitTime			= 0;
	lastSavingThrowTime		= 0;
	
	pdaModelDefHandle = -1;
	memset( &pdaRenderEntity, 0, sizeof( pdaRenderEntity ) );
	
	holsterModelDefHandle = -1;
	memset( &holsterRenderEntity, 0, sizeof( holsterRenderEntity ) );

	// Koz begin
	headingBeamHandle = -1;
	memset( &headingBeamEntity, 0, sizeof( headingBeamEntity ) );

	hudHandle = -1;
	memset( &hudEntity, 0, sizeof( hudEntity ) );
	// Koz end


	primaryObjective		= NULL;
	
	hudManager				= new idMenuHandler_HUD();
	hud						= NULL;
	objectiveSystemOpen		= false;
	memset( quickSlot, -1, sizeof( quickSlot ) );
	
	pdaMenu = new( TAG_SWF ) idMenuHandler_PDA();
	pdaVideoMat				= NULL;
	mpMessages				= NULL;
	
	mountedObject			= NULL;
	enviroSuitLight			= NULL;
	
	heartRate				= BASE_HEARTRATE;
	heartInfo.Init( 0, 0, 0, 0 );
	lastHeartAdjust			= 0;
	lastHeartBeat			= 0;
	lastDmgTime				= 0;
	deathClearContentsTime	= 0;
	lastArmorPulse			= -10000;
	stamina					= 0.0f;
	healthPool				= 0.0f;
	nextHealthPulse			= 0;
	healthPulse				= false;
	nextHealthTake			= 0;
	healthTake				= false;
	
	forceScoreBoard			= false;
	forceRespawn			= false;
	spectating				= false;
	spectator				= 0;
	wantSpectate			= true;
	
	carryingFlag			= false;
	
	lastHitToggle			= false;
	
	minRespawnTime			= 0;
	maxRespawnTime			= 0;
	
	firstPersonViewOrigin	= vec3_zero;
	firstPersonViewAxis		= mat3_identity;

	hipJoint				= INVALID_JOINT;
	chestJoint				= INVALID_JOINT;
	headJoint				= INVALID_JOINT;
	
	bobFoot					= 0;
	bobFrac					= 0.0f;
	bobfracsin				= 0.0f;
	bobCycle				= 0;
	xyspeed					= 0.0f;
	stepUpTime				= 0;
	stepUpDelta				= 0.0f;
	idealLegsYaw			= 0.0f;
	legsYaw					= 0.0f;
	legsForward				= true;
	oldViewYaw				= 0.0f;
	viewBobAngles			= ang_zero;
	viewBob					= vec3_zero;
	landChange				= 0;
	landTime				= 0;
	
	weaponEnabled			= true;
	risingWeaponHand		= -1;
	weapon_soulcube			= -1;
	weapon_pda				= -1;
	weapon_fists			= -1;
	weapon_chainsaw			= -1;
	weapon_bloodstone		= -1;
	weapon_bloodstone_active1 = -1;
	weapon_bloodstone_active2 = -1;
	weapon_bloodstone_active3 = -1;
	harvest_lock			= false;
	
	hudPowerup				= -1;
	lastHudPowerup			= -1;
	hudPowerupDuration		= 0;
	
	skinIndex				= 0;
	skin					= NULL;
	powerUpSkin				= NULL;
	
	numProjectileKills		= 0;
	numProjectilesFired		= 0;
	numProjectileHits		= 0;
	
	airless					= false;
	airMsec					= 0;
	lastAirDamage			= 0;
	
	gibDeath				= false;
	gibsLaunched			= false;
	gibsDir					= vec3_zero;
	
	zoomFov.Init( 0, 0, 0, 0 );
	centerView.Init( 0, 0, 0, 0 );
	fxFov					= false;
	
	influenceFov			= 0;
	influenceActive			= 0;
	influenceRadius			= 0.0f;
	influenceEntity			= NULL;
	influenceMaterial		= NULL;
	influenceSkin			= NULL;
	
	privateCameraView		= NULL;
	
	memset( loggedViewAngles, 0, sizeof( loggedViewAngles ) );
	memset( loggedAccel, 0, sizeof( loggedAccel ) );
	currentLoggedAccel	= 0;
	
	focusTime				= 0;
	focusGUIent				= NULL;
	focusUI					= NULL;
	focusCharacter			= NULL;
	talkCursor				= 0;
	focusVehicle			= NULL;
	cursor					= NULL;
	
	oldMouseX				= 0;
	oldMouseY				= 0;
	
	lastDamageDef			= 0;
	lastDamageDir			= vec3_zero;
	lastDamageLocation		= 0;
	smoothedFrame			= 0;
	smoothedOriginUpdated	= false;
	smoothedOrigin			= vec3_zero;
	smoothedAngles			= ang_zero;
	
	fl.networkSync			= true;
	
	doingDeathSkin			= false;
	weaponGone				= false;
	useInitialSpawns		= false;
	tourneyRank				= 0;
	lastSpectateTeleport	= 0;
	tourneyLine				= 0;
	hiddenWeapon			= false;
	tipUp					= false;
	objectiveUp				= false;
	teleportEntity			= NULL;
	teleportKiller			= -1;
	respawning				= false;
	leader					= false;
	lastSpectateChange		= 0;
	lastTeleFX				= -9999;
	weaponCatchup			= false;
	clientFireCount			= 0;
	
	MPAim					= -1;
	lastMPAim				= -1;
	lastMPAimTime			= 0;
	MPAimFadeTime			= 0;
	MPAimHighlight			= false;
	
	spawnedTime				= 0;
	lastManOver				= false;
	lastManPlayAgain		= false;
	lastManPresent			= false;
	
	isTelefragged			= false;
	
	isLagged				= false;
	isChatting				= 0;
	
	selfSmooth				= false;
	
	playedTimeSecs			= 0;
	playedTimeResidual		= 0;
	
	ResetControllerShake();
	
	memset( pdaHasBeenRead, 0, sizeof( pdaHasBeenRead ) );
	memset( videoHasBeenViewed, 0, sizeof( videoHasBeenViewed ) );
	memset( audioHasBeenHeard, 0, sizeof( audioHasBeenHeard ) );

	// Koz begin
	//common->Printf( "Setting headingbeam active\n" );
	//headingBeamActive = vr_headingBeamMode.GetInteger() != 0;
	//hudActive = true;
	// Koz end

	blink = false;
}

/*
==============
idPlayer::LinkScriptVariables

set up conditions for animation
==============
*/
void idPlayer::LinkScriptVariables()
{
	AI_FORWARD.LinkTo(	scriptObject, "AI_FORWARD" );
	AI_BACKWARD.LinkTo(	scriptObject, "AI_BACKWARD" );
	AI_STRAFE_LEFT.LinkTo(	scriptObject, "AI_STRAFE_LEFT" );
	AI_STRAFE_RIGHT.LinkTo(	scriptObject, "AI_STRAFE_RIGHT" );
	AI_ATTACK_HELD.LinkTo(	scriptObject, "AI_ATTACK_HELD" );
	AI_WEAPON_FIRED.LinkTo(	scriptObject, "AI_WEAPON_FIRED" );
	AI_JUMP.LinkTo(	scriptObject, "AI_JUMP" );
	AI_DEAD.LinkTo(	scriptObject, "AI_DEAD" );
	AI_CROUCH.LinkTo(	scriptObject, "AI_CROUCH" );
	AI_ONGROUND.LinkTo(	scriptObject, "AI_ONGROUND" );
	AI_ONLADDER.LinkTo(	scriptObject, "AI_ONLADDER" );
	AI_HARDLANDING.LinkTo(	scriptObject, "AI_HARDLANDING" );
	AI_SOFTLANDING.LinkTo(	scriptObject, "AI_SOFTLANDING" );
	AI_RUN.LinkTo(	scriptObject, "AI_RUN" );
	AI_PAIN.LinkTo(	scriptObject, "AI_PAIN" );
	AI_RELOAD.LinkTo(	scriptObject, "AI_RELOAD" );
	AI_TELEPORT.LinkTo(	scriptObject, "AI_TELEPORT" );
	AI_TURN_LEFT.LinkTo(	scriptObject, "AI_TURN_LEFT" );
	AI_TURN_RIGHT.LinkTo(	scriptObject, "AI_TURN_RIGHT" );
}

/*
==============
idPlayer::SetupWeaponEntity
==============
*/
void idPlayer::SetupWeaponEntity()
{
	int w;
	const char* weap;

	// Carl: dual wielding
	for( int h = 0; h < 2; h++ )
	{
		if( hands[h].weapon.GetEntity() )
		{
			// get rid of old weapon
			hands[h].weapon.GetEntity()->Clear();
			hands[h].currentWeapon = -1; // carl: todo dual wielding
		}
		else if( !common->IsClient() )
		{
			hands[h].weapon = static_cast< idWeapon* >( gameLocal.SpawnEntityType( idWeapon::Type, NULL ) );
			hands[h].weapon.GetEntity()->SetOwner( this, h );
			hands[h].currentWeapon = -1; // carl: todo dual wielding
		}
	}
	if( flashlight.GetEntity() )
	{
	}
	else if( !common->IsClient() )
	{
		// flashlight
		flashlight = static_cast<idWeapon*>( gameLocal.SpawnEntityType( idWeapon::Type, NULL ) );
		flashlight.GetEntity()->SetFlashlightOwner( this );
		//FlashlightOff();
				
	}
	
	for( w = 0; w < MAX_WEAPONS; w++ )
	{
		weap = spawnArgs.GetString( va( "def_weapon%d", w ) );
		if( weap != NULL && *weap != '\0' )
		{
			idWeapon::CacheWeapon( weap );
		}
	}
}

/*
==============
idPlayer::Init
==============
*/
void idPlayer::Init()
{
	const char*			value;
	const idKeyValue*	kv;
	
	noclip					= false;
	godmode					= false;
	
	oldButtons				= 0;
	oldImpulseSequence		= 0;
	
	weaponEnabled			= true;
	risingWeaponHand		= -1;
	weapon_soulcube			= SlotForWeapon( "weapon_soulcube" );
	weapon_pda				= SlotForWeapon( "weapon_pda" );
	weapon_fists			= SlotForWeapon( "weapon_fists" );
	weapon_flashlight		= SlotForWeapon( "weapon_flashlight" );
	weapon_chainsaw			= SlotForWeapon( "weapon_chainsaw" );
	weapon_bloodstone		= SlotForWeapon( "weapon_bloodstone_passive" );
	weapon_bloodstone_active1 = SlotForWeapon( "weapon_bloodstone_active1" );
	weapon_bloodstone_active2 = SlotForWeapon( "weapon_bloodstone_active2" );
	weapon_bloodstone_active3 = SlotForWeapon( "weapon_bloodstone_active3" );
	harvest_lock			= false;

	// Koz begin;
	weapon_pistol			= SlotForWeapon( "weapon_pistol" );
	weapon_shotgun			= SlotForWeapon( "weapon_shotgun" );
	weapon_shotgun_double	= SlotForWeapon( "weapon_shotgun_double" );
	weapon_machinegun		= SlotForWeapon( "weapon_machinegun" );
	weapon_chaingun			= SlotForWeapon( "weapon_chaingun" );
	weapon_handgrenade		= SlotForWeapon( "weapon_handgrenade" );
	weapon_plasmagun		= SlotForWeapon( "weapon_plasmagun" );
	weapon_rocketlauncher	= SlotForWeapon( "weapon_rocketlauncher" );
	weapon_bfg				= SlotForWeapon( "weapon_bfg" );
	weapon_flashlight_new	= SlotForWeapon( "weapon_flashlight_new" );
	weapon_grabber			= SlotForWeapon( "weapon_grabber" );
	// Koz end
	
	lastDmgTime				= 0;
	lastArmorPulse			= -10000;
	lastHeartAdjust			= 0;
	lastHeartBeat			= 0;
	heartInfo.Init( 0, 0, 0, 0 );
	
	bobCycle				= 0;
	bobFrac					= 0.0f;
	landChange				= 0;
	landTime				= 0;
	zoomFov.Init( 0, 0, 0, 0 );
	centerView.Init( 0, 0, 0, 0 );
	fxFov					= false;
	
	influenceFov			= 0;
	influenceActive			= 0;
	influenceRadius			= 0.0f;
	influenceEntity			= NULL;
	influenceMaterial		= NULL;
	influenceSkin			= NULL;
	
	mountedObject			= NULL;
	if( enviroSuitLight.IsValid() )
	{
		enviroSuitLight.GetEntity()->PostEventMS( &EV_Remove, 0 );
	}
	enviroSuitLight			= NULL;
	healthRecharge			= false;
	lastHealthRechargeTime	= 0;
	rechargeSpeed			= 500;
	new_g_damageScale		= 1.f;
	bloomEnabled			= false;
	bloomSpeed				= 1.f;
	bloomIntensity			= -0.01f;
	inventory.InitRechargeAmmo( this );
	hudPowerup				= -1;
	lastHudPowerup			= -1;
	hudPowerupDuration		= 0;
	
	currentLoggedAccel		= 0;
	
	handRaised = false;
	handLowered = false;

	focusTime				= 0;
	focusGUIent				= NULL;
	focusUI					= NULL;
	focusCharacter			= NULL;
	talkCursor				= 0;
	focusVehicle			= NULL;
	
	// remove any damage effects
	playerView.ClearEffects();
	
	// damage values
	fl.takedamage			= true;
	ClearPain();
	
	// Koz reset holster slots
	
	holsteredWeapon = weapon_fists;
	extraHolsteredWeapon = weapon_fists;
	extraHolsteredWeaponModel = NULL;
	
	// restore persistent data
	RestorePersistantInfo();
	
	bobCycle		= 0;
	stamina			= 0.0f;
	healthPool		= 0.0f;
	nextHealthPulse = 0;
	healthPulse		= false;
	nextHealthTake	= 0;
	healthTake		= false;
	
	for( int hand = 0; hand < 2; hand++ )
		hands[ hand ].Init( this, hand );
	SetupWeaponEntity();
	
	heartRate = BASE_HEARTRATE;
	AdjustHeartRate( BASE_HEARTRATE, 0.0f, 0.0f, true );
	
	idealLegsYaw = 0.0f;
	legsYaw = 0.0f;
	legsForward	= true;
	oldViewYaw = 0.0f;
	
	// set the pm_ cvars
	if( !common->IsMultiplayer() || common->IsServer() )
	{
		kv = spawnArgs.MatchPrefix( "pm_", NULL );
		while( kv )
		{
			cvarSystem->SetCVarString( kv->GetKey(), kv->GetValue() );
			kv = spawnArgs.MatchPrefix( "pm_", kv );
		}
	}
	
	// disable stamina on hell levels
	if( gameLocal.world && gameLocal.world->spawnArgs.GetBool( "no_stamina" ) )
	{
		pm_stamina.SetFloat( 0.0f );
	}
	
	// stamina always initialized to maximum
	stamina = pm_stamina.GetFloat();
	
	// air always initialized to maximum too
	airMsec = pm_airMsec.GetFloat();
	airless = false;
	
	gibDeath = false;
	gibsLaunched = false;
	gibsDir.Zero();
	
	// set the gravity
	physicsObj.SetGravity( gameLocal.GetGravity() );
	
	// start out standing
	SetEyeHeight( pm_normalviewheight.GetFloat() );
	
	stepUpTime = 0;
	stepUpDelta = 0.0f;
	viewBobAngles.Zero();
	viewBob.Zero();
	
	value = spawnArgs.GetString( "model" );
	if( value != NULL && ( *value != 0 ) )
	{
		SetModel( value );
	}
	
	if( hud )
	{
		hud->SetCursorState( this, CURSOR_TALK, 0 );
		hud->SetCursorState( this, CURSOR_IN_COMBAT, 1 );
		hud->SetCursorState( this, CURSOR_ITEM, 0 );
		hud->SetCursorState( this, CURSOR_GRABBER, 0 );
		hud->SetCursorState( this, CURSOR_NONE, 0 );
		hud->UpdateCursorState();
	}
	
	if( ( common->IsMultiplayer() || g_testDeath.GetBool() ) && skin )
	{
		SetSkin( skin );
		renderEntity.shaderParms[6] = 0.0f;
	}
	else if( spawnArgs.GetString( "spawn_skin", NULL, &value ) )
	{
		skin = declManager->FindSkin( value );
		SetSkin( skin );
		renderEntity.shaderParms[6] = 0.0f;
	}
	
	InitPlayerBones(); 

	commonVr->currentFlashlightMode = vr_flashlightMode.GetInteger();

	commonVr->thirdPersonMovement = false;
	commonVr->thirdPersonDelta = 0.0f;
	commonVr->thirdPersonHudPos = vec3_zero;
	commonVr->thirdPersonHudAxis = mat3_identity;

	// Koz end	

	// initialize the script variables
	AI_FORWARD		= false;
	AI_BACKWARD		= false;
	AI_STRAFE_LEFT	= false;
	AI_STRAFE_RIGHT	= false;
	AI_ATTACK_HELD	= false;
	AI_WEAPON_FIRED	= false;
	AI_JUMP			= false;
	AI_DEAD			= false;
	AI_CROUCH		= false;
	AI_ONGROUND		= true;
	AI_ONLADDER		= false;
	AI_HARDLANDING	= false;
	AI_SOFTLANDING	= false;
	AI_RUN			= false;
	AI_PAIN			= false;
	AI_RELOAD		= false;
	AI_TELEPORT		= false;
	AI_TURN_LEFT	= false;
	AI_TURN_RIGHT	= false;
	
	// reset the script object
	ConstructScriptObject();
	
	// execute the script so the script object's constructor takes effect immediately
	scriptThread->Execute();
	
	forceScoreBoard		= false;
	
	privateCameraView	= NULL;
	
	lastSpectateChange	= 0;
	lastTeleFX			= -9999;
	
	hiddenWeapon		= false;
	tipUp				= false;
	objectiveUp			= false;
	teleportEntity		= NULL;
	teleportKiller		= -1;
	leader				= false;
	
	SetPrivateCameraView( NULL );
	
	MPAim				= -1;
	lastMPAim			= -1;
	lastMPAimTime		= 0;
	MPAimFadeTime		= 0;
	MPAimHighlight		= false;
	
	//isChatting = false;
	
	achievementManager.Init( this );
	
	flashlightBattery = flashlight_batteryDrainTimeMS.GetInteger();		// fully charged
	
	aimAssist.Init( this );
	
	SetupPDASlot( true );
	

	// Koz begin
	
	// model to place hud in 3d space
	memset( &hudEntity, 0, sizeof( hudEntity ) );
	hudEntity.hModel = renderModelManager->FindModel( "/models/mapobjects/hud.lwo" );
	hudEntity.customShader = declManager->FindMaterial( "vr/hud" );
	hudEntity.weaponDepthHack = vr_hudOcclusion.GetBool();
	
	skinCrosshairDot = declManager->FindSkin( "skins/vr/crosshairDot" );
	skinCrosshairCircleDot = declManager->FindSkin( "skins/vr/crosshairCircleDot" );
	skinCrosshairCross = declManager->FindSkin( "skins/vr/crosshairCross" );
		

	// heading indicator for VR - point the direction the body is facing.
	memset( &headingBeamEntity, 0, sizeof( headingBeamEntity ) );
	headingBeamEntity.hModel = renderModelManager->FindModel( "/models/mapobjects/headingbeam.lwo" );
	skinHeadingSolid = declManager->FindSkin( "skins/models/headingbeamsolid" );
	skinHeadingArrows = declManager->FindSkin( "skins/models/headingbeamarrows" );
	skinHeadingArrowsScroll = declManager->FindSkin( "skins/models/headingbeamarrowsscroll" );
				
//	common->Printf( "Setting headingbeam active\n" );
//	headingBeamActive = vr_headingBeamMode.GetInteger() != 0;
	hudActive = true;

	PDAorigin = vec3_zero;
	PDAaxis = mat3_identity;
	
	
	InitTeleportTarget();

	aasState = 0;
	// Koz end

}


/*
==============
idPlayer::InitPlayerBones
Koz - moved bone inits here, called during player restore as well.
==============
*/
void idPlayer::InitPlayerBones()
{
	const char*			value;
	
	value = spawnArgs.GetString( "bone_hips", "" );
	hipJoint = animator.GetJointHandle( value );
	if ( hipJoint == INVALID_JOINT )
	{
		gameLocal.Error( "Joint '%s' not found for 'bone_hips' on '%s'", value, name.c_str() );
	}

	value = spawnArgs.GetString( "bone_chest", "" );
	chestJoint = animator.GetJointHandle( value );
	if ( chestJoint == INVALID_JOINT )
	{
		gameLocal.Error( "Joint '%s' not found for 'bone_chest' on '%s'", value, name.c_str() );
	}

	value = spawnArgs.GetString( "bone_head", "" );
	headJoint = animator.GetJointHandle( value );
	if ( headJoint == INVALID_JOINT )
	{
		gameLocal.Error( "Joint '%s' not found for 'bone_head' on '%s'", value, name.c_str() );
	}

	// Koz begin
	value = spawnArgs.GetString( "bone_neck", "" );
	neckJoint = animator.GetJointHandle( value );
	if ( neckJoint == INVALID_JOINT )
	{
		gameLocal.Error( "Joint '%s' not found for 'bone_neck' on '%s'", value, name.c_str() );
	}

	value = spawnArgs.GetString( "bone_chest_pivot", "" );
	chestPivotJoint = animator.GetJointHandle( value );
	if ( chestPivotJoint == INVALID_JOINT )
	{
		gameLocal.Error( "Joint '%s' not found for 'bone_chest_pivot' on '%s'", value, name.c_str() );
	}

	// we need to load the starting joint orientations for the hands so we can compute correct offsets later
	value = spawnArgs.GetString( "ik_hand1", "" ); // right hand
	ik_hand[0] = animator.GetJointHandle( value );
	if ( ik_hand[0] == INVALID_JOINT )
	{
		gameLocal.Error( "Joint '%s' not found for 'ik_hand1' on '%s'", value, name.c_str() );
	}

	value = spawnArgs.GetString( "ik_hand2", "" );// left hand
	ik_hand[1] = animator.GetJointHandle( value );
	if ( ik_hand[1] == INVALID_JOINT )
	{
		gameLocal.Error( "Joint '%s' not found for 'ik_hand2' on '%s'", value, name.c_str() );
	}

	ik_handAttacher[0] = animator.GetJointHandle( "RhandWeap" );
	if ( ik_handAttacher[0] == INVALID_JOINT )
	{
		gameLocal.Error( "Joint RhandWeap not found for player anim default\n" );
	}

	ik_handAttacher[1] = animator.GetJointHandle( "LhandWeap" );

	if ( ik_handAttacher[1] == INVALID_JOINT )
	{
		gameLocal.Error( "Joint LhandWeap not found for player anim default\n" );
	}

	idStr animPre = "default";// this is the anim that has the default/normal hand and weapon attacher orientations (relationsh

	int animNo = animator.GetAnim( animPre.c_str() );
	if ( animNo == 0 )
	{
		gameLocal.Error( "Player default animation not found\n" );
	}

	int numJoints = animator.NumJoints();

	idJointMat* joints = (idJointMat*)_alloca16( numJoints * sizeof( joints[0] ) );

	// create the idle default pose ( in this case set to default which should tranlsate to pistol_idle )
	gameEdit->ANIM_CreateAnimFrame( animator.ModelHandle(), animator.GetAnim( animNo )->MD5Anim( 0 ), numJoints, joints, 1, animator.ModelDef()->GetVisualOffset() + modelOffset, animator.RemoveOrigin() );



	static idVec3 defaultWeapAttachOff[2]; // the default distance between the weapon attacher and the hand joint;
	defaultWeapAttachOff[0] = joints[ik_handAttacher[0]].ToVec3() - joints[ik_hand[0]].ToVec3(); // default 
	defaultWeapAttachOff[1] = joints[ik_handAttacher[1]].ToVec3() - joints[ik_hand[1]].ToVec3();

	jointHandle_t j1;
	value = spawnArgs.GetString( "ik_elbow1", "" );// right
	j1 = animator.GetJointHandle( value );
	if ( j1 == INVALID_JOINT )
	{
		gameLocal.Error( "Joint ik_elbow1 not found for player anim default\n" );
	}
	ik_elbowCorrectAxis[0] = joints[j1].ToMat3();

	value = spawnArgs.GetString( "ik_elbow2", "" );// left 
	j1 = animator.GetJointHandle( value );
	if ( j1 == INVALID_JOINT )
	{
		gameLocal.Error( "Joint ik_elbow2 not found for player anim default\n" );
	}
	ik_elbowCorrectAxis[1] = joints[j1].ToMat3();

	chestPivotCorrectAxis = joints[chestPivotJoint].ToMat3();
	chestPivotDefaultPos = joints[chestPivotJoint].ToVec3();
	commonVr->chestDefaultDefined = true;



	common->Printf( "Animpre hand 0 default offset = %s\n", defaultWeapAttachOff[0].ToString() );
	common->Printf( "Animpre hand 1 default offset = %s\n", defaultWeapAttachOff[1].ToString() );

	// now calc the weapon attacher offsets
	for ( int hand = 0; hand < 2; hand++ )
	{
		for ( int weap = 0; weap < 32; weap++ ) // should be max weapons
		{

			idStr animPre = spawnArgs.GetString( va( "def_weapon%d", weap ) );
			animPre.Strip( "weapon_" );
			animPre += "_idle";

			int animNo = animator.GetAnim( animPre.c_str() );
			int numJoints = animator.NumJoints();

			if ( animNo == 0 ) continue;

			//	common->Printf( "Animpre = %s animNo = %d\n", animPre.c_str(), animNo );

			// create the idle pose for this weapon
			gameEdit->ANIM_CreateAnimFrame( animator.ModelHandle(), animator.GetAnim( animNo )->MD5Anim( 0 ), numJoints, joints, 1, animator.ModelDef()->GetVisualOffset() + modelOffset, animator.RemoveOrigin() );

			ik_handCorrectAxis[hand][weap] = joints[ik_hand[hand]].ToMat3();
			//	common->Printf( "Hand %d weap %d anim %s attacher pos %s   default pos %s\n", hand, weap, animPre.c_str(), joints[ik_handAttacher[hand]].ToVec3().ToString(), defaultWeapAttachOff[hand].ToString() );

			//this is the translation between the hand joint ( the wrist ) and the attacher joint.  The attacher joint is 
			//the location in space where the motion control is locating the weapon / hand, but IK is using the 'wrist' to 
			//drive animation, so use this offset to derive the wrist position from the attacher joint orientation
			handWeaponAttachertoWristJointOffset[hand][weap] = joints[ik_handAttacher[hand]].ToVec3() - joints[ik_hand[hand]].ToVec3();

			// the is the delta if the attacher joint was moved from the position in the default animation to aid with alignment in 
			// different weapon animations.  To keep the hand in a consistant location when weapon is changed, 
			// the weapon and hand positions will need to be adjusted by this amount when presented
			handWeaponAttacherToDefaultOffset[hand][weap] = handWeaponAttachertoWristJointOffset[hand][weap] - defaultWeapAttachOff[hand];

			//	common->Printf( "Hand %d weap %d anim %s attacher offset = %s\n", hand, weap, animPre.c_str(), handWeaponAttacherToDefaultOffset[hand][weap].ToString() );
		}
	}

}

/*
==============
idPlayer::InitTeleportTarget
==============
*/
void idPlayer::InitTeleportTarget()
{
	idVec3 origin;
	int targetAnim; 
	idStr jointName;

	skinTelepadCrouch = declManager->FindSkin( "skins/vr/padcrouch" );
	
	commonVr->teleportButtonCount = 0;

	common->Printf( "Initializing teleport target\n" );
	origin = GetPhysics()->GetOrigin() + (origin + modelOffset) * GetPhysics()->GetAxis();

	if ( !( teleportTarget = (idAnimatedEntity*)gameLocal.FindEntity( "vrTeleportTarget" ) ) )
	{
		teleportTarget = (idAnimatedEntity*)gameLocal.SpawnEntityType( idAnimatedEntity::Type, NULL );
		teleportTarget->SetName( "vrTeleportTarget" );
	}
	teleportTarget.GetEntity()->SetModel( "telepad1" );
	teleportTarget.GetEntity()->SetOrigin( origin );
	teleportTarget.GetEntity()->SetAxis( GetPhysics()->GetAxis() );

	idAnimatedEntity *duplicate;
	if (duplicate = (idAnimatedEntity*)gameLocal.FindEntity("vrTeleportTarget2"))
	{
		common->Warning("Loading game which had a duplicate vrTeleportTarget.");
		duplicate->PostEventMS(&EV_Remove, 0);
	}
	
	teleportTargetAnimator = teleportTarget.GetEntity()->GetAnimator();
	targetAnim = teleportTargetAnimator->GetAnim( "idle" );
	//common->Printf( "Teleport target idle anim # = %d\n", targetAnim );
	teleportTargetAnimator->PlayAnim( ANIMCHANNEL_ALL, targetAnim, gameLocal.time, 0 );

	teleportPadJoint = teleportTargetAnimator->GetJointHandle( "pad" );

	if ( teleportPadJoint == INVALID_JOINT )
	{
		common->Printf( "Unable to find joint teleportPadJoint \n" );
	}

	teleportCenterPadJoint = teleportTargetAnimator->GetJointHandle( "centerpad" );

	if ( teleportCenterPadJoint == INVALID_JOINT )
	{
		common->Printf( "Unable to find joint teleportCenterPadJoint \n" );
	}

	for ( int i = 0; i < 24; i++ )
	{
		jointName = va( "padbeam%d", i + 1 );
		teleportBeamJoint[i] = teleportTargetAnimator->GetJointHandle( jointName.c_str() );
		if ( teleportBeamJoint[i] == INVALID_JOINT )
		{
			common->Printf( "Unable to find teleportBeamJoint %s\n", jointName.c_str() );
		}
	}

}

bool idPlayer::HasHoldableFlashlight()
{
	int fm = commonVr->currentFlashlightMode;
	if( vr_flashlightStrict.GetBool() )
	{
		if( fm != FLASHLIGHT_HAND && fm != FLASHLIGHT_INVENTORY )
			return false;
	}
	else
	{
		// In non-strict mode you can move the flashlight from your armour or helmet to your hand,
		// but you can't move it from your weapon.
		if( fm == FLASHLIGHT_GUN || fm == FLASHLIGHT_PISTOL || fm == FLASHLIGHT_NONE )
			return false;
	}
	// Carl: I'm not checking the weapons in the inventory. Maybe I should check that too? But I don't trust those values for the flashlight.
	return flashlight.GetEntity() && flashlight.GetEntity()->IsLinked() && !spectating && weaponEnabled && !hiddenWeapon && !gameLocal.world->spawnArgs.GetBool( "no_Weapons" );
}

/*
==============
idPlayer::Spawn

Prepare any resources used by the player.
==============
*/
void idPlayer::Spawn()
{
	idStr		temp;
	idBounds	bounds;
	
	if( entityNumber >= MAX_CLIENTS )
	{
		gameLocal.Error( "entityNum > MAX_CLIENTS for player.  Player may only be spawned with a client." );
	}
	
	// allow thinking during cinematics
	cinematic = true;
	
	if( common->IsMultiplayer() )
	{
		// always start in spectating state waiting to be spawned in
		// do this before SetClipModel to get the right bounding box
		spectating = true;
	}
	
	// set our collision model
	physicsObj.SetSelf( this );
	SetClipModel();
	physicsObj.SetMass( spawnArgs.GetFloat( "mass", "100" ) );
	physicsObj.SetContents( CONTENTS_BODY );
	physicsObj.SetClipMask( MASK_PLAYERSOLID );
	SetPhysics( &physicsObj );

	SetAAS();
	InitAASLocation();
	
	skin = renderEntity.customSkin;
	
	// only the local player needs guis
	if( !common->IsMultiplayer() || IsLocallyControlled() )
	{
	
		// load HUD
		if( hudManager != NULL )
		{
			hudManager->Initialize( "hud", common->SW() );
			hudManager->ActivateMenu( true );
			hud = hudManager->GetHud();
		}
		
		// load cursor
		if( spawnArgs.GetString( "cursor", "", temp ) )
		{
			cursor = uiManager->FindGui( temp, true, common->IsMultiplayer(), common->IsMultiplayer() );
		}
		if( cursor )
		{
			cursor->Activate( true, gameLocal.time );
		}
		
		if( pdaMenu != NULL )
		{
			pdaMenu->Initialize( "pda", common->SW() );
		}
		objectiveSystemOpen = false;
	}
	
	if( common->IsMultiplayer() && mpMessages == NULL )
	{
		mpMessages = new idSWF( "mp_messages", common->SW() );
		mpMessages->Activate( true );
	}
	
	SetLastHitTime( 0 );
	
	// load the armor sound feedback
	declManager->FindSound( "player_sounds_hitArmor" );
	
	// set up conditions for animation
	LinkScriptVariables();
	
	animator.RemoveOriginOffset( true );
	
	// create combat collision hull for exact collision detection
	SetCombatModel();
	
	// init the damage effects
	playerView.SetPlayerEntity( this );
	
	// Koz
	if ( game->isVR )
	{
		//Carl: don't suppress drawing the player's body in 1st person if we want to see it (in VR)
		renderEntity.suppressSurfaceInViewID = 0;
	}
	else
	{
		// supress model in non-player views, but allow it in mirrors and remote views
		renderEntity.suppressSurfaceInViewID = entityNumber + 1;
	
	}
	// Koz end
	
	// don't project shadow on self or weapon
	renderEntity.noSelfShadow = true;
	
	idAFAttachment* headEnt = head.GetEntity();
	if( headEnt )
	{
		headEnt->GetRenderEntity()->suppressSurfaceInViewID = entityNumber + 1; 
		headEnt->GetRenderEntity()->noSelfShadow = true;
		
	}
	
	if( common->IsMultiplayer() )
	{
		Init();
		Hide();	// properly hidden if starting as a spectator
		if( !common->IsClient() )
		{
			// set yourself ready to spawn. idMultiplayerGame will decide when/if appropriate and call SpawnFromSpawnSpot
			SetupWeaponEntity();
			SpawnFromSpawnSpot();
			forceRespawn = true;
			wantSpectate = true;
			assert( spectating );
		}
	}
	else
	{
		SetupWeaponEntity();
		SpawnFromSpawnSpot();
	}
	
	// trigger playtesting item gives, if we didn't get here from a previous level
	// the devmap key will be set on the first devmap, but cleared on any level
	// transitions
	if( !common->IsMultiplayer() && gameLocal.serverInfo.FindKey( "devmap" ) )
	{
		// fire a trigger with the name "devmap"
		idEntity* ent = gameLocal.FindEntity( "devmap" );
		if( ent )
		{
			ent->ActivateTargets( this );
		}
	}
	
	if( hud )
	{
		if( weapon_soulcube > 0 && ( inventory.weapons & ( 1 << weapon_soulcube ) ) )
		{
			int max_souls = inventory.MaxAmmoForAmmoClass( this, "ammo_souls" );
			if( inventory.GetInventoryAmmoForType( idWeapon::GetAmmoNumForName( "ammo_souls" ) ) >= max_souls )
			{
				hud->SetShowSoulCubeOnLoad( true );
			}
		}
	}
	
	if( GetPDA() )
	{
		// Add any emails from the inventory
		for( int i = 0; i < inventory.emails.Num(); i++ )
		{
			GetPDA()->AddEmail( inventory.emails[i] );
		}
		GetPDA()->SetSecurity( idLocalization::GetString( "#str_00066" ) );
	}
	
	if( gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) )
	{
		hiddenWeapon = true;
		// Carl: dual wielding
		for( int h = 0; h < 2; h++ )
		{
			if( hands[h].weapon.GetEntity() )
			{
				if( !game->isVR ) hands[ h ].weapon.GetEntity()->LowerWeapon(); // Koz
			}
			hands[ h ].idealWeapon = weapon_fists;
		}
	}
	else
	{
		hiddenWeapon = false;
	}
	
	UpdateHudWeapon( vr_weaponHand.GetInteger() );
	
	tipUp = false;
	objectiveUp = false;
	
	if( inventory.levelTriggers.Num() )
	{
		PostEventMS( &EV_Player_LevelTrigger, 0 );
	}
	
	inventory.pdaOpened = false;
	inventory.selPDA = 0;
	
	if( !common->IsMultiplayer() )
	{
		int startingHealth = gameLocal.world->spawnArgs.GetInt( "startingHealth", health );
		if( health > startingHealth )
		{
			health = startingHealth;
		}
		if( g_skill.GetInteger() < 2 )
		{
			if( health < 25 )
			{
				health = 25;
			}
			if( g_useDynamicProtection.GetBool() )
			{
				new_g_damageScale = 1.0f;
			}
		}
		else
		{
			new_g_damageScale = 1.0f;
			g_armorProtection.SetFloat( ( g_skill.GetInteger() < 2 ) ? 0.4f : 0.2f );
			if( g_skill.GetInteger() == 3 )
			{
				nextHealthTake = gameLocal.time + g_healthTakeTime.GetInteger() * 1000;
			}
		}
	}
	
	//Setup the weapon toggle lists
	const idKeyValue* kv;
	kv = spawnArgs.MatchPrefix( "weapontoggle", NULL );
	while( kv )
	{
		WeaponToggle_t newToggle;
		strcpy( newToggle.name, kv->GetKey().c_str() );
		
		idStr toggleData = kv->GetValue();
		
		idLexer src;
		idToken token;
		src.LoadMemory( toggleData, toggleData.Length(), "toggleData" );
		while( 1 )
		{
			if( !src.ReadToken( &token ) )
			{
				break;
			}
			int index = atoi( token.c_str() );
			newToggle.toggleList.Append( index );
			
			//Skip the ,
			src.ReadToken( &token );
		}
		newToggle.lastUsed = 0;
		weaponToggles.Set( newToggle.name, newToggle );
		
		kv = spawnArgs.MatchPrefix( "weapontoggle", kv );
	}
	
	if( g_skill.GetInteger() >= 3 || cvarSystem->GetCVarBool( "fs_buildresources" ) )
	{
		if( !WeaponAvailable( "weapon_bloodstone_passive" ) )
		{
			GiveInventoryItem( "weapon_bloodstone_passive" );
		}
		if( !WeaponAvailable( "weapon_bloodstone_active1" ) )
		{
			GiveInventoryItem( "weapon_bloodstone_active1" );
		}
		if( !WeaponAvailable( "weapon_bloodstone_active2" ) )
		{
			GiveInventoryItem( "weapon_bloodstone_active2" );
		}
		if( !WeaponAvailable( "weapon_bloodstone_active3" ) )
		{
			GiveInventoryItem( "weapon_bloodstone_active3" );
		}
	}
	
	bloomEnabled			= false;
	bloomSpeed				= 1;
	bloomIntensity			= -0.01f;
	
	if( g_demoMode.GetBool() )
	{
		for( int h = 0; h < 2; h++ )
		{
			idWeapon* weapon = GetWeaponInHand( h );
			if( weapon && weapon->AmmoInClip() == 0 )
				weapon->ForceAmmoInClip();
		}
	}
	
	// Koz fixme ovr_RecenterTrackingOrigin( commonVr->hmdSession ); // Koz reset hmd orientation  Koz fixme check if still appropriate here.
	common->Printf( "Spawn orientation reset\n" );
	//commonVr->HMDResetTrackingOriginOffset();
	OrientHMDBody();	


}

/*
==============
idPlayer::~idPlayer()

Release any resources used by the player.
==============
*/
idPlayer::~idPlayer()
{
	FreePDASlot();
	FreeHolsterSlot();

	delete flashlight.GetEntity();
	flashlight = NULL;
		
	if( enviroSuitLight.IsValid() )
	{
		enviroSuitLight.GetEntity()->ProcessEvent( &EV_Remove );
	}
	// have to do this here, idMultiplayerGame::DisconnectClient() is too late
	if( common->IsMultiplayer() && gameLocal.mpGame.IsGametypeFlagBased() )
	{
		ReturnFlag();
	}
	
	delete hudManager;
	hudManager = NULL;
	
	delete pdaMenu;
	pdaMenu = NULL;
	
	delete mpMessages;
	mpMessages = NULL;
}

/*
===========
idPlayer::Save
===========
*/
void idPlayer::Save( idSaveGame* savefile ) const
{
	int i;
	
	savefile->WriteUsercmd( usercmd );
	playerView.Save( savefile );
	
	savefile->WriteBool( noclip );
	savefile->WriteBool( godmode );
	
	// don't save spawnAnglesSet, since we'll have to reset them after loading the savegame
	savefile->WriteAngles( spawnAngles );
	savefile->WriteAngles( viewAngles );
	savefile->WriteAngles( cmdAngles );
	
	savefile->WriteInt( buttonMask );
	savefile->WriteInt( oldButtons );
	savefile->WriteInt( oldImpulseSequence );
	
	savefile->WriteInt( lastHitTime );
	savefile->WriteInt( lastSndHitTime );
	savefile->WriteInt( lastSavingThrowTime );
	
	// idBoolFields don't need to be saved, just re-linked in Restore
	
	savefile->WriteObject( primaryObjective );
	inventory.Save( savefile );
	// Carl: todo dual wielding
	hands[vr_weaponHand.GetInteger()].weapon.Save( savefile ); // Carl: Don't make saves incompatible. Note, we are saving the entity number here, not the object
	
	for( int i = 0; i < NUM_QUICK_SLOTS; ++i )
	{
		savefile->WriteInt( quickSlot[ i ] );
	}
	
	savefile->WriteInt( weapon_soulcube );
	savefile->WriteInt( weapon_pda );
	savefile->WriteInt( weapon_fists );
	savefile->WriteInt( weapon_flashlight );
	savefile->WriteInt( weapon_chainsaw );
	savefile->WriteInt( weapon_bloodstone );
	savefile->WriteInt( weapon_bloodstone_active1 );
	savefile->WriteInt( weapon_bloodstone_active2 );
	savefile->WriteInt( weapon_bloodstone_active3 );
	// Koz
	savefile->WriteInt( weapon_pistol );
	savefile->WriteInt( weapon_shotgun );
	savefile->WriteInt( weapon_shotgun_double );
	savefile->WriteInt( weapon_machinegun );
	savefile->WriteInt( weapon_chaingun );
	savefile->WriteInt( weapon_handgrenade );
	savefile->WriteInt( weapon_plasmagun );
	savefile->WriteInt( weapon_rocketlauncher );
	savefile->WriteInt( weapon_bfg );
	savefile->WriteInt( weapon_flashlight_new );
	savefile->WriteInt( weapon_grabber );
	// Koz end

	savefile->WriteBool( harvest_lock );
	savefile->WriteInt( hudPowerup );
	savefile->WriteInt( lastHudPowerup );
	savefile->WriteInt( hudPowerupDuration );
		
	savefile->WriteInt( heartRate );
	
	savefile->WriteFloat( heartInfo.GetStartTime() );
	savefile->WriteFloat( heartInfo.GetDuration() );
	savefile->WriteFloat( heartInfo.GetStartValue() );
	savefile->WriteFloat( heartInfo.GetEndValue() );
	
	savefile->WriteInt( lastHeartAdjust );
	savefile->WriteInt( lastHeartBeat );
	savefile->WriteInt( lastDmgTime );
	savefile->WriteInt( deathClearContentsTime );
	savefile->WriteBool( doingDeathSkin );
	savefile->WriteInt( lastArmorPulse );
	savefile->WriteFloat( stamina );
	savefile->WriteFloat( healthPool );
	savefile->WriteInt( nextHealthPulse );
	savefile->WriteBool( healthPulse );
	savefile->WriteInt( nextHealthTake );
	savefile->WriteBool( healthTake );
	
	savefile->WriteBool( hiddenWeapon );
	soulCubeProjectile.Save( savefile );
	
	savefile->WriteInt( spectator );
	savefile->WriteBool( forceScoreBoard );
	savefile->WriteBool( forceRespawn );
	savefile->WriteBool( spectating );
	savefile->WriteInt( lastSpectateTeleport );
	savefile->WriteBool( lastHitToggle );
	savefile->WriteBool( wantSpectate );
	savefile->WriteBool( hands[0].weaponGone || hands[1].weaponGone ); // Carl: don't change save format, dual wielding
	savefile->WriteBool( useInitialSpawns );
	savefile->WriteInt( tourneyRank );
	savefile->WriteInt( tourneyLine );
	
	teleportEntity.Save( savefile );
	savefile->WriteInt( teleportKiller );
	
	savefile->WriteInt( minRespawnTime );
	savefile->WriteInt( maxRespawnTime );
	
	savefile->WriteVec3( firstPersonViewOrigin );
	savefile->WriteMat3( firstPersonViewAxis );
	
	// don't bother saving dragEntity since it's a dev tool
	
	savefile->WriteJoint( hipJoint );
	savefile->WriteJoint( chestJoint );
	savefile->WriteJoint( headJoint );

	// Koz begin
	savefile->WriteJoint( neckJoint );
	savefile->WriteJoint( chestPivotJoint );

	for ( i = 0; i < 2; i++ )
	{
		savefile->WriteJoint( ik_hand[i] );
		savefile->WriteJoint( ik_elbow[i] );
		savefile->WriteJoint( ik_shoulder[i] );
		savefile->WriteJoint( ik_handAttacher[i] );
	}

	for ( i = 0; i < 2; i++ )
	{
		for ( int j = 0; j < 32; j++ )
		{
			savefile->WriteMat3( ik_handCorrectAxis[i][j] );
			savefile->WriteVec3( handWeaponAttachertoWristJointOffset[i][j] );
			savefile->WriteVec3( handWeaponAttacherToDefaultOffset[i][j] );
		}
	}

	savefile->WriteBool( handLowered );
	savefile->WriteBool( handRaised );
	savefile->WriteBool( commonVr->handInGui );
	// Koz end

	savefile->WriteStaticObject( physicsObj );
	
	savefile->WriteInt( aasLocation.Num() );
	for( i = 0; i < aasLocation.Num(); i++ )
	{
		savefile->WriteInt( aasLocation[ i ].areaNum );
		savefile->WriteVec3( aasLocation[ i ].pos );
	}
	
	savefile->WriteInt( bobFoot );
	savefile->WriteFloat( bobFrac );
	savefile->WriteFloat( bobfracsin );
	savefile->WriteInt( bobCycle );
	savefile->WriteFloat( xyspeed );
	savefile->WriteInt( stepUpTime );
	savefile->WriteFloat( stepUpDelta );
	savefile->WriteFloat( idealLegsYaw );
	savefile->WriteFloat( legsYaw );
	savefile->WriteBool( legsForward );
	savefile->WriteFloat( oldViewYaw );
	savefile->WriteAngles( viewBobAngles );
	savefile->WriteVec3( viewBob );
	savefile->WriteInt( landChange );
	savefile->WriteInt( landTime );
	
	savefile->WriteInt( hands[ vr_weaponHand.GetInteger() ].currentWeapon );
	savefile->WriteInt( hands[ vr_weaponHand.GetInteger() ].idealWeapon.Get() );
	savefile->WriteInt( hands[ vr_weaponHand.GetInteger() ].previousWeapon );
	savefile->WriteInt( hands[ vr_weaponHand.GetInteger() ].weaponSwitchTime );
	savefile->WriteBool( weaponEnabled );
	
	savefile->WriteInt( skinIndex );
	savefile->WriteSkin( skin );
	savefile->WriteSkin( powerUpSkin );
	
	savefile->WriteInt( numProjectilesFired );
	savefile->WriteInt( numProjectileHits );
	
	savefile->WriteBool( airless );
	savefile->WriteInt( airMsec );
	savefile->WriteInt( lastAirDamage );
	
	savefile->WriteBool( gibDeath );
	savefile->WriteBool( gibsLaunched );
	savefile->WriteVec3( gibsDir );
	
	savefile->WriteFloat( zoomFov.GetStartTime() );
	savefile->WriteFloat( zoomFov.GetDuration() );
	savefile->WriteFloat( zoomFov.GetStartValue() );
	savefile->WriteFloat( zoomFov.GetEndValue() );
	
	savefile->WriteFloat( centerView.GetStartTime() );
	savefile->WriteFloat( centerView.GetDuration() );
	savefile->WriteFloat( centerView.GetStartValue() );
	savefile->WriteFloat( centerView.GetEndValue() );
	
	savefile->WriteBool( fxFov );
	
	savefile->WriteFloat( influenceFov );
	savefile->WriteInt( influenceActive );
	savefile->WriteFloat( influenceRadius );
	savefile->WriteObject( influenceEntity );
	savefile->WriteMaterial( influenceMaterial );
	savefile->WriteSkin( influenceSkin );
	
	savefile->WriteObject( privateCameraView );
	
	for( i = 0; i < NUM_LOGGED_VIEW_ANGLES; i++ )
	{
		savefile->WriteAngles( loggedViewAngles[ i ] );
	}
	for( i = 0; i < NUM_LOGGED_ACCELS; i++ )
	{
		savefile->WriteInt( loggedAccel[ i ].time );
		savefile->WriteVec3( loggedAccel[ i ].dir );
	}
	savefile->WriteInt( currentLoggedAccel );
	
	savefile->WriteObject( focusGUIent );
	// can't save focusUI
	savefile->WriteObject( focusCharacter );
	savefile->WriteInt( talkCursor );
	savefile->WriteInt( focusTime );
	savefile->WriteObject( focusVehicle );
	savefile->WriteUserInterface( cursor, false );
	
	savefile->WriteInt( oldMouseX );
	savefile->WriteInt( oldMouseY );
	
	savefile->WriteBool( tipUp );
	savefile->WriteBool( objectiveUp );
	
	savefile->WriteInt( lastDamageDef );
	savefile->WriteVec3( lastDamageDir );
	savefile->WriteInt( lastDamageLocation );
	savefile->WriteInt( smoothedFrame );
	savefile->WriteBool( smoothedOriginUpdated );
	savefile->WriteVec3( smoothedOrigin );
	savefile->WriteAngles( smoothedAngles );
	
	savefile->WriteBool( respawning );
	savefile->WriteBool( leader );
	savefile->WriteInt( lastSpectateChange );
	savefile->WriteInt( lastTeleFX );
	
	savefile->WriteFloat( pm_stamina.GetFloat() );
	
	// TODO_SPARTY hook this up with new hud
	//if ( hud ) {
	//	hud->SetStateString( "message", idLocalization::GetString( "#str_02916" ) );
	//	hud->HandleNamedEvent( "Message" );
	//}
	
	savefile->WriteInt( weaponToggles.Num() );
	for( i = 0; i < weaponToggles.Num(); i++ )
	{
		WeaponToggle_t* weaponToggle = weaponToggles.GetIndex( i );
		savefile->WriteString( weaponToggle->name );
		savefile->WriteInt( weaponToggle->toggleList.Num() );
		for( int j = 0; j < weaponToggle->toggleList.Num(); j++ )
		{
			savefile->WriteInt( weaponToggle->toggleList[j] );
		}
	}
	savefile->WriteObject( mountedObject );
	enviroSuitLight.Save( savefile );
	savefile->WriteBool( healthRecharge );
	savefile->WriteInt( lastHealthRechargeTime );
	savefile->WriteInt( rechargeSpeed );
	savefile->WriteFloat( new_g_damageScale );
	
	savefile->WriteBool( bloomEnabled );
	savefile->WriteFloat( bloomSpeed );
	savefile->WriteFloat( bloomIntensity );
	
	savefile->WriteObject( flashlight.GetEntity() );
	savefile->WriteInt( flashlightBattery );
	
	achievementManager.Save( savefile );
	
	savefile->WriteInt( playedTimeSecs );
	savefile->WriteInt( playedTimeResidual );
	
	for( int i = 0; i < MAX_PLAYER_PDA; i++ )
	{
		savefile->WriteBool( pdaHasBeenRead[i] );
	}
	
	for( int i = 0; i < MAX_PLAYER_VIDEO; i++ )
	{
		savefile->WriteBool( videoHasBeenViewed[i] );
	}
	
	for( int i = 0; i < MAX_PLAYER_AUDIO; i++ )
	{
		for( int j = 0; j < MAX_PLAYER_AUDIO_ENTRIES; j++ )
		{
			savefile->WriteBool( audioHasBeenHeard[i][j] );
		}
	}

	// Koz begin
	savefile->WriteBool( hands[0].laserSightActive || hands[1].laserSightActive );
	savefile->WriteBool( headingBeamActive );
	savefile->WriteBool( hudActive );
	
	savefile->WriteInt( commonVr->currentFlashlightMode );
	savefile->WriteSkin( hands[vr_weaponHand.GetInteger()].crosshairEntity.customSkin );
	
	savefile->WriteBool( hands[0].PDAfixed || hands[1].PDAfixed ); // Carl: Dual wielding, don't make saves incompatible
	savefile->WriteVec3( PDAorigin );
	savefile->WriteMat3( PDAaxis );


	//blech.  Im going to pad the savegame file with a few diff var types,
	// so if more changes are needed in the future, maybe save game compat can be preserved.
	
	//padded ints have been used now.
	savefile->WriteInt( 666 ); // flag that holster has been saved.
	savefile->WriteInt( holsteredWeapon );
	savefile->WriteInt( extraHolsteredWeapon );
	savefile->WriteInt( (int) holsterModelDefHandle );
	
	
	savefile->WriteFloat( 0 );
	savefile->WriteFloat( 0 );
	savefile->WriteFloat( 0 );
	savefile->WriteFloat( 0 );
	savefile->WriteBool( false );
	savefile->WriteBool( false );
	savefile->WriteBool( false );
	savefile->WriteBool( false );
	savefile->WriteVec3( vec3_zero );
	savefile->WriteVec3( vec3_zero );
	savefile->WriteVec3( vec3_zero );
	savefile->WriteVec3( vec3_zero );
		
	savefile->WriteMat3( holsterAxis );
	savefile->WriteMat3( mat3_identity );
	savefile->WriteMat3( mat3_identity );
	savefile->WriteMat3( mat3_identity );
	
	// end padding

	savefile->WriteRenderEntity( holsterRenderEntity ); // have to check if this has been saved
	savefile->WriteString( extraHolsteredWeaponModel );
	
	// Koz end

}

/*
===========
idPlayer::Restore
===========
*/
void idPlayer::Restore( idRestoreGame* savefile )
{
	int	  i;
	int	  num;
	float set;
	
	savefile->ReadUsercmd( usercmd );
	playerView.Restore( savefile );
	
	savefile->ReadBool( noclip );
	savefile->ReadBool( godmode );
	
	savefile->ReadAngles( spawnAngles );
	savefile->ReadAngles( viewAngles );
	savefile->ReadAngles( cmdAngles );
	
	memset( usercmd.angles, 0, sizeof( usercmd.angles ) );
	SetViewAngles( viewAngles );
	spawnAnglesSet = true;
	
	savefile->ReadInt( buttonMask );
	savefile->ReadInt( oldButtons );
	savefile->ReadInt( oldImpulseSequence );
	
	usercmd.impulseSequence = 0;
	oldImpulseSequence = 0;
	
	savefile->ReadInt( lastHitTime );
	savefile->ReadInt( lastSndHitTime );
	savefile->ReadInt( lastSavingThrowTime );
	
	// Re-link idBoolFields to the scriptObject, values will be restored in scriptObject's restore
	LinkScriptVariables();

	// re-init the hand's laser model (and other things)
	for( int hand = 0; hand < 2; hand++ )
		hands[hand].Init( this, hand );

	savefile->ReadObject( reinterpret_cast<idClass*&>( primaryObjective ) );
	inventory.Restore( savefile );
	idEntityPtr<idWeapon> weapon;
	weapon.Restore( savefile ); // Carl: Dual wielding, don't change save file format
	// Carl: if it's the PDA, then we saved the PDA hand, otherwise we saved the weapon hand.
	int savedWeaponHand;
	if( weapon.GetEntity() && weapon.GetEntity()->IdentifyWeapon() == WEAPON_PDA )
		savedWeaponHand = 1 - vr_weaponHand.GetInteger();
	else
		savedWeaponHand = vr_weaponHand.GetInteger();
	hands[savedWeaponHand].weapon = weapon;
	// Carl: other hand's weapon isn't saved yet, so recreate it
	hands[ 1 - savedWeaponHand ].weapon = static_cast< idWeapon* >( gameLocal.SpawnEntityType( idWeapon::Type, NULL ) );
	hands[ 1 - savedWeaponHand ].weapon.GetEntity()->SetOwner( this, 1 - savedWeaponHand );

	if( hudManager != NULL )
	{
		hudManager->Initialize( "hud", common->SW() );
		hudManager->ActivateMenu( true );
		hud = hudManager->GetHud();
	}
	
	if( pdaMenu != NULL )
	{
		pdaMenu->Initialize( "pda", common->SW() );
	}
	
	for( i = 0; i < inventory.emails.Num(); i++ )
	{
		GetPDA()->AddEmail( inventory.emails[i] );
	}
	
	
	for( int i = 0; i < NUM_QUICK_SLOTS; ++i )
	{
		savefile->ReadInt( quickSlot[ i ] );
	}
	
	savefile->ReadInt( weapon_soulcube );
	savefile->ReadInt( weapon_pda );
	savefile->ReadInt( weapon_fists );
	savefile->ReadInt( weapon_flashlight );
	savefile->ReadInt( weapon_chainsaw );
	savefile->ReadInt( weapon_bloodstone );
	savefile->ReadInt( weapon_bloodstone_active1 );
	savefile->ReadInt( weapon_bloodstone_active2 );
	savefile->ReadInt( weapon_bloodstone_active3 );

	// Koz
	if (savefile->version >= BUILD_NUMBER_FULLY_POSSESSED)
	{
		savefile->ReadInt( weapon_pistol );
		savefile->ReadInt( weapon_shotgun );
		savefile->ReadInt( weapon_shotgun_double );
		savefile->ReadInt( weapon_machinegun );
		savefile->ReadInt( weapon_chaingun );
		savefile->ReadInt( weapon_handgrenade );
		savefile->ReadInt( weapon_plasmagun );
		savefile->ReadInt( weapon_rocketlauncher );
		savefile->ReadInt( weapon_bfg );
		savefile->ReadInt( weapon_flashlight_new );
		savefile->ReadInt( weapon_grabber );
	}
	else
	{
		weapon_grabber = 1;
		weapon_pistol = 2;
		weapon_shotgun = 3;
		weapon_shotgun_double = 4;
		weapon_machinegun = 5;
		weapon_chaingun = 6;
		weapon_handgrenade = 7;
		weapon_plasmagun = 8;
		weapon_plasmagun = 8;
		weapon_rocketlauncher = 9;
		weapon_bfg = 10;
		weapon_flashlight_new = 17;
	}
	// Koz end

	
	savefile->ReadBool( harvest_lock );
	savefile->ReadInt( hudPowerup );
	savefile->ReadInt( lastHudPowerup );
	savefile->ReadInt( hudPowerupDuration );
	
	
	
	savefile->ReadInt( heartRate );
	
	savefile->ReadFloat( set );
	heartInfo.SetStartTime( set );
	savefile->ReadFloat( set );
	heartInfo.SetDuration( set );
	savefile->ReadFloat( set );
	heartInfo.SetStartValue( set );
	savefile->ReadFloat( set );
	heartInfo.SetEndValue( set );
	
	savefile->ReadInt( lastHeartAdjust );
	savefile->ReadInt( lastHeartBeat );
	savefile->ReadInt( lastDmgTime );
	savefile->ReadInt( deathClearContentsTime );
	savefile->ReadBool( doingDeathSkin );
	savefile->ReadInt( lastArmorPulse );
	savefile->ReadFloat( stamina );
	savefile->ReadFloat( healthPool );
	savefile->ReadInt( nextHealthPulse );
	savefile->ReadBool( healthPulse );
	savefile->ReadInt( nextHealthTake );
	savefile->ReadBool( healthTake );
	
	savefile->ReadBool( hiddenWeapon );
	soulCubeProjectile.Restore( savefile );
	
	savefile->ReadInt( spectator );
	savefile->ReadBool( forceScoreBoard );
	savefile->ReadBool( forceRespawn );
	savefile->ReadBool( spectating );
	savefile->ReadInt( lastSpectateTeleport );
	savefile->ReadBool( lastHitToggle );
	savefile->ReadBool( wantSpectate );

	// Carl: Don't change save format
	bool gone = false;
	savefile->ReadBool( gone );
	hands[ 0 ].weaponGone = gone;
	hands[ 1 ].weaponGone = gone;

	savefile->ReadBool( useInitialSpawns );
	savefile->ReadInt( tourneyRank );
	savefile->ReadInt( tourneyLine );
	
	teleportEntity.Restore( savefile );
	savefile->ReadInt( teleportKiller );
	
	savefile->ReadInt( minRespawnTime );
	savefile->ReadInt( maxRespawnTime );
	
	savefile->ReadVec3( firstPersonViewOrigin );
	savefile->ReadMat3( firstPersonViewAxis );
	
	// don't bother saving dragEntity since it's a dev tool
	dragEntity.Clear();
	
	savefile->ReadJoint( hipJoint );
	savefile->ReadJoint( chestJoint );
	savefile->ReadJoint( headJoint );

	// Koz begin - update - will leave this here, but the player bone joints will all be re-initialized to help with cross version save compatability
	if (savefile->version >= BUILD_NUMBER_FULLY_POSSESSED)
	{
		savefile->ReadJoint( neckJoint );
		savefile->ReadJoint( chestPivotJoint );

		for ( i = 0; i < 2; i++ )
		{
			savefile->ReadJoint( ik_hand[i] );
			savefile->ReadJoint( ik_elbow[i] );
			savefile->ReadJoint( ik_shoulder[i] );
			savefile->ReadJoint( ik_handAttacher[i] );
		}

		for ( i = 0; i < 2; i++ )
		{
			for ( int j = 0; j < 32; j++ )
			{
				savefile->ReadMat3( ik_handCorrectAxis[i][j] );
				savefile->ReadVec3( handWeaponAttachertoWristJointOffset[i][j] );
				savefile->ReadVec3( handWeaponAttacherToDefaultOffset[i][j] );
			}
		}

		savefile->ReadBool( handLowered );
		savefile->ReadBool( handRaised );
		savefile->ReadBool( commonVr->handInGui );
	}
	// Koz end
	
	savefile->ReadStaticObject( physicsObj );
	RestorePhysics( &physicsObj );
	
	savefile->ReadInt( num );
	aasLocation.SetGranularity( 1 );
	aasLocation.SetNum( num );
	for( i = 0; i < num; i++ )
	{
		savefile->ReadInt( aasLocation[ i ].areaNum );
		savefile->ReadVec3( aasLocation[ i ].pos );
	}

	// Set the AAS if the character has the correct gravity vector
	//idVec3 gravity = spawnArgs.GetVector("gravityDir", "0 0 -1");
	//gravity *= g_gravity.GetFloat();
	//if (gravity == gameLocal.GetGravity())
	//{
		SetAAS();
	//}


	savefile->ReadInt( bobFoot );
	savefile->ReadFloat( bobFrac );
	savefile->ReadFloat( bobfracsin );
	savefile->ReadInt( bobCycle );
	savefile->ReadFloat( xyspeed );
	savefile->ReadInt( stepUpTime );
	savefile->ReadFloat( stepUpDelta );
	savefile->ReadFloat( idealLegsYaw );
	savefile->ReadFloat( legsYaw );
	savefile->ReadBool( legsForward );
	savefile->ReadFloat( oldViewYaw );
	savefile->ReadAngles( viewBobAngles );
	savefile->ReadVec3( viewBob );
	savefile->ReadInt( landChange );
	savefile->ReadInt( landTime );
	
	// carl: dual wielding, don't change save format
	savefile->ReadInt( hands[ vr_weaponHand.GetInteger() ].currentWeapon );
	int savedIdealWeapon = -1;
	savefile->ReadInt( savedIdealWeapon );
	hands[ vr_weaponHand.GetInteger() ].idealWeapon.Set( savedIdealWeapon );
	savefile->ReadInt( hands[ vr_weaponHand.GetInteger() ].previousWeapon );
	savefile->ReadInt( hands[ vr_weaponHand.GetInteger() ].weaponSwitchTime );

	hands[ 1 - vr_weaponHand.GetInteger() ].currentWeapon = -1;
	hands[ 1 - vr_weaponHand.GetInteger() ].idealWeapon.Set( -1 );
	hands[ 1 - vr_weaponHand.GetInteger() ].previousWeapon = -1;
	hands[ 1 - vr_weaponHand.GetInteger() ].weaponSwitchTime = 0;

	savefile->ReadBool( weaponEnabled );
	risingWeaponHand = -1;
	
	savefile->ReadInt( skinIndex );
	savefile->ReadSkin( skin );
	savefile->ReadSkin( powerUpSkin );
	
	savefile->ReadInt( numProjectilesFired );
	savefile->ReadInt( numProjectileHits );
	
	savefile->ReadBool( airless );
	savefile->ReadInt( airMsec );
	savefile->ReadInt( lastAirDamage );
	
	savefile->ReadBool( gibDeath );
	savefile->ReadBool( gibsLaunched );
	savefile->ReadVec3( gibsDir );
	
	savefile->ReadFloat( set );
	zoomFov.SetStartTime( set );
	savefile->ReadFloat( set );
	zoomFov.SetDuration( set );
	savefile->ReadFloat( set );
	zoomFov.SetStartValue( set );
	savefile->ReadFloat( set );
	zoomFov.SetEndValue( set );
	
	savefile->ReadFloat( set );
	centerView.SetStartTime( set );
	savefile->ReadFloat( set );
	centerView.SetDuration( set );
	savefile->ReadFloat( set );
	centerView.SetStartValue( set );
	savefile->ReadFloat( set );
	centerView.SetEndValue( set );
	
	savefile->ReadBool( fxFov );
	
	savefile->ReadFloat( influenceFov );
	savefile->ReadInt( influenceActive );
	savefile->ReadFloat( influenceRadius );
	savefile->ReadObject( reinterpret_cast<idClass*&>( influenceEntity ) );
	savefile->ReadMaterial( influenceMaterial );
	savefile->ReadSkin( influenceSkin );
	
	savefile->ReadObject( reinterpret_cast<idClass*&>( privateCameraView ) );
	
	for( i = 0; i < NUM_LOGGED_VIEW_ANGLES; i++ )
	{
		savefile->ReadAngles( loggedViewAngles[ i ] );
	}
	for( i = 0; i < NUM_LOGGED_ACCELS; i++ )
	{
		savefile->ReadInt( loggedAccel[ i ].time );
		savefile->ReadVec3( loggedAccel[ i ].dir );
	}
	savefile->ReadInt( currentLoggedAccel );
	
	savefile->ReadObject( reinterpret_cast<idClass*&>( focusGUIent ) );
	// can't save focusUI
	focusUI = NULL;
	savefile->ReadObject( reinterpret_cast<idClass*&>( focusCharacter ) );
	savefile->ReadInt( talkCursor );
	savefile->ReadInt( focusTime );
	savefile->ReadObject( reinterpret_cast<idClass*&>( focusVehicle ) );
	savefile->ReadUserInterface( cursor );
	
	savefile->ReadInt( oldMouseX );
	savefile->ReadInt( oldMouseY );
	
	savefile->ReadBool( tipUp );
	savefile->ReadBool( objectiveUp );
	
	savefile->ReadInt( lastDamageDef );
	savefile->ReadVec3( lastDamageDir );
	savefile->ReadInt( lastDamageLocation );
	savefile->ReadInt( smoothedFrame );
	savefile->ReadBool( smoothedOriginUpdated );
	savefile->ReadVec3( smoothedOrigin );
	savefile->ReadAngles( smoothedAngles );
	
	savefile->ReadBool( respawning );
	savefile->ReadBool( leader );
	savefile->ReadInt( lastSpectateChange );
	savefile->ReadInt( lastTeleFX );
	
	// set the pm_ cvars
	const idKeyValue*	kv;
	kv = spawnArgs.MatchPrefix( "pm_", NULL );
	while( kv )
	{
		cvarSystem->SetCVarString( kv->GetKey(), kv->GetValue() );
		kv = spawnArgs.MatchPrefix( "pm_", kv );
	}
	
	savefile->ReadFloat( set );
	pm_stamina.SetFloat( set );
	
	// create combat collision hull for exact collision detection
	SetCombatModel();
	
	int weaponToggleCount;
	savefile->ReadInt( weaponToggleCount );
	for( i = 0; i < weaponToggleCount; i++ )
	{
		WeaponToggle_t newToggle;
		memset( &newToggle, 0, sizeof( newToggle ) );
		
		idStr name;
		savefile->ReadString( name );
		strcpy( newToggle.name, name.c_str() );
		
		int indexCount;
		savefile->ReadInt( indexCount );
		for( int j = 0; j < indexCount; j++ )
		{
			int temp;
			savefile->ReadInt( temp );
			newToggle.toggleList.Append( temp );
		}
		newToggle.lastUsed = 0;
		weaponToggles.Set( newToggle.name, newToggle );
	}
	savefile->ReadObject( reinterpret_cast<idClass*&>( mountedObject ) );
	enviroSuitLight.Restore( savefile );
	savefile->ReadBool( healthRecharge );
	savefile->ReadInt( lastHealthRechargeTime );
	savefile->ReadInt( rechargeSpeed );
	savefile->ReadFloat( new_g_damageScale );
	
	savefile->ReadBool( bloomEnabled );
	savefile->ReadFloat( bloomSpeed );
	savefile->ReadFloat( bloomIntensity );
	
	// flashlight
	idWeapon* tempWeapon;
	savefile->ReadObject( reinterpret_cast<idClass*&>( tempWeapon ) );
	tempWeapon->SetIsPlayerFlashlight( true );
	flashlight = tempWeapon;
	savefile->ReadInt( flashlightBattery );
	
	achievementManager.Restore( savefile );
	
	savefile->ReadInt( playedTimeSecs );
	savefile->ReadInt( playedTimeResidual );
	
	aimAssist.Init( this );
	
	SetupPDASlot( true );
	holsteredWeapon = weapon_fists;
	extraHolsteredWeapon = weapon_fists;
	extraHolsteredWeaponModel = NULL;

	for( int i = 0; i < MAX_PLAYER_PDA; i++ )
	{
		savefile->ReadBool( pdaHasBeenRead[i] );
	}
	
	for( int i = 0; i < MAX_PLAYER_VIDEO; i++ )
	{
		savefile->ReadBool( videoHasBeenViewed[i] );
	}
	
	for( int i = 0; i < MAX_PLAYER_AUDIO; i++ )
	{
		for( int j = 0; j < MAX_PLAYER_AUDIO_ENTRIES; j++ )
		{
			savefile->ReadBool( audioHasBeenHeard[i][j] );
		}
	}
	
	// Update the soul cube HUD indicator
	if( hud )
	{
		if( weapon_soulcube > 0 && ( inventory.weapons & ( 1 << weapon_soulcube ) ) )
		{
			int max_souls = inventory.MaxAmmoForAmmoClass( this, "ammo_souls" );
			if( inventory.GetInventoryAmmoForType( idWeapon::GetAmmoNumForName( "ammo_souls" ) ) >= max_souls )
			{
				hud->SetShowSoulCubeOnLoad( true );
			}
		}
	}
	
	
	//------------------------------------------------------------
	// Koz begin - VR specific initialization
	
	// Koz fixme ovr_RecenterTrackingOrigin( commonVr->hmdSession ); // Koz reset hmd orientation  Koz fixme check if still appropriate here.
	
	// make sure the clipmodels for the body and head are re-initialized.
	SetClipModel();
	
	// re-init the player render model if we're loading this savegame from a different mod
	
	if( 1 || savefile->version < BUILD_NUMBER_FULLY_POSSESSED )
	{
		memset(&renderEntity, 0, sizeof(renderEntity));
		renderEntity.numJoints = animator.NumJoints();
		animator.GetJoints(&renderEntity.numJoints, &renderEntity.joints);
		renderEntity.hModel = animator.ModelHandle();
		if (renderEntity.hModel)
		{
			//renderEntity.hModel->Reset();
			
			renderEntity.hModel->InitFromFile( renderEntity.hModel->Name() );
			animator.ClearAllJoints();
			
			renderEntity.bounds = renderEntity.hModel->Bounds(&renderEntity);
		}
		renderEntity.shaderParms[SHADERPARM_RED] = 1.0f;
		renderEntity.shaderParms[SHADERPARM_GREEN] = 1.0f;
		renderEntity.shaderParms[SHADERPARM_BLUE] = 1.0f;
		renderEntity.shaderParms[3] = 1.0f;
		renderEntity.shaderParms[SHADERPARM_TIMEOFFSET] = 0.0f;
		renderEntity.shaderParms[5] = 0.0f;
		renderEntity.shaderParms[6] = 0.0f;
		renderEntity.shaderParms[7] = 0.0f;
	}

	// Koz re-init the player model bones. Changes to player model require this to allow compatability with older savegames.
	InitPlayerBones();

	//re-init the VR ui models
	headingBeamHandle = -1;
	hudHandle = -1;

	// re-init hud model
	memset( &hudEntity, 0, sizeof( hudEntity ) );
	hudEntity.hModel = renderModelManager->FindModel( "/models/mapobjects/hud.lwo" );
	hudEntity.customShader = declManager->FindMaterial( "vr/hud" );
	hudEntity.weaponDepthHack = vr_hudOcclusion.GetBool();

	// re-init the heading beam model
	memset( &headingBeamEntity, 0, sizeof( headingBeamEntity ) );
	headingBeamEntity.hModel = renderModelManager->FindModel( "/models/mapobjects/headingbeam.lwo" );
	
	// re-init skins for heading indicator, crosshair, and telepad
	skinHeadingSolid = declManager->FindSkin( "skins/models/headingbeamsolid" );
	skinHeadingArrows = declManager->FindSkin( "skins/models/headingbeamarrows" );
	skinHeadingArrowsScroll = declManager->FindSkin( "skins/models/headingbeamarrowsscroll" );
	skinCrosshairDot = declManager->FindSkin( "skins/vr/crosshairDot" );
	skinCrosshairCircleDot = declManager->FindSkin( "skins/vr/crosshairCircleDot" );
	skinCrosshairCross = declManager->FindSkin( "skins/vr/crosshairCross" );
	skinTelepadCrouch = declManager->FindSkin( "skins/vr/padcrouch" );
		
	const idDeclSkin* blag;
	// Koz begin
	if ( savefile->version >= BUILD_NUMBER_FULLY_POSSESSED )
	{
		bool laserSightActive;
		savefile->ReadBool( laserSightActive );
		hands[ 0 ].laserSightActive = laserSightActive;
		hands[ 1 ].laserSightActive = laserSightActive;
		savefile->ReadBool( headingBeamActive );
		savefile->ReadBool( hudActive );
	
		savefile->ReadInt( commonVr->currentFlashlightMode );
	//	savefile->ReadSkin( hands[0].crosshairEntity.customSkin );
		savefile->ReadSkin( blag );

		bool PDAfixed = false;
		savefile->ReadBool( PDAfixed );
		hands[ 1 - vr_weaponHand.GetInteger() ].PDAfixed = PDAfixed;
		hands[ vr_weaponHand.GetInteger() ].PDAfixed = false;
		savefile->ReadVec3( PDAorigin );
		savefile->ReadMat3( PDAaxis );
		
		int tempInt;
		float tempFloat;
		bool tempBool;
		idVec3 tempVec3;
		idMat3 tempMat3;
		
		int holsterFlag;

		//blech.  Im going to pad the savegame file with a few diff var types,
		// so if more changes are needed in the future, maybe save game compat can be preserved.
		
		//ints used saving weapon holsters.
		savefile->ReadInt( holsterFlag );
		if ( holsterFlag == 666 )
		{
			savefile->ReadInt( holsteredWeapon );
			savefile->ReadInt( extraHolsteredWeapon );
			savefile->ReadInt( tempInt );
			holsterModelDefHandle = tempInt;
		}
		else
		{
			savefile->ReadInt( tempInt );
			savefile->ReadInt( tempInt );
			savefile->ReadInt( tempInt );
		}

		savefile->ReadFloat( tempFloat );
		savefile->ReadFloat( tempFloat );
		savefile->ReadFloat( tempFloat );
		savefile->ReadFloat( tempFloat );
		savefile->ReadBool( tempBool );
		savefile->ReadBool( tempBool );
		savefile->ReadBool( tempBool );
		savefile->ReadBool( tempBool );
		savefile->ReadVec3( tempVec3 );
		savefile->ReadVec3( tempVec3 );
		savefile->ReadVec3( tempVec3 );
		savefile->ReadVec3( tempVec3 );
	
		savefile->ReadMat3( tempMat3 );
		if ( holsterFlag == 666 ) holsterAxis = tempMat3;

		savefile->ReadMat3( tempMat3 );
		savefile->ReadMat3( tempMat3 );
		savefile->ReadMat3( tempMat3 );
		
		if ( holsterFlag == 666 )
		{
			
			idStr	ehwm;
			savefile->ReadRenderEntity( holsterRenderEntity );
			savefile->ReadString( ehwm );
			extraHolsteredWeaponModel = ehwm.c_str();

			if ( extraHolsteredWeapon != weapon_fists )
			{
				
				/*
				If the game was autosaved, the holster and holster model will be correct,
				but if the game was saved through the pause menu, the active weapon was pushed to the holster,
				and the holstered weapon was pushed to extraHolsteredWeapon. This is the only time extraholstered
				will not hold weapon_fists.  

				Check if the holstered weapon was pushed to the extraholster, and switch it back on load.
				*/
				
				holsteredWeapon = extraHolsteredWeapon;
				extraHolsteredWeapon = weapon_fists;
				//common->Printf( "Loading holster model %s\n", extraHolsteredWeaponModel );
				holsterRenderEntity.hModel = renderModelManager->FindModel( extraHolsteredWeaponModel );

				if ( strcmp( extraHolsteredWeaponModel, "models/weapons/pistol/w_pistol.lwo" ) == 0 )
				{
					holsterAxis = idAngles( 90, 0, 0 ).ToMat3() * 0.75f;
				}
				else if ( strcmp( extraHolsteredWeaponModel, "models/weapons/shotgun/w_shotgun2.lwo" ) == 0 ||
					strcmp( extraHolsteredWeaponModel, "models/weapons/bfg/bfg_world.lwo" ) == 0 )
				{
					holsterAxis = idAngles( 0, -90, -90 ).ToMat3();
				}
				else if ( strcmp( extraHolsteredWeaponModel, "models/weapons/grabber/grabber_world.ase" ) == 0 )
				{
					holsterAxis = idAngles( -90, 180, 0 ).ToMat3() * 0.5f;
				}
				else if ( strcmp( extraHolsteredWeaponModel, "models/weapons/machinegun/w_machinegun.lwo" ) == 0 )
				{
					holsterAxis = idAngles( 0, 90, 90 ).ToMat3() * 0.75f;
				}
				else if ( strcmp( extraHolsteredWeaponModel, "models/weapons/plasmagun/plasmagun_world.lwo" ) == 0 )
				{
					holsterAxis = idAngles( 0, 90, 90 ).ToMat3() * 0.75f;
				}
				else if ( strcmp( extraHolsteredWeaponModel, "models/weapons/chainsaw/w_chainsaw.lwo" ) == 0 )
				{
					holsterAxis = idAngles( 0, 90, 90 ).ToMat3() * 0.9f;
				}
				else if ( strcmp( extraHolsteredWeaponModel, "models/weapons/chaingun/w_chaingun.lwo" ) == 0 )
				{
					holsterAxis = idAngles( 0, 90, 90 ).ToMat3() * 0.9f;
				}
				else
				{
					holsterAxis = idAngles( 0, 90, 90 ).ToMat3();
				}
			}
		}

	}
	else
	{
		spawnArgs.SetInt("ik_numArms", 2);
	}


	armIK.Init( this, IK_ANIM, modelOffset );

	if( vr_debugHands.GetBool() )
	{
		common->Printf( "\nRestored Hands:\n" );
		hands[HAND_LEFT].debugPrint();
		hands[HAND_RIGHT].debugPrint();
	}

	if (savefile->version < BUILD_NUMBER_FULLY_POSSESSED)
		NextWeapon();

	vr_weaponSight.SetModified(); // make sure these get initialized properly
	vr_headingBeamMode.SetModified();
	
	aasState = 0;

	// Koz end

}

/*
===============
idPlayer::PrepareForRestart
================
*/
void idPlayer::PrepareForRestart()
{
	ClearPowerUps();
	
	if( common->IsClient() == false )
	{
		ServerSpectate( true );
	}
	
	forceRespawn = true;
	
	// Confirm reset hud states
	DropFlag();
	
	if( hud )
	{
		hud->SetFlagState( 0, 0 );
		hud->SetFlagState( 1, 0 );
	}
	
	// we will be restarting program, clear the client entities from program-related things first
	ShutdownThreads();
	
	// the sound world is going to be cleared, don't keep references to emitters
	FreeSoundEmitter( false );
}

/*
===============
idPlayer::Restart
================
*/
void idPlayer::Restart()
{
	idActor::Restart();
	
	// client needs to setup the animation script object again
	if( common->IsClient() )
	{
		// Make sure the weapon spawnId gets re-linked on the next snapshot.
		// Otherwise, its owner might not be set after the map restart, which causes asserts and crashes.
		for( int h = 0; h < 2; h++ )
			hands[ h ].weapon = NULL;
		flashlight = NULL;
		enviroSuitLight = NULL;
		Init();
	}
	else
	{
		// choose a random spot and prepare the point of view in case player is left spectating
		assert( spectating );
		SpawnFromSpawnSpot();
	}
	
	useInitialSpawns = true;
	UpdateSkinSetup();

}

/*
===============
idPlayer::ServerSpectate
================
*/
void idPlayer::ServerSpectate( bool spectate )
{
	assert( !common->IsClient() );
	
	if( spectating != spectate )
	{
		Spectate( spectate );
		if( !spectate )
		{
			// When coming out of spectate, join the team with the least number of players
			if( gameLocal.mpGame.IsGametypeTeamBased() )
			{
				int teamCounts[2] = { 0, 0 };
				gameLocal.mpGame.NumActualClients( false, teamCounts );
				teamCounts[team]--;
				if( teamCounts[0] < teamCounts[1] )
				{
					team = 0;
				}
				else if( teamCounts[1] < teamCounts[0] )
				{
					team = 1;
				}
				gameLocal.mpGame.SwitchToTeam( entityNumber, -1, team );
			}
			if( gameLocal.gameType == GAME_DM )
			{
				// make sure the scores are reset so you can't exploit by spectating and entering the game back
				// other game types don't matter, as you either can't join back, or it's team scores
				gameLocal.mpGame.ClearFrags( entityNumber );
			}
		}
	}
	if( !spectate )
	{
		SpawnFromSpawnSpot();
	}
	
	// drop the flag if player was carrying it
	if( spectate && common->IsMultiplayer() && gameLocal.mpGame.IsGametypeFlagBased() &&
			carryingFlag )
	{
		DropFlag();
	}
}

/*
===========
idPlayer::SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
void idPlayer::SelectInitialSpawnPoint( idVec3& origin, idAngles& angles )
{
	idEntity* spot;
	idStr skin;
	
	spot = gameLocal.SelectInitialSpawnPoint( this );
	
	// set the player skin from the spawn location
	if( spot->spawnArgs.GetString( "skin", NULL, skin ) )
	{
		spawnArgs.Set( "spawn_skin", skin );
	}
	
	// activate the spawn locations targets
	spot->PostEventMS( &EV_ActivateTargets, 0, this );
	
	origin = spot->GetPhysics()->GetOrigin();
	origin[2] += 4.0f + CM_BOX_EPSILON;		// move up to make sure the player is at least an epsilon above the floor
	angles = spot->GetPhysics()->GetAxis().ToAngles();
}

/*
===========
idPlayer::SpawnFromSpawnSpot

Chooses a spawn location and spawns the player
============
*/
void idPlayer::SpawnFromSpawnSpot()
{
	idVec3		spawn_origin;
	idAngles	spawn_angles;
	
	SelectInitialSpawnPoint( spawn_origin, spawn_angles );
	SpawnToPoint( spawn_origin, spawn_angles );
}

/*
===========
idPlayer::SpawnToPoint

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState

when called here with spectating set to true, just place yourself and init
============
*/
void idPlayer::SpawnToPoint( const idVec3& spawn_origin, const idAngles& spawn_angles )
{
	idVec3 spec_origin;
	
	assert( !common->IsClient() );
	
	respawning = true;
	
	Init();
	
	fl.noknockback = false;
	
	// stop any ragdolls being used
	StopRagdoll();
	
	// set back the player physics
	SetPhysics( &physicsObj );
	
	physicsObj.SetClipModelAxis();
	physicsObj.EnableClip();
	
	if( !spectating )
	{
		SetCombatContents( true );
	}
	
	physicsObj.SetLinearVelocity( vec3_origin );
	
	// setup our initial view
	if( !spectating )
	{
		SetOrigin( spawn_origin );
	}
	else
	{
		spec_origin = spawn_origin;
		spec_origin[ 2 ] += pm_normalheight.GetFloat();
		spec_origin[ 2 ] += SPECTATE_RAISE;
		SetOrigin( spec_origin );
	}
	
	// if this is the first spawn of the map, we don't have a usercmd yet,
	// so the delta angles won't be correct.  This will be fixed on the first think.
	viewAngles = ang_zero;
	SetDeltaViewAngles( ang_zero );
	SetViewAngles( spawn_angles );
	spawnAngles = spawn_angles;
	spawnAnglesSet = false;
	
	legsForward = true;
	legsYaw = 0.0f;
	idealLegsYaw = 0.0f;
	oldViewYaw = viewAngles.yaw;
	
	if( spectating )
	{
		Hide();
	}
	else
	{
		Show();
	}
	
	if( common->IsMultiplayer() )
	{
		if( !spectating )
		{
			// we may be called twice in a row in some situations. avoid a double fx and 'fly to the roof'
			if( lastTeleFX < gameLocal.time - 1000 )
			{
				idEntityFx::StartFx( spawnArgs.GetString( "fx_spawn" ), &spawn_origin, NULL, this, true );
				lastTeleFX = gameLocal.time;
			}
		}
		AI_TELEPORT = true;
	}
	else
	{
		AI_TELEPORT = false;
	}
	
	// kill anything at the new position
	if( !spectating )
	{
		physicsObj.SetClipMask( MASK_PLAYERSOLID ); // the clip mask is usually maintained in Move(), but KillBox requires it
		gameLocal.KillBox( this );
	}
	
	// don't allow full run speed for a bit
	physicsObj.SetKnockBack( 100 );
	
	// set our respawn time and buttons so that if we're killed we don't respawn immediately
	minRespawnTime = gameLocal.time;
	maxRespawnTime = gameLocal.time;
	if( !spectating )
	{
		forceRespawn = false;
	}
	
	Respawn_Shared();
	
	privateCameraView = NULL;
	
	BecomeActive( TH_THINK );
	
	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	Think();
	
	respawning			= false;
	lastManOver			= false;
	lastManPlayAgain	= false;
	isTelefragged		= false;
}

/*
===============
idPlayer::Respawn_Shared
Called on server and client players when they respawn (including on initial spawn)
===============
*/
void idPlayer::Respawn_Shared()
{
	respawn_netEvent.Set();
	
	serverOverridePositionTime = gameLocal.GetServerGameTimeMs();
	
	// Remove the hud respawn message.
	HideRespawnHudMessage();
	
	FlashlightOff();
}

/*
===============
idPlayer::SavePersistantInfo

Saves any inventory and player stats when changing levels.
===============
*/
void idPlayer::SavePersistantInfo()
{
	idDict& playerInfo = gameLocal.persistentPlayerInfo[entityNumber];
	
	playerInfo.Clear();
	inventory.GetPersistantData( playerInfo );
	playerInfo.SetInt( "health", health );
	playerInfo.SetInt( "current_weapon", hands[ GetBestWeaponHand() ].currentWeapon );
	playerInfo.SetInt( "playedTime", playedTimeSecs );

	// Koz begin
	playerInfo.SetBool( "headingBeamActive", headingBeamActive );
	playerInfo.SetBool( "laserSightActive", hands[0].laserSightActive || hands[1].laserSightActive ); // Carl: don't change save format
	playerInfo.SetBool( "hudActive", hudActive );
	playerInfo.SetInt( "currentFlashMode", commonVr->currentFlashlightMode );

	playerInfo.SetInt( "holsteredWeapon", holsteredWeapon );
	playerInfo.SetInt( "extraHolsteredWeapon", extraHolsteredWeapon );
	if ( holsterRenderEntity.hModel )
	{
		playerInfo.Set( "holsteredWeaponModel", holsterRenderEntity.hModel->Name() );
	}
	playerInfo.SetMatrix( "holsterAxis", holsterAxis );
	// Koz end

	achievementManager.SavePersistentData( playerInfo );
}

/*
===============
idPlayer::RestorePersistantInfo

Restores any inventory and player stats when changing levels.
===============
*/
void idPlayer::RestorePersistantInfo()
{
	if( common->IsMultiplayer() || g_demoMode.GetBool() )
	{
		gameLocal.persistentPlayerInfo[entityNumber].Clear();
	}
	
	spawnArgs.Copy( gameLocal.persistentPlayerInfo[entityNumber] );
	
	inventory.RestoreInventory( this, spawnArgs );
	health = spawnArgs.GetInt( "health", "100" );
	hands[vr_weaponHand.GetInteger()].idealWeapon = spawnArgs.GetInt( "current_weapon", "1" );
	hands[ 1 - vr_weaponHand.GetInteger() ].idealWeapon = weapon_fists;
	
	// Koz begin
	headingBeamActive = spawnArgs.GetBool( "headingBeamActive", "1" );
	bool laserSightActive = spawnArgs.GetBool( "laserSightActive", "1" );
	hands[ 0 ].laserSightActive = laserSightActive;
	hands[ 1 ].laserSightActive = laserSightActive;
	commonVr->currentFlashlightMode = spawnArgs.GetInt( "currentFlashMode", "3" );
	hudActive = spawnArgs.GetBool( "hudActive", "1" );
	
	holsteredWeapon = spawnArgs.GetInt( "holsteredWeapon", "-1" );
	extraHolsteredWeapon = spawnArgs.GetInt( "extraHolsteredWeapon", "-1" );

	idStr hwm;

	//playerInfo.Get( "holsteredWeaponModel", holsterRenderEntity.hModel->Name() );
	hwm = spawnArgs.GetString( "holsteredWeaponModel", "" );
	holsterAxis = spawnArgs.GetMatrix( "holsterAxis", "" );


	holsterRenderEntity.hModel = renderModelManager->FindModel( hwm.c_str() );
	

	common->Printf( "Restored holsteredWeapon %d\n", holsteredWeapon );
	common->Printf( "Restored extraHolsteredWeapon %d\n", extraHolsteredWeapon );


	//playerInfo.SetInt( "holsteredWeapon", holsteredWeapon );
	//playerInfo.SetInt( "extraHolsteredWeapon", extraHolsteredWeapon );
	//playerInfo.Set( "extraHolsteredWeaponModel", extraHolsteredWeaponModel );
	// Koz end

	playedTimeSecs = spawnArgs.GetInt( "playedTime" );
	
	achievementManager.RestorePersistentData( spawnArgs );
}

/*
==============
idPlayer::UpdateSkinSetup
==============
*/
void idPlayer::UpdateSkinSetup()
{
	const char* handsOnly = "/vrHandsOnly";
	const char* weaponOnly = "/vrWeaponsOnly";
	const char* body = "/vrBody";

	gameExpansionType_t gameType;

	if ( game->isVR )
	{
		
		idStr skinN = skin->GetName();
		
		if ( strstr( skinN.c_str(), "skins/characters/player/tshirt_mp" ) )
		{
			skinN = "skins/characters/player/tshirt_mp";
		}
		else if ( strstr( skinN.c_str(), "skins/characters/player/greenmarine_arm2" ) )
		{
			skinN = "skins/characters/player/greenmarine_arm2";
		}
		else if ( strstr( skinN.c_str(), "skins/characters/player/d3xp_sp_vrik" ) )
		{
			skinN = "skins/characters/player/d3xp_sp_vrik";
		}
		else
		{
			gameType = GetExpansionType();

			if ( gameType == GAME_D3XP || gameType == GAME_D3LE )
			{
				skinN = "skins/characters/player/d3xp_sp_vrik";
			}
			else
			{
				skinN = "skins/characters/player/greenmarine_arm2";
			}
		}

		if ( commonVr->thirdPersonMovement )
		{
			skinN += body;
		}
		else if ( vr_playerBodyMode.GetInteger() == 1 || ( vr_playerBodyMode.GetInteger() == 2 && (hands[ 0 ].currentWeapon == weapon_fists || hands[ 1 ].currentWeapon == weapon_fists || commonVr->handInGui) ) )
		{
			skinN += handsOnly;
		}
		else if ( vr_playerBodyMode.GetInteger() == 2 ) 
		{
			skinN += weaponOnly;
		}
		else
		{
			// if crouched more than 16 inches hide the body if enabled.
			if ( (commonVr->headHeightDiff < -16.0f || IsCrouching()) && vr_crouchHideBody.GetBool() )
			{
				skinN += handsOnly;
			}
			else
			{
				skinN += body;
			}
		}
		
		skin = declManager->FindSkin( skinN.c_str(), false );
	//	common->Printf( "UpdateSkinSetup returning player skin %s\n", skinN.c_str() );
		return;
	}
		
	
	if( !common->IsMultiplayer() )
	{
		return;
	}
	
	if( gameLocal.mpGame.IsGametypeTeamBased() )    /* CTF */
	{
		skinIndex = team + 1;
	}
	else
	{
		// Each player will now have their Skin Index Reflect their entity number  ( host = 0, client 1 = 1, client 2 = 2 etc )
		skinIndex = entityNumber; // session->GetActingGameStateLobbyBase().GetLobbyUserSkinIndex( gameLocal.lobbyUserIDs[entityNumber] );
	}
	const char* baseSkinName = gameLocal.mpGame.GetSkinName( skinIndex );
	skin = declManager->FindSkin( baseSkinName, false );
	if( PowerUpActive( BERSERK ) )
	{
		idStr powerSkinName = baseSkinName;
		powerSkinName.Append( "_berserk" );
		powerUpSkin = declManager->FindSkin( powerSkinName );
	}
	else if( PowerUpActive( INVULNERABILITY ) )
	{
		idStr powerSkinName = baseSkinName;
		powerSkinName.Append( "_invuln" );
		powerUpSkin = declManager->FindSkin( powerSkinName );
	}
	else if( PowerUpActive( INVISIBILITY ) )
	{
		const char* invisibleSkin = "";
		spawnArgs.GetString( "skin_invisibility", "", &invisibleSkin );
		powerUpSkin = declManager->FindSkin( invisibleSkin );
	}
}


/*
===============
idPlayer::UpdateHudStats
===============
*/
void idPlayer::UpdateHudStats( idMenuHandler_HUD* _hudManager )
{

	if( _hudManager && _hudManager->GetHud() )
	{
	
		idMenuScreen_HUD* hud = _hudManager->GetHud();
		hud->UpdateHealthArmor( this );
		hud->UpdateStamina( this );
		hud->UpdateWeaponInfo( this );
		
		if( inventory.weaponPulse )
		{
			UpdateHudWeapon( vr_weaponHand.GetInteger() );
			inventory.weaponPulse = false;
		}
		
		if( gameLocal.mpGame.IsGametypeFlagBased() )
		{
			hud->SetFlagState( 0, gameLocal.mpGame.GetFlagStatus( 0 ) );
			hud->SetFlagState( 1, gameLocal.mpGame.GetFlagStatus( 1 ) );
			
			hud->SetTeamScore( 0, gameLocal.mpGame.GetFlagPoints( 0 ) );
			hud->SetTeamScore( 1, gameLocal.mpGame.GetFlagPoints( 1 ) );
			
			hud->SetTeam( team );
		}
		
	}
}

/*
===============
idPlayer::UpdateHudWeapon
Carl: -1 = don't flash the weapon, 0 = flash right hand weapon, 1 = flash left hand weapon
===============
*/
void idPlayer::UpdateHudWeapon( int flashWeaponHand )
{

	idMenuScreen_HUD* curDisplay = hud;
	idPlayer* p = this;
	if( gameLocal.GetLocalClientNum() >= 0 && gameLocal.entities[ gameLocal.GetLocalClientNum() ] && gameLocal.entities[ gameLocal.GetLocalClientNum() ]->IsType( idPlayer::Type ) )
	{
		p = static_cast< idPlayer* >( gameLocal.entities[ gameLocal.GetLocalClientNum() ] );
		if( p->spectating && p->spectator == entityNumber )
		{
			assert( p->hud );
			curDisplay = p->hud;
		}
	}
	
	if( !curDisplay )
	{
		return;
	}
	
	curDisplay->UpdateWeaponStates( p, flashWeaponHand );
}

/*
===============
idPlayer::UpdateChattingHud
===============
*/
void idPlayer::UpdateChattingHud()
{

	idMenuScreen_HUD* curDisplay = hud;
	idPlayer* p = this;
	if( gameLocal.GetLocalClientNum() >= 0 && gameLocal.entities[ gameLocal.GetLocalClientNum() ] && gameLocal.entities[ gameLocal.GetLocalClientNum() ]->IsType( idPlayer::Type ) )
	{
		p = static_cast< idPlayer* >( gameLocal.entities[ gameLocal.GetLocalClientNum() ] );
		if( p->spectating && p->spectator == entityNumber )
		{
			assert( p->hud );
			curDisplay = p->hud;
		}
	}
	
	if( !curDisplay )
	{
		return;
	}
	
	curDisplay->UpdateChattingHud( p );
}


/*
========================
idMenuScreen_Scoreboard::UpdateSpectating
========================
*/
void idPlayer::UpdateSpectatingText()
{

	idSWF* spectatorMessages = mpMessages;
	idPlayer* p = this;
	if( gameLocal.GetLocalClientNum() >= 0 && gameLocal.entities[ gameLocal.GetLocalClientNum() ] && gameLocal.entities[ gameLocal.GetLocalClientNum() ]->IsType( idPlayer::Type ) )
	{
		p = static_cast< idPlayer* >( gameLocal.entities[ gameLocal.GetLocalClientNum() ] );
		if( p && p->spectating )
		{
			spectatorMessages = p->mpMessages;
		}
	}
	
	if( !spectatorMessages || !spectatorMessages->IsActive() )
	{
		return;
	}
	
	idPlayer* viewPlayer = static_cast<idPlayer*>( gameLocal.entities[ p->spectator ] );
	if( viewPlayer == NULL )
	{
		return;
	}
	
	idStr spectatetext[ 2 ];
	if( !gameLocal.mpGame.IsScoreboardActive() )
	{
		gameLocal.mpGame.GetSpectateText( p, spectatetext, false );
	}
	
	idSWFScriptObject& root = spectatorMessages->GetRootObject();
	idSWFTextInstance* txtVal = root.GetNestedText( "txtSpectating" );
	if( txtVal != NULL )
	{
		txtVal->tooltip = true;
		txtVal->SetText( spectatetext[0] );
		txtVal->SetStrokeInfo( true, 0.75f, 1.75f );
	}
	
	txtVal = root.GetNestedText( "txtFollow" );
	if( txtVal != NULL )
	{
		txtVal->SetText( spectatetext[1] );
		txtVal->SetStrokeInfo( true, 0.75f, 1.75f );
	}
}

/*
===============
idPlayer::UpdateMpMessages
===============
*/
void idPlayer::AddChatMessage( int index, int alpha, const idStr& message )
{

	if( mpMessages == NULL || !mpMessages->IsActive() )
	{
		return;
	}
	
	idSWFScriptObject* mpChat = mpMessages->GetRootObject().GetNestedObj( "_left", "mpChat" );
	
	idSWFSpriteInstance* info = mpChat->GetNestedSprite( va( "info%i", index ) );
	idSWFTextInstance* txtVal = mpChat->GetNestedText( va( "info%i", index ), "txtVal" );
	if( info )
	{
		info->SetVisible( true );
		if( alpha >= 4 )
		{
			info->SetAlpha( 1.0f );
		}
		else if( alpha == 3 )
		{
			info->SetAlpha( 0.875f );
		}
		else if( alpha == 2 )
		{
			info->SetAlpha( 0.75f );
		}
		else if( alpha == 1 )
		{
			info->SetAlpha( 0.625f );
		}
		else
		{
			info->SetAlpha( 0.5f );
		}
	}
	
	if( txtVal )
	{
		txtVal->SetIgnoreColor( false );
		txtVal->SetText( message );
		txtVal->SetStrokeInfo( true, 0.9f, 1.75f );
	}
	
}

/*
===============
idPlayer::UpdateMpMessages
===============
*/
void idPlayer::ClearChatMessage( int index )
{

	if( mpMessages == NULL || !mpMessages->IsActive() )
	{
		return;
	}
	
	idSWFScriptObject* mpChat = mpMessages->GetRootObject().GetNestedObj( "_left", "mpChat" );
	
	idSWFSpriteInstance* info = mpChat->GetNestedSprite( va( "info%i", index ) );
	idSWFTextInstance* txtVal = mpChat->GetNestedText( va( "info%i", index ), "txtVal" );
	if( info )
	{
		info->SetVisible( false );
	}
	
	if( txtVal )
	{
		txtVal->SetText( "" );
	}
	
}

/*
===============
idPlayer::DrawHUD
===============
*/
void idPlayer::DrawHUD( idMenuHandler_HUD* _hudManager )
{
	SCOPED_PROFILE_EVENT( "idPlayer::DrawHUD" );
	// Koz begin
	if ( game->isVR && vr_hudType.GetInteger() == VR_HUD_NONE && !vr_flicksyncCharacter.GetInteger() )
	{
		return;
	}
	// Koz end

	idWeapon* weapon = GetMainWeapon();

	if ( ( !weapon || influenceActive != INFLUENCE_NONE || privateCameraView || gameLocal.GetCamera() || !g_showHud.GetBool() ) && !vr_flicksyncCharacter.GetInteger() )
	{
		return;
	}

	if( common->IsMultiplayer() )
	{
		UpdateChattingHud();
		UpdateSpectatingText();
	}
	
	// Always draw the local client's messages so that chat works correctly while spectating another player.
	idPlayer* localPlayer = static_cast< idPlayer* >( gameLocal.entities[ gameLocal.GetLocalClientNum() ] );
	
	if( localPlayer != NULL && localPlayer->mpMessages != NULL )
	{
		localPlayer->mpMessages->Render( renderSystem, Sys_Milliseconds() );
	}
	
	
	UpdateHudStats( _hudManager );
	
	if( spectating )
	{
		return;
	}
	
	if( _hudManager )
	{
		_hudManager->Update();
	}
	
	weapon->UpdateGUI();
	
	// weapon targeting crosshair
	if( !GuiActive() )
	{
		// don't show the 2D crosshair in stereo rendering, use the
		// laser sight model instead
		if( _hudManager && _hudManager->GetHud() )
		{
		
			idMenuScreen_HUD* hud = _hudManager->GetHud();
			
			if( weapon->ShowCrosshair() && !IsGameStereoRendered() )
			{
				if( weapon->GetGrabberState() == 1 || weapon->GetGrabberState() == 2 )
				{
					hud->SetCursorState( this, CURSOR_GRABBER, 1 );
					hud->SetCursorState( this, CURSOR_IN_COMBAT, 0 );
				}
				else
				{
					hud->SetCursorState( this, CURSOR_GRABBER, 0 );
					hud->SetCursorState( this, CURSOR_IN_COMBAT, 1 );
				}
			}
			else
			{
				hud->SetCursorState( this, CURSOR_NONE, 1 );
			}
			
			hud->UpdateCursorState();
			
		}
	}
	else if( _hudManager && _hudManager->GetHud() )
	{
	
		idMenuScreen_HUD* hud = _hudManager->GetHud();
		
		hud->SetCursorState( this, CURSOR_NONE, 1 );
		hud->UpdateCursorState();
	}
}

/*
===============
idPlayer::DrawHUDVR
===============
*/
void idPlayer::DrawHUDVR( idMenuHandler_HUD* _hudManager )
{
	SCOPED_PROFILE_EVENT( "idPlayer::DrawHUD" );
	// Koz begin
	console->Draw( false );

	if ( !GuiActive()  ) // only draw crosshairs if enabled in VR.
	{
		
		if ( _hudManager && _hudManager->GetHud() )
		{

			idMenuScreen_HUD* hud = _hudManager->GetHud();
			
			// we don't want the 2d crosshair in vr
			hud->SetCursorState( this, CURSOR_NONE, 1 );
			hud->UpdateCursorState();

		}
	}
	else if ( _hudManager && _hudManager->GetHud() )
	{

		idMenuScreen_HUD* hud = _hudManager->GetHud();

		hud->SetCursorState( this, CURSOR_NONE, 1 );
		hud->UpdateCursorState();
	}

	//renderSystem->CaptureRenderToImage( "_crosshairImage", true );
	

	idWeapon* weapon = GetMainWeapon();
	if ((!weapon || influenceActive != INFLUENCE_NONE || privateCameraView || gameLocal.GetCamera() || vr_hudType.GetInteger() == VR_HUD_NONE) && !vr_flicksyncCharacter.GetInteger())
	{
		return;
	}

	if ( common->IsMultiplayer() )
	{
		UpdateChattingHud();
		UpdateSpectatingText();
	}

	// Always draw the local client's messages so that chat works correctly while spectating another player.
	idPlayer* localPlayer = static_cast< idPlayer* >(gameLocal.entities[gameLocal.GetLocalClientNum()]);

	if ( localPlayer != NULL && localPlayer->mpMessages != NULL )
	{
		localPlayer->mpMessages->Render( renderSystem, Sys_Milliseconds() );
	}


	UpdateHudStats( _hudManager );

	if ( spectating )
	{
		return;
	}

	if ( _hudManager )
	{
		_hudManager->Update();
	}

	weapon->UpdateGUI();
	
	
	
	renderSystem->CaptureRenderToImage( "_hudImage", true );
		
}

/*
===============
idPlayer::EnterCinematic
===============
*/
void idPlayer::EnterCinematic()
{
	if( PowerUpActive( HELLTIME ) )
	{
		StopHelltime();
	}
	
	Hide();
	StopSound( SND_CHANNEL_PDA_AUDIO, false );
	StopSound( SND_CHANNEL_PDA_VIDEO, false );
	
	if( hudManager && !vr_flicksyncCharacter.GetInteger() )
	{
		hudManager->SetRadioMessage( false );
	}
	physicsObj.SetLinearVelocity( vec3_origin );
	
	SetState( "EnterCinematic" );
	UpdateScript();
	
	if( weaponEnabled )
	{
		for( int h = 0; h < 2; h++ )
		{
			idWeapon* weapon = GetWeaponInHand( h );
			if ( weapon )
				weapon->EnterCinematic();
		}
	}
	if( flashlight.GetEntity() )
	{
		flashlight.GetEntity()->EnterCinematic();
	}
	
	AI_FORWARD		= false;
	AI_BACKWARD		= false;
	AI_STRAFE_LEFT	= false;
	AI_STRAFE_RIGHT	= false;
	AI_RUN			= false;
	AI_ATTACK_HELD	= false;
	AI_WEAPON_FIRED	= false;
	AI_JUMP			= false;
	AI_CROUCH		= false;
	AI_ONGROUND		= true;
	AI_ONLADDER		= false;
	AI_DEAD			= ( health <= 0 );
	AI_RUN			= false;
	AI_PAIN			= false;
	AI_HARDLANDING	= false;
	AI_SOFTLANDING	= false;
	AI_RELOAD		= false;
	AI_TELEPORT		= false;
	AI_TURN_LEFT	= false;
	AI_TURN_RIGHT	= false;

	if( vr_flicksyncCharacter.GetInteger() )
	{
		HideTip();
		if( hud )
		{
			if( objectiveUp )
			{
				hud->HideObjective( false );
				objectiveUp = false;
			}
			hud->SetCursorState(this, CURSOR_TALK, 0);
			hud->SetCursorState(this, CURSOR_IN_COMBAT, 0);
			hud->SetCursorState(this, CURSOR_ITEM, 0);
			hud->SetCursorState(this, CURSOR_GRABBER, 0);
			hud->SetCursorState(this, CURSOR_NONE, 0);
			hud->UpdateCursorState();
		}
		Flicksync_StartCutscene();
	}
}

/*
===============
idPlayer::ExitCinematic
===============
*/
void idPlayer::ExitCinematic()
{
	Show();
	
	if( weaponEnabled )
	{
		for( int h = 0; h < 2; h++ )
		{
			idWeapon* weapon = GetWeaponInHand( h );
			if( weapon )
				weapon->ExitCinematic();
		}
	}
	if( flashlight.GetEntity() )
	{
		flashlight.GetEntity()->ExitCinematic();
	}
	
	// long cinematics would have surpassed the healthTakeTime, causing the player to take damage
	// immediately after the cinematic ends.  Instead we start the healthTake cooldown again once
	// the cinematic ends.
	if( g_skill.GetInteger() == 3 )
	{
		nextHealthTake = gameLocal.time + g_healthTakeTime.GetInteger() * 1000;
	}
	
	SetState( "ExitCinematic" );
	UpdateScript();
}

/*
=====================
idPlayer::UpdateConditions
=====================
*/
void idPlayer::UpdateConditions()
{
	idVec3	velocity;
	float	fallspeed;
	float	forwardspeed;
	float	sidespeed;
	
	// minus the push velocity to avoid playing the walking animation and sounds when riding a mover
	velocity = physicsObj.GetLinearVelocity() - physicsObj.GetPushedLinearVelocity();
	fallspeed = velocity * physicsObj.GetGravityNormal();
	
	if( influenceActive )
	{
		AI_FORWARD		= false;
		AI_BACKWARD		= false;
		AI_STRAFE_LEFT	= false;
		AI_STRAFE_RIGHT	= false;
	}
	else if( gameLocal.time - lastDmgTime < 500 )
	{
		forwardspeed = velocity * viewAxis[ 0 ];
		sidespeed = velocity * viewAxis[ 1 ];
		AI_FORWARD		= AI_ONGROUND && ( forwardspeed > 20.01f );
		AI_BACKWARD		= AI_ONGROUND && ( forwardspeed < -20.01f );
		AI_STRAFE_LEFT	= AI_ONGROUND && ( sidespeed > 20.01f );
		AI_STRAFE_RIGHT	= AI_ONGROUND && ( sidespeed < -20.01f );
	}
	else if( xyspeed > MIN_BOB_SPEED )
	{
		AI_FORWARD		= AI_ONGROUND && ( usercmd.forwardmove > 0 );
		AI_BACKWARD		= AI_ONGROUND && ( usercmd.forwardmove < 0 );
		AI_STRAFE_LEFT	= AI_ONGROUND && ( usercmd.rightmove < 0 );
		AI_STRAFE_RIGHT	= AI_ONGROUND && ( usercmd.rightmove > 0 );
	}
	else
	{
		AI_FORWARD		= false;
		AI_BACKWARD		= false;
		AI_STRAFE_LEFT	= false;
		AI_STRAFE_RIGHT	= false;
	}
	
	AI_RUN			= ( usercmd.buttons & BUTTON_RUN ) && ( ( !pm_stamina.GetFloat() ) || ( stamina > pm_staminathreshold.GetFloat() ) );
	AI_DEAD			= ( health <= 0 );
}

/*
==================
WeaponFireFeedback

Called when a weapon fires, generates head twitches, etc
==================
*/
void idPlayer::WeaponFireFeedback( int hand, const idDict* weaponDef )
{
	// force a blink
	blink_time = 0;
	
	// play the fire animation
	AI_WEAPON_FIRED = true;

	// update view feedback
	playerView.WeaponFireFeedback( weaponDef );
	
	// shake controller
	float highMagnitude = weaponDef->GetFloat( "controllerShakeHighMag" );
	int highDuration = weaponDef->GetInt( "controllerShakeHighTime" );
	float lowMagnitude = weaponDef->GetFloat( "controllerShakeLowMag" );
	int lowDuration = weaponDef->GetInt( "controllerShakeLowTime" );
	//const char *name = weaponDef->GetString( "inv_name" );
	
	if( IsLocallyControlled() )
	{
		hands[hand].SetControllerShake( highMagnitude, highDuration, lowMagnitude, lowDuration );
	}
}

/*
===============
idPlayer::StopFiring
Carl: TODO In most cases this is because we want to stop the player from firing all their weapons
But it is also called when our hand is in a GUI, in which case maybe stop only that hand? Dual wielding
===============
*/
void idPlayer::StopFiring()
{
	if( vr_debugHands.GetBool() )
	{
		common->Printf( "Before StopFiring():\n");
		hands[HAND_LEFT].debugPrint();
		hands[HAND_RIGHT].debugPrint();
	}
	AI_ATTACK_HELD	= false;
	AI_WEAPON_FIRED = false;
	AI_RELOAD		= false;
	for( int h = 0; h < 2; h++ )
	{
		if( hands[h].weapon.GetEntity() )
		{
			hands[h].weapon.GetEntity()->EndAttack();
		}
	}
	if( vr_debugHands.GetBool() )
	{
		common->Printf( "After StopFiring():\n" );
		hands[HAND_LEFT].debugPrint();
		hands[HAND_RIGHT].debugPrint();
	}
}

/*
===============
idPlayer::FireWeapon
===============
*/
idCVar g_infiniteAmmo( "g_infiniteAmmo", "0", CVAR_GAME | CVAR_BOOL, "infinite ammo" );
extern idCVar ui_autoSwitch;
void idPlayer::FireWeapon( int hand, idWeapon *weap )
{
	if( vr_debugHands.GetBool() )
	{
		common->Printf( "Before FireWeapon(%d):\n", hand );
		hands[hand].debugPrint();
	}
	idMat3 axis;
	idVec3 muzzle;
	
	if( privateCameraView )
	{
		return;
	}
	
	if( g_editEntityMode.GetInteger() )
	{
		GetViewPos( muzzle, axis );
		if( gameLocal.editEntities->SelectEntity( muzzle, axis[0], this ) )
		{
			return;
		}
	}
	
	if( !hiddenWeapon && weap->IsReady() )
	{
		if( g_infiniteAmmo.GetBool() || weap->AmmoInClip() || weap->AmmoAvailable() )
		{
			weapon_t w = weap->IdentifyWeapon();
			// Koz grabber doesn't fire projectiles, so player script won't trigger fire anim for hand if we dont do this
			if( w == WEAPON_GRABBER ) AI_WEAPON_FIRED = true;

			AI_ATTACK_HELD = true;
			weap->BeginAttack();
			if( ( weapon_soulcube >= 0 ) && ( w == WEAPON_SOULCUBE ) )
			{
				if( hud )
				{
					hud->UpdateSoulCube( false );
				}
				hands[hand].SelectWeapon( hands[hand].previousWeapon, false, false );
			}
			if( ( weapon_bloodstone >= 0 ) && ( hands[hand].currentWeapon == weapon_bloodstone ) && (inventory.weapons & ( 1 << weapon_bloodstone_active1 )) && hands[hand].weapon.GetEntity()->GetStatus() == WP_READY )
			{
				// tell it to switch to the previous weapon. Only do this once to prevent
				// weapon toggling messing up the previous weapon
				if( hands[hand].idealWeapon == weapon_bloodstone )
				{
					if( hands[hand].previousWeapon == weapon_bloodstone || hands[hand].previousWeapon == -1 )
					{
						hands[hand].NextBestWeapon();
					}
					else
					{
						//Since this is a toggle weapon just select itself and it will toggle to the last weapon
						hands[hand].SelectWeapon( weapon_bloodstone, false, false );
					}
				}
			}
		}
		else
		{
		
			idLobbyBase& lobby = session->GetActingGameStateLobbyBase();
			lobbyUserID_t& lobbyUserID = gameLocal.lobbyUserIDs[ entityNumber ];
			bool autoSwitch = lobby.GetLobbyUserWeaponAutoSwitch( lobbyUserID );
			if( !autoSwitch )
			{
				return;
			}
			
			// update our ammo clip in our inventory
			if( ( hands[ hand ].currentWeapon >= 0 ) && ( hands[ hand ].currentWeapon < MAX_WEAPONS ) )
			{
				inventory.SetClipAmmoForWeapon( hands[hand].currentWeapon, hands[hand].isTheDuplicate, hands[hand].weapon.GetEntity()->AmmoInClip() );
			}
			
			hands[ hand ].NextBestWeapon();
		}
	}
	
	
	if( tipUp )
	{
		HideTip();
	}
	
	if( objectiveUp )
	{
		HideObjective();
	}
}

/*
===============
idPlayer::CacheWeapons
===============
*/
void idPlayer::CacheWeapons()
{
	idStr	weap;
	int		w;
	
	// check if we have any weapons
	if( !inventory.weapons )
	{
		return;
	}
	
	for( w = 0; w < MAX_WEAPONS; w++ )
	{
		if( inventory.weapons & ( 1 << w ) )
		{
			weap = spawnArgs.GetString( va( "def_weapon%d", w ) );
			if( weap != "" )
			{
				idWeapon::CacheWeapon( weap );
			}
			else
			{
				inventory.weapons &= ~( 1 << w );
				inventory.foundWeapons &= ~( 1 << w );
				inventory.duplicateWeapons &= ~( 1 << w );
			}
		}
	}
}

/*
===============
idPlayer::SetQuickSlot
===============
*/
void idPlayer::SetQuickSlot( int index, int val )
{
	if( index >= NUM_QUICK_SLOTS || index < 0 )
	{
		return;
	}
	
	quickSlot[ index ] = val;
}

/*
===============
idPlayer::GetQuickSlot
===============
*/
int idPlayer::GetQuickSlot( int index )
{

	if( index >= NUM_QUICK_SLOTS || index < 0 )
	{
		return -1;
	}
	
	return quickSlot[ index ];
}

/*
===============
idPlayer::Give
===============
*/
bool idPlayer::Give( const char* statname, const char* value, unsigned int giveFlags, int hand )
{
	int amount;
	
	if( AI_DEAD )
	{
		return false;
	}
	
	if( !idStr::Icmp( statname, "health" ) )
	{
		if( health >= inventory.maxHealth )
		{
			return false;
		}
		if( giveFlags & ITEM_GIVE_UPDATE_STATE )
		{
			amount = atoi( value );
			if( amount )
			{
				health += amount;
				if( health > inventory.maxHealth )
				{
					health = inventory.maxHealth;
				}
				healthPulse = true;
			}
		}
	}
	else if( !idStr::Icmp( statname, "stamina" ) )
	{
		if( stamina >= 100 )
		{
			return false;
		}
		if( giveFlags & ITEM_GIVE_UPDATE_STATE )
		{
			stamina += atof( value );
			if( stamina > 100 )
			{
				stamina = 100;
			}
		}
	}
	else if( !idStr::Icmp( statname, "heartRate" ) )
	{
		if( giveFlags & ITEM_GIVE_UPDATE_STATE )
		{
			heartRate += atoi( value );
			if( heartRate > MAX_HEARTRATE )
			{
				heartRate = MAX_HEARTRATE;
			}
		}
	}
	else if( !idStr::Icmp( statname, "air" ) )
	{
		if( airMsec >= pm_airMsec.GetInteger() )
		{
			return false;
		}
		if( giveFlags & ITEM_GIVE_UPDATE_STATE )
		{
			airMsec += pm_airMsec.GetInteger() * atoi( value ) / 100;
			if( airMsec > pm_airMsec.GetInteger() )
			{
				airMsec = pm_airMsec.GetInteger();
			}
		}
	}
	else if( !idStr::Icmp( statname, "enviroTime" ) )
	{
		if( ( giveFlags & ITEM_GIVE_UPDATE_STATE ) && PowerUpActive( ENVIROTIME ) )
		{
			inventory.powerupEndTime[ ENVIROTIME ] += ( atof( value ) * 1000 );
		}
		else
		{
			GivePowerUp( ENVIROTIME, atoi( value ) * 1000, giveFlags );
		}
	}
	else
	{
		if( hand < 0 )
			hand = vr_weaponHand.GetInteger();
		bool ret = inventory.Give( this, spawnArgs, statname, value, &hands[hand].idealWeapon, true, giveFlags );
		return ret;
	}
	return true;
}


/*
===============
idPlayer::GiveHealthPool

adds health to the player health pool
===============
*/
void idPlayer::GiveHealthPool( float amt )
{

	if( AI_DEAD )
	{
		return;
	}
	
	if( health > 0 )
	{
		healthPool += amt;
		if( healthPool > inventory.maxHealth - health )
		{
			healthPool = inventory.maxHealth - health;
		}
		nextHealthPulse = gameLocal.time;
	}
}

/*
===============
idPlayer::GiveItem

Returns false if the item shouldn't be picked up
===============
*/
bool idPlayer::GiveItem( idItem* item, unsigned int giveFlags )
{
	int					i;
	const idKeyValue*	arg;
	idDict				attr;
	bool				gave;
	int					numPickup;
	
	if( common->IsMultiplayer() && spectating )
	{
		return false;
	}
	
	if( idStr::FindText( item->GetName(), "weapon_flashlight_new" ) > -1 )
	{
		return false;
	}
	
	if( idStr::FindText( item->GetName(), "weapon_flashlight" ) > -1 )
	{
		// don't allow flashlight weapon unless classic mode is enabled
		return false;
	}
	
	item->GetAttributes( attr );
	
	gave = false;
	numPickup = inventory.pickupItemNames.Num();
	for( i = 0; i < attr.GetNumKeyVals(); i++ )
	{
		arg = attr.GetKeyVal( i );
		if( Give( arg->GetKey(), arg->GetValue(), giveFlags, -1 ) )
		{
			gave = true;
		}
	}
	
	if( giveFlags & ITEM_GIVE_FEEDBACK )
	{
		arg = item->spawnArgs.MatchPrefix( "inv_weapon", NULL );
		if( arg )
		{
			// We need to update the weapon hud manually, but not
			// the armor/ammo/health because they are updated every
			// frame no matter what
			UpdateHudWeapon( -1 );
		}
		
		// display the pickup feedback on the hud
		if( gave && ( numPickup == inventory.pickupItemNames.Num() ) )
		{
			inventory.AddPickupName( item->spawnArgs.GetString( "inv_name" ), this ); //_D3XP
		}
	}
	
	return gave;
}

/*
===============
idPlayer::PowerUpModifier
===============
*/
float idPlayer::PowerUpModifier( int type )
{
	float mod = 1.0f;
	
	if( PowerUpActive( BERSERK ) )
	{
		switch( type )
		{
			case SPEED:
			{
				mod *= 1.7f;
				break;
			}
			case PROJECTILE_DAMAGE:
			{
				mod *= 2.0f;
				break;
			}
			case MELEE_DAMAGE:
			{
				mod *= 30.0f;
				break;
			}
			case MELEE_DISTANCE:
			{
				mod *= 2.0f;
				break;
			}
		}
	}
	
	if( common->IsMultiplayer() && !common->IsClient() )
	{
		if( PowerUpActive( MEGAHEALTH ) )
		{
			if( healthPool <= 0 )
			{
				GiveHealthPool( 100 );
			}
		}
		else
		{
			healthPool = 0;
		}
		
		/*if( PowerUpActive( HASTE ) ) {
			switch( type ) {
			case SPEED: {
				mod = 1.7f;
				break;
						}
			}
		}*/
	}
	
	return mod;
}

/*
===============
idPlayer::PowerUpActive
===============
*/
bool idPlayer::PowerUpActive( int powerup ) const
{
	return ( inventory.powerups & ( 1 << powerup ) ) != 0;
}

/*
===============
idPlayer::GivePowerUp
===============
*/
bool idPlayer::GivePowerUp( int powerup, int time, unsigned int giveFlags )
{
	const char* sound;
	
	if( powerup >= 0 && powerup < MAX_POWERUPS )
	{
	
		if( giveFlags & ITEM_GIVE_UPDATE_STATE )
		{
			if( common->IsServer() )
			{
				idBitMsg	msg;
				byte		msgBuf[MAX_EVENT_PARAM_SIZE];
				
				msg.InitWrite( msgBuf, sizeof( msgBuf ) );
				msg.WriteShort( powerup );
				msg.WriteShort( time );
				ServerSendEvent( EVENT_POWERUP, &msg, false );
			}
			
			if( powerup != MEGAHEALTH )
			{
				inventory.GivePowerUp( this, powerup, time );
			}
		}
		
		switch( powerup )
		{
			case BERSERK:
			{
				if( giveFlags & ITEM_GIVE_FROM_WEAPON )
				{
					// Berserk is granted by the bloodstone in ROE, but we don't want any of the
					// standard behavior (sound fx, switch to fists) when you get it this way.
				}
				else
				{
					if( giveFlags & ITEM_GIVE_FEEDBACK )
					{
						inventory.AddPickupName( "#str_00100627", this );
						
						if( spawnArgs.GetString( "snd_berserk_third", "", &sound ) && sound[ 0 ] != '\0' )
						{
							StartSoundShader( declManager->FindSound( sound ), SND_CHANNEL_DEMONIC, 0, false, NULL );
						}
					}
					
					if( giveFlags & ITEM_GIVE_UPDATE_STATE )
					{
						if( !common->IsClient() )
						{
							hands[ 0 ].idealWeapon = weapon_fists;
							hands[ 1 ].idealWeapon = weapon_fists;
						}
					}
				}
				break;
			}
			case INVISIBILITY:
			{
				if( common->IsMultiplayer() && ( giveFlags & ITEM_GIVE_FEEDBACK ) )
				{
					inventory.AddPickupName( "#str_00100628", this );
				}
				if( giveFlags & ITEM_GIVE_UPDATE_STATE )
				{
					// remove any decals from the model
					if( modelDefHandle != -1 )
					{
						gameRenderWorld->RemoveDecals( modelDefHandle );
					}
					for( int h = 0; h < 2; h++ )
					{
						if( hands[h].weapon.GetEntity() )
						{
							hands[h].weapon.GetEntity()->UpdateSkin();
						}
					}
					if( flashlight.GetEntity() )
					{
						flashlight.GetEntity()->UpdateSkin();
					}
				}
				
				/*				if ( spawnArgs.GetString( "snd_invisibility", "", &sound ) ) {
									StartSoundShader( declManager->FindSound( sound ), SND_CHANNEL_ANY, 0, false, NULL );
								} */
				break;
			}
			case ADRENALINE:
			{
				if( giveFlags & ITEM_GIVE_FEEDBACK )
				{
					inventory.AddPickupName( "#str_00100799", this );
				}
				
				if( giveFlags & ITEM_GIVE_UPDATE_STATE )
				{
					stamina = 100.0f;
				}
				break;
			}
			case MEGAHEALTH:
			{
				if( giveFlags & ITEM_GIVE_FEEDBACK )
				{
					if( common->IsMultiplayer() )
					{
						inventory.AddPickupName( "#str_00100629", this );
					}
					if( spawnArgs.GetString( "snd_megahealth", "", &sound ) )
					{
						StartSoundShader( declManager->FindSound( sound ), SND_CHANNEL_ANY, 0, false, NULL );
					}
				}
				if( giveFlags & ITEM_GIVE_UPDATE_STATE )
				{
					health = 200;
				}
				break;
			}
			case HELLTIME:
			{
				if( spawnArgs.GetString( "snd_helltime_start", "", &sound ) )
				{
					PostEventMS( &EV_StartSoundShader, 0, sound, SND_CHANNEL_ANY );
				}
				if( spawnArgs.GetString( "snd_helltime_loop", "", &sound ) )
				{
					PostEventMS( &EV_StartSoundShader, 0, sound, SND_CHANNEL_DEMONIC );
				}
				break;
			}
			case ENVIROSUIT:
			{
				if( giveFlags & ITEM_GIVE_FEEDBACK )
				{
					// Turn on the envirosuit sound
					if( gameSoundWorld )
					{
						gameSoundWorld->SetEnviroSuit( true );
					}
				}
				
				if( giveFlags & ITEM_GIVE_UPDATE_STATE )
				{
					// Put the helmet and lights on the player
					idDict	args;
					
					// Light
					const idDict* lightDef = gameLocal.FindEntityDefDict( "envirosuit_light", false );
					if( lightDef )
					{
						idEntity* temp;
						gameLocal.SpawnEntityDef( *lightDef, &temp, false );
						
						idLight* eLight = static_cast<idLight*>( temp );
						eLight->GetPhysics()->SetOrigin( firstPersonViewOrigin );
						eLight->UpdateVisuals();
						eLight->Present();
						
						enviroSuitLight = eLight;
					}
				}
				break;
			}
			case ENVIROTIME:
			{
				if( giveFlags & ITEM_GIVE_UPDATE_STATE )
				{
					hudPowerup = ENVIROTIME;
					// The HUD display bar is fixed at 60 seconds
					hudPowerupDuration = 60000;
				}
				break;
			}
			case INVULNERABILITY:
			{
				if( common->IsMultiplayer() && ( giveFlags & ITEM_GIVE_FEEDBACK ) )
				{
					inventory.AddPickupName( "#str_00100630", this );
				}
				break;
			}
		}
		
		if( giveFlags & ITEM_GIVE_UPDATE_STATE )
		{
			UpdateSkinSetup();
		}
		
		return true;
	}
	else
	{
		gameLocal.Warning( "Player given power up %i\n which is out of range", powerup );
	}
	return false;
}

/*
==============
idPlayer::ClearPowerup
==============
*/
void idPlayer::ClearPowerup( int i )
{

	if( common->IsServer() )
	{
		idBitMsg	msg;
		byte		msgBuf[MAX_EVENT_PARAM_SIZE];
		
		msg.InitWrite( msgBuf, sizeof( msgBuf ) );
		msg.WriteShort( i );
		msg.WriteShort( 0 );
		ServerSendEvent( EVENT_POWERUP, &msg, false );
	}
	
	powerUpSkin = NULL;
	inventory.powerups &= ~( 1 << i );
	inventory.powerupEndTime[ i ] = 0;
	switch( i )
	{
		case BERSERK:
		{
			if( common->IsMultiplayer() )
			{
				StopSound( SND_CHANNEL_DEMONIC, false );
			}
			if( !common->IsMultiplayer() )
			{
				StopHealthRecharge();
			}
			break;
		}
		case INVISIBILITY:
		{
			for( int h = 0; h < 2; h++ )
			{
				if( hands[ h ].weapon.GetEntity() )
				{
					hands[ h ].weapon.GetEntity()->UpdateSkin();
				}
			}
			if( flashlight.GetEntity() )
			{
				flashlight.GetEntity()->UpdateSkin();
			}
			break;
		}
		case HELLTIME:
		{
			GetAchievementManager().ResetHellTimeKills();
			StopSound( SND_CHANNEL_DEMONIC, false );
			break;
		}
		case ENVIROSUIT:
		{
		
			hudPowerup = -1;
			
			// Turn off the envirosuit sound
			if( gameSoundWorld )
			{
				gameSoundWorld->SetEnviroSuit( false );
			}
			
			// Take off the helmet and lights
			if( enviroSuitLight.IsValid() )
			{
				enviroSuitLight.GetEntity()->PostEventMS( &EV_Remove, 0 );
			}
			enviroSuitLight = NULL;
			break;
		}
		case INVULNERABILITY:
		{
			if( common->IsMultiplayer() )
			{
				StopSound( SND_CHANNEL_DEMONIC, false );
			}
		}
			/*case HASTE: {
				if(common->IsMultiplayer()) {
					StopSound( SND_CHANNEL_DEMONIC, false );
				}
			}*/
	}
}

/*
==============
idPlayer::UpdatePowerUps
==============
*/
void idPlayer::UpdatePowerUps()
{
	int i;
	
	if( !common->IsClient() )
	{
		for( i = 0; i < MAX_POWERUPS; i++ )
		{
			if( ( inventory.powerups & ( 1 << i ) ) && inventory.powerupEndTime[i] > gameLocal.time )
			{
				switch( i )
				{
					case ENVIROSUIT:
					{
						if( enviroSuitLight.IsValid() )
						{
							idAngles lightAng = firstPersonViewAxis.ToAngles();
							idVec3 lightOrg = firstPersonViewOrigin;
							const idDict* lightDef = gameLocal.FindEntityDefDict( "envirosuit_light", false );
							
							idVec3 enviroOffset = lightDef->GetVector( "enviro_offset" );
							idVec3 enviroAngleOffset = lightDef->GetVector( "enviro_angle_offset" );
							
							lightOrg += ( enviroOffset.x * firstPersonViewAxis[0] );
							lightOrg += ( enviroOffset.y * firstPersonViewAxis[1] );
							lightOrg += ( enviroOffset.z * firstPersonViewAxis[2] );
							lightAng.pitch += enviroAngleOffset.x;
							lightAng.yaw += enviroAngleOffset.y;
							lightAng.roll += enviroAngleOffset.z;
							
							enviroSuitLight.GetEntity()->GetPhysics()->SetOrigin( lightOrg );
							enviroSuitLight.GetEntity()->GetPhysics()->SetAxis( lightAng.ToMat3() );
							enviroSuitLight.GetEntity()->UpdateVisuals();
							enviroSuitLight.GetEntity()->Present();
						}
						break;
					}
					default:
					{
						break;
					}
				}
			}
			if( PowerUpActive( i ) && inventory.powerupEndTime[i] <= gameLocal.time )
			{
				ClearPowerup( i );
			}
		}
	}
	
	if( health > 0 )
	{
		if( powerUpSkin )
		{
			renderEntity.customSkin = powerUpSkin;
		}
		else
		{
			renderEntity.customSkin = skin;
		}
	}
	
	if( healthPool && gameLocal.time > nextHealthPulse && !AI_DEAD && health > 0 )
	{
		assert( !common->IsClient() );	// healthPool never be set on client
		int amt = ( healthPool > 5.0f ) ? 5 : healthPool;
		health += amt;
		if( health > inventory.maxHealth )
		{
			health = inventory.maxHealth;
			healthPool = 0;
		}
		else
		{
			healthPool -= amt;
		}
		if( healthPool < 1.0f )
		{
			healthPool = 0.0f;
		}
		else
		{
			nextHealthPulse = gameLocal.time + HEALTHPULSE_TIME;
			healthPulse = true;
		}
	}
	if( !gameLocal.inCinematic && !Flicksync_InCutscene && influenceActive == 0 && g_skill.GetInteger() == 3 && gameLocal.time > nextHealthTake && !AI_DEAD && health > g_healthTakeLimit.GetInteger() )
	{
		assert( !common->IsClient() );	// healthPool never be set on client
		
		if( !PowerUpActive( INVULNERABILITY ) )
		{
			health -= g_healthTakeAmt.GetInteger();
			if( health < g_healthTakeLimit.GetInteger() )
			{
				health = g_healthTakeLimit.GetInteger();
			}
		}
		nextHealthTake = gameLocal.time + g_healthTakeTime.GetInteger() * 1000;
		healthTake = true;
	}
}

/*
===============
idPlayer::ClearPowerUps
===============
*/
void idPlayer::ClearPowerUps()
{
	int i;
	for( i = 0; i < MAX_POWERUPS; i++ )
	{
		if( PowerUpActive( i ) )
		{
			ClearPowerup( i );
		}
	}
	inventory.ClearPowerUps();
	
	if( common->IsMultiplayer() )
	{
		if( enviroSuitLight.IsValid() )
		{
			enviroSuitLight.GetEntity()->PostEventMS( &EV_Remove, 0 );
		}
	}
}

/*
===============
idPlayer::GiveInventoryItem
===============
*/
bool idPlayer::GiveInventoryItem( idDict* item, unsigned int giveFlags )
{
	if( common->IsMultiplayer() && spectating )
	{
		return false;
	}
	
	if( giveFlags & ITEM_GIVE_UPDATE_STATE )
	{
		inventory.items.Append( new( TAG_ENTITY ) idDict( *item ) );
	}
	
	const char* itemName = item->GetString( "inv_name" );
	
	if( giveFlags & ITEM_GIVE_FEEDBACK )
	{
		if( idStr::Cmpn( itemName, STRTABLE_ID, STRTABLE_ID_LENGTH ) == 0 )
		{
			inventory.pickupItemNames.Append( idLocalization::GetString( itemName ) );
		}
		else
		{
			inventory.pickupItemNames.Append( itemName );
		}
		
		const char* icon = item->GetString( "inv_icon" );
		if( hud != NULL )
		{
			hud->ShowNewItem( itemName, icon );
		}
	}
	
	// D3XP added to support powercells
	if( ( giveFlags & ITEM_GIVE_UPDATE_STATE ) && item->GetInt( "inv_powercell" ) && focusUI )
	{
		//Reset the powercell count
		int powerCellCount = 0;
		for( int j = 0; j < inventory.items.Num(); j++ )
		{
			idDict* item = inventory.items[ j ];
			if( item->GetInt( "inv_powercell" ) )
			{
				powerCellCount++;
			}
		}
		focusUI->SetStateInt( "powercell_count", powerCellCount );
	}
	
	return true;
}

/*
==============
idPlayer::GiveInventoryItem
==============
*/
bool idPlayer::GiveInventoryItem( const char* name )
{
	idDict args;
	
	args.Set( "classname", name );
	args.Set( "owner", this->name.c_str() );
	gameLocal.SpawnEntityDef( args );
	return true;
}

/*
===============
idPlayer::GiveObjective
===============
*/
void idPlayer::GiveObjective( const char* title, const char* text, const idMaterial* screenshot )
{
	idObjectiveInfo& info = inventory.objectiveNames.Alloc();
	info.title = title;
	info.text = text;
	info.screenshot = screenshot;
	
	StartSound( "snd_objectiveup", SND_CHANNEL_ANY, 0, false, NULL );
	
	if( hud )
	{
		hud->SetupObjective( title, text, screenshot );
		hud->ShowObjective( false );
		objectiveUp = true;
	}
}

/*
===============
idPlayer::CompleteObjective
===============
*/
void idPlayer::CompleteObjective( const char* title )
{
	int c = inventory.objectiveNames.Num();
	for( int i = 0;  i < c; i++ )
	{
		if( idStr::Icmp( inventory.objectiveNames[i].title, title ) == 0 )
		{
			inventory.objectiveNames.RemoveIndex( i );
			break;
		}
	}
	
	StartSound( "snd_objectiveup", SND_CHANNEL_ANY, 0, false, NULL );
	
	if( hud )
	{
		hud->SetupObjectiveComplete( title );
		hud->ShowObjective( true );
	}
}

/*
===============
idPlayer::GiveVideo
===============
*/
void idPlayer::GiveVideo( const idDeclVideo* video, const char* itemName )
{

	if( video == NULL )
	{
		return;
	}
	
	int oldNumVideos = inventory.videos.Num();
	inventory.videos.AddUnique( video );
	
	if( oldNumVideos < inventory.videos.Num() )
	{
		GetAchievementManager().EventCompletesAchievement( ACHIEVEMENT_WATCH_ALL_VIDEOS );
	}
	
	if( itemName != NULL && itemName[0] != 0 )
	{
		inventory.pickupItemNames.Append( itemName );
	}
	
	if( hud )
	{
		hud->DownloadVideo();
	}
}

/*
===============
idPlayer::GiveSecurity
===============
*/
void idPlayer::GiveSecurity( const char* security )
{
	GetPDA()->SetSecurity( security );
	
	if( hud )
	{
		hud->UpdatedSecurity();
	}
}

/*
===============
idPlayer::GiveEmail
===============
*/
void idPlayer::GiveEmail( const idDeclEmail* email )
{
	if( email == NULL )
	{
		return;
	}
	
	inventory.emails.AddUnique( email );
	GetPDA()->AddEmail( email );
	
	// TODO_SPARTY: hook up new email notification in new hud
	//if ( hud ) {
	//	hud->HandleNamedEvent( "emailPickup" );
	//}
}

/*
===============
idPlayer::GivePDA
===============
*/
void idPlayer::GivePDA( const idDeclPDA* pda, const char* securityItem, bool toggle )
{
	if( common->IsMultiplayer() && spectating )
	{
		return;
	}
	
	if( securityItem != NULL && securityItem[0] != 0 )
	{
		inventory.pdaSecurity.AddUnique( securityItem );
	}
	
	// Just to make sure they want the default player spawn defined pda.
	// Some what of a hack, so i dont have to change any map scripts that initially give
	// the player "personal" pda.
	if( pda == NULL || idStr::Icmp( pda->GetName(), "personal" ) == 0 )
	{
		pda = static_cast<const idDeclPDA*>( declManager->FindType( DECL_PDA, spawnArgs.GetString( "pda_name", "personal" ) ) );
	}
	if( pda == NULL )
	{
		return;
	}
	
	int oldNumPDAs = inventory.pdas.Num();
	inventory.pdas.AddUnique( pda );
	int newNumPDAs = inventory.pdas.Num();
	
	// Set the stat for # of PDAs...
	// Only increment the PDA stat if we've added a new one....
	if( oldNumPDAs < newNumPDAs )
	{
		switch( GetExpansionType() )
		{
			case GAME_BASE:
				GetAchievementManager().EventCompletesAchievement( ACHIEVEMENT_PDAS_BASE );
				break;
			case GAME_D3XP:
				GetAchievementManager().EventCompletesAchievement( ACHIEVEMENT_PDAS_ROE );
				break;
			case GAME_D3LE:
				GetAchievementManager().EventCompletesAchievement( ACHIEVEMENT_PDAS_LE );
				break;
		}
	}
	
	// Copy any videos over
	for( int i = 0; i < pda->GetNumVideos(); i++ )
	{
		const idDeclVideo* video = pda->GetVideoByIndex( i );
		if( video != NULL )
		{
			inventory.videos.AddUnique( video );
		}
	}
	
	// This is kind of a hack, but it works nicely
	// We don't want to display the 'you got a new pda' message during a map load
	// or if we're about to skip to another cutscene
	if ( gameLocal.GetFrameNum() > 10 && vr_cutscenesOnly.GetInteger() != 1 && Flicksync_skipToCutscene == CUTSCENE_NONE ) 
	{
				
		const char* sec = pda->GetSecurity();
		if ( hud && inventory.pdas.Num() != 1 ) // Koz dont show the pda loading indicator for the initial PDA.
		{
			hud->DownloadPDA( pda, ( sec != NULL && sec[0] != 0 ) ? true : false );
		}
		if( inventory.pdas.Num() == 1 )
		{
			GetPDA()->RemoveAddedEmailsAndVideos();
			if( !objectiveSystemOpen )
			{
				if ( toggle ) // Koz: toggle pda renders a fullscreen PDA in normal play, for VR we need to select the pda 'weapon'.
				{
					if ( !game->isVR )
					{
						common->Printf( "idPlayer::GivePDA calling TogglePDA\n" );
						TogglePDA( 1 - vr_weaponHand.GetInteger() );
					}
					else
					{
						common->Printf( "idPlayer::GivePDA calling Select Weapon for PDA\n" );
						SetupPDASlot( false ); // show flashlight in PDA holster
						SetupHolsterSlot( vr_weaponHand.GetInteger(), false ); // stash weapon hand's gun in weapon holster
						hands[ 1 - vr_weaponHand.GetInteger() ].SelectWeapon( weapon_pda, true, false );
					}
				}
			}
			//ShowTip( spawnArgs.GetString( "text_infoTitle" ), spawnArgs.GetString( "text_firstPDA" ), true );
		}
	}
}

/*
===============
idPlayer::FindInventoryItem
===============
*/
idDict* idPlayer::FindInventoryItem( const char* name )
{
	for( int i = 0; i < inventory.items.Num(); i++ )
	{
		const char* iname = inventory.items[i]->GetString( "inv_name" );
		if( iname != NULL && *iname != '\0' )
		{
			if( idStr::Icmp( name, iname ) == 0 )
			{
				return inventory.items[i];
			}
		}
	}
	return NULL;
}

/*
===============
idPlayer::FindInventoryItem
===============
*/
idDict* idPlayer::FindInventoryItem( int index )
{
	if( index <= inventory.items.Num() )
	{
		return inventory.items[ index ];
	}
	return NULL;
}

/*
===============
idPlayer::GetNumInventoryItems
===============
*/
int idPlayer::GetNumInventoryItems()
{
	return inventory.items.Num();
}

/*
===============
idPlayer::RemoveInventoryItem
===============
*/
void idPlayer::RemoveInventoryItem( const char* name )
{
	//Hack for localization
	if( !idStr::Icmp( name, "Pwr Cell" ) )
	{
		name = idLocalization::GetString( "#str_00101056" );
	}
	idDict* item = FindInventoryItem( name );
	if( item )
	{
		RemoveInventoryItem( item );
	}
}

/*
===============
idPlayer::RemoveInventoryItem
===============
*/
void idPlayer::RemoveInventoryItem( idDict* item )
{
	inventory.items.Remove( item );
	
	if( item->GetInt( "inv_powercell" ) && focusUI )
	{
		//Reset the powercell count
		int powerCellCount = 0;
		for( int j = 0; j < inventory.items.Num(); j++ )
		{
			idDict* item = inventory.items[ j ];
			if( item->GetInt( "inv_powercell" ) )
			{
				powerCellCount++;
			}
		}
		focusUI->SetStateInt( "powercell_count", powerCellCount );
	}
	
	delete item;
}

/*
===============
idPlayer::GiveItem
===============
*/
void idPlayer::GiveItem( const char* itemname )
{
	idDict args;
	
	args.Set( "classname", itemname );
	args.Set( "owner", name.c_str() );
	gameLocal.SpawnEntityDef( args );
}

bool idPlayer::OtherHandImpulseSlot()
{
	if( !commonVr->hasHMD )
	{
		return false;
	}
	int hand = 1 - vr_weaponHand.GetInteger();
	slotIndex_t otherHandSlot = hands[ hand ].handSlot;
	if( otherHandSlot == SLOT_PDA_HIP )
	{
		if( !common->IsMultiplayer() )
		{
			// we don't have a PDA, so toggle the menu instead
			if ( commonVr->PDAforced || inventory.pdas.Num() == 0 )
			{
				PerformImpulse( 40 );
			}
			else if( objectiveSystemOpen ) // if we're holding our PDA and we try to grab the PDA slot
			{
				// our hand is always full in this case, but that's only an issue we care and the holster contains the flashlight
				if( vr_mustEmptyHands.GetBool() && commonVr->currentFlashlightMode == FLASHLIGHT_HAND )
					return false;
				TogglePDA( hand );
			}
			else if( weapon_pda >= 0 ) // if the PDA is in the slot and we try to grab it
			{
				if( hands[ hand ].tooFullToInteract() )
					return false;
				SetupPDASlot( false );
				SetupHolsterSlot( 1 - hand, false );
				hands[ hand ].SelectWeapon( weapon_pda, true, false );
			}
		}
		return true;
	}
	if( otherHandSlot == SLOT_FLASHLIGHT_HEAD && !commonVr->PDAforced && !objectiveSystemOpen
		&& flashlight.IsValid() && !spectating && weaponEnabled && !hiddenWeapon && !gameLocal.world->spawnArgs.GetBool("no_Weapons") && !vr_flashlightStrict.GetBool() )
	{
		// swap flashlight between head and hand
		if ( commonVr->currentFlashlightPosition == FLASHLIGHT_HEAD && HasHoldableFlashlight() )
		{
			// Carl: move weapon from hand to inventory
			hands[hand].idealWeapon = weapon_fists;
			// move flashlight from head to hand
			vr_flashlightMode.SetInteger(FLASHLIGHT_HAND);
			vr_flashlightMode.SetModified();
		}
		else if ( commonVr->currentFlashlightPosition == FLASHLIGHT_HAND )
		{
			vr_flashlightMode.SetInteger(FLASHLIGHT_HEAD);
			vr_flashlightMode.SetModified();
		}
		return true;
	}
	if( otherHandSlot == SLOT_FLASHLIGHT_SHOULDER && !commonVr->PDAforced && !objectiveSystemOpen
		&& flashlight.IsValid() && !spectating && weaponEnabled && !hiddenWeapon && !gameLocal.world->spawnArgs.GetBool("no_Weapons") && !vr_flashlightStrict.GetBool() )
	{
		// swap flashlight between body and hand
		if ( commonVr->currentFlashlightPosition == FLASHLIGHT_BODY && HasHoldableFlashlight() )
		{
			// Carl: move weapon from hand to inventory
			hands[hand].idealWeapon = weapon_fists;
			// move flashlight from shoulder to hand
			vr_flashlightMode.SetInteger(FLASHLIGHT_HAND);
			vr_flashlightMode.SetModified();
		}
		else if ( commonVr->currentFlashlightPosition == FLASHLIGHT_HAND )
		{
			vr_flashlightMode.SetInteger(FLASHLIGHT_BODY);
			vr_flashlightMode.SetModified();
		}
		return true;
	}
	if (otherHandSlot == SLOT_WEAPON_HIP)
	{
		// Holster the PDA we are holding on the other side
		if( commonVr->PDAforced )
		{
			SwapWeaponHand();
			PerformImpulse( 40 );
		}
		else if( !common->IsMultiplayer() && objectiveSystemOpen )
		{
			SwapWeaponHand();
			if (hands[hand].previousWeapon == weapon_fists)
				hands[hand].previousWeapon = holsteredWeapon;
			TogglePDA( hand );
		}
		else
		{
			int h = 1 - vr_weaponHand.GetInteger();
			if( hands[ h ].tooFullToInteract() && holsteredWeapon != weapon_fists )
				return false;
			SwapWeaponHand();
			// pick up whatever weapon we have holstered, and magically holster our current weapon
			SetupHolsterSlot( vr_weaponHand.GetInteger() );
		}
		return true;
	}
	if ( otherHandSlot == SLOT_WEAPON_BACK_BOTTOM )
	{
		SwapWeaponHand();
		// Holster the PDA we are holding on the other side
		if (!common->IsMultiplayer())
		{
			// we don't have a PDA, so toggle the menu instead
			if ( commonVr->PDAforced )
			{
				PerformImpulse( 40 );
			}
			else if( objectiveSystemOpen )
			{
				TogglePDA( hand );
			}
		}
		hands[ hand ].PrevWeapon();
		return true;
	}
	if ( otherHandSlot == SLOT_WEAPON_BACK_TOP )
	{
		// SwapWeaponHand();
		// If we are holding a PDA, put it away.
		if (!common->IsMultiplayer())
		{
			// if it's the pause menu "PDA", toggle the menu instead
			if ( commonVr->PDAforced )
			{
				PerformImpulse(40);
			}
			else if ( objectiveSystemOpen )
			{
				TogglePDA( hand );
			}
		}
		hands[ hand ].NextWeapon();
		return true;
	}
	return false;
}

bool idPlayer::WeaponHandImpulseSlot()
{
	if( !commonVr->hasHMD )
	{
		return false;
	}
	int hand = vr_weaponHand.GetInteger();
	slotIndex_t weaponHandSlot = hands[ hand ].handSlot;
	if( weaponHandSlot == SLOT_WEAPON_HIP )
	{
		if ( objectiveSystemOpen ) // for now this means our weapon hand was empty
		{
			if ( hands[ hand ].previousWeapon == weapon_fists )
			{
				hands[ hand ].previousWeapon = holsteredWeapon;
			}
			TogglePDA( hand );
		}
		else
		{
			// if our hand is too full, we can't pick up the weapon
			if( hands[ hand ].tooFullToInteract() && holsteredWeapon != weapon_fists )
				return false;
			SetupHolsterSlot( hand );
		}
		return true;
	}
	if( weaponHandSlot == SLOT_WEAPON_BACK_BOTTOM )
	{
		if ( objectiveSystemOpen )
		{
			TogglePDA( hand );
		}
		hands[ hand ].PrevWeapon();
		return true;
	}
	if( weaponHandSlot == SLOT_WEAPON_BACK_TOP )
	{
		if ( objectiveSystemOpen )
		{
			TogglePDA( hand );
		}
		hands[ hand ].NextWeapon();
		return true;
	}
	if ( weaponHandSlot == SLOT_PDA_HIP )
	{
		// if we're holding a gun ( not a pointer finger or fist ) then holster the gun
		//if ( !commonVr->PDAforced && !objectiveSystemOpen && currentWeapon != weapon_fists )
		//	SetupHolsterSlot( hand );
		// pick up PDA in our weapon hand, or pick up the torch if our hand is a pointer finger
		if ( !common->IsMultiplayer() )
		{
			// we don't have a PDA, so toggle the menu instead
			if ( commonVr->PDAforced || inventory.pdas.Num() == 0 )
			{
				if( !commonVr->PDAforced && hands[ vr_weaponHand.GetInteger() ].tooFullToInteract() )
					return false;
				SwapWeaponHand();
				PerformImpulse( 40 );
			}
			else if( objectiveSystemOpen )
			{
				SwapWeaponHand();
				TogglePDA( hand );
			}
			else if( weapon_pda >= 0 )
			{
				if( hands[ hand ].tooFullToInteract() )
					return false;
				SwapWeaponHand();
				SetupPDASlot( false );
				SetupHolsterSlot( 1 - hand, false );
				hands[ hand ].SelectWeapon( weapon_pda, true, false );
			}
		}
		return true;
	}
	if( weaponHandSlot == SLOT_FLASHLIGHT_HEAD && !commonVr->PDAforced && !objectiveSystemOpen
		&& flashlight.IsValid() && !spectating && weaponEnabled && !hiddenWeapon && !gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) && !vr_flashlightStrict.GetBool() )
	{
		// if the flashlight is mounted on our head, and we're unambiguously trying to grab it with our weapon hand
		if( commonVr->currentFlashlightPosition == FLASHLIGHT_HEAD && (hands[ hand ].currentWeapon == weapon_fists || vr_gripMode.GetInteger() == VR_GRIP_TOGGLE_WITH_DROP ) && HasHoldableFlashlight() )
		{
			// move weapon from hand to inventory
			hands[hand].idealWeapon = weapon_fists;
			SwapWeaponHand();
			// swap flashlight between head and hand
			vr_flashlightMode.SetInteger( FLASHLIGHT_HAND );
			vr_flashlightMode.SetModified();
			return true;
		}
		// if we're unambiguously trying to grab it, but there's nothing there, process and ignore the grip
		else if( vr_gripMode.GetInteger() == VR_GRIP_TOGGLE_WITH_DROP )
		{
			return true;
		}
	}
	if( weaponHandSlot == SLOT_FLASHLIGHT_SHOULDER && !commonVr->PDAforced && !objectiveSystemOpen
		&& flashlight.IsValid() && !spectating && weaponEnabled && !hiddenWeapon && !gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) && !vr_flashlightStrict.GetBool() )
	{
		// if the flashlight is mounted on our shoulder, and we're unambiguously trying to grab it with our weapon hand
		if( commonVr->currentFlashlightPosition == FLASHLIGHT_BODY && ( hands[ hand ].currentWeapon == weapon_fists || vr_gripMode.GetInteger() == VR_GRIP_TOGGLE_WITH_DROP ) && HasHoldableFlashlight() )
		{
			// move weapon from hand to inventory
			hands[hand].idealWeapon = weapon_fists;
			// swap flashlight from shoulder to hand
			SwapWeaponHand();
			vr_flashlightMode.SetInteger( FLASHLIGHT_HAND );
			vr_flashlightMode.SetModified();
			return true;
		}
		// if we're unambiguously trying to grab it, but there's nothing there, process and ignore the grip
		else if( vr_gripMode.GetInteger() == VR_GRIP_TOGGLE_WITH_DROP )
		{
			return true;
		}
	}
	return false;
}

// Carl: Context sensitive VR grabs, and dual wielding
// 0 = right hand, 1 = left hand; true if pressed, false if released; returns true if handled as grab
// WARNING! Called from the input thread?
bool idPlayer::GrabWorld( int hand, bool pressed )
{
	bool b;
	if( !pressed )
	{
		b = hands[ hand ].grabbingWorld;
		hands[ hand ].grabbingWorld = false;
		// We're releasing something
		if( vr_gripMode.GetInteger() == VR_GRIP_HOLD )
		{
			hands[ hand ].releaseVirtualGrab();
			// releasing outside a holster will drop the weapon
			// releasing inside an empty holster should place the weapon in the holster
			// releasing inside a full holster, when vr_mustEmptyHands is true, should drop the weapon
			// releasing inside a full holster, when vr_mustEmptyHands is false, should put the holster in an intermediate state
			// where it contains two weapons, your hand contains nothing, and you are waiting to pick up the other weapon
			// if you then grab in that state, it should pick up the original weapon that was in the holster and show the gun you just put there
			// if you move your hand away from the holster in that state, we could do one of three things (make it a cvar?)
			//   1. move the original weapon into the inventory (accessed via your back) and show the new weapon in the holster
			//   2. keep the original weapon in the holster and drop the weapon you tried to stash on the floor
			//   3. move the new weapon into the inventory and keep the original weapon in the holster
			// releasing inside a back holster should place the weapon in your inventory
			// but should not change the next/previous weapon until your hand leaves the holster
		}
		else if( vr_gripMode.GetInteger() == VR_GRIP_DEAD_AND_BURIED )
		{
			hands[ hand ].releaseVirtualGrab();
			// releasing outside a holster will return the weapon to the holster it came from
			// (if it didn't come from a holster, move it to the hip holster on the same side as the hand)
			// (anything in that holster always gets moved to inventory)
			// releasing inside an empty holster should place the weapon in the holster
			// releasing inside a full holster, when vr_mustEmptyHands is true, should return the weapon to the holster it came from
			// releasing inside a full holster, when vr_mustEmptyHands is false, should put the holster in an intermediate state
			// (option 2. above would seem weird in this case, but we should probably still allow it)
		}

		return b;
	}
	if ( hand == vr_weaponHand.GetInteger() )
		b = WeaponHandImpulseSlot();
	else
		b = OtherHandImpulseSlot();
	hands[hand].grabbingWorld = b;
	if( vr_gripMode.GetInteger() == VR_GRIP_TOGGLE_WITH_DROP )
	{
		if( !b ) // if we didn't grab a holster
		{
			if( hands[ hand ].holdingSomethingDroppable() )
			{
				b = hands[ hand ].releaseVirtualGrab();
			}
		}
		return b;
	}
	return b;
}

// Carl: Context sensitive VR trigger buttons, and dual wielding
// 0 = right hand, 1 = left hand; true if pressed, false if released; returns true if handled as trigger pull
// WARNING! Called from the input thread?
bool idPlayer::TriggerClickWorld( int hand, bool pressed )
{
	bool b;
	if( !pressed )
	{
		b = hands[ hand ].triggerDown;
		hands[ hand ].triggerDown = false;
		//common->Printf( " Releasing Trigger\n" );
		return b;
	}
	// if we're holding or floating a weapon, then the trigger button activates the trigger of that weapon
	b = hands[ hand ].controllingWeapon();
	// if we're over a flashlight then the trigger button activates the button of that flashlight
	b = b || ( hands[ hand ].isOverMountedFlashlight() && !hands[ hand ].tooFullToInteract() );
	hands[ hand ].triggerDown = b;
	//common->Printf( " Pressing Trigger = %d\n", b );
	return b;
}

// Carl: Context sensitive VR thumb clicks, and dual wielding
// 0 = right hand, 1 = left hand; true if pressed, false if released; returns true if handled as thumb click
// WARNING! Called from the input thread?
bool idPlayer::ThumbClickWorld( int hand, bool pressed )
{
	bool b;
	if( !pressed )
	{
		b = hands[ hand ].thumbDown;
		hands[ hand ].thumbDown = false;
		return b;
	}
	// if we're holding or over a flashlight, then the thumb button activates the button of that flashlight
	b = hands[ hand ].holdingFlashlight();
	b = b || ( hands[ hand ].isOverMountedFlashlight() && !hands[ hand ].tooFullToInteract() );
	hands[ hand ].thumbDown = b;
	return b;
}


/*
==================
idPlayer::SlotForWeapon
==================
*/
int idPlayer::SlotForWeapon( const char* weaponName )
{
	int i;
	
	for( i = 0; i < MAX_WEAPONS; i++ )
	{
		const char* weap = spawnArgs.GetString( va( "def_weapon%d", i ) );
		if( !idStr::Cmp( weap, weaponName ) )
		{
			return i;
		}
	}
	
	// not found
	return -1;
}

/*
===============
idPlayer::Reload
===============
*/
void idPlayer::Reload()
{
	if( spectating || gameLocal.inCinematic || Flicksync_InCutscene || influenceActive || commonVr->handInGui ) // Koz don't reload when in gui
	{
		return;
	}
	
	if( common->IsClient() && !IsLocallyControlled() )
	{
		return;
	}
	// Carl: Dual wielding, just reload both weapons
	for( int h = 0; h < 2; h++ )
	{
		idWeapon* weapon = GetWeaponInHand( h );
		if( weapon && weapon->IsLinked() && weapon->hideOffset == 0.0f ) // Koz don't reload when in gui
		{
			weapon->Reload();
		}
	}
}

/*
===============
idPlayer::NextBestWeapon
===============
*/
void idPlayer::NextBestWeapon()
{
	hands[ vr_weaponHand.GetInteger() ].NextBestWeapon();
}

/*
===============
idPlayer::NextWeapon
===============
*/
void idPlayer::NextWeapon()
{
	hands[ vr_weaponHand.GetInteger() ].NextWeapon();
}

/*
===============
idPlayer::PrevWeapon
===============
*/
void idPlayer::PrevWeapon()
{
	hands[ vr_weaponHand.GetInteger() ].PrevWeapon();
}

/*
===============
idPlayerHand::SelectWeapon
===============
*/
void idPlayerHand::SelectWeapon( int num, bool force, bool specific )
{
	if( vr_debugHands.GetBool() )
	{
		common->Printf( "Before SelectWeapon(%d, %d, %d):\n", num, force, specific );
		debugPrint();
	}
	const char* weap;
	
	if( !owner->weaponEnabled || owner->spectating || gameLocal.inCinematic || Flicksync_InCutscene || owner->health < 0 || commonVr->handInGui ) // Koz don't let the player change weapons if hand is currently in a gui
	{
		return;
	}

	if( !handExists() )
		return;

	if( num == weapon_empty_hand )
	{
		idealWeapon = owner->weapon_fists;
		// Carl: TODO
		return;
	}

	if( ( num < 0 ) || ( num >= MAX_WEAPONS ) )
	{
		return;
	}
	
	if( ( num != owner->weapon_pda ) && gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) )
	{
		num = owner->weapon_fists;
		owner->hiddenWeapon ^= 1;
		if( owner->hiddenWeapon && (!game->isVR || commonVr->handInGui == true) )
		{
			if( weapon.GetEntity() )
				weapon.GetEntity()->LowerWeapon(); // Koz
		}
		else
		{
			if( weapon.GetEntity() )
				weapon.GetEntity()->RaiseWeapon();
		}
	}

	// Carl: Dual wielding, handle switching to and from flashlight properly
	if( num == owner->weapon_flashlight || num == owner->weapon_flashlight_new )
	{
		// if we don't have a holdable flashlight, do nothing
		if( !owner->HasHoldableFlashlight() )
			return;
		// if we can't dual wield a flashlight...
		if( !owner->CanDualWield( num ) && !owner->CanDualWield( owner->hands[1 - whichHand].currentWeapon ) && !owner->CanDualWield( owner->hands[1 - whichHand].idealWeapon.Get() ) )
		{
			// if we can't automagically empty our other hand, do nothing
			if( vr_mustEmptyHands.GetBool() )
				return;
			// otherwise, first automagically empty our other hand
			if( owner->CanDualWield( owner->weapon_fists ) )
				owner->hands[1 - whichHand].SelectWeapon( owner->weapon_fists, true, true );
			else
				owner->hands[1 - whichHand].SelectWeapon( weapon_empty_hand, true, true );
		}
		
		idealWeapon = owner->weapon_fists;
		commonVr->currentFlashlightMode = FLASHLIGHT_HAND;
		vr_weaponHand.SetInteger( 1 - whichHand );
		return;
	}
	else if( holdingFlashlight() )
	{
		if( vr_flashlightStrict.GetBool() )
			commonVr->currentFlashlightMode = FLASHLIGHT_INVENTORY;
		else
			commonVr->currentFlashlightMode = FLASHLIGHT_HEAD;
		commonVr->currentFlashlightPosition = commonVr->currentFlashlightMode;
	}

	//Is the weapon a toggle weapon (eg, fists/chainsaw/grabber, or shotgun/super-shotgun, or artifact/soulcube)
	WeaponToggle_t* weaponToggle;
	if( !specific && owner->weaponToggles.Get( va( "weapontoggle%d", num ), &weaponToggle ) )
	{
	
		int weaponToggleIndex = 0;
		
		//Find the current Weapon in the list
		int currentIndex = -1;
		for( int i = 0; i < weaponToggle->toggleList.Num(); i++ )
		{
			if( weaponToggle->toggleList[i] == idealWeapon )
			{
				currentIndex = i;
				break;
			}
		}
		if( currentIndex == -1 )
		{
			//Didn't find the current weapon so select the first item
			weaponToggleIndex = weaponToggle->lastUsed;
		}
		else
		{
			//Roll to the next available item in the list
			weaponToggleIndex = currentIndex;
			weaponToggleIndex++;
			if( weaponToggleIndex >= weaponToggle->toggleList.Num() )
			{
				weaponToggleIndex = 0;
			}
		}
		
		for( int i = 0; i < weaponToggle->toggleList.Num(); i++ )
		{
			int weapNum = weaponToggle->toggleList[weaponToggleIndex];
			// Carl: dual wielding
			int availableWeaponsOfThisType = 0;
			if( weapNum == owner->weapon_fists )
			{
				if( owner->inventory.weapons & ( 1 << weapNum ) )
					availableWeaponsOfThisType++;
			}
			else
			{
				if( owner->inventory.weapons & ( 1 << weapNum ) )
					availableWeaponsOfThisType++;
				if( owner->inventory.duplicateWeapons & ( 1 << weapNum ) )
					availableWeaponsOfThisType++;
				// Carl: skip weapons in the holster unless we have a duplicate, TODO make it optional
				if( weapNum == owner->holsteredWeapon )
					availableWeaponsOfThisType--;
				// Carl: skip weapons in the other hand unless we have a duplicate (dual wielding)
				if( weapNum == owner->hands[ 1 - whichHand ].idealWeapon.Get() )
					availableWeaponsOfThisType--;
			}

			//Is it available
			if( availableWeaponsOfThisType > 0 )
			{
				//Do we have ammo for it
				if( owner->inventory.HasAmmo( owner->spawnArgs.GetString( va( "def_weapon%d", weapNum ) ), true, owner ) || owner->spawnArgs.GetBool( va( "weapon%d_allowempty", weapNum ) ) )
				{
					break;
				}
			}
			
			weaponToggleIndex++;
			if( weaponToggleIndex >= weaponToggle->toggleList.Num() )
			{
				weaponToggleIndex = 0;
			}
		}
		weaponToggle->lastUsed = weaponToggleIndex;
		num = weaponToggle->toggleList[weaponToggleIndex];
	}
	
	// Is there an actual weapon for this weapon slot?
	weap = owner->spawnArgs.GetString( va( "def_weapon%d", num ) );
	if( !weap[ 0 ] )
	{
		gameLocal.Printf( "Invalid weapon\n" );
		return;
	}
	
	// Carl: dual wielding
	int availableWeaponsOfThisType = 0;
	if( num == owner->weapon_fists )
	{
		if( owner->inventory.weapons & ( 1 << num ) )
			availableWeaponsOfThisType++;
	}
	else
	{
		if( owner->inventory.weapons & ( 1 << num ) )
			availableWeaponsOfThisType++;
		if( owner->inventory.duplicateWeapons & ( 1 << num ) )
			availableWeaponsOfThisType++;
		// Carl: skip weapons in the holster unless we have a duplicate, TODO make it optional
		if( num == owner->holsteredWeapon )
			availableWeaponsOfThisType--;
		// Carl: skip weapons in the other hand unless we have a duplicate (dual wielding)
		if( num == owner->hands[ 1 - whichHand ].idealWeapon.Get() )
			availableWeaponsOfThisType--;
	}

	if( force || availableWeaponsOfThisType > 0 )
	{
		if( !owner->inventory.HasAmmo( weap, true, owner ) && !owner->spawnArgs.GetBool( va( "weapon%d_allowempty", num ) ) )
		{
			return;
		}
		if( ( previousWeapon >= 0 ) && ( idealWeapon == num ) && ( owner->spawnArgs.GetBool( va( "weapon%d_toggle", num ) ) ) )
		{
			weap = owner->spawnArgs.GetString( va( "def_weapon%d", previousWeapon ) );
			if( !owner->inventory.HasAmmo( weap, true, owner ) && !owner->spawnArgs.GetBool( va( "weapon%d_allowempty", previousWeapon ) ) )
			{
				return;
			}
			num = previousWeapon;
		}
		else if( ( owner->weapon_pda >= 0 ) && ( num == owner->weapon_pda ) && ( owner->inventory.pdas.Num() == 0 ) )
		{
			
			if ( game->isVR )
			{
				//GivePDA( NULL, NULL, false ); // hack to allow the player to change system settings in the mars city level before the PDA is given by the receptionist.
				//idealWeapon = num;
			}
			else
			{
				owner->ShowTip( owner->spawnArgs.GetString( "text_infoTitle" ), owner->spawnArgs.GetString( "text_noPDA" ), true );
				return;
			}
		}
		
		// If we can't we dual-wield it?
		if( !owner->CanDualWield( num ) && !owner->CanDualWield( owner->hands[1 - whichHand].currentWeapon ) && !owner->CanDualWield( owner->hands[1 - whichHand].idealWeapon.Get() ) )
		{
			// if we can't automagically empty our other hand, do nothing
			if( vr_mustEmptyHands.GetBool() )
				return;
			// otherwise, first automagically empty our other hand
			if( owner->CanDualWield( owner->weapon_fists ) )
				owner->hands[1 - whichHand].SelectWeapon( owner->weapon_fists, true, true );
			else
				owner->hands[1 - whichHand].SelectWeapon( weapon_empty_hand, true, true );
		}

		// If we made it all this way, we want weapon "num" and we can switch to it
		idealWeapon = num;
		owner->UpdateHudWeapon( whichHand );
	}
	if( vr_debugHands.GetBool() )
	{
		common->Printf( "After SelectWeapon():\n" );
		debugPrint();
	}
}

/*
=================
idPlayer::DropWeapon
=================
*/
void idPlayer::DropWeapons( bool died )
{
	for( int h = 0; h < 2; h++ )
		hands[ h ].DropWeapon( died );
	// Carl: Note that the old drop weapon code here looked like this, so perhaps we should pass a parameter to change the throw speed and drop time?
	// item = weapon.GetEntity()->DropItem( 250.0f * forward + 150.0f * up, 500, WEAPON_DROP_TIME, died );
}

/*
=================
idPlayer::StealWeapon
steal the best weapon that the target player is holding in their hands
=================
*/
void idPlayer::StealWeapon( idPlayer* player )
{
	// Carl: TODO Dual Wielding
	assert( !common->IsClient() );
	
	// make sure there's something to steal
	int h = player->GetBestWeaponHandToSteal( this );
	idWeapon* player_weapon = player->GetWeaponInHand( h );
	if( !player_weapon || !player_weapon->CanDrop() || player->hands[ h ].weaponGone )
	{
		return;
	}
	// steal - we need to effectively force the other player to abandon his weapon
	int newweap = player->hands[ h ].currentWeapon;
	if( newweap == -1 )
	{
		return;
	}
	// might be just dropped - check inventory
	if( !( player->inventory.weapons & ( 1 << newweap ) ) )
	{
		return;
	}
	const char* weapon_classname = spawnArgs.GetString( va( "def_weapon%d", newweap ) );
	assert( weapon_classname );
	int ammoavailable = player_weapon->AmmoAvailable();
	int inclip = player_weapon->AmmoInClip();
	
	ammoavailable += inclip;
	
	if( ( ammoavailable != -1 ) && ( ammoavailable < 0 ) )
	{
		// see DropWeapon
		common->DPrintf( "idPlayer::StealWeapon: bad ammo setup\n" );
		// we still steal the weapon, so let's use the default ammo levels
		inclip = -1;
		const idDeclEntityDef* decl = gameLocal.FindEntityDef( weapon_classname );
		assert( decl );
		const idKeyValue* keypair = decl->dict.MatchPrefix( "inv_ammo_" );
		assert( keypair );
		ammoavailable = atoi( keypair->GetValue() );
	}
	
	player_weapon->WeaponStolen();
	player->inventory.Drop( player->spawnArgs, NULL, newweap );
	player->SelectWeapon( weapon_fists, false );
	// in case the robbed player is firing rounds with a continuous fire weapon like the chaingun/plasma etc.
	// this will ensure the firing actually stops
	player->hands[ h ].weaponGone = true;
	
	// give weapon, setup the ammo count
	int myhand = vr_weaponHand.GetInteger();
	Give( "weapon", weapon_classname, ITEM_GIVE_FEEDBACK | ITEM_GIVE_UPDATE_STATE, myhand );
	ammo_t ammo_i = player->inventory.AmmoIndexForWeaponClass( weapon_classname, NULL );
	hands[ myhand ].idealWeapon = newweap;
	const int currentAmmo = inventory.GetInventoryAmmoForType( ammo_i );
	inventory.SetInventoryAmmoForType( ammo_i, currentAmmo + ammoavailable );
}

/*
===============
idPlayer::ActiveGui
===============
*/
idUserInterface* idPlayer::ActiveGui()
{
	if( objectiveSystemOpen )
	{
		return NULL;
	}
	
	return focusUI;
}

/*
===============
idPlayer::Weapon_Combat
===============
*/
void idPlayer::Weapon_Combat()
{
	if( influenceActive || !weaponEnabled || gameLocal.inCinematic || Flicksync_InCutscene || privateCameraView )
	{
		commonVr->ForceChaperone(0, false);
		return;
	}

	// Carl: check if we're reloading either weapon
	bool reloading = false;
	for( int h = 0; h < 2; h++ )
	{
		hands[h].weapon.GetEntity()->RaiseWeapon();
		if( hands[h].weapon.GetEntity()->IsReloading() )
		{
			if( !AI_RELOAD )
			{
				reloading = true;
				AI_RELOAD = true;
				SetState( "ReloadWeapon" );
				UpdateScript();
			}
		}
	}
	if( !reloading )
		AI_RELOAD = false;

	for( int h = 0; h < 2; h++ )
	{
		// Carl: If we're trying to change to the soul cube, but the soul cube is already flying towards the enemy, stop changing weapon
		if( hands[h].idealWeapon == weapon_soulcube && soulCubeProjectile.GetEntity() != NULL )
		{
			hands[h].idealWeapon = hands[h].currentWeapon;
		}
		// Carl: Otherwise, change to the chosen weapon before we fire
		// Carl: TODO dual wielding, currently this only does the vr_weaponHand hand
		if( hands[ h ].idealWeapon != hands[ h ].currentWeapon &&  hands[ h ].idealWeapon.Get() < MAX_WEAPONS )
		{
			// multiplayer stuff
			if( weaponCatchup )
			{
				assert( common->IsClient() );

				hands[h].currentWeapon = hands[h].idealWeapon.Get();
				weaponGone = false;
				hands[h].animPrefix = spawnArgs.GetString( va( "def_weapon%d", hands[h].currentWeapon ) );
				// Carl: Dual Wielding, we need to choose whether this is the original weapon or the duplicate
				// Carl: if we're already holding the same weapon in the other hand, use the opposite copy
				// Carl: otherwise use whichever copy has more ammo in it
				if( hands[h].currentWeapon == hands[1 - h].currentWeapon )
					hands[h].isTheDuplicate = !hands[1 - h].isTheDuplicate;
				else
					hands[h].isTheDuplicate = inventory.GetClipAmmoForWeapon( hands[h].currentWeapon, true ) > inventory.GetClipAmmoForWeapon( hands[h].currentWeapon, false );
				hands[h].weapon->GetWeaponDef( hands[h].animPrefix, inventory.GetClipAmmoForWeapon( hands[h].currentWeapon, hands[h].isTheDuplicate ) );
				hands[h].animPrefix.Strip( "weapon_" );

				hands[h].weapon->NetCatchup();
				const function_t* newstate = GetScriptFunction( "NetCatchup" );
				if( newstate )
				{
					SetState( newstate );
					UpdateScript();
				}
				weaponCatchup = false;
			}
			else
			{
				if( hands[ h ].weapon.GetEntity()->IsReady() )
				{
					hands[ h ].weapon.GetEntity()->PutAway();
				}

				if( hands[ h ].weapon.GetEntity()->IsHolstered() )
				{
					assert( hands[h].idealWeapon.Get() >= 0 );
					assert( hands[h].idealWeapon.Get() < MAX_WEAPONS );

					if( hands[h].currentWeapon != weapon_pda && !spawnArgs.GetBool( va( "weapon%d_toggle", hands[h].currentWeapon ) ) )
					{
						hands[h].previousWeapon = hands[h].currentWeapon;
					}
					hands[h].currentWeapon = hands[h].idealWeapon.Get();
					weaponGone = false;
					hands[h].weaponGone = false;
					hands[h].animPrefix = spawnArgs.GetString( va( "def_weapon%d", hands[h].currentWeapon ) );
					// Carl: Dual Wielding, we need to choose whether this is the original weapon or the duplicate
					// Carl: if we're already holding the same weapon in the other hand, use the opposite copy
					// Carl: otherwise use whichever copy has more ammo in it
					if( hands[h].currentWeapon == hands[1 - h].currentWeapon )
						hands[h].isTheDuplicate = !hands[1 - h].isTheDuplicate;
					else
						hands[h].isTheDuplicate = inventory.GetClipAmmoForWeapon( hands[h].currentWeapon, true ) > inventory.GetClipAmmoForWeapon( hands[h].currentWeapon, false );
					hands[h].weapon->GetWeaponDef( hands[h].animPrefix, inventory.GetClipAmmoForWeapon( hands[h].currentWeapon, hands[h].isTheDuplicate ) );
					hands[h].animPrefix.Strip( "weapon_" );

					hands[h].weapon.GetEntity()->Raise();
					if( hands[h].holdingFlashlight() )
						PlayAnim( h ? ANIMCHANNEL_LEFTHAND : ANIMCHANNEL_RIGHTHAND, "flashlight_idle" );
					else
						PlayAnim( h ? ANIMCHANNEL_LEFTHAND : ANIMCHANNEL_RIGHTHAND, "idle" );
				}
			}
		}
		// Carl: If we're already using the chosen weapon
		else
		{
			weaponGone = false;	// if you drop and re-get weap, you may miss the = false above
			hands[ h ].weaponGone = false;
			if( hands[ h ].weapon.GetEntity()->IsHolstered() )
			{
				if( !hands[ h ].weapon.GetEntity()->AmmoAvailable() )
				{
					// weapons can switch automatically if they have no more ammo
					hands[ h ].NextBestWeapon();
				}
				else
				{
					risingWeaponHand = h;
					hands[ risingWeaponHand ].weapon.GetEntity()->Raise();
					state = GetScriptFunction( "RaiseWeapon" );
					if( state )
					{
						SetState( state );
					}
					if( !hands[h].holdingFlashlight() )
						PlayAnim( h ? ANIMCHANNEL_LEFTHAND : ANIMCHANNEL_RIGHTHAND, "idle" );
				}
			}
		}
	}
	
	// check for attack
	AI_WEAPON_FIRED = false;
	if( !influenceActive )
	{
		// Carl Dual wielding - check both hands for weapons being fired
		for( int h = 0; h < 2; h++ )
		{
			bool pullingTrigger = hands[ h ].triggerDown;
			// Check if we're actually turning our helmet/armour-mounted light on or off instead of firing
			if( pullingTrigger && hands[ h ].isOverMountedFlashlight() && !hands[ h ].tooFullToInteract() && !hands[h].oldFlashlightTriggerDown )
			{
				pullingTrigger = false;
				if( flashlight->lightOn )
					FlashlightOff();
				else
					FlashlightOn();
				hands[ h ].oldFlashlightTriggerDown = true;
			}
			// Fire weapon
			if( (pullingTrigger && !hands[ h ].oldFlashlightTriggerDown ) || ( usercmd.buttons & BUTTON_ATTACK ) )
			{
				//common->Printf( "trigger down\n" );
				if( hands[ h ].controllingWeapon() && !hands[h].weaponGone && !weaponGone )
				{
					FireWeapon( h, GetWeaponInHand( h ) );
					if( !hands[h].oldTriggerDown )
						PlayAnim( h ? ANIMCHANNEL_LEFTHAND : ANIMCHANNEL_RIGHTHAND, "fire1" );
				}
			}
			else if( (hands[ h ].oldTriggerDown && !hands[ h ].oldFlashlightTriggerDown) || oldButtons & BUTTON_ATTACK )
			{
				//common->Printf( "old trigger down\n" );
				AI_ATTACK_HELD = false;
				GetWeaponInHand( h )->EndAttack();
				PlayAnim( h ? ANIMCHANNEL_LEFTHAND : ANIMCHANNEL_RIGHTHAND, "idle" );
			}
			// remember the old state
			if( hands[ h ].oldFlashlightTriggerDown && !hands[ h ].triggerDown )
			{
				hands[ h ].oldFlashlightTriggerDown = false;
			}
			hands[ h ].oldTriggerDown = hands[ h ].triggerDown;
		}
	}
	
	// update our ammo clip in our inventory
	for( int h = 0; h < 2; h++ )
	{
		if( ( hands[h].currentWeapon >= 0 ) && ( hands[h].currentWeapon < MAX_WEAPONS ) )
		{
			inventory.SetClipAmmoForWeapon( hands[h].currentWeapon, hands[h].isTheDuplicate, hands[h].weapon.GetEntity()->AmmoInClip() );
		}
	}

	// Carl: Optionally activate the chaperone if we're likely to be swinging or throwing stuff around IRL
	int c = vr_chaperone.GetInteger();
	bool force;
	int w1 = hands[ 0 ].currentWeapon;
	int w2 = hands[ 1 ].currentWeapon;
	bool hasThrowingWeapon = w1 == weapon_handgrenade || w2 == weapon_handgrenade;
	bool hasSwingingWeapon = w1 == weapon_chainsaw || w2 == weapon_chainsaw 
		|| w1 == weapon_fists || w2 == weapon_fists || w1 == weapon_grabber || w2 == weapon_grabber;

	if( !weaponEnabled || spectating || Flicksync_InCutscene || gameLocal.inCinematic || health < 0 || hiddenWeapon || ( hands[ 0 ].currentWeapon < 0 && hands[ 1 ].currentWeapon < 0 ) )
		force = c >= 4;
	else
		force = ( c >= 4 ) || ( c >= 1 && hasThrowingWeapon ) || ( c >= 2 && hasSwingingWeapon );
	commonVr->ForceChaperone( 0, force );
}

/*
===============
idPlayer::Weapon_NPC
===============
*/
void idPlayer::Weapon_NPC()
{
	if( hands[0].idealWeapon != hands[0].currentWeapon || hands[ 1 ].idealWeapon != hands[ 1 ].currentWeapon )
	{
		Weapon_Combat();
	}
	StopFiring();
	for ( int h=0; h<2; h++ )
		hands[h].weapon.GetEntity()->LowerWeapon();
	
	int talkButtons = 0;
	if ( vr_talkMode.GetInteger() < 2 )
		talkButtons |= BUTTON_ATTACK | BUTTON_USE;
	if ( vr_talkMode.GetInteger() > 0 )
		talkButtons |= BUTTON_CHATTING;
	bool wasDown = ( oldButtons & talkButtons ) != 0;
	bool isDown = ( usercmd.buttons & talkButtons ) != 0;
	if ( isDown && !wasDown )
	{
		buttonMask |= BUTTON_ATTACK;
		focusCharacter->ListenTo( this );
	}
	else if ( wasDown && !isDown )
	{
		focusCharacter->TalkTo( this );
	}
}

/*
===============
idPlayer::WeaponLoweringCallback
===============
*/
void idPlayer::WeaponLoweringCallback()
{
	SetState( "LowerWeapon" );
	UpdateScript();
}

/*
===============
idPlayer::WeaponRisingCallback
===============
*/
void idPlayer::WeaponRisingCallback()
{
	SetState( "RaiseWeapon" );
	UpdateScript();
}

/*
===============
idPlayer::Weapon_GUI
===============
*/
void idPlayer::Weapon_GUI()
{

	if( !objectiveSystemOpen )
	{
		if( hands[ 0 ].idealWeapon != hands[ 0 ].currentWeapon || hands[ 1 ].idealWeapon != hands[ 1 ].currentWeapon )
		{
			Weapon_Combat();
		}
		StopFiring();
		
		for( int h = 0; h < 2; h++ )
		{
			hands[ h ].weapon.GetEntity()->LowerWeapon();
			hands[ h ].weapon->GetRenderEntity()->allowSurfaceInViewID = -1;
			hands[ h ].weapon->GetRenderEntity()->suppressShadowInViewID = entityNumber + 1;
		}
		
	}
	
	commonVr->ForceChaperone( 0, vr_chaperone.GetInteger() >= 4 );

	// disable click prediction for the GUIs. handy to check the state sync does the right thing
	if( common->IsClient() && !net_clientPredictGUI.GetBool() )
	{
		return;
	}
	
	bool wasDown = ( oldButtons & ( BUTTON_ATTACK | BUTTON_USE ) ) != 0;
	bool isDown = ( usercmd.buttons & ( BUTTON_ATTACK | BUTTON_USE ) ) != 0;
	if( isDown != wasDown )
	{
		const char* command = NULL;
		idUserInterface* ui = ActiveGui();
		if( ui )
		{
			bool updateVisuals = false;
			sysEvent_t ev = sys->GenerateMouseButtonEvent( 1, isDown );
			command = ui->HandleEvent( &ev, gameLocal.time, &updateVisuals );
			if( updateVisuals && focusGUIent && ui == focusUI )
			{
				focusGUIent->UpdateVisuals();
			}
		}
		if( common->IsClient() )
		{
			// we predict enough, but don't want to execute commands
			return;
		}
		
		// HACK - Check to see who is activating the frag chamber. Im sorry.
		if( common->IsMultiplayer() && focusGUIent )
		{
			if( strcmp( focusGUIent->GetName(), "chamber_gui_console" ) == 0 && strcmp( command, " ; runScript chamber_trigger" ) == 0 )
			{
				gameLocal.playerActivateFragChamber = this;
			}
		}
		
		if( focusGUIent )
		{
			HandleGuiCommands( focusGUIent, command );
		}
		else
		{
			HandleGuiCommands( this, command );
		}
	}
}

/*
===============
idPlayer::UpdateWeapon
===============
*/
void idPlayer::UpdateWeapon()
{
	if( health <= 0 )
	{
		return;
	}
	
	assert( !spectating );
	

	static bool wasTalking = false;
	bool talking = ( usercmd.buttons & BUTTON_CHATTING ) > 0;

	// Voice wakes up nearby monsters while you're speaking
	// Carl: weapon method (voice wakes up monsters that respond to weapon sound)
	if ( (vr_talkWakeMonsters.GetInteger() & 1) && talking )
		gameLocal.AlertAI( this, 788 * commonVoice->currentVolume * (vr_talkWakeMonsterRadius.GetFloat() / 120 / vr_scale.GetFloat()) ); // maximum mic volume = 20 metres, normal volume = 5 m
	// Koz (voice wakes up monsters that respond to flashlight)
	if ( vr_talkWakeMonsters.GetInteger() > 0 && vr_talkWakeMonsters.GetInteger() <= 2 )
	{
		if ( !talking || commonVoice->maxVolume == 0 ) 
		{
			wasTalking = false;
		}
		else if ( !wasTalking )
		{
			wasTalking = true;
			idEntity* entityList[MAX_GENTITIES];
			int listedEntities;
			float radius = 788 * commonVoice->maxVolume * (vr_talkWakeMonsterRadius.GetFloat() / 120 / vr_scale.GetFloat());

			listedEntities = gameLocal.EntitiesWithinRadius( GetPhysics()->GetOrigin(), radius, entityList, MAX_GENTITIES );
			for ( int i = 0; i < listedEntities; i++ )
			{
				idEntity* ent = entityList[i];
				if ( ent && !ent->IsHidden() )
				{
					if ( ent->IsType( idAI::Type ) )
					{
						static_cast<idAI*>(ent)->TouchedByFlashlight( this );
					}
				}
			}
		}
	}
	// Koz end
		
	if( common->IsClient() )
	{
		// clients need to wait till the weapon and it's world model entity
		// are present and synchronized ( weapon.worldModel idEntityPtr to idAnimatedEntity )
		if( !hands[ 0 ].weapon.GetEntity()->IsWorldModelReady() || !hands[ 1 ].weapon.GetEntity()->IsWorldModelReady() )
		{
			return;
		}
	}
	
	// always make sure the weapons are correctly setup before accessing them
	for( int h = 0; h < 2; h++ )
	{
		if( !hands[h].weapon.GetEntity()->IsLinked() )
		{
			if( hands[ h ].idealWeapon == -1 )
				hands[ h ].idealWeapon = 0;
			if( hands[ h ].idealWeapon != -1 )
			{
				animPrefix = spawnArgs.GetString( va( "def_weapon%d", hands[h].idealWeapon.Get() ) );
				// Carl: Dual Wielding, we need to choose whether this is the original weapon or the duplicate
				// if we're already holding the same weapon in the other hand, use the opposite copy
				// otherwise use whichever copy has more ammo in it
				if( hands[1-h].weapon.GetEntity()->IsLinked() && hands[h].idealWeapon.Get() == hands[1 - h].idealWeapon.Get() )
					hands[h].isTheDuplicate = !hands[1 - h].isTheDuplicate;
				else
					hands[h].isTheDuplicate = inventory.GetClipAmmoForWeapon( hands[h].idealWeapon.Get(), true ) > inventory.GetClipAmmoForWeapon( hands[h].idealWeapon.Get(), false );
				int ammoInClip = inventory.GetClipAmmoForWeapon( hands[ h ].idealWeapon.Get(), hands[h].isTheDuplicate );
				if( common->IsMultiplayer() && respawning )
				{
					// Do not load ammo into the clip here on MP respawn, as it will be done
					// elsewhere. If we take ammo out here then the player will end up losing
					// a clip of ammo for their initial weapon upon respawn.
					ammoInClip = 0;
				}
				hands[ h ].weapon.GetEntity()->GetWeaponDef( animPrefix, ammoInClip );
				assert( hands[ h ].weapon.GetEntity()->IsLinked() );
			}
			else
			{
				return;
			}
		}
	}
	
	if( hiddenWeapon && tipUp && usercmd.buttons & BUTTON_ATTACK )
	{
		HideTip();
	}
	
	if( g_dragEntity.GetBool() )
	{
		StopFiring();
		for( int h = 0; h < 2; h++ )
			hands[ h ].weapon.GetEntity()->LowerWeapon();
		dragEntity.Update( this );
	}
	else if( ActiveGui() )
	{
		// gui handling overrides weapon use
		Weapon_GUI();
	}
	else 	if( focusCharacter && ( focusCharacter->health > 0 ) )
	{
		Weapon_NPC();
	}
	else
	{
		Weapon_Combat();
	}
	
	
	if( hiddenWeapon  )
	{
		if( !game->isVR || commonVr->handInGui == false )
		{
			for( int h = 0; h < 2; h++ )
				hands[ h ].weapon.GetEntity()->LowerWeapon();  // KOZ FIXME HIDE WEAPon
		}
		
	}
	else
	{
		for( int h = 0; h < 2; h++ )
			hands[ h ].weapon.GetEntity()->GetRenderEntity()->suppressShadowInViewID = 0;
	}

	if( game->isVR && commonVr->handInGui )
	{
		for( int h = 0; h < 2; h++ )
			hands[ h ].weapon.GetEntity()->GetRenderEntity()->suppressShadowInViewID = entityNumber + 1;
	}

	// update weapon state, particles, dlights, etc
	for( int h = 0; h < 2; h++ )
		hands[ h ].weapon.GetEntity()->PresentWeapon( CanShowWeaponViewmodel(), h );
}

/*
===============
idPlayer::UpdateFlashLight
===============
*/
void idPlayer::UpdateFlashlight()
{
	for( int h = 0; h < 2; h++ )
	{
		if( hands[h].idealWeapon == weapon_flashlight )
		{
			// force classic flashlight to go away
			hands[h].NextWeapon();
		}
	}
	
	if( !flashlight.IsValid() )
	{
		return;
	}
	
	if( !flashlight.GetEntity()->GetOwner() )
	{
		return;
	}
	
	// Don't update the flashlight if dead in MP.
	// Otherwise you can see a floating flashlight worldmodel near player's skeletons.
	if( common->IsMultiplayer() )
	{
		if( health < 0 )
		{
			return;
		}
	}
	
	// Flashlight has an infinite battery in multiplayer and in RoE XBox mode when mounted on your pistol
	if( !common->IsMultiplayer() && commonVr->currentFlashlightMode != FLASHLIGHT_PISTOL )
	{
		if( flashlight.GetEntity()->lightOn )
		{
			if( flashlight_batteryDrainTimeMS.GetInteger() > 0 )
			{
				flashlightBattery -= ( gameLocal.time - gameLocal.previousTime );
				if( flashlightBattery < 0 )
				{
					FlashlightOff();
					flashlightBattery = 0;
				}
			}
		}
		else
		{
			if( flashlightBattery < flashlight_batteryDrainTimeMS.GetInteger() )
			{
				flashlightBattery += ( gameLocal.time - gameLocal.previousTime ) * Max( 1, ( flashlight_batteryDrainTimeMS.GetInteger() / flashlight_batteryChargeTimeMS.GetInteger() ) );
				if( flashlightBattery > flashlight_batteryDrainTimeMS.GetInteger() )
				{
					flashlightBattery = flashlight_batteryDrainTimeMS.GetInteger();
				}
			}
		}
	}
	
	if( hud )
	{
		hud->UpdateFlashlight( this );
	}
	
	if( common->IsClient() )
	{
		// clients need to wait till the weapon and it's world model entity
		// are present and synchronized ( weapon.worldModel idEntityPtr to idAnimatedEntity )
		if( !flashlight.GetEntity()->IsWorldModelReady() )
		{
			return;
		}
	}
	
	// always make sure the weapon is correctly setup before accessing it
	if( !flashlight.GetEntity()->IsLinked() )
	{
		flashlight.GetEntity()->GetWeaponDef( "weapon_flashlight_new", 0 );
		flashlight.GetEntity()->SetIsPlayerFlashlight( true );
		
		// adjust position / orientation of flashlight
		idAnimatedEntity* worldModel = flashlight.GetEntity()->GetWorldModel();
		worldModel->BindToJoint( this, "Chest", true );
		// Don't interpolate the flashlight world model in mp, let it bind like normal.
		worldModel->SetUseClientInterpolation( false );
		
		assert( flashlight.GetEntity()->IsLinked() );
	}
	
	// this positions the third person flashlight model! (as seen in the mirror)
	idAnimatedEntity* worldModel = flashlight.GetEntity()->GetWorldModel();
	static const idVec3 fl_pos = idVec3( 3.0f, 9.0f, 2.0f );
	worldModel->GetPhysics()->SetOrigin( fl_pos );
	static float fl_pitch = 0.0f;
	static float fl_yaw = 0.0f;
	static float fl_roll = 0.0f;
	static idAngles ang = ang_zero;
	ang.Set( fl_pitch, fl_yaw, fl_roll );
	worldModel->GetPhysics()->SetAxis( ang.ToMat3() );
	
	if( flashlight.GetEntity()->lightOn )
	{
		if( ( flashlightBattery < flashlight_batteryChargeTimeMS.GetInteger() / 2 ) && ( gameLocal.random.RandomFloat() < flashlight_batteryFlickerPercent.GetFloat() ) )
		{
			flashlight.GetEntity()->RemoveMuzzleFlashlight();
		}
		else
		{
			flashlight.GetEntity()->MuzzleFlashLight();
		}
	}
	
	flashlight.GetEntity()->PresentWeapon( true, 1 - vr_weaponHand.GetInteger() );
	
	if( gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) || gameLocal.inCinematic || spectating || fl.hidden || commonVr->currentFlashlightPosition == FLASHLIGHT_INVENTORY )
	{
		worldModel->Hide();
	}
	else
	{
		worldModel->Show();
	}
}

/*
===============
idPlayer::FlashlightOn
===============
*/
void idPlayer::FlashlightOn()
{
	if( commonVr->currentFlashlightPosition == FLASHLIGHT_NONE || commonVr->currentFlashlightPosition == FLASHLIGHT_INVENTORY )
		return;
	if( !flashlight.IsValid() )
	{
		return;
	}
	if( flashlightBattery < idMath::Ftoi( flashlight_minActivatePercent.GetFloat() * flashlight_batteryDrainTimeMS.GetFloat() ) )
	{
		return;
	}
	if( gameLocal.inCinematic || Flicksync_InCutscene )
	{
		return;
	}
	if( flashlight.GetEntity()->lightOn )
	{
		return;
	}
	if( health <= 0 )
	{
		return;
	}
	if( spectating )
	{
		return;
	}
	
	flashlight->FlashlightOn();
		
	// Koz pose the hand
	const function_t* func;
		
	func = scriptObject.GetFunction( "SetFlashHandPose" ); // Set flashlight hand pose
	if ( func )
	{
		// use the frameCommandThread since it's safe to use outside of framecommands
		// Koz debug common->Printf( "Calling SetFlashHandPose\n" ); // Set flashlight hand pose
		gameLocal.frameCommandThread->CallFunction( this, func, true );
		gameLocal.frameCommandThread->Execute();

	}
	else
	{
		common->Warning( "Can't find function 'SetFlashHandPose' in object '%s'", scriptObject.GetTypeName() ); // Set flashlight hand pose
		return;
	}
	// Koz end
	
	
}

/*
===============
idPlayer::FlashlightOff
===============
*/
void idPlayer::FlashlightOff()
{
	if( !flashlight.IsValid() )
	{
		return;
	}
	if( !flashlight.GetEntity()->lightOn )
	{
		return;
	}
	flashlight->FlashlightOff();

	// Koz
	const function_t* func;
	func = scriptObject.GetFunction( "SetFlashHandPose" ); // Set flashlight hand pose
	if ( func )
	{
		// use the frameCommandThread since it's safe to use outside of framecommands
		// Koz debug common->Printf( "Calling SetFlashHandPose\n" ); // Set flashlight hand pose
		gameLocal.frameCommandThread->CallFunction( this, func, true );
		gameLocal.frameCommandThread->Execute();

	}
	else
	{
		common->Warning( "Can't find function 'SetFlashHandPose' in object '%s'", scriptObject.GetTypeName() ); // Set flashlight hand pose
		return;
	}
	// Koz
}

/*
===============
idPlayer::SpectateFreeFly
===============
*/
void idPlayer::SpectateFreeFly( bool force )
{
	idPlayer*	player;
	idVec3		newOrig;
	idVec3		spawn_origin;
	idAngles	spawn_angles;
	
	player = gameLocal.GetClientByNum( spectator );
	if( force || gameLocal.time > lastSpectateChange )
	{
		spectator = entityNumber;
		if( player != NULL && player != this && !player->spectating && !player->IsInTeleport() )
		{
			newOrig = player->GetPhysics()->GetOrigin();
			if( player->physicsObj.IsCrouching() )
			{
				newOrig[ 2 ] += pm_crouchviewheight.GetFloat();
			}
			else
			{
				newOrig[ 2 ] += pm_normalviewheight.GetFloat();
			}
			newOrig[ 2 ] += SPECTATE_RAISE;
			idBounds b = idBounds( vec3_origin ).Expand( pm_spectatebbox.GetFloat() * 0.5f );
			idVec3 start = player->GetPhysics()->GetOrigin();
			start[2] += pm_spectatebbox.GetFloat() * 0.5f;
			trace_t t;
			// assuming spectate bbox is inside stand or crouch box
			gameLocal.clip.TraceBounds( t, start, newOrig, b, MASK_PLAYERSOLID, player );
			newOrig.Lerp( start, newOrig, t.fraction );
			SetOrigin( newOrig );
			idAngles angle = player->viewAngles;
			angle[ 2 ] = 0;
			SetViewAngles( angle );
		}
		else
		{
			SelectInitialSpawnPoint( spawn_origin, spawn_angles );
			spawn_origin[ 2 ] += pm_normalviewheight.GetFloat();
			spawn_origin[ 2 ] += SPECTATE_RAISE;
			SetOrigin( spawn_origin );
			SetViewAngles( spawn_angles );
			// This may happen during GAMESTATE_STARTUP in mp, so we must set the spawnAngles too.
			spawnAngles = spawn_angles;
			
			if( force == false )
			{
				// only do this if they hit the cycle button.
				if( common->IsServer() )
				{
					if( player != NULL )
					{
						idBitMsg	msg;
						byte		msgBuf[MAX_EVENT_PARAM_SIZE];
						
						msg.InitWrite( msgBuf, sizeof( msgBuf ) );
						msg.WriteFloat( GetPhysics()->GetOrigin()[0] );
						msg.WriteFloat( GetPhysics()->GetOrigin()[1] );
						msg.WriteFloat( GetPhysics()->GetOrigin()[2] );
						msg.WriteFloat( viewAngles[0] );
						msg.WriteFloat( viewAngles[1] );
						msg.WriteFloat( viewAngles[2] );
						
						ServerSendEvent( idPlayer::EVENT_FORCE_ORIGIN, &msg, false );
					}
				}
			}
		}
		lastSpectateChange = gameLocal.time + 500;
	}
	
	
}

/*
===============
idPlayer::SpectateCycle
===============
*/
void idPlayer::SpectateCycle()
{
	idPlayer* player;
	
	if( gameLocal.time > lastSpectateChange )
	{
		int latchedSpectator = spectator;
		spectator = gameLocal.GetNextClientNum( spectator );
		player = gameLocal.GetClientByNum( spectator );
		assert( player ); // never call here when the current spectator is wrong
		// ignore other spectators
		while( latchedSpectator != spectator && player->spectating )
		{
			spectator = gameLocal.GetNextClientNum( spectator );
			player = gameLocal.GetClientByNum( spectator );
		}
		lastSpectateChange = gameLocal.time + 500;
	}
}

/*
===============
idPlayer::UpdateSpectating
===============
*/
void idPlayer::UpdateSpectating()
{
	assert( spectating );
	assert( !common->IsClient() );
	assert( IsHidden() );
	idPlayer* player;
	if( !common->IsMultiplayer() )
	{
		return;
	}
	player = gameLocal.GetClientByNum( spectator );
	if( !player || ( player->spectating && player != this ) )
	{
		SpectateFreeFly( true );
	}
	else if( usercmd.buttons & BUTTON_JUMP )
	{
		SpectateFreeFly( false );
	}
	else if( usercmd.buttons & BUTTON_USE )
	{
		SpectateCycle();
	}
	else if( usercmd.buttons & BUTTON_ATTACK )
	{
		wantSpectate = false;
	}
}

/*
===============
idPlayer::HandleSingleGuiCommand
===============
*/
bool idPlayer::HandleSingleGuiCommand( idEntity* entityGui, idLexer* src )
{
	idToken token;
	
	if( !src->ReadToken( &token ) )
	{
		return false;
	}
	
	if( token == ";" )
	{
		return false;
	}
	
	if( token.Icmp( "addhealth" ) == 0 )
	{
		if( entityGui && health < 100 )
		{
			int _health = entityGui->spawnArgs.GetInt( "gui_parm1" );
			int amt = ( _health >= HEALTH_PER_DOSE ) ? HEALTH_PER_DOSE : _health;
			_health -= amt;
			entityGui->spawnArgs.SetInt( "gui_parm1", _health );
			if( entityGui->GetRenderEntity() && entityGui->GetRenderEntity()->gui[ 0 ] )
			{
				entityGui->GetRenderEntity()->gui[ 0 ]->SetStateInt( "gui_parm1", _health );
			}
			health += amt;
			if( health > 100 )
			{
				health = 100;
			}
		}
		return true;
	}
	src->UnreadToken( &token );
	return false;
}

/*
==============
idPlayer::PlayAudioLog
==============
*/
void idPlayer::PlayAudioLog( const idSoundShader* shader )
{
	EndVideoDisk();
	if( name.Length() > 0 )
	{
		int ms;
		StartSoundShader( shader, SND_CHANNEL_PDA_AUDIO, 0, false, &ms );
		CancelEvents( &EV_Player_StopAudioLog );
		PostEventMS( &EV_Player_StopAudioLog, ms + 150 );
	}
}

/*
==============
idPlayer::EndAudioLog
==============
*/
void idPlayer::EndAudioLog()
{
	StopSound( SND_CHANNEL_PDA_AUDIO, false );
}

/*
==============
idPlayer::PlayVideoDisk
==============
*/
void idPlayer::PlayVideoDisk( const idDeclVideo* decl )
{
	EndAudioLog();
	pdaVideoMat = decl->GetRoq();
	if( pdaVideoMat )
	{
		int c = pdaVideoMat->GetNumStages();
		for( int i = 0; i < c; i++ )
		{
			const shaderStage_t* stage = pdaVideoMat->GetStage( i );
			if( stage != NULL && stage->texture.cinematic )
			{
				stage->texture.cinematic->ResetTime( Sys_Milliseconds() );
			}
		}
		if( decl->GetWave() != NULL )
		{
			StartSoundShader( decl->GetWave(), SND_CHANNEL_PDA_VIDEO, 0, false, NULL );
		}
	}
}

/*
==============
idPlayer::EndVideoDisk
==============
*/
void idPlayer::EndVideoDisk()
{
	pdaVideoMat = NULL;
	StopSound( SND_CHANNEL_PDA_VIDEO, false );
}

/*
==============
idPlayer::Collide
==============
*/
bool idPlayer::Collide( const trace_t& collision, const idVec3& velocity )
{
	idEntity* other;
	
	if( common->IsClient() && spectating == false )
	{
		return false;
	}
	
	other = gameLocal.entities[ collision.c.entityNum ];
	if( other )
	{
		other->Signal( SIG_TOUCH );
		if( !spectating )
		{
			if( other->RespondsTo( EV_Touch ) )
			{
				other->ProcessEvent( &EV_Touch, this, &collision );
			}
		}
		else
		{
			if( other->RespondsTo( EV_SpectatorTouch ) )
			{
				other->ProcessEvent( &EV_SpectatorTouch, this, &collision );
			}
		}
	}
	return false;
}


/*
================
idPlayer::UpdateLocation

Searches nearby locations
================
*/
void idPlayer::UpdateLocation()
{

	if( hud )
	{
		hud->UpdateLocation( this );
	}
}

/*
================
idPlayer::ClearFocus

Clears the focus cursor
================
*/
void idPlayer::ClearFocus()
{
	focusCharacter	= NULL;
	focusGUIent		= NULL;
	focusUI			= NULL;
	focusVehicle	= NULL;
	talkCursor		= 0;

	// Koz
	commonVr->handInGui = false;
	//weapon.GetEntity()->GetRenderEntity()->allowSurfaceInViewID = 0;


}


void idPlayer::SendPDAEvent( const sysEvent_t* sev )
{
	if ( common->Dialog().IsDialogActive() || game->Shell_IsActive() ) //commonVr->VR_GAME_PAUSED )
	{
		if ( common->Dialog().IsDialogActive() )
		{
			common->Dialog().HandleDialogEvent( sev );
		}
		else
		{
			game->Shell_HandleGuiEvent( sev );
		}
	}
	else
	{
		HandleGuiEvents( sev );
	}
}

/*
================
idPlayer::UpdateFocusPDA

Searches nearby entities for interactive guis, possibly making one of them
the focus and sending it a mouse move event
================
*/
bool idPlayer::UpdateFocusPDA()
{
	static bool touching = false;
	jointHandle_t fingerJoint[2] = { animator.GetJointHandle( "RindexTip" ), animator.GetJointHandle( "LindexTip" ) };
	idVec3 fingerPosLocal = vec3_zero;
	idMat3 fingerAxisLocal = mat3_identity;
	idVec3 fingerPosGlobal = vec3_zero;
	idVec3 scanStart = vec3_zero;
	idVec3 scanEnd = vec3_zero;
	int pdaScrX = 1024;// vr_pdaScreenX.GetInteger();
	int pdaScrY = 768; // vr_pdaScreenY.GetInteger();
	const int fingerForwDist = 1.0f;
	const int fingerBackwDist = 12.0f;
	guiPoint_t	pt;
	sysEvent_t	ev;
	
	if ( !game->isVR || !( game->IsPDAOpen() || commonVr->VR_GAME_PAUSED || hands[0].currentWeapon == weapon_pda || hands[1].currentWeapon == weapon_pda ) )
	{
		touching = false;
		return false;
	}

	int pdahand;
	if( hands[ 0 ].holdingPDA() )
		pdahand = 0;
	else if( hands[ 1 ].holdingPDA() )
		pdahand = 1;
	else
		pdahand = 1 - vr_weaponHand.GetInteger();
	int fingerHand = 1 - pdahand;

	commonVr->scanningPDA = true; // let the swf event handler know its ok to take mouse input, even if the mouse cursor is not in the window.


	// game is VR and PDA active

	
	// Koz fixme
	// i have completely borked swf rendering resolutions to try to make them fit better on the PDA screen,
	// really really need to straighten out all this hacked crap.
	// pdaScrX,Y are the resolutions mouse movements need to be scaled to align the cursor with the results
	// of guitrace.  I'm very sorry.
	
	
	if ( common->Dialog().IsDialogActive() || game->Shell_IsActive() )

	{
		pdaScrX = 1280; 
		pdaScrY = 1200; 
	}
	else
	{
		pdaScrX = 1024;
		pdaScrY = 768;
	}
			
	if ( vr_guiMode.GetInteger() == 2 && commonVr->VR_USE_MOTION_CONTROLS )
	{
		// the game is vr, player is using motion controls, gui mode is set to use touchscreens
		// and the pda is open
		// wait for lower weapon to drop the hand to hidedistance and hide the weapon model
		// then raise the empty hand with pointy finger back to original position
				
		if ( !touching )
		{
			// send a cursor event to get it off the screen since it is hidden.
			ev = sys->GenerateMouseMoveEvent( -2000, -2000 );
			SendPDAEvent( &ev );
		}

		commonVr->handInGui = true;
		focusTime = gameLocal.time + FOCUS_TIME;
				
		// get current position of the pointy finger tip joint and
		// see if we have touched the gui
		idWeapon* pdaWeapon = GetPDAWeapon();

		animator.GetJointTransform( fingerJoint[ fingerHand ], gameLocal.time, fingerPosLocal, fingerAxisLocal );
		fingerPosGlobal = fingerPosLocal * GetRenderEntity()->axis + GetRenderEntity()->origin;
		
		
		scanStart = fingerPosGlobal - PDAaxis[0] * fingerBackwDist;//  PDAaxis.Inverse();// surfaceNormal;
		scanEnd = fingerPosGlobal + PDAaxis[0] * fingerForwDist;// *surfaceNormal;
	
		
		pt = gameRenderWorld->GuiTrace( pdaWeapon->GetModelDefHandle(), pdaWeapon->GetAnimator(), scanStart, scanEnd );
		pt.y = 1.0f - pt.y;
	
		/*
		//debug finger 
		if ( pt.fraction >= 1.0f )
		{
			gameRenderWorld->DebugLine( colorRed, scanStart, scanEnd, 20 );
		}
		else
		{
			gameRenderWorld->DebugLine( colorGreen, scanStart, scanEnd, 20 );
		}
		*/

		if ( common->Dialog().IsDialogActive() || game->Shell_IsActive() ) // commonVr->VR_GAME_PAUSED )
		{
			pt.y -= .12f;
		}
		else
		{
			pt.y += 0.12f; // add offset for screensafe borders
		}

		if ( pt.fraction >= 1.0f ) // no hit if > = 1.0f 
		{
			//send mouse button up
			ev = sys->GenerateMouseButtonEvent( 1, false );
			SendPDAEvent( &ev );

			touching = false;
		}
		else
		{
			//we have a hit
			if ( touching )
			{
				if ( pt.fraction >= 0.94f ) // no longer touching
				{

					//common->Printf( "Sending mouse off 1 fraction = %f\n",trace.fraction );
					ev = sys->GenerateMouseButtonEvent( 1, false );
					SendPDAEvent( &ev );

					touching = false;
				}
				else
				{
					if ( pt.x != -1 )
					{
						// clamp the mouse to the corner
						ev = sys->GenerateMouseMoveEvent( -2000, -2000 );
						SendPDAEvent( &ev );
						
						// move to an absolute position
						ev = sys->GenerateMouseMoveEvent( pt.x * pdaScrX, pt.y * pdaScrY );//( pt.x * SCREEN_WIDTH, pt.y * SCREEN_HEIGHT );
						SendPDAEvent( &ev );
						
					}
				}
			}
			else // if ( !touching )
			{
				//common->Printf( "Fraction = %f\n", pt.fraction );
				if ( pt.fraction < 0.94f )
				{
					//common->Printf( "Setting touching true\n" );
					touching = true;
					if ( pt.x != -1 )
					{
						// clamp the mouse to the corner
						ev = sys->GenerateMouseMoveEvent( -2000, -2000 );
						SendPDAEvent( &ev );
						
						// move to an absolute position
						ev = sys->GenerateMouseMoveEvent( pt.x * pdaScrX, pt.y * pdaScrY );//( pt.x * SCREEN_WIDTH, pt.y * SCREEN_HEIGHT );
						SendPDAEvent( &ev );
						
						//send mouse button down
						ev = sys->GenerateMouseButtonEvent( 1, true );
						SendPDAEvent( &ev );
						
						//Rumble the controller to let player know they scored a touch.
						//SetControllerShake( 0.5f, 100, 0.3f, 60 ); // these are the values from the machine gun

						// Carl: Should the PDA vibrate/haptic feedback too?
						hands[fingerHand].SetControllerShake( 0.1f, 12, 0.8f, 12 );
						hands[pdahand].SetControllerShake( 0.1f, 12, 0.8f, 12 );
					}
					commonVr->scanningPDA = false;
					return true;
				}
				else
				{

					//send mouse button up
					ev = sys->GenerateMouseButtonEvent( 1, false );
					SendPDAEvent( &ev );

					touching = false;
				}
			}
		}
		commonVr->scanningPDA = false;
		return false;
	}
	
	//-------------------------------------------------------
	//not using motion controls, scan from view

	scanStart = commonVr->lastViewOrigin;
	scanEnd = scanStart + commonVr->lastViewAxis[0] * 60.0f; // not sure why the PDA would be farther than 60 inches away. Thats one LOOONG arm.

	//gameRenderWorld->DebugLine( colorYellow, scanStart, scanEnd, 10 );
	
	idWeapon* pdaWeapon = GetPDAWeapon();
	pt = gameRenderWorld->GuiTrace( pdaWeapon->GetModelDefHandle(), pdaWeapon->GetAnimator(), scanStart, scanEnd );

	if ( pt.x != -1 )
	{
		
		pt.y = 1 - pt.y; // texture was copied from framebuffer and is upside down so invert y 
		if ( !commonVr->VR_GAME_PAUSED ) pt.y += 0.12f; // add offset for screensafe borders if using PDA

		focusTime = gameLocal.time + FOCUS_TIME;
		
		// move to an absolute position
		
		ev = sys->GenerateMouseMoveEvent( -2000, -2000 );
		SendPDAEvent( &ev );

		ev = sys->GenerateMouseMoveEvent( pdaScrX * pt.x, pdaScrY * pt.y );
		SendPDAEvent( &ev );
		
		commonVr->scanningPDA = false;
		
		return true;
	}
	
	commonVr->scanningPDA = false;
	
	return false; // view didn't hit pda
}

/*
================
idPlayer::UpdateFocus

Searches nearby entities for interactive guis, possibly making one of them
the focus and sending it a mouse move event
================
*/
void idPlayer::UpdateFocus()
{
	idClipModel* clipModelList[MAX_GENTITIES];
	idClipModel* clip;
	int			listedClipModels;
	idEntity*	oldFocus;
	idEntity*	ent;
	idUserInterface* oldUI;
	idAI*		oldChar;
	int			oldTalkCursor;
	idAFEntity_Vehicle* oldVehicle;
	int			i, j;
	idVec3		start, end;
	bool		allowFocus;
	const char* command;
	trace_t		trace;
	guiPoint_t	pt;
	const idKeyValue* kv;
	sysEvent_t	ev;
	idUserInterface* ui;

	static idMat3 lastScanAxis = commonVr->lastViewAxis;
	static idVec3 lastScanStart = commonVr->lastViewOrigin;
	static idVec3 lastBodyPosition = physicsObj.GetOrigin();
	static float lastBodyYaw = 0.0f;
	static bool lowered = false;
	static bool raised = false;

	static idVec3 surfaceNormal = vec3_zero;
	static idVec3 fingerPosLocal = vec3_zero;
	static idMat3 fingerAxisLocal = mat3_identity;
	static idVec3 fingerPosGlobal = vec3_zero;
	static idVec3 scanStart = vec3_zero;
	static idVec3 scanEnd = vec3_zero;
	static idVec3 talkScanEnd = vec3_zero;
	static jointHandle_t fingerJoint;
	static bool	touching = false;

	static idMat3 weaponAxis;
	static bool scanFromWeap;
	static float scanRange;
	static float scanRangeCorrected;
	static float hmdAbsPitch;

	idMat3 viewPitchAdj = idAngles( vr_guiFocusPitchAdj.GetFloat(), 0.0f, 0.0f ).ToMat3();
	
	scanRange = 50.0f;
	
	if ( Flicksync_InCutscene || gameLocal.inCinematic || commonVr->thirdPersonMovement ) 
	{
		return;
	}

	//check for PDA interaction.
	//if the PDA is being interacted with, there is no need to check for other guis
	//or scan for character names so bail.
	if ( UpdateFocusPDA() ) 
	{
		return;
	}
	
	// only update the focus character when attack button isn't pressed so players
	// can still chainsaw NPC's
	if ( common->IsMultiplayer() || ( !focusCharacter && (usercmd.buttons & BUTTON_ATTACK )) )
	{
		allowFocus = false;
	}
	else
	{
		allowFocus = true;
	}

	oldFocus = focusGUIent;
	oldUI = focusUI;
	oldChar = focusCharacter;
	oldTalkCursor = talkCursor;
	oldVehicle = focusVehicle;

	if ( focusTime <= gameLocal.time || commonVr->teleportButtonCount != 0) // Koz kill the focus and drop the hand if teleport pressed.
	{
		ClearFocus();
		raised = false;
		lowered = false;
		if ( commonVr->teleportButtonCount != 0 ) return;
	}

	// don't let spectators interact with GUIs
	if ( spectating )
	{
		return;
	}

	start = GetEyePosition();

	// Koz begin
	if ( game->isVR ) // Koz fixme only when vr actually active.
	{
		// Koz  in VR, if weapon equipped, use muzzle orientation to scan for accessible guis, 
		// otherwise use player center eye.
		
		scanFromWeap = hands[vr_weaponHand.GetInteger()].weapon->GetMuzzlePositionWithHacks( start, weaponAxis );
		if ( !scanFromWeap || vr_guiMode.GetInteger() == 1 || (  vr_guiMode.GetInteger() == 2 && commonVr->VR_USE_MOTION_CONTROLS ) ) // guiMode 2 = use guis as touch screen
		{
			//weapon has no muzzle ( fists, grenades, chainsaw) or we are using the guis as touchscreens so scan from center of view.
			start = commonVr->lastViewOrigin;
			weaponAxis = commonVr->lastViewAxis;
			
			scanRange += 36.0f;

			if ( vr_guiMode.GetInteger() == 2 )
			{
				
				weaponAxis = viewPitchAdj * weaponAxis; // add a little down pitch to help my neck.

				scanRange = 36.0f; // get pretty close for touchscreens
				// if the hand is already in the gui, or has completed the lower cycle after hitting a gui,
				// scan from the same point in space until the player exceeds movement threshold
				// this prevents getting kicked from a gui when just looking around
				if ( commonVr->handInGui || raised ) 
				{
					static idMat3 yawAdj;
					static idVec3 distMoved;
					scanRange += 4;
					yawAdj = idAngles( 0.0f, viewAngles.yaw - lastBodyYaw, 0.0f ).Normalize180().ToMat3(); 
					weaponAxis = yawAdj * lastScanAxis;
					distMoved = physicsObj.GetOrigin() - lastBodyPosition;
					if ( distMoved.Length() < 12.0f ) distMoved = vec3_zero;
					start = lastScanStart + distMoved;
									
				}
				else
				{
					lastScanStart = start;
					lastScanAxis = weaponAxis;
					lastBodyYaw = viewAngles.yaw;
					lastBodyPosition = physicsObj.GetOrigin();
				}
			}
			hmdAbsPitch = abs( commonVr->lastHMDViewAxis.ToAngles().Normalize180().pitch + vr_guiFocusPitchAdj.GetFloat() );
			if ( hmdAbsPitch > 60.0f ) hmdAbsPitch = 60.0f;
			scanRangeCorrected = scanRange / cos( DEG2RAD( hmdAbsPitch ) );
			scanRange = scanRangeCorrected;
		}
		else
		{
			// Koz - if weapon has been lowered (in gui), raise pointer to compensate.
			start.z -= hands[ vr_weaponHand.GetInteger() ].weapon->hideOffset;
		}
				
		end = start + weaponAxis[0] * scanRange;//  Koz originial value was 80.0f - allowed access to gui from too great a distance (IMO), reduced to 50.0f Koz fixme - make cvar?
	}
	else
	{
		end = start + firstPersonViewAxis[0] * 80.0f;
	}
	// Koz end
	if ( game->isVR && commonVr->VR_USE_MOTION_CONTROLS && vr_guiMode.GetInteger() == 2 )
	{
		talkScanEnd = start + weaponAxis[0] * (scanRange + 40);
	}
	else
	{
		talkScanEnd = end;
	}


	// player identification -> names to the hud
	if ( common->IsMultiplayer() && IsLocallyControlled() )
	{
		idVec3 end = start + viewAngles.ToForward() * 768.0f;
		gameLocal.clip.TracePoint( trace, start, end, MASK_SHOT_BOUNDINGBOX, this );
		int iclient = -1;
		if ( (trace.fraction < 1.0f) && (trace.c.entityNum < MAX_CLIENTS) )
		{
			iclient = trace.c.entityNum;
		}
		if ( MPAim != iclient )
		{
			lastMPAim = MPAim;
			MPAim = iclient;
			lastMPAimTime = gameLocal.realClientTime;
		}
	}
	
	idBounds bounds( start );
	bounds.AddPoint( end );
	
	listedClipModels = gameLocal.clip.ClipModelsTouchingBounds( bounds, -1, clipModelList, MAX_GENTITIES );
	// no pretense at sorting here, just assume that there will only be one active
	// gui within range along the trace

	if ( vr_debugGui.GetBool() )
	{
		gameRenderWorld->DebugLine( colorYellow, start, end, 20, true );
		common->Printf( "Handin gui %d raised %d lowered %d hideoffset %f\n", commonVr->handInGui, raised, lowered, hands[ vr_weaponHand.GetInteger() ].weapon->hideOffset );
	}

	for ( i = 0; i < listedClipModels; i++ )
	{
		clip = clipModelList[i];
		ent = clip->GetEntity();

		if ( ent->IsHidden() )
		{
			continue;
		}

		if ( allowFocus )
		{
			if ( ent->IsType( idAFAttachment::Type ) )
			{
				idEntity* body = static_cast<idAFAttachment*>(ent)->GetBody();
				if ( body != NULL && body->IsType( idAI::Type ) && (static_cast<idAI*>(body)->GetTalkState() >= TALK_OK) )
				{
					gameLocal.clip.TracePoint( trace, start, end, MASK_SHOT_RENDERMODEL, this );
					if ( (trace.fraction < 1.0f) && (trace.c.entityNum == ent->entityNumber) )
					{
						ClearFocus();
						focusCharacter = static_cast<idAI*>(body);
						talkCursor = 1;
						focusTime = gameLocal.time + FOCUS_TIME;
						break;
					}
				}
				continue;
			}

			if ( ent->IsType( idAI::Type ) )
			{
				if ( static_cast<idAI*>(ent)->GetTalkState() >= TALK_OK )
				{
					gameLocal.clip.TracePoint( trace, start, talkScanEnd /*end*/, MASK_SHOT_RENDERMODEL, this );
					if ( (trace.fraction < 1.0f) && (trace.c.entityNum == ent->entityNumber) )
					{
						ClearFocus();
						focusCharacter = static_cast<idAI*>(ent);
						talkCursor = 1;
						focusTime = gameLocal.time + FOCUS_TIME;
						break;
					}
				}
				continue;
			}

			if ( ent->IsType( idAFEntity_Vehicle::Type ) )
			{
				gameLocal.clip.TracePoint( trace, start, end, MASK_SHOT_RENDERMODEL, this );
				if ( (trace.fraction < 1.0f) && (trace.c.entityNum == ent->entityNumber) )
				{
					ClearFocus();
					focusVehicle = static_cast<idAFEntity_Vehicle*>(ent);
					focusTime = gameLocal.time + FOCUS_TIME;
					break;
				}
				continue;
			}
		}

		if ( (!ent->GetRenderEntity() || !ent->GetRenderEntity()->gui[0] || !ent->GetRenderEntity()->gui[0]->IsInteractive()) && (!game->IsPDAOpen() /* && !commonVr->PDAclipModelSet */) ) // Koz don't bail if the PDA is open and clipmodel is set.

		{
			continue;
		}

		if ( ent->spawnArgs.GetBool( "inv_item" ) )
		{
			// don't allow guis on pickup items focus
			continue;
		}
		
		int fingerHand = vr_weaponHand.GetInteger();

		// Koz : if the weapon is reloading, don't let the hand enter a gui, or the weapon anims
		// will still be driving the hand and it looks stupid.

		if ( hands[ fingerHand ].weapon->IsReloading() ) continue;

		// Koz : the shotgun reload script cycles through WP_RELOAD and WP_READY for each shell,
		// so we cant just check the reload state or else the hand will enter
		// the gui in between two shells loading.  Make sure when using the shotgun
		// that the idle animation is playing before entering a gui.  
		// fixme : this sucks - find a better way to check this.		
		if( hands[ fingerHand ].currentWeapon == weapon_shotgun )
		{
			if( idStr::Cmp( hands[ fingerHand ].weapon->GetAnimator()->CurrentAnim( ANIMCHANNEL_ALL )->AnimName(), "idle" ) != 0 ) continue;
		}


		ent->GetAnimator();
		pt = gameRenderWorld->GuiTrace( ent->GetModelDefHandle(), ent->GetAnimator(), start, end ); // Koz
		if ( pt.x != -1 )
		{
			// we have a hit
			renderEntity_t* focusGUIrenderEntity = ent->GetRenderEntity();
			if ( !focusGUIrenderEntity )
			{
				continue;
			}

			if ( pt.guiId == 1 )
			{
				ui = focusGUIrenderEntity->gui[0];
			}
			else if ( pt.guiId == 2 )
			{
				ui = focusGUIrenderEntity->gui[1];
			}
			else
			{
				ui = focusGUIrenderEntity->gui[2];
			}

			if ( ui == NULL )
			{
				continue;
			}

			ClearFocus();
			focusGUIent = ent;
			focusUI = ui;

			if ( oldFocus != ent )
			{
				// new activation
				// going to see if we have anything in inventory a gui might be interested in
				// need to enumerate inventory items
				
				focusUI->SetStateInt( "inv_count", inventory.items.Num() );
				for ( j = 0; j < inventory.items.Num(); j++ )
				{
					idDict* item = inventory.items[j];
					const char* iname = item->GetString( "inv_name" );
					const char* iicon = item->GetString( "inv_icon" );
					const char* itext = item->GetString( "inv_text" );

					focusUI->SetStateString( va( "inv_name_%i", j ), iname );
					focusUI->SetStateString( va( "inv_icon_%i", j ), iicon );
					focusUI->SetStateString( va( "inv_text_%i", j ), itext );
					kv = item->MatchPrefix( "inv_id", NULL );
					if ( kv )
					{
						focusUI->SetStateString( va( "inv_id_%i", j ), kv->GetValue() );
					}
					focusUI->SetStateInt( iname, 1 );
				}


				for ( j = 0; j < inventory.pdaSecurity.Num(); j++ )
				{
					const char* p = inventory.pdaSecurity[j];
					if ( p && *p )
					{
						focusUI->SetStateInt( p, 1 );
					}
				}

				int powerCellCount = 0;
				for ( j = 0; j < inventory.items.Num(); j++ )
				{
					idDict* item = inventory.items[j];
					if ( item->GetInt( "inv_powercell" ) )
					{
						powerCellCount++;
					}
				}
				focusUI->SetStateInt( "powercell_count", powerCellCount );

				int staminapercentage = (int)(100.0f * stamina / pm_stamina.GetFloat());
				focusUI->SetStateString( "player_health", va( "%i", health ) );
				focusUI->SetStateString( "player_stamina", va( "%i%%", staminapercentage ) );
				focusUI->SetStateString( "player_armor", va( "%i%%", inventory.armor ) );

				kv = focusGUIent->spawnArgs.MatchPrefix( "gui_parm", NULL );
				while ( kv )
				{
					focusUI->SetStateString( kv->GetKey(), kv->GetValue() );
					kv = focusGUIent->spawnArgs.MatchPrefix( "gui_parm", kv );
				}
			}

			if ( !game->isVR || !( game->isVR && ( vr_guiMode.GetInteger() == 2 && commonVr->VR_USE_MOTION_CONTROLS) ) )
			{
				// handle event normally
				// clamp the mouse to the corner
				ev = sys->GenerateMouseMoveEvent( -2000, -2000 );
				command = focusUI->HandleEvent( &ev, gameLocal.time );
				HandleGuiCommands( focusGUIent, command );

				// move to an absolute position
				ev = sys->GenerateMouseMoveEvent( pt.x * SCREEN_WIDTH, pt.y * SCREEN_HEIGHT );
				command = focusUI->HandleEvent( &ev, gameLocal.time );
				HandleGuiCommands( focusGUIent, command );

				focusTime = gameLocal.time + FOCUS_GUI_TIME;
				break;
			}
			else
			{
				// the game is vr, player is using motion controls, gui mode is set to use touchscreens
				// and the view has found a gui to interact with.
				// wait for lower weapon to drop the hand to hidedistance and hide the weapon model
				// then raise the empty hand with pointy finger back to original position
				int fingerHand = vr_weaponHand.GetInteger();
				idWeapon* fingerWeapon = hands[ fingerHand ].weapon.GetEntity();
				focusTime = gameLocal.time + FOCUS_GUI_TIME * 3 ;
				if ( !lowered )
				{
					focusTime = gameLocal.time + FOCUS_GUI_TIME * 3;
					if ( fingerWeapon->hideOffset != fingerWeapon->hideDistance ) break;
					lowered = true;
				}

				commonVr->handInGui = true;
								
				if ( !raised )
				{
					fingerWeapon->hideStart = fingerWeapon->hideDistance;
					fingerWeapon->hideEnd = 0.0f;
					if ( gameLocal.time - fingerWeapon->hideStartTime < fingerWeapon->hideTime )
					{
						fingerWeapon->hideStartTime = gameLocal.time - ( fingerWeapon->hideTime - (gameLocal.time - fingerWeapon->hideStartTime));
					}
					else
					{
						fingerWeapon->hideStartTime = gameLocal.time;
					}
					
					raised = true;
					focusTime = gameLocal.time + FOCUS_GUI_TIME * 3;
					break;
				}
				
				if ( raised == true && fingerWeapon->hideOffset != 0.0f )
				{
					focusTime = gameLocal.time + FOCUS_GUI_TIME;
					break;
				}
								

				// so now get current position of the pointy finger tip joint and
				// see if we have touched the gui

				gameLocal.clip.TracePoint( trace, start, end, MASK_SHOT_RENDERMODEL, this );
				
				if ( fingerHand == HAND_RIGHT )
				{
					fingerJoint = animator.GetJointHandle( "RindexTip" );
				}
				else
				{
					fingerJoint = animator.GetJointHandle( "LindexTip" );
				}

				surfaceNormal = -trace.c.normal;

				animator.GetJointTransform( fingerJoint, gameLocal.time, fingerPosLocal, fingerAxisLocal );
				fingerPosGlobal = fingerPosLocal * GetRenderEntity()->axis + GetRenderEntity()->origin;
							
				static int fingerForwDist = 1.0f;
				static int fingerBackwDist = 12.0f;

				scanStart = fingerPosGlobal - fingerBackwDist * surfaceNormal;
				scanEnd = fingerPosGlobal + fingerForwDist * surfaceNormal;
				
				//gameRenderWorld->DebugLine( colorRed, scanStart, scanEnd, 20 );
				
				focusTime = gameLocal.time + FOCUS_GUI_TIME;
				
				pt = gameRenderWorld->GuiTrace( focusGUIent->GetModelDefHandle(), focusGUIent->GetAnimator(), scanStart, scanEnd );
				
				if ( pt.fraction >= 1.0f ) // no hit if > = 1.0f 
				{
					//send mouse button up
					ev = sys->GenerateMouseButtonEvent( 1, false );
					command = focusUI->HandleEvent( &ev, gameLocal.time );
					HandleGuiCommands( focusGUIent, command );
					touching = false;
					break;
				}
				else
				{
					//we have a hit
					if ( touching )
					{
						if ( pt.fraction >= 0.94f )//  || pt.fraction < 0.875f ) // not touching any more was 0.965 and 0.0875 
						{
														
							//common->Printf( "Sending mouse off 1 fraction = %f\n",trace.fraction );
							ev = sys->GenerateMouseButtonEvent( 1, false );
							command = focusUI->HandleEvent( &ev, gameLocal.time );
							HandleGuiCommands( focusGUIent, command );
							touching = false;
							break;
						}
						else
						{
							if ( pt.x != -1 )
							{
								// clamp the mouse to the corner
								ev = sys->GenerateMouseMoveEvent( -2000, -2000 );
								command = focusUI->HandleEvent( &ev, gameLocal.time );
								HandleGuiCommands( focusGUIent, command );

								// move to an absolute position
								ev = sys->GenerateMouseMoveEvent( pt.x * SCREEN_WIDTH, pt.y * SCREEN_HEIGHT );
								command = focusUI->HandleEvent( &ev, gameLocal.time );
								HandleGuiCommands( focusGUIent, command );
								break;
							}
						}
					}

					else // if ( !touching )

					{
						//common->Printf( "Fraction = %f\n", pt.fraction );
						if ( pt.fraction < 0.94f )
						{
							//common->Printf( "Setting touching true\n" );
							touching = true;
							if ( pt.x != -1 )
							{
								// clamp the mouse to the corner
								ev = sys->GenerateMouseMoveEvent( -2000, -2000 );
								command = focusUI->HandleEvent( &ev, gameLocal.time );
								HandleGuiCommands( focusGUIent, command );

								// move to an absolute position
								ev = sys->GenerateMouseMoveEvent( pt.x * SCREEN_WIDTH, pt.y * SCREEN_HEIGHT );
								command = focusUI->HandleEvent( &ev, gameLocal.time );
								HandleGuiCommands( focusGUIent, command );
																
								//send mouse button down
								ev = sys->GenerateMouseButtonEvent( 1, true );
								command = focusUI->HandleEvent( &ev, gameLocal.time );
								HandleGuiCommands( focusGUIent, command );

								//Rumble the controller to let player know they scored a touch.
								//SetControllerShake( 0.5f, 100, 0.3f, 60 ); // these are the values from the machine gun
								hands[fingerHand].SetControllerShake( 0.1f, 12, 0.8f, 12 );
								
								focusTime = gameLocal.time + FOCUS_GUI_TIME;
								break;
								 
							}
						}
						/*
						else
						{
														
							//send mouse button up
							ev = sys->GenerateMouseButtonEvent( 1, false );
							command = focusUI->HandleEvent( &ev, gameLocal.time );
							HandleGuiCommands( focusGUIent, command );
							touching = false;
							break;
						}
						*/
					}
				}
			}
		}
	}

	if ( focusGUIent && focusUI )
	{
		if ( !oldFocus || oldFocus != focusGUIent )
		{
			command = focusUI->Activate( true, gameLocal.time );
			HandleGuiCommands( focusGUIent, command );
			StartSound( "snd_guienter", SND_CHANNEL_ANY, 0, false, NULL );
			// HideTip();
			// HideObjective();
		}
	}
	else if ( oldFocus && oldUI )
	{
			
		lowered = false;
		raised = false;

		command = oldUI->Activate( false, gameLocal.time );
		HandleGuiCommands( oldFocus, command );
		StartSound( "snd_guiexit", SND_CHANNEL_ANY, 0, false, NULL );
	}

	if ( hud )
	{
		hud->SetCursorState( this, CURSOR_TALK, talkCursor );
	}

	if ( oldChar != focusCharacter && hud )
	{
		if ( focusCharacter )
		{
			hud->SetCursorText( "#str_02036", focusCharacter->spawnArgs.GetString( "npc_name", "Joe" ) );
			hud->UpdateCursorState();
		}
		else
		{
			hud->SetCursorText( "", "" );
			hud->UpdateCursorState();
		}
	}
}

/*
=================
idPlayer::CrashLand

Check for hard landings that generate sound events
=================
*/
void idPlayer::CrashLand( const idVec3& oldOrigin, const idVec3& oldVelocity )
{
	idVec3		origin, velocity;
	idVec3		gravityVector, gravityNormal;
	float		delta;
	float		hardDelta, fatalDelta, softDelta;
	float		dist;
	float		vel, acc;
	float		t;
	float		a, b, c, den;
	waterLevel_t waterLevel;
	bool		noDamage;
	
	AI_SOFTLANDING = false;
	AI_HARDLANDING = false;
	
	// if the player is not on the ground
	if( !physicsObj.HasGroundContacts() )
	{
		return;
	}
	
	gravityNormal = physicsObj.GetGravityNormal();
	
	// if the player wasn't going down
	if( ( oldVelocity * -gravityNormal ) >= 0.0f )
	{
		return;
	}
	
	waterLevel = physicsObj.GetWaterLevel();
	
	// never take falling damage if completely underwater
	if( waterLevel == WATERLEVEL_HEAD )
	{
		return;
	}
	
	// no falling damage if touching a nodamage surface
	noDamage = false;
	for( int i = 0; i < physicsObj.GetNumContacts(); i++ )
	{
		const contactInfo_t& contact = physicsObj.GetContact( i );
		if( contact.material->GetSurfaceFlags() & SURF_NODAMAGE )
		{
			noDamage = true;
			StartSound( "snd_land_hard", SND_CHANNEL_ANY, 0, false, NULL );
			break;
		}
	}
	
	origin = GetPhysics()->GetOrigin();
	gravityVector = physicsObj.GetGravity();
	
	// calculate the exact velocity on landing
	dist = ( origin - oldOrigin ) * -gravityNormal;
	vel = oldVelocity * -gravityNormal;
	acc = -gravityVector.Length();
	
	a = acc / 2.0f;
	b = vel;
	c = -dist;
	
	den = b * b - 4.0f * a * c;
	if( den < 0 )
	{
		return;
	}
	t = ( -b - idMath::Sqrt( den ) ) / ( 2.0f * a );
	
	delta = vel + t * acc;
	delta = delta * delta * 0.0001;
	
	// reduce falling damage if there is standing water
	if( waterLevel == WATERLEVEL_WAIST )
	{
		delta *= 0.25f;
	}
	if( waterLevel == WATERLEVEL_FEET )
	{
		delta *= 0.5f;
	}
	
	if( delta < 1.0f )
	{
		return;
	}
	
	// allow falling a bit further for multiplayer
	if( common->IsMultiplayer() )
	{
		fatalDelta	= 75.0f;
		hardDelta	= 50.0f;
		softDelta	= 45.0f;
	}
	else
	{
		fatalDelta	= 65.0f;
		hardDelta	= 45.0f;
		softDelta	= 30.0f;
	}
	
	if( delta > fatalDelta )
	{
		AI_HARDLANDING = true;
		landChange = -32;
		landTime = gameLocal.time;
		if( !noDamage )
		{
			pain_debounce_time = gameLocal.time + pain_delay + 1;  // ignore pain since we'll play our landing anim
			Damage( NULL, NULL, idVec3( 0, 0, -1 ), "damage_fatalfall", 1.0f, 0 );
		}
	}
	else if( delta > hardDelta )
	{
		AI_HARDLANDING = true;
		landChange	= -24;
		landTime	= gameLocal.time;
		if( !noDamage )
		{
			pain_debounce_time = gameLocal.time + pain_delay + 1;  // ignore pain since we'll play our landing anim
			Damage( NULL, NULL, idVec3( 0, 0, -1 ), "damage_hardfall", 1.0f, 0 );
		}
	}
	else if( delta > softDelta )
	{
		AI_HARDLANDING = true;
		landChange	= -16;
		landTime	= gameLocal.time;
		if( !noDamage )
		{
			pain_debounce_time = gameLocal.time + pain_delay + 1;  // ignore pain since we'll play our landing anim
			Damage( NULL, NULL, idVec3( 0, 0, -1 ), "damage_softfall", 1.0f, 0 );
		}
	}
	else if( delta > 7 )
	{
		AI_SOFTLANDING = true;
		landChange	= -8;
		landTime	= gameLocal.time;
	}
	else if( delta > 3 )
	{
		// just walk on
	}
}

/*
===============
idPlayer::BobCycle
===============
*/
void idPlayer::BobCycle( const idVec3& pushVelocity )
{
	float		bobmove;
	int			old, deltaTime;
	idVec3		vel, gravityDir, velocity;
	idMat3		viewaxis;
	float		bob;
	float		delta;
	float		speed;
	float		f;
	
	//
	// calculate speed and cycle to be used for
	// all cyclic walking effects
	//
	velocity = physicsObj.GetLinearVelocity() - pushVelocity;
	
	gravityDir = physicsObj.GetGravityNormal();
	vel = velocity - ( velocity * gravityDir ) * gravityDir;
	xyspeed = vel.LengthFast();
	
	// do not evaluate the bob for other clients
	// when doing a spectate follow, don't do any weapon bobbing
	if( common->IsClient() && !IsLocallyControlled() )
	{
		viewBobAngles.Zero();
		viewBob.Zero();
		return;
	}
	
	if( !physicsObj.HasGroundContacts() || influenceActive == INFLUENCE_LEVEL2 || ( common->IsMultiplayer() && spectating ) )
	{
		// airborne
		bobCycle = 0;
		bobFoot = 0;
		bobfracsin = 0;
	}
	else if( ( !usercmd.forwardmove && !usercmd.rightmove ) || ( xyspeed <= MIN_BOB_SPEED ) )
	{
		// start at beginning of cycle again
		bobCycle = 0;
		bobFoot = 0;
		bobfracsin = 0;
	}
	else
	{
		if( physicsObj.IsCrouching() )
		{
			bobmove = pm_crouchbob.GetFloat();
			// ducked characters never play footsteps
		}
		else
		{
			// vary the bobbing based on the speed of the player
			bobmove = pm_walkbob.GetFloat() * ( 1.0f - bobFrac ) + pm_runbob.GetFloat() * bobFrac;
		}
		
		// check for footstep / splash sounds
		old = bobCycle;
		bobCycle = ( int )( old + bobmove * ( gameLocal.time - gameLocal.previousTime ) ) & 255;
		bobFoot = ( bobCycle & 128 ) >> 7;
		bobfracsin = idMath::Fabs( sin( ( bobCycle & 127 ) / 127.0 * idMath::PI ) );
	}
	
	// calculate angles for view bobbing
	viewBobAngles.Zero();
	
	viewaxis = viewAngles.ToMat3() * physicsObj.GetGravityAxis();
	
	// add angles based on velocity
	delta = velocity * viewaxis[0];
	viewBobAngles.pitch += delta * pm_runpitch.GetFloat();
	
	delta = velocity * viewaxis[1];
	viewBobAngles.roll -= delta * pm_runroll.GetFloat();
	
	// add angles based on bob
	// make sure the bob is visible even at low speeds
	speed = xyspeed > 200 ? xyspeed : 200;
	
	delta = bobfracsin * pm_bobpitch.GetFloat() * speed;
	if( physicsObj.IsCrouching() )
	{
		delta *= 3;		// crouching
	}
	viewBobAngles.pitch += delta;
	delta = bobfracsin * pm_bobroll.GetFloat() * speed;
	if( physicsObj.IsCrouching() )
	{
		delta *= 3;		// crouching accentuates roll
	}
	if( bobFoot & 1 )
	{
		delta = -delta;
	}
	viewBobAngles.roll += delta;
	
	// calculate position for view bobbing
	viewBob.Zero();
	
	if( physicsObj.HasSteppedUp() )
	{
	
		// check for stepping up before a previous step is completed
		deltaTime = gameLocal.time - stepUpTime;
		if( deltaTime < STEPUP_TIME )
		{
			stepUpDelta = stepUpDelta * ( STEPUP_TIME - deltaTime ) / STEPUP_TIME + physicsObj.GetStepUp();
		}
		else
		{
			stepUpDelta = physicsObj.GetStepUp();
		}
		if( stepUpDelta > 2.0f * pm_stepsize.GetFloat() )
		{
			stepUpDelta = 2.0f * pm_stepsize.GetFloat();
		}
		stepUpTime = gameLocal.time;
	}
	
	idVec3 gravity = physicsObj.GetGravityNormal();
	
	// if the player stepped up recently
	deltaTime = gameLocal.time - stepUpTime;
	if( deltaTime < STEPUP_TIME )
	{
		if ( game->isVR )
			viewBob += gravity * vr_stepSmooth.GetFloat() * ( stepUpDelta * ( STEPUP_TIME - deltaTime ) / STEPUP_TIME );
		else
			viewBob += gravity * ( stepUpDelta * ( STEPUP_TIME - deltaTime ) / STEPUP_TIME );
	}
	
	// add bob height after any movement smoothing
	bob = bobfracsin * xyspeed * pm_bobup.GetFloat();
	if( bob > 6 )
	{
		bob = 6;
	}
	viewBob[2] += bob;
	
	// add fall height
	delta = gameLocal.time - landTime;
	if( delta < LAND_DEFLECT_TIME )
	{
		f = delta / LAND_DEFLECT_TIME;
		if ( game->isVR )
			viewBob -= gravity * vr_jumpBounce.GetFloat() * ( landChange * f );
		else
			viewBob -= gravity * ( landChange * f );
	}
	else if( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME )
	{
		delta -= LAND_DEFLECT_TIME;
		f = 1.0 - ( delta / LAND_RETURN_TIME );
		if ( game->isVR )
			viewBob -= gravity * vr_jumpBounce.GetFloat() * ( landChange * f );
		else
			viewBob -= gravity * ( landChange * f );
	}
}

/*
================
idPlayer::UpdateDeltaViewAngles
================
*/
void idPlayer::UpdateDeltaViewAngles( const idAngles& angles )
{
	
	//if ( game->isVR ) return;

	// set the delta angle
	idAngles delta;
	for( int i = 0; i < 3; i++ )
	{
		delta[ i ] = angles[ i ] - SHORT2ANGLE( usercmd.angles[ i ] );
	}
	SetDeltaViewAngles( delta );
}

/*
================
idPlayer::SetViewAngles
================
*/
void idPlayer::SetViewAngles( const idAngles& angles )
{
	UpdateDeltaViewAngles( angles );
	viewAngles = angles;
}

/*
================
idPlayer::UpdateViewAngles
================
*/
void idPlayer::UpdateViewAngles()
{
	int i;
	idAngles delta;
	
	if( !noclip && ( gameLocal.inCinematic || privateCameraView || gameLocal.GetCamera() || influenceActive == INFLUENCE_LEVEL2 || objectiveSystemOpen ) )
	{
		// no view changes at all, but we still want to update the deltas or else when
		// we get out of this mode, our view will snap to a kind of random angle
		UpdateDeltaViewAngles( viewAngles );

		// Koz fixme - this was in tmeks fork, verify what we are doing here is still appropriate.
		for ( i = 0; i < 3; i++ )
		{
			cmdAngles[i] = SHORT2ANGLE( usercmd.angles[i] );
			if ( influenceActive == INFLUENCE_LEVEL3 )
			{
				viewAngles[i] += idMath::ClampFloat( -1.0f, 1.0f, idMath::AngleDelta( idMath::AngleNormalize180( SHORT2ANGLE( usercmd.angles[i] ) + deltaViewAngles[i] ), viewAngles[i] ) );
			}
			else
			{
				viewAngles[i] = idMath::AngleNormalize180( SHORT2ANGLE( usercmd.angles[i] ) + deltaViewAngles[i] );
			}
		}
		// Koz end

		return;
	}
	
	// if dead
	if( health <= 0 && !game->isVR ) // Carl: never roll or they'll get sick! Also don't steal control.  // Koz - only skip in vr
	{
		if( pm_thirdPersonDeath.GetBool() )
		{
			viewAngles.roll = 0.0f;
			viewAngles.pitch = 30.0f;
		}
		else
		{
			viewAngles.roll = 40.0f;
			viewAngles.pitch = -15.0f;
		}
		return;
	}
	
	//
	
	
	// circularly clamp the angles with deltas
	for( i = 0; i < 3; i++ )
	{
		cmdAngles[i] = SHORT2ANGLE( usercmd.angles[i] );
		if( influenceActive == INFLUENCE_LEVEL3 )
		{
			viewAngles[i] += idMath::ClampFloat( -1.0f, 1.0f, idMath::AngleDelta( idMath::AngleNormalize180( SHORT2ANGLE( usercmd.angles[i] ) + deltaViewAngles[i] ) , viewAngles[i] ) );
		}
		else
		{
			viewAngles[i] = idMath::AngleNormalize180( SHORT2ANGLE( usercmd.angles[i] ) + deltaViewAngles[i] );
		}
	}
	if( !centerView.IsDone( gameLocal.time ) )
	{
		viewAngles.pitch = centerView.GetCurrentValue( gameLocal.time );
	}
	
	// clamp the pitch
	if( noclip )
	{
		if( viewAngles.pitch > 89.0f )
		{
			// don't let the player look down more than 89 degrees while noclipping
			viewAngles.pitch = 89.0f;
		}
		else if( viewAngles.pitch < -89.0f )
		{
			// don't let the player look up more than 89 degrees while noclipping
			viewAngles.pitch = -89.0f;
		}
	}
	else if( mountedObject )
	{
		int yaw_min, yaw_max, varc;
		
		mountedObject->GetAngleRestrictions( yaw_min, yaw_max, varc );
		
		if( yaw_min < yaw_max )
		{
			viewAngles.yaw = idMath::ClampFloat( yaw_min, yaw_max, viewAngles.yaw );
		}
		else
		{
			if( viewAngles.yaw < 0 )
			{
				viewAngles.yaw = idMath::ClampFloat( -180.f, yaw_max, viewAngles.yaw );
			}
			else
			{
				viewAngles.yaw = idMath::ClampFloat( yaw_min, 180.f, viewAngles.yaw );
			}
		}
		viewAngles.pitch = idMath::ClampFloat( -varc, varc, viewAngles.pitch );
	}
	else
	{
		// don't let the player look up or down more than 90 degrees normally
		if ( !game->isVR ) // Koz skip check in vr
		{
			const float restrict = 1.0f;

			viewAngles.pitch = std::min( viewAngles.pitch, pm_maxviewpitch.GetFloat() * restrict );
			viewAngles.pitch = std::max( viewAngles.pitch, pm_minviewpitch.GetFloat() * restrict );
		}
	}
	
	UpdateDeltaViewAngles( viewAngles );
	
	// orient the model towards the direction we're looking
	SetAngles( idAngles( 0, viewAngles.yaw, 0 ) );
	
	// save in the log for analyzing weapon angle offsets
	loggedViewAngles[ gameLocal.framenum & ( NUM_LOGGED_VIEW_ANGLES - 1 ) ] = viewAngles;
}

/*
==============
idPlayer::AdjustHeartRate

Player heartrate works as follows

DEF_HEARTRATE is resting heartrate

Taking damage when health is above 75 adjusts heart rate by 1 beat per second
Taking damage when health is below 75 adjusts heart rate by 5 beats per second
Maximum heartrate from damage is MAX_HEARTRATE

Firing a weapon adds 1 beat per second up to a maximum of COMBAT_HEARTRATE

Being at less than 25% stamina adds 5 beats per second up to ZEROSTAMINA_HEARTRATE

All heartrates are target rates.. the heart rate will start falling as soon as there have been no adjustments for 5 seconds
Once it starts falling it always tries to get to DEF_HEARTRATE

The exception to the above rule is upon death at which point the rate is set to DYING_HEARTRATE and starts falling
immediately to zero

Heart rate volumes go from zero ( -40 db for DEF_HEARTRATE to 5 db for MAX_HEARTRATE ) the volume is
scaled linearly based on the actual rate

Exception to the above rule is once the player is dead, the dying heart rate starts at either the current volume if
it is audible or -10db and scales to 8db on the last few beats
==============
*/
void idPlayer::AdjustHeartRate( int target, float timeInSecs, float delay, bool force )
{

	if( heartInfo.GetEndValue() == target )
	{
		return;
	}
	
	if( AI_DEAD && !force )
	{
		return;
	}
	
	lastHeartAdjust = gameLocal.time;
	
	heartInfo.Init( gameLocal.time + delay * 1000, timeInSecs * 1000, heartRate, target );
}

/*
==============
idPlayer::GetBaseHeartRate
==============
*/
int idPlayer::GetBaseHeartRate()
{
	int base = idMath::Ftoi( ( BASE_HEARTRATE + LOWHEALTH_HEARTRATE_ADJ ) - ( ( float )health / 100.0f ) * LOWHEALTH_HEARTRATE_ADJ );
	int rate = idMath::Ftoi( base + ( ZEROSTAMINA_HEARTRATE - base ) * ( 1.0f - stamina / pm_stamina.GetFloat() ) );
	int diff = ( lastDmgTime ) ? gameLocal.time - lastDmgTime : 99999;
	rate += ( diff < 5000 ) ? ( diff < 2500 ) ? ( diff < 1000 ) ? 15 : 10 : 5 : 0;
	return rate;
}

/*
==============
idPlayer::SetCurrentHeartRate
==============
*/
void idPlayer::SetCurrentHeartRate()
{

	int base = idMath::Ftoi( ( BASE_HEARTRATE + LOWHEALTH_HEARTRATE_ADJ ) - ( ( float ) health / 100.0f ) * LOWHEALTH_HEARTRATE_ADJ );
	
	if( PowerUpActive( ADRENALINE ) )
	{
		heartRate = 135;
	}
	else
	{
		heartRate = idMath::Ftoi( heartInfo.GetCurrentValue( gameLocal.time ) );
		int currentRate = GetBaseHeartRate();
		if( health >= 0 && gameLocal.time > lastHeartAdjust + 2500 )
		{
			AdjustHeartRate( currentRate, 2.5f, 0.0f, false );
		}
	}
	
	int bps = idMath::Ftoi( 60.0f / heartRate * 1000.0f );
	if( gameLocal.time - lastHeartBeat > bps )
	{
		int dmgVol = DMG_VOLUME;
		int deathVol = DEATH_VOLUME;
		int zeroVol = ZERO_VOLUME;
		float pct = 0.0;
		if( heartRate > BASE_HEARTRATE && health > 0 )
		{
			pct = ( float )( heartRate - base ) / ( MAX_HEARTRATE - base );
			pct *= ( ( float )dmgVol - ( float )zeroVol );
		}
		else if( health <= 0 )
		{
			pct = ( float )( heartRate - DYING_HEARTRATE ) / ( BASE_HEARTRATE - DYING_HEARTRATE );
			if( pct > 1.0f )
			{
				pct = 1.0f;
			}
			else if( pct < 0.0f )
			{
				pct = 0.0f;
			}
			pct *= ( ( float )deathVol - ( float )zeroVol );
		}
		
		pct += ( float )zeroVol;
		
		if( pct != zeroVol )
		{
			StartSound( "snd_heartbeat", SND_CHANNEL_HEART, SSF_PRIVATE_SOUND, false, NULL );
			// modify just this channel to a custom volume
			soundShaderParms_t	parms;
			memset( &parms, 0, sizeof( parms ) );
			parms.volume = pct;
			refSound.referenceSound->ModifySound( SND_CHANNEL_HEART, &parms );
		}
		
		lastHeartBeat = gameLocal.time;
	}
}

/*
==============
idPlayer::UpdateAir
==============
*/
void idPlayer::UpdateAir()
{
	if( health <= 0 )
	{
		return;
	}
	
	// see if the player is connected to the info_vacuum
	bool	newAirless = false;
	
	if( gameLocal.vacuumAreaNum != -1 )
	{
		int	num = GetNumPVSAreas();
		if( num > 0 )
		{
			int		areaNum;
			
			// if the player box spans multiple areas, get the area from the origin point instead,
			// otherwise a rotating player box may poke into an outside area
			if( num == 1 )
			{
				const int*	pvsAreas = GetPVSAreas();
				areaNum = pvsAreas[0];
			}
			else
			{
				areaNum = gameRenderWorld->PointInArea( this->GetPhysics()->GetOrigin() );
			}
			newAirless = gameRenderWorld->AreasAreConnected( gameLocal.vacuumAreaNum, areaNum, PS_BLOCK_AIR );
		}
	}
	
	if( PowerUpActive( ENVIROTIME ) )
	{
		newAirless = false;
	}
	
	if( newAirless )
	{
		if( !airless )
		{
			StartSound( "snd_decompress", SND_CHANNEL_ANY, SSF_GLOBAL, false, NULL );
			StartSound( "snd_noAir", SND_CHANNEL_BODY2, 0, false, NULL );
		}
		airMsec -= ( gameLocal.time - gameLocal.previousTime );
		if( airMsec < 0 )
		{
			airMsec = 0;
			// check for damage
			const idDict* damageDef = gameLocal.FindEntityDefDict( "damage_noair", false );
			int dmgTiming = 1000 * ( ( damageDef ) ? damageDef->GetFloat( "delay", "3.0" ) : 3.0f );
			if( gameLocal.time > lastAirDamage + dmgTiming )
			{
				Damage( NULL, NULL, vec3_origin, "damage_noair", 1.0f, 0 );
				lastAirDamage = gameLocal.time;
			}
		}
		
	}
	else
	{
		if( airless )
		{
			StartSound( "snd_recompress", SND_CHANNEL_ANY, SSF_GLOBAL, false, NULL );
			StopSound( SND_CHANNEL_BODY2, false );
		}
		airMsec += ( gameLocal.time - gameLocal.previousTime );	// regain twice as fast as lose
		if( airMsec > pm_airMsec.GetInteger() )
		{
			airMsec = pm_airMsec.GetInteger();
		}
	}
	
	airless = newAirless;
	
	if( hud )
	{
		hud->UpdateOxygen( airless, 100 * airMsec / pm_airMsec.GetInteger() );
	}
}

void idPlayer::UpdatePowerupHud()
{

	if( health <= 0 )
	{
		return;
	}
	
	if( lastHudPowerup != hudPowerup )
	{
	
		if( hudPowerup == -1 )
		{
			//The powerup hud should be turned off
			// TODO_SPARTY: powerups??
			//if ( hud ) {
			//	hud->HandleNamedEvent( "noPowerup" );
			//}
		}
		else
		{
			//Turn the pwoerup hud on
			// TODO_SPARTY: powerups??
			//if ( hud ) {
			//	hud->HandleNamedEvent( "Powerup" );
			//}
		}
		
		lastHudPowerup = hudPowerup;
	}
	
	if( hudPowerup != -1 && hudPowerup < MAX_POWERUPS )
	{
		if( PowerUpActive( hudPowerup ) )
		{
			//int remaining = inventory.powerupEndTime[ hudPowerup ] - gameLocal.time;
			//int filledbar = idMath::ClampInt( 0, hudPowerupDuration, remaining );
			
			// TODO_SPARTY: powerups??
			//if ( hud ) {
			//	hud->SetStateInt( "player_powerup", 100 * filledbar / hudPowerupDuration );
			//	hud->SetStateInt( "player_poweruptime", remaining / 1000 );
			//}
		}
	}
}

/*
==============
idPlayer::GetPDA
==============
 */
const idDeclPDA* idPlayer::GetPDA() const
{
	if( inventory.pdas.Num() > 0 )
	{
		return inventory.pdas[ 0 ];
	}
	else
	{
		return NULL;
	}
}


/*
==============
idPlayer::GetVideo
==============
*/
const idDeclVideo* idPlayer::GetVideo( int index )
{
	if( index >= 0 && index < inventory.videos.Num() )
	{
		return inventory.videos[index];
	}
	return NULL;
}

/*
==============
idPlayer::TogglePDA
==============
*/
void idPlayer::TogglePDA( int hand )
{
	// Koz debug common->Printf( "Toggle PDA\n" );
	if( vr_debugHands.GetBool() )
	{
		common->Printf( "Before TogglePDA(%d):\n", hand );
		hands[hand].debugPrint();
		hands[1-hand].debugPrint();
	}

	// Koz begin : reset PDA controls
	commonVr->forceLeftStick = true;
	// Koz end
		
	if ( inventory.pdas.Num() == 0 )
	{
		if ( game->isVR )
		{
			// Koz : hack to allow the player to change system settings in the mars city level before the PDA is given by the receptionist.
			//GivePDA( NULL, NULL, false ); 
		}
		else
		{
			ShowTip( spawnArgs.GetString( "text_infoTitle" ), spawnArgs.GetString( "text_noPDA" ), true );
			return;
		}
	}
	
	if( pdaMenu != NULL )
	{
		SetupPDASlot( objectiveSystemOpen );
		SetupHolsterSlot( 1 - hand, objectiveSystemOpen );
		objectiveSystemOpen = !objectiveSystemOpen;
		pdaMenu->ActivateMenu( objectiveSystemOpen );
		
		if( objectiveSystemOpen )
		{
			if( hud )
			{
				hud->ClearNewPDAInfo();
			}
		}
	}
	if( vr_debugHands.GetBool() )
	{
		common->Printf( "After TogglePDA(%d):\n", hand );
		hands[hand].debugPrint();
		hands[1 - hand].debugPrint();
	}
}

/*
==============
idPlayer::Spectate
==============
*/
void idPlayer::Spectate( bool spectate, bool force )
{
	spectating = spectate;
	
	if( spectating )
	{
		// join the spectators
		ClearPowerUps();
		spectator = this->entityNumber;
		Init();
		StopRagdoll();
		SetPhysics( &physicsObj );
		physicsObj.DisableClip();
		FlashlightOff();
		Hide();
		Event_DisableWeapon();
		
		// Raise me up by a little bit. if i'm the local client.
		if( IsLocallyControlled() )
		{
			SetSpectateOrigin();
		}
		
		HideRespawnHudMessage();
		
		idLib::Printf( "DMP _ GENERAL :> Player %d Spectating \n", entityNumber );
	}
	else
	{
		// put everything back together again
		hands[ 0 ].currentWeapon = -1;	// to make sure the def will be loaded if necessary
		hands[ 1 ].currentWeapon = -1;
		Show();
		Event_EnableWeapon();
		idLib::Printf( "DMP _ GENERAL :> Player %d Not Spectating \n", entityNumber );
		SetEyeHeight( pm_normalviewheight.GetFloat() );
	}
	SetClipModel();
}

/*
==============
idPlayer::SetClipModel
==============
*/
void idPlayer::SetClipModel()
{
	idBounds bounds;
	
	if( spectating )
	{
		bounds = idBounds( vec3_origin ).Expand( pm_spectatebbox.GetFloat() * 0.5f );
	}
	else
	{
		bounds[0].Set( -pm_bboxwidth.GetFloat() * 0.5f, -pm_bboxwidth.GetFloat() * 0.5f, 0 );
		bounds[1].Set( pm_bboxwidth.GetFloat() * 0.5f, pm_bboxwidth.GetFloat() * 0.5f, pm_normalheight.GetFloat() );
	}
	// the origin of the clip model needs to be set before calling SetClipModel
	// otherwise our physics object's current origin value gets reset to 0
	idClipModel* newClip;
	if( pm_usecylinder.GetBool() )
	{
		newClip = new( TAG_PHYSICS_CLIP_ENTITY ) idClipModel( idTraceModel( bounds, 8 ) );
		newClip->Translate( physicsObj.PlayerGetOrigin() );
		physicsObj.SetClipModel( newClip, 1.0f );

		
	}
	else
	{
		newClip = new( TAG_PHYSICS_CLIP_ENTITY ) idClipModel( idTraceModel( bounds ) );
		newClip->Translate( physicsObj.PlayerGetOrigin() );
		physicsObj.SetClipModel( newClip, 1.0f );
	}

	if ( game->isVR )
	{
		commonVr->bodyClip = newClip;
		
		static idClipModel* newClip2;

		bounds[0].Set( -vr_headbbox.GetFloat() * 0.5f, -vr_headbbox.GetFloat() * 0.5f, 0 );
		bounds[1].Set( vr_headbbox.GetFloat() * 0.5f, vr_headbbox.GetFloat() * 0.5f, pm_normalheight.GetFloat() );
		
		if ( pm_usecylinder.GetBool() )
		{
			newClip2 = new(TAG_PHYSICS_CLIP_ENTITY)idClipModel( idTraceModel( bounds, 8 ) );
			newClip2->Translate( physicsObj.PlayerGetOrigin() );
			//physicsObj.SetClipModel( newClip2, 1.0f );
		}
		else
		{
			newClip2 = new(TAG_PHYSICS_CLIP_ENTITY)idClipModel( idTraceModel( bounds ) );
			newClip2->Translate( physicsObj.PlayerGetOrigin() );
			//physicsObj.SetClipModel( newClip2, 1.0f );
		}

		commonVr->headClip = newClip2;
		
	}
	
}

/*
==============
idPlayer::UseVehicle
==============
*/
void idPlayer::UseVehicle()
{
	trace_t	trace;
	idVec3 start, end;
	idEntity* ent;
	
	if( GetBindMaster() && GetBindMaster()->IsType( idAFEntity_Vehicle::Type ) )
	{
		Show();
		static_cast<idAFEntity_Vehicle*>( GetBindMaster() )->Use( this );
	}
	else
	{
		start = GetEyePosition();
		end = start + viewAngles.ToForward() * 80.0f;
		gameLocal.clip.TracePoint( trace, start, end, MASK_SHOT_RENDERMODEL, this );
		if( trace.fraction < 1.0f )
		{
			ent = gameLocal.entities[ trace.c.entityNum ];
			if( ent && ent->IsType( idAFEntity_Vehicle::Type ) )
			{
				Hide();
				static_cast<idAFEntity_Vehicle*>( ent )->Use( this );
			}
		}
	}
}

/*
==============
idPlayer::PerformImpulse
==============
*/
void idPlayer::PerformImpulse( int impulse )
{
	bool isIntroMap = ( idStr::FindText( gameLocal.GetMapFileName(), "mars_city1" ) >= 0 );

	// Normal 1 - 0 Keys.
	if ( impulse >= IMPULSE_0 && impulse <= IMPULSE_12 && !isIntroMap ) // Koz dont change weapons if in gui
	{
		// Carl: impulse 1, 4, and 11 were unbound, so I'm using them for specific versions of weapons
		if (impulse == 1)
			SelectWeapon( weapon_grabber, false, true ); // not chainsaw or fists (was originally chainsaw/grabber)
		else if (impulse == 4)
			SelectWeapon( weapon_shotgun_double, false, true ); // double barreled (this was already the super shotgun)
		else if (impulse == 11)
			SelectWeapon( weapon_shotgun, false, true ); // single barreled (this was empty before)
		else
			SelectWeapon( impulse, false );
		return;
	}
	
	// DPAD Weapon Quick Select
	if( impulse >= IMPULSE_28 && impulse <= IMPULSE_31 && !isIntroMap )
	{
		SelectWeapon( impulse, false );
		return;
	}
	
	switch( impulse )
	{
		case IMPULSE_13:
		{
			Reload();
			break;
		}
		case IMPULSE_14:
		{
			if( !isIntroMap )
			{
				NextWeapon();
			}
			break;
		}
		case IMPULSE_15:
		{
			if( !isIntroMap )
			{
				PrevWeapon();
			}
			break;
		}
		case IMPULSE_16:
		{
			if( flashlight.IsValid() )
			{
				if( flashlight.GetEntity()->lightOn )
				{
					FlashlightOff();
				}
				else if( !spectating && weaponEnabled && !hiddenWeapon && !gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) )
				{
					FlashlightOn();
				}
			}
			break;
		}
		case IMPULSE_19:
		{
			
			// when we're not in single player, IMPULSE_19 is used for showScores
			// otherwise it opens the pda
			if( !common->IsMultiplayer() )
			{
#if !defined(ID_RETAIL) && !defined(ID_RETAIL_INTERNAL)
				if( !common->KeyState( 56 ) )  		// don't toggle PDA when LEFT ALT is down
				{
#endif
					if( objectiveSystemOpen )
					{
						common->Printf( "idPlayer::PerformImpulse calling TogglePDA\n" );
						TogglePDA( 1 - vr_weaponHand.GetInteger() );
					}
					else if( weapon_pda >= 0 && inventory.pdas.Num() )
					{
						
						common->Printf( "idPlayer::PerformImpulse  calling Select Weapon for PDA\n" );
						commonVr->pdaToggleTime = Sys_Milliseconds();
						SetupPDASlot( false );
						SetupHolsterSlot( vr_weaponHand.GetInteger(), false );
						hands[ 1 - vr_weaponHand.GetInteger() ].SelectWeapon(weapon_pda, true, false);
					}
#if !defined(ID_RETAIL) && !defined(ID_RETAIL_INTERNAL)
				}
#endif
			}
			else
			{
				if( IsLocallyControlled() )
				{
					gameLocal.mpGame.SetScoreboardActive( true );
				}
			}
			break;
		}
		case IMPULSE_22:
		{
			if( common->IsClient() || IsLocallyControlled() )
			{
				gameLocal.mpGame.ToggleSpectate();
			}
			break;
		}
		case IMPULSE_25:
		{
			if( common->IsServer() && gameLocal.mpGame.IsGametypeFlagBased() && ( gameLocal.serverInfo.GetInt( "si_midnight" ) == 2 ) )
			{
				if( enviroSuitLight.IsValid() )
				{
					enviroSuitLight.GetEntity()->PostEventMS( &EV_Remove, 0 );
					enviroSuitLight = NULL;
				}
				else
				{
					const idDict* lightDef = gameLocal.FindEntityDefDict( "envirosuit_light", false );
					if( lightDef )
					{
						idEntity* temp = static_cast<idEntity*>( enviroSuitLight.GetEntity() );
						idAngles lightAng = firstPersonViewAxis.ToAngles();
						idVec3 lightOrg = firstPersonViewOrigin;
						
						idVec3 enviroOffset = lightDef->GetVector( "enviro_offset" );
						idVec3 enviroAngleOffset = lightDef->GetVector( "enviro_angle_offset" );
						
						gameLocal.SpawnEntityDef( *lightDef, &temp, false );
						enviroSuitLight = static_cast<idLight*>( temp );
						
						enviroSuitLight.GetEntity()->fl.networkSync = true;
						
						lightOrg += ( enviroOffset.x * firstPersonViewAxis[0] );
						lightOrg += ( enviroOffset.y * firstPersonViewAxis[1] );
						lightOrg += ( enviroOffset.z * firstPersonViewAxis[2] );
						lightAng.pitch += enviroAngleOffset.x;
						lightAng.yaw += enviroAngleOffset.y;
						lightAng.roll += enviroAngleOffset.z;
						
						enviroSuitLight.GetEntity()->GetPhysics()->SetOrigin( lightOrg );
						enviroSuitLight.GetEntity()->GetPhysics()->SetAxis( lightAng.ToMat3() );
						
						enviroSuitLight.GetEntity()->UpdateVisuals();
						enviroSuitLight.GetEntity()->Present();
					}
				}
			}
			break;
		}
		// Carl specific fists weapon
		case IMPULSE_26:
		{
			if ( !isIntroMap )
				SelectWeapon( weapon_fists, false, true );
			break;
		}
		//Hack so the chainsaw will work in MP
		case IMPULSE_27:
		{
			SelectWeapon( weapon_chainsaw, false, true );
			break;
		}
		// Carl:
		case IMPULSE_SOULCUBE:
		{
			if (!isIntroMap)
				SelectWeapon(weapon_soulcube, false, true);
			break;
		}
		// Carl:
		case IMPULSE_ARTIFACT:
		{
			// The Artifact
			if (!isIntroMap)
				SelectWeapon(weapon_bloodstone, false, false); // we want it to toggle through to the correct active version
			break;
		}

				
		// Koz Begin
		case IMPULSE_32: // HMD/Body orienataion reset. Make fisrtperson axis match view direction.
		{
			OrientHMDBody();
			break;
		}

		case IMPULSE_33: // toggle lasersight on/off.
		{
			ToggleLaserSight();
			break;
		}

		case IMPULSE_34: // comfort turn right
		{
			// Koz fixme
			// this performs comfort turns for key input,
			// really need to move this to usercmdgen
			/*
			static idAngles angles;
			angles.Set( 0.0f, vr_comfortDelta.GetFloat(), 0.0f );
			SetDeltaViewAngles( deltaViewAngles - angles );
			break;
			*/
		}
		
		case IMPULSE_35: // comfort turn left
		{
			// Koz fixme
			// this performs comfort turns for key input,
			// really need to move this to usercmdgen
			/*
			static idAngles angles;
			angles.Set( 0.0f, vr_comfortDelta.GetFloat(), 0.0f );
			SetDeltaViewAngles( deltaViewAngles + angles );
			break;
			*/
		}
		
		case IMPULSE_36: //  Toggle Hud
		{
			ToggleHud();
			break;
		}

		case IMPULSE_37: //  toggle heading beam
		{ 
			ToggleHeadingBeam();
			break;
		}

		case IMPULSE_39:// next flashlight mode
		{
			commonVr->NextFlashlightMode();
			break;
		}

		case IMPULSE_40:
		{
			//bring up the system menu
			
			if ( !( vr_guiMode.GetInteger() == 2 && commonVr->handInGui ) && ( Sys_Milliseconds() - commonVr->pdaToggleTime > 3000 ) )
			{
#ifdef _WIN32 //TODO: FIX
				void Sys_QueEvent( sysEventType_t type, int value, int value2, int ptrLength, void *ptr, int inputDeviceNum );
				Sys_QueEvent( SE_KEY, K_ESCAPE, 1, 0, NULL, 0 );
#endif
			}
			break;
		}
		// Koz end
		// Carl:
		case IMPULSE_PAUSE:
		{
			if ( gameLocal.inCinematic || Flicksync_InCutscene )
			{

			}
			else if ( commonVr->PDAforced && !commonVr->PDAforcetoggle )
			{
				// If we're in the menu, just exit
				PerformImpulse( 40 );
			}
			else
			{
				g_stopTime.SetBool( !g_stopTime.GetBool() );
			}
			break;
		}
		case IMPULSE_RESUME:
		{
			if ( gameLocal.inCinematic || Flicksync_InCutscene )
			{

			}
			else
			{
				g_stopTime.SetBool( false );
				if ( objectiveSystemOpen || ( commonVr->PDAforced && !commonVr->PDAforcetoggle ))
				{
					// If we're in the menu, just exit
					PerformImpulse( 40 );
				}
			}
			break;
		}

	}
}

// Koz Begin

/*
==============
idPlayer::ToggleLaserSight  Koz toggle the lasersight
==============
*/
void idPlayer::ToggleLaserSight()
{
	if( !hands[ 0 ].laserSightActive || !hands[ 1 ].laserSightActive )
	{
		hands[ 0 ].laserSightActive = true;
		hands[ 1 ].laserSightActive = true;
	}
	else
	{
		hands[ 0 ].laserSightActive = false;
		hands[ 1 ].laserSightActive = false;
	}
}

/*
==============
idPlayer::ToggleHeadingBeam  Koz toggle heading beam
==============
*/
void idPlayer::ToggleHeadingBeam()
{
	if ( vr_headingBeamMode.GetInteger() != 0 )
	{
		headingBeamActive = !headingBeamActive;
	}
}

/*
==============
idPlayer::ToggleHud  Koz toggle hud
==============
*/
void idPlayer::ToggleHud()
{
	hudActive = !hudActive;
}


void idPlayer::SnapBodyToView()
{
	idAngles newBodyAngles;
	newBodyAngles = viewAngles;
	newBodyAngles.pitch = 0;
	newBodyAngles.roll = 0;
	newBodyAngles.yaw += commonVr->lastHMDYaw - commonVr->bodyYawOffset;
	newBodyAngles.Normalize180();
	SetViewAngles( newBodyAngles );
}
/*
==============
idPlayer::OrientHMDBody  Koz align the body with the view  ( move the body to point the same direction as the HMD view - does not change the view. ) 
==============
*/
void idPlayer::OrientHMDBody()
{
	SnapBodyToView();
	commonVr->bodyYawOffset = 0;
	commonVr->lastHMDYaw = 0;
	commonVr->HMDResetTrackingOriginOffset();
	commonVr->MotionControlSetOffset();
	commonVr->bodyMoveAng = 0.0f;
}

// Koz end

/*
==============
idPlayer::EvaluateControls
==============
*/
void idPlayer::EvaluateControls()
{
	
	static float strafeHiThresh = commonVr->hasOculusRift ? 0.3f : 0.6f;
	static float strafeLoThresh = commonVr->hasOculusRift ? 0.25f : 0.3f;

	static bool lastOculusStrafe = false;
	static bool currentOculusStrafe = false;

	// check for respawning
	if( health <= 0 && !g_testDeath.GetBool() )
	{
		if( common->IsMultiplayer() )
		{
			// in MP, idMultiplayerGame decides spawns
			if( ( gameLocal.time > minRespawnTime ) && ( usercmd.buttons & BUTTON_ATTACK ) )
			{
				forceRespawn = true;
			}
			else if( gameLocal.time > maxRespawnTime )
			{
				forceRespawn = true;
			}
		}
		else
		{
			// in single player, we let the session handle restarting the level or loading a game
			if( gameLocal.time > minRespawnTime )
			{
				gameLocal.sessionCommand = "died";
				
				if ( game->isVR )
				{
					commonVr->wasLoaded = false;
					commonVr->playerDead = true;
					extern idCVar timescale;
					int comfortMode = vr_motionSickness.GetInteger();
					if ( ( comfortMode == 6 ) || ( comfortMode == 7 ) || ( comfortMode == 8 ) || ( comfortMode == 9 ) || warpAim || warpMove ) timescale.SetFloat( 1 );
				}
		
			}
		}
	}

	if( usercmd.impulseSequence != oldImpulseSequence )
	{
		PerformImpulse( usercmd.impulse );
	}
	
	for( int h = 0; h < 2; h++ )
	{
		if( hands[ h ].thumbDown && !hands[ h ].oldThumbDown && ( hands[ h ].holdingFlashlight() || ( hands[ h ].isOverMountedFlashlight() && !hands[ h ].tooFullToInteract() ) ) )
		{
			PerformImpulse( 16 ); // toggle flashlight
		}
		hands[ h ].oldThumbDown = hands[ h ].thumbDown;
	}

	bool doTeleport = false;

	currentOculusStrafe = (commonVr->hasOculusRift && (vr_teleportMode.GetInteger() == 2 ) && ((fabs( commonVr->leftMapped.x ) > strafeHiThresh) || (fabs( commonVr->leftMapped.y ) > strafeHiThresh)));

	extern idCVar timescale;

	if ( game->IsPDAOpen() || commonVr->VR_GAME_PAUSED || hands[ 0 ].currentWeapon == weapon_pda || hands[ 1 ].currentWeapon == weapon_pda || commonVr->PDAforcetoggle ) // no teleporting in these cases
	{
		commonVr->teleportButtonCount = 0;
	}
	else
	{
		
		if ( (common->ButtonState( UB_TELEPORT ) && !commonVr->oldTeleportButtonState) || ( currentOculusStrafe && !lastOculusStrafe )) // on transit from no press to press.
		{
			if ( vr_teleportMode.GetInteger() != 0 ) // Doom VFR style
			{
				if ( ((commonVr->leftMapped.x > -0.5f) && (commonVr->leftMapped.x < 0.5f) && (commonVr->leftMapped.y > -0.5f) && (commonVr->leftMapped.y < 0.5f) && !commonVr->hasOculusRift ) || commonVr->hasOculusRift && common->ButtonState( UB_TELEPORT ) )
				{
					warpAim = true;
					timescale.SetFloat(0.5f);
					commonVr->teleportButtonCount++;
				} 
				else if ( vr_teleportMode.GetInteger() == 2 )
				{
					
					if (!jetMove && gameLocal.time > jetMoveCoolDownTime) {
					
						// tie to the teleport button
											
						if ( commonVr->leftMapped.y < -strafeHiThresh && commonVr->leftMapped.x > -strafeLoThresh && commonVr->leftMapped.x < strafeLoThresh )
						{
							//common->Printf( "Jet Strafe forward\n" );
							jetMove = true;
							jetMoveTime = gameLocal.time + 60;
							idVec3 vf = physicsObj.viewForward; 
							vf.z = 0.0f;// Koz set vertical vel to 0, otherwise jetstraffing down a small incline kills you when you hit the floor due to high impact vel.

							jetMoveVel = ( vf * (100.0f)) / 0.060f;  // 60 ms
						}
						else if ( commonVr->leftMapped.y > strafeHiThresh && commonVr->leftMapped.x > -strafeLoThresh && commonVr->leftMapped.x < strafeLoThresh )
						{
							//common->Printf( "Jet Strafe Back\n" );
							jetMove = true;
							jetMoveTime = gameLocal.time + 60;
							idVec3 vf = physicsObj.viewForward;
							vf.z = 0.0f;
							jetMoveVel = ( vf  * (-100.0f)) / 0.060f;  // 60 ms
						}
						else if ( commonVr->leftMapped.x < -strafeHiThresh && commonVr->leftMapped.y > -strafeLoThresh && commonVr->leftMapped.y < strafeLoThresh )
						{
							//common->Printf( "Jet Strafe Left\n" );
							jetMove = true;
							jetMoveTime = gameLocal.time + 60;
							idVec3 vf = physicsObj.viewRight;
							vf.z = 0.0f;
							jetMoveVel = ( vf * (-100.0f)) / 0.060f;  // 60 ms
						}
						else if ( commonVr->leftMapped.x > strafeHiThresh && commonVr->leftMapped.y > -strafeLoThresh && commonVr->leftMapped.y < strafeLoThresh )
						{
							//common->Printf( "Jet Strafe Right\n" );
							jetMove = true;
							jetMoveTime = gameLocal.time + 60;
							idVec3 vf = physicsObj.viewRight;
							vf.z = 0.0f;
							jetMoveVel = ( vf * (100.0f)) / 0.060f;  // 60 ms
						}
					}
				}
			}
			else
			{
				commonVr->teleportButtonCount++;
			}
		}

		// Jack: do not cancel if Doom VFR style
		if ( usercmd.buttons & BUTTON_ATTACK && vr_teleportMode.GetInteger() == 0 )
		{
			commonVr->teleportButtonCount = 0; // let the fire button abort teleporting.
		}

		if ( ( vr_teleport.GetInteger() == 1 && commonVr->VR_USE_MOTION_CONTROLS && commonVr->teleportButtonCount != 0 ) ||
			( commonVr->teleportButtonCount > 1 ) ||
			( ( commonVr->oldTeleportButtonState && !common->ButtonState( UB_TELEPORT ) ) && !vr_teleportButtonMode.GetBool() )
			|| lastOculusStrafe && currentOculusStrafe && !!vr_teleportButtonMode.GetBool() ) // on transit from press to release
		{
			
			if ( vr_teleportMode.GetInteger() == 2 )
			{
				warpAim = false;
				timescale.SetFloat( 1.0f );
				// if touch within the map teleport boundary
				if ( ( commonVr->leftMapped.x > -0.5f ) && ( commonVr->leftMapped.x < 0.5f ) && ( commonVr->leftMapped.y > -0.5f ) && ( commonVr->leftMapped.y < 0.5f ) ) {
					doTeleport = true;  
					jetMoveCoolDownTime = 0;
				}
				commonVr->teleportButtonCount = 0;
			} 
			else
			{
				doTeleport = true;  
			}
		}
		commonVr->oldTeleportButtonState = common->ButtonState( UB_TELEPORT );
		lastOculusStrafe = currentOculusStrafe;
	}
	
	bool didTeleport = false;
	
	if( doTeleport )
	{
		commonVr->teleportButtonCount = 0;
		timescale.SetFloat( 1.0f ); // Koz make sure timescale is returned to normal if teleport was aborted or invalid target.
		
		// teleport
		if ( aimValidForTeleport )
		{
			aimValidForTeleport = false;
			int t = vr_teleport.GetInteger();
			if( t > 0 )
			{
				if ( vr_teleportMode.GetInteger() == 0 ) 
				{
					playerView.Flash( colorBlack, 140 );
				}
				
				TeleportPath( teleportPoint );
								
				// Koz begin
				// some of the entities in the game, like the sentry bot and the scientist in alpha2, want you to follow them.
				// If you fall too far behind, they will stop, and they monitor your direction of movement and velocity
				// to determine if you are moving towards them before they start moving again.
				// Teleporting imparts no velocity to the player, so those entities will not start moving again. ( unless you teleport way in front of them )
				// Impart a tiny velocity here, just enough so the entities know you have moved, not enough to really change the player position.
				
				didTeleport = true;
				physicsObj.SetLinearVelocity( physicsObj.GetLinearVelocity() - teleportDir * 10.0f );
				// Koz end
				
				PlayFootStepSound();
			}
		}
	}

	if( forceScoreBoard )
	{
		gameLocal.mpGame.SetScoreboardActive( true );
	}
	
	oldImpulseSequence = usercmd.impulseSequence;
	
	AdjustSpeed();
	
	// update the viewangles
	UpdateViewAngles();
	
	if ( didTeleport && vr_teleportMode.GetInteger() == 0 )
	{
		commonVr->didTeleport = true;
		commonVr->teleportDir = ((teleportTarget.GetEntity()->GetRenderEntity()->origin - teleportPoint).ToAngles().yaw - 180.0f) - viewAngles.yaw;
	}
	

}

/*
==============
idPlayer::AdjustSpeed
==============
*/
void idPlayer::AdjustSpeed()
{
	float speed;
	float rate;
	
	if( spectating )
	{
		speed = pm_spectatespeed.GetFloat();
		bobFrac = 0.0f;
	}
	else if( noclip )
	{
		speed = pm_noclipspeed.GetFloat();
		bobFrac = 0.0f;
	}
	else if( !physicsObj.OnLadder() && ( usercmd.buttons & BUTTON_RUN ) && ( usercmd.forwardmove || usercmd.rightmove ) && !( usercmd.buttons & BUTTON_CROUCH ) )
	{
		if( !common->IsMultiplayer() && !physicsObj.IsCrouching() && !PowerUpActive( ADRENALINE ) )
		{
			stamina -= MS2SEC( gameLocal.time - gameLocal.previousTime );
		}
		if( stamina < 0 )
		{
			stamina = 0;
		}
		if( ( !pm_stamina.GetFloat() ) || ( stamina > pm_staminathreshold.GetFloat() ) )
		{
			bobFrac = 1.0f;
		}
		else if( pm_staminathreshold.GetFloat() <= 0.0001f )
		{
			bobFrac = 0.0f;
		}
		else
		{
			bobFrac = stamina / pm_staminathreshold.GetFloat();
		}
		if ( game->isVR )
		{
			speed = ( pm_walkspeed.GetFloat() + vr_walkSpeedAdjust.GetFloat() ) * (1.0f - bobFrac) + pm_runspeed.GetFloat() * bobFrac;
		}
		else
		{
			speed = pm_walkspeed.GetFloat() * (1.0f - bobFrac) + pm_runspeed.GetFloat() * bobFrac;
		}
	}
	else
	{
		rate = pm_staminarate.GetFloat();
		
		// increase 25% faster when not moving
		if( ( usercmd.forwardmove == 0 ) && ( usercmd.rightmove == 0 ) && ( !physicsObj.OnLadder() || ( ( usercmd.buttons & ( BUTTON_CROUCH | BUTTON_JUMP ) ) == 0 ) ) )
		{
			rate *= 1.25f;
		}
		
		stamina += rate * MS2SEC( gameLocal.time - gameLocal.previousTime );
		if( stamina > pm_stamina.GetFloat() )
		{
			stamina = pm_stamina.GetFloat();
		}
		
		if ( game->isVR )
		{
			speed = pm_walkspeed.GetFloat() + vr_walkSpeedAdjust.GetFloat();
		}
		else
		{
			speed = pm_walkspeed.GetFloat();
		}

		bobFrac = 0.0f;
	}
	
	speed *= PowerUpModifier( SPEED );
	
	if( influenceActive == INFLUENCE_LEVEL3 )
	{
		speed *= 0.33f;
	}
	
	physicsObj.SetSpeed( speed, pm_crouchspeed.GetFloat() );
}

/*
==============
idPlayer::AdjustBodyAngles
==============
*/
void idPlayer::AdjustBodyAngles()
{
	idMat3	lookAxis;
	idMat3	legsAxis;
	bool	blend;
	float	diff;
	float	frac;
	float	upBlend;
	float	forwardBlend;
	float	downBlend;
	
	if( health < 0 )
	{
		return;
	}
	
	blend = true;
	
	if( !physicsObj.HasGroundContacts() )
	{
		idealLegsYaw = 0.0f;
		legsForward = true;
	}
	else if( usercmd.forwardmove < 0 )
	{
		idealLegsYaw = idMath::AngleNormalize180( idVec3( -usercmd.forwardmove, usercmd.rightmove, 0.0f ).ToYaw() );
		legsForward = false;
	}
	else if( usercmd.forwardmove > 0 )
	{
		idealLegsYaw = idMath::AngleNormalize180( idVec3( usercmd.forwardmove, -usercmd.rightmove, 0.0f ).ToYaw() );
		legsForward = true;
	}
	else if( ( usercmd.rightmove != 0 ) && physicsObj.IsCrouching() )
	{
		if( !legsForward )
		{
			idealLegsYaw = idMath::AngleNormalize180( idVec3( idMath::Abs( usercmd.rightmove ), usercmd.rightmove, 0.0f ).ToYaw() );
		}
		else
		{
			idealLegsYaw = idMath::AngleNormalize180( idVec3( idMath::Abs( usercmd.rightmove ), -usercmd.rightmove, 0.0f ).ToYaw() );
		}
	}
	else if( usercmd.rightmove != 0 )
	{
		idealLegsYaw = 0.0f;
		legsForward = true;
	}
	else
	{
		legsForward = true;
		diff = idMath::Fabs( idealLegsYaw - legsYaw );
		idealLegsYaw = idealLegsYaw - idMath::AngleNormalize180( viewAngles.yaw - oldViewYaw );
		if( diff < 0.1f )
		{
			legsYaw = idealLegsYaw;
			blend = false;
		}
	}
	
	if( !physicsObj.IsCrouching() )
	{
		legsForward = true;
	}
	
	oldViewYaw = viewAngles.yaw;
	
	AI_TURN_LEFT = false;
	AI_TURN_RIGHT = false;
	if( idealLegsYaw < -45.0f )
	{
		idealLegsYaw = 0;
		AI_TURN_RIGHT = true;
		blend = true;
	}
	else if( idealLegsYaw > 45.0f )
	{
		idealLegsYaw = 0;
		AI_TURN_LEFT = true;
		blend = true;
	}
	
	if( blend )
	{
		legsYaw = legsYaw * 0.9f + idealLegsYaw * 0.1f;
	}
	legsAxis = idAngles( 0.0f, legsYaw, 0.0f ).ToMat3();
	animator.SetJointAxis( hipJoint, JOINTMOD_WORLD, legsAxis );
	
	// calculate the blending between down, straight, and up
	frac = viewAngles.pitch / 90.0f;
	
	if ( game->isVR ) // Koz fixme check this way in vr only.
	{
		//mmdanggg2: stop the model from bending down and getting in the way!!

		if ( frac > 0.0f ) {
			downBlend = 0.0f;	// frac;
			forwardBlend = 1.0f;// -frac;
			upBlend = 0.0f;
		}
		else {
			downBlend = 0.0f;
			forwardBlend = 1.0f;// +frac;
			upBlend = 0.0f;		// -frac;
		}
	}
	else
	{

		if ( frac > 0.0f )
		{
			downBlend = frac;
			forwardBlend = 1.0f - frac;
			upBlend = 0.0f;
		}
		else
		{
			downBlend = 0.0f;
			forwardBlend = 1.0f + frac;
			upBlend = -frac;
		}
	}

	// Koz end
	animator.CurrentAnim( ANIMCHANNEL_TORSO )->SetSyncedAnimWeight( 0, downBlend );
	animator.CurrentAnim( ANIMCHANNEL_TORSO )->SetSyncedAnimWeight( 1, forwardBlend );
	animator.CurrentAnim( ANIMCHANNEL_TORSO )->SetSyncedAnimWeight( 2, upBlend );
	
	animator.CurrentAnim( ANIMCHANNEL_LEGS )->SetSyncedAnimWeight( 0, downBlend );
	animator.CurrentAnim( ANIMCHANNEL_LEGS )->SetSyncedAnimWeight( 1, forwardBlend );
	animator.CurrentAnim( ANIMCHANNEL_LEGS )->SetSyncedAnimWeight( 2, upBlend );
}

/* Carl
=====================
idPlayer::SetAAS
=====================
*/
void idPlayer::SetAAS( bool forceAAS48 )
{
	idStr use_aas;

	spawnArgs.GetString("use_aas", NULL, use_aas);
	gameLocal.Printf("Player AAS: use_aas = %s\n", use_aas.c_str());
	aas = gameLocal.GetAAS(use_aas);
	// Carl: use our own custom generated AAS specifically for player movement (teleporting).
	if (!aas && !forceAAS48 )
		aas = gameLocal.GetAAS("aas_player");
	// Every map has aas48, used for zombies and imps. It's close enough to player.
	if (!aas || forceAAS48 )
		aas = gameLocal.GetAAS("aas48");
	if (aas)
	{
		const idAASSettings* settings = aas->GetSettings();
		if (settings)
		{
			/*
			if (!ValidForBounds(settings, physicsObj.GetBounds()))
			{
				gameLocal.Error("%s cannot use use_aas %s\n", name.c_str(), use_aas.c_str());
			}
			*/
			float height = settings->maxStepHeight;
			gameLocal.Printf("Player AAS = %s: AAS step height = %f, player step height = %f\n", settings->fileExtension.c_str(), height, pm_stepsize.GetFloat());
			return;
		}
		else
		{
			aas = NULL;
		}
	}
	gameLocal.Printf("WARNING: Player %s has no AAS file\n", name.c_str());
}

/*
==============
idPlayer::InitAASLocation
==============
*/
void idPlayer::InitAASLocation()
{
	int		i;
	int		num;
	idVec3	size;
	idBounds bounds;
	idAAS*	aas;
	idVec3	origin;
	
	GetFloorPos( 64.0f, origin );
	
	num = gameLocal.NumAAS();
	aasLocation.SetGranularity( 1 );
	aasLocation.SetNum( num );
	for( i = 0; i < aasLocation.Num(); i++ )
	{
		aasLocation[ i ].areaNum = 0;
		aasLocation[ i ].pos = origin;
		aas = gameLocal.GetAAS( i );
		if( aas != NULL && aas->GetSettings() )
		{
			size = aas->GetSettings()->boundingBoxes[0][1];
			bounds[0] = -size;
			size.z = 32.0f;
			bounds[1] = size;
			
			aasLocation[ i ].areaNum = aas->PointReachableAreaNum( origin, bounds, AREA_REACHABLE_WALK );
			const idAASSettings* settings = aas->GetSettings();
			if (settings)
			{
				settings->allowFlyReachabilities;
				size = aas->GetSettings()->boundingBoxes[0][1];
				gameLocal.Printf("AAS %d: %s: fly=%d, swim=%d, step=%f, barrier=%f, fall=%f, water=%f; %f x %f x %f\n", i, settings->fileExtension.c_str(),
					settings->allowFlyReachabilities, settings->allowSwimReachabilities,
					settings->maxStepHeight, settings->maxBarrierHeight, settings->maxFallHeight, settings->maxWaterJumpHeight,
					size.x, size.y, size.z);
				bounds = this->physicsObj.GetBounds();
				size = bounds[1];
				gameLocal.Printf("Player: %s, step=%f, jump=%f, crouch=%f; %f x %f x %f\n", name.c_str(),
					pm_stepsize.GetFloat(), pm_jumpheight.GetFloat(), pm_crouchheight.GetFloat(),
					size.x, size.y, size.z);
			}
		}
	}
}

/*
==============
idPlayer::SetAASLocation
==============
*/
void idPlayer::SetAASLocation()
{
	int		i;
	int		areaNum;
	idVec3	size;
	idBounds bounds;
	idAAS*	aas;
	idVec3	origin;
	
	if( !GetFloorPos( 64.0f, origin ) )
	{
		return;
	}
	
	for( i = 0; i < aasLocation.Num(); i++ )
	{
		aas = gameLocal.GetAAS( i );
		if( !aas )
		{
			continue;
		}
		
		size = aas->GetSettings()->boundingBoxes[0][1];
		bounds[0] = -size;
		size.z = 32.0f;
		bounds[1] = size;
		
		areaNum = aas->PointReachableAreaNum( origin, bounds, AREA_REACHABLE_WALK );
		if( areaNum )
		{
			aasLocation[ i ].pos = origin;
			aasLocation[ i ].areaNum = areaNum;
		}
	}
}

/*
==============
idPlayer::GetAASLocation
==============
*/
void idPlayer::GetAASLocation( idAAS* aas, idVec3& pos, int& areaNum ) const
{
	int i;
	
	if( aas != NULL )
	{
		for( i = 0; i < aasLocation.Num(); i++ )
		{
			if( aas == gameLocal.GetAAS( i ) )
			{
				areaNum = aasLocation[ i ].areaNum;
				pos = aasLocation[ i ].pos;
				return;
			}
		}
	}
	
	areaNum = 0;
	pos = physicsObj.GetOrigin();
}

/* Carl: Teleport
=====================
idPlayer::PointReachableAreaNum
=====================
*/
int idPlayer::PointReachableAreaNum(const idVec3& pos, const float boundsScale) const
{
	int areaNum;
	idVec3 size;
	idBounds bounds;

	if (!aas)
	{
		return 0;
	}

	size = aas->GetSettings()->boundingBoxes[0][1] * boundsScale;
	bounds[0] = -size;
	size.z = 32.0f;
	bounds[1] = size;

	areaNum = aas->PointReachableAreaNum(pos, bounds, AREA_REACHABLE_WALK);

	return areaNum;
}

/*
=====================
idPlayer::PathToGoal
=====================
*/
bool idPlayer::PathToGoal(aasPath_t& path, int areaNum, const idVec3& origin, int goalAreaNum, const idVec3& goalOrigin) const
{
	idVec3 org;
	idVec3 goal;

	if (!aas)
	{
		return false;
	}

	org = origin;

	if (ai_debugMove.GetBool())
	{
		aas->DrawArea( areaNum );
		aas->DrawArea( goalAreaNum );
	}

	aas->PushPointIntoAreaNum(areaNum, org);
	if (!areaNum)
	{
		return false;
	}

	goal = goalOrigin;
	aas->PushPointIntoAreaNum(goalAreaNum, goal);
	if (!goalAreaNum)
	{
		return false;
	}

	if (ai_debugMove.GetBool())
	{
		aas->ShowWalkPath(org, goalAreaNum, goal, travelFlags);
	}
	return aas->WalkPathToGoal( path, areaNum, org, goalAreaNum, goal, travelFlags );
}

/* Carl: Teleport
================
idPlayer::CanReachPosition
================
*/
bool idPlayer::CanReachPosition( const idVec3& pos, idVec3& betterPos )
{
	aasPath_t	path;
	int			toAreaNum;
	int			areaNum;
	idVec3 origin;

	toAreaNum = PointReachableAreaNum(pos);
	betterPos = pos;
	if (aas)
		aas->PushPointIntoAreaNum( toAreaNum, betterPos );

	idVec3 floorPos = betterPos;

	origin = physicsObj.GetOrigin();
	areaNum = PointReachableAreaNum(origin);

	// check relative to the AAS area's official floor
	if (aas)
	{
		floorPos.z -= 1000;
		aas->PushPointIntoAreaNum(toAreaNum, floorPos);
		// sloped floors will change x or y, not just z, wrecking our algorithm
		if (floorPos.x != betterPos.x || floorPos.y != betterPos.y)
			floorPos = betterPos;
		// AAS areas have a valid floor (except for stairs), but not a valid ceiling
		// if it's stairs, or our point is higher, then use our point
		if (floorPos.z - pos.z < pm_stepsize.GetFloat() + 2 )
			betterPos.z = pos.z;
		// but if our point is too much lower than the AAS floor, use the AAS floor
	}
	// if in the same area, check relative to our feet
	if (toAreaNum == areaNum)
	{
		floorPos.z = origin.z;
	}
	float height = pos.z - floorPos.z;

	// if it's higher off the floor than we can jump, or lower than we can fall, then give up now
	if (height > pm_jumpheight.GetFloat() + 2 || height < -140)
		return false;

	// if there's no AAS, we can teleport anywhere horizontal we can see, as long as it's height is within jumping or falling height
	if (!aas)
		return true;

	if (ai_debugMove.GetBool())
	{
		aas->DrawArea(areaNum);
		aas->DrawArea(toAreaNum);
	}
	if (!toAreaNum)
		return false;
	if (ai_debugMove.GetBool())
		aas->ShowWalkPath(origin, toAreaNum, betterPos, travelFlags);
	if (areaNum == toAreaNum)
		return true;

	aas->PushPointIntoAreaNum(areaNum, origin);
	idReachability* reach = NULL;
	int travelTime;

	bool result = aas->RouteToGoalArea(areaNum, origin, toAreaNum, travelFlags, travelTime, &reach) && reach && (travelTime <= vr_teleportMaxTravel.GetInteger())
		&& CheckTeleportPath(betterPos, toAreaNum);
	return result;
}

/*
==============
idPlayer::Move_Interpolated
==============
*/
void idPlayer::Move_Interpolated( float fraction )
{

	float newEyeOffset = 0.0f;
	idVec3 oldOrigin;
	idVec3 oldVelocity;
	idVec3 pushVelocity;
	
	// save old origin and velocity for crashlanding
	oldOrigin = physicsObj.GetOrigin();
	oldVelocity = physicsObj.GetLinearVelocity();
	pushVelocity = physicsObj.GetPushedLinearVelocity();
	
	// set physics variables
	physicsObj.SetMaxStepHeight( pm_stepsize.GetFloat() );
	physicsObj.SetMaxJumpHeight( pm_jumpheight.GetFloat() );
	
	if( noclip )
	{
		physicsObj.SetContents( 0 );
		physicsObj.SetMovementType( PM_NOCLIP );
	}
	else if( spectating )
	{
		physicsObj.SetContents( 0 );
		physicsObj.SetMovementType( PM_SPECTATOR );
	}
	else if( health <= 0 )
	{
		physicsObj.SetContents( CONTENTS_CORPSE | CONTENTS_MONSTERCLIP );
		physicsObj.SetMovementType( PM_DEAD );
	}
	else if( gameLocal.inCinematic || Flicksync_InCutscene || gameLocal.GetCamera() || privateCameraView || ( influenceActive == INFLUENCE_LEVEL2 ) )
	{
		physicsObj.SetContents( CONTENTS_BODY );
		physicsObj.SetMovementType( PM_FREEZE );
	}
	else if( mountedObject )
	{
		physicsObj.SetContents( 0 );
		physicsObj.SetMovementType( PM_FREEZE );
	}
	else
	{
		physicsObj.SetContents( CONTENTS_BODY );
		physicsObj.SetMovementType( PM_NORMAL );
	}
	
	if( spectating )
	{
		physicsObj.SetClipMask( MASK_DEADSOLID );
	}
	else if( health <= 0 )
	{
		physicsObj.SetClipMask( MASK_DEADSOLID );
	}
	else
	{
		physicsObj.SetClipMask( MASK_PLAYERSOLID );
	}
	
	physicsObj.SetDebugLevel( g_debugMove.GetBool() );
	
	{
		idVec3	org;
		idMat3	axis;
		GetViewPos( org, axis );
		
		physicsObj.SetPlayerInput( usercmd, axis[0] );
	}
	
	// FIXME: physics gets disabled somehow
	BecomeActive( TH_PHYSICS );
	InterpolatePhysics( fraction );
	
	// update our last valid AAS location for the AI
	SetAASLocation();
	
	if( spectating )
	{
		newEyeOffset = 0.0f;
	}
	else if( health <= 0 )
	{
		newEyeOffset = pm_deadviewheight.GetFloat();
	}
	else if ( physicsObj.IsCrouching() )
	{
		// Koz begin
		// dont change the eyeoffset if using full motion crouch.
		if ( game->isVR )
		{
			if ( vr_crouchMode.GetInteger() != 0 || (usercmd.buttons & BUTTON_CROUCH) )
			{
				newEyeOffset = 34; //Carl: When showing our body, our body doesn't crouch enough, so move eyes as high as possible (any higher and the top of our head wouldn't fit)
			}
		}
		else
		{
			newEyeOffset = pm_crouchviewheight.GetFloat();
		}
		// Koz end
	}
	else if( GetBindMaster() && GetBindMaster()->IsType( idAFEntity_Vehicle::Type ) )
	{
		newEyeOffset = 0.0f;
	}
	// Koz begin
	else if ( game->isVR )
	{
		newEyeOffset = pm_normalviewheight.GetFloat();
		//Carl: Our body is too tall, so move our eyes higher so they don't clip the body
	}
	// Koz end
	else
	{
		newEyeOffset = pm_normalviewheight.GetFloat();
	}
	
	if( EyeHeight() != newEyeOffset ) // Koz fixme - do we want a slow or instant crouch in VR?
	{
		if( spectating )
		{
			SetEyeHeight( newEyeOffset );
		}
		else
		{
			// smooth out duck height changes
			SetEyeHeight( EyeHeight() * pm_crouchrate.GetFloat() + newEyeOffset * ( 1.0f - pm_crouchrate.GetFloat() ) );
		}
	}
	
	if( AI_JUMP )
	{
		// bounce the view weapon
		loggedAccel_t*	acc = &loggedAccel[currentLoggedAccel & ( NUM_LOGGED_ACCELS - 1 )];
		currentLoggedAccel++;
		acc->time = gameLocal.time;
		acc->dir[2] = 200;
		acc->dir[0] = acc->dir[1] = 0;
	}
	
	if( AI_ONLADDER )
	{
		int old_rung = oldOrigin.z / LADDER_RUNG_DISTANCE;
		int new_rung = physicsObj.GetOrigin().z / LADDER_RUNG_DISTANCE;
		
		if( old_rung != new_rung )
		{
			StartSound( "snd_stepladder", SND_CHANNEL_ANY, 0, false, NULL );
		}
	}
	
	BobCycle( pushVelocity );
	CrashLand( oldOrigin, oldVelocity );
	
}

/*
==============
idPlayer::Move
==============
*/
void idPlayer::Move()
{
	float newEyeOffset;
	idVec3 oldOrigin;
	idVec3 oldVelocity;
	idVec3 pushVelocity;
		
	static bool testLean = false;
	static idVec3 leanOrigin = vec3_zero;
			
	// save old origin and velocity for crashlanding
	oldOrigin = physicsObj.GetOrigin();
	oldVelocity = physicsObj.GetLinearVelocity();
	pushVelocity = physicsObj.GetPushedLinearVelocity();
	
	// set physics variables
	physicsObj.SetMaxStepHeight( pm_stepsize.GetFloat() );
	physicsObj.SetMaxJumpHeight( pm_jumpheight.GetFloat() );
	
	if( noclip )
	{
		physicsObj.SetContents( 0 );
		physicsObj.SetMovementType( PM_NOCLIP );
	}
	else if( spectating )
	{
		physicsObj.SetContents( 0 );
		physicsObj.SetMovementType( PM_SPECTATOR );
	}
	else if( health <= 0 )
	{
		physicsObj.SetContents( CONTENTS_CORPSE | CONTENTS_MONSTERCLIP );
		physicsObj.SetMovementType( PM_DEAD );
	}
	else if( gameLocal.inCinematic || Flicksync_InCutscene || gameLocal.GetCamera() || privateCameraView || ( influenceActive == INFLUENCE_LEVEL2 ) )
	{
		physicsObj.SetContents( CONTENTS_BODY );
		physicsObj.SetMovementType( PM_FREEZE );
	}
	else if( mountedObject )
	{
		physicsObj.SetContents( 0 );
		physicsObj.SetMovementType( PM_FREEZE );
	}
	else
	{
		physicsObj.SetContents( CONTENTS_BODY );
		physicsObj.SetMovementType( PM_NORMAL );
	}
	
	if( spectating )
	{
		physicsObj.SetClipMask( MASK_DEADSOLID );
	}
	else if( health <= 0 )
	{
		physicsObj.SetClipMask( MASK_DEADSOLID );
	}
	else
	{
		physicsObj.SetClipMask( MASK_PLAYERSOLID );
	}
	
	physicsObj.SetDebugLevel( g_debugMove.GetBool() );
	
	{
				
		idVec3	org;
		idMat3	axis;
		if ( !game->isVR )
		{
			GetViewPos( org, axis ); // Koz default movement
			physicsObj.SetPlayerInput( usercmd, axis[0] );
		}
		else
		{
			if ( !physicsObj.OnLadder() ) // Koz fixme, dont move if on a ladder or player will fall/stick
			{

				idVec3 bodyOrigin = vec3_zero;
				idVec3 movedBodyOrigin = vec3_zero;
				idVec3 movedRemainder = vec3_zero;
				idMat3 bodyAxis;
				idMat3 origPhysAxis;

				GetViewPos( bodyOrigin, bodyAxis );
				bodyOrigin = physicsObj.GetOrigin();
				origPhysAxis = physicsObj.GetAxis();

				idVec3 newBodyOrigin;

				idAngles bodyAng = bodyAxis.ToAngles();
				idMat3 bodyAx = idAngles( bodyAng.pitch, bodyAng.yaw - commonVr->bodyYawOffset, bodyAng.roll ).Normalize180().ToMat3();

				newBodyOrigin = bodyOrigin + bodyAx[0] * commonVr->remainingMoveHmdBodyPositionDelta.x + bodyAx[1] * commonVr->remainingMoveHmdBodyPositionDelta.y;
				commonVr->remainingMoveHmdBodyPositionDelta.x = commonVr->remainingMoveHmdBodyPositionDelta.y = 0;

				commonVr->motionMoveDelta = newBodyOrigin - bodyOrigin;
				commonVr->motionMoveVelocity = commonVr->motionMoveDelta / ((1000 / commonVr->hmdHz) * 0.001f);

				if ( !commonVr->isLeaning )
				{
					movedBodyOrigin = physicsObj.MotionMove( commonVr->motionMoveVelocity );
					physicsObj.SetAxis( origPhysAxis ); // make sure motion move doesnt change the axis

					movedRemainder = (newBodyOrigin - movedBodyOrigin);

					if ( movedRemainder.Length() > commonVr->motionMoveDelta.Length() * 0.25f )
					{
						commonVr->isLeaning = true;
						testLean = false;
						leanOrigin = movedBodyOrigin;
						commonVr->leanOffset = movedRemainder;
					}
					else
					{
						// if the pda is fixed in space, we need to keep track of how much we have moved the player body
						// so we can keep the PDA in the same position relative to the player while accounting for external movement ( on a lift / eleveator etc )
						if ( !hands[0].PDAfixed && !hands[1].PDAfixed )
						{
							commonVr->fixedPDAMoveDelta = vec3_zero;
						}
						else
						{
							commonVr->fixedPDAMoveDelta += (movedBodyOrigin - bodyOrigin);
						}
					}
				}
				else

				{
					// player body blocked by object. let the head move some by accruing all ( body ) movement  here. 
					// check to see if player body can move to the new location without clipping anything
					// if it can, move it and clear leanoffsets, otherwise limit the distance 
					idVec3 testOrigin = vec3_zero;

					commonVr->leanOffset += commonVr->motionMoveDelta;

					if ( commonVr->leanOffset.LengthSqr() > 36.0f * 36.0f ) // dont move head more than 36 inches // Koz fixme should me measure distance from waist?
					{
						commonVr->leanOffset.Normalize();
						commonVr->leanOffset *= 36.0f;
					}

					if ( commonVr->leanBlank )
					{
						if ( commonVr->leanOffset.LengthSqr() > commonVr->leanBlankOffsetLengthSqr )
						{
							commonVr->leanOffset = commonVr->leanBlankOffset;
						}
					}

					testOrigin = bodyOrigin + commonVr->leanOffset;

					if ( commonVr->leanOffset.LengthSqr() > 4.0f || bodyOrigin != leanOrigin ) testLean = true; // dont check to cancel lean until player body has moved, or head has moved at least two inches.

					if ( testLean )
					{
						// clip against the player clipmodel
						trace_t trace;
						idMat3 clipAxis;

						idClipModel* clip;
						clip = physicsObj.GetClipModel();
						clipAxis = physicsObj.GetClipModel()->GetAxis();


						gameLocal.clip.Translation( trace, testOrigin, testOrigin, clip, clipAxis, MASK_SHOT_RENDERMODEL /* CONTENTS_SOLID */, this );
						if ( trace.fraction < 1.0f )
						{

							// do ik stuff here
							// trying to do this now in player walkIk

						}
						else
						{
							// not leaning, clear the offsets and move the player origin
							physicsObj.SetOrigin( testOrigin );
							commonVr->isLeaning = false;
							//common->Printf("Setting Leaning FALSE %d\n", Sys_Milliseconds());
							commonVr->leanOffset = vec3_zero;
							//animator.ClearJoint( chestPivotJoint );
						}
					}
				}
			}
			GetViewPos( org, axis ); // Koz default movement
			physicsObj.SetPlayerInput( usercmd, axis[0] );
		}

	}
	
	// FIXME: physics gets disabled somehow
	BecomeActive( TH_PHYSICS );

	// Carl: check if we're experiencing artificial locomotion
	idVec3 before = physicsObj.GetOrigin();
	RunPhysics();
	idVec3 after = physicsObj.GetOrigin();

	// update our last valid AAS location for the AI
	SetAASLocation();
	
	if( spectating )
	{
		newEyeOffset = 0.0f;
	}
	else if( health <= 0 )
	{
		newEyeOffset = pm_deadviewheight.GetFloat();
	}
	else if( physicsObj.IsCrouching() )
	{
		// Koz begin
		
		if ( game->isVR )
		{
			if ( vr_crouchMode.GetInteger() != 0 || (usercmd.buttons & BUTTON_CROUCH) )
			{
				newEyeOffset = 34;  //Carl: When showing our body, our body doesn't crouch enough, so move eyes as high as possible (any higher and the top of our head wouldn't fit)
				if ( vr_crouchMode.GetInteger() != 0 && commonVr->poseHmdHeadPositionDelta.z < -vr_crouchTriggerDist.GetFloat() )
				{
					// crouch was initiated by the trigger, adjust eyeOffset by trigger val so view isnt too low.
					newEyeOffset += vr_crouchTriggerDist.GetFloat();
				}
			}
			else
				newEyeOffset = pm_normalviewheight.GetFloat();
		}
		else
		{
			newEyeOffset = pm_crouchviewheight.GetFloat();
		}
		// Koz end
	}
	else if( GetBindMaster() && GetBindMaster()->IsType( idAFEntity_Vehicle::Type ) )
	{
		newEyeOffset = 0.0f;
	}
	else
	{
		newEyeOffset = pm_normalviewheight.GetFloat();
	}

	float oldEyeOffset = EyeHeight();
	if( oldEyeOffset != newEyeOffset )
	{
		if( spectating )
		{
			SetEyeHeight( newEyeOffset );
		}
		else
		{
			// smooth out duck height changes
			SetEyeHeight( EyeHeight() * pm_crouchrate.GetFloat() + newEyeOffset * ( 1.0f - pm_crouchrate.GetFloat() ) );
		}
	}
	
	if( noclip || gameLocal.inCinematic || Flicksync_InCutscene || ( influenceActive == INFLUENCE_LEVEL2 ) )
	{
		AI_CROUCH	= false;
		AI_ONGROUND	= ( influenceActive == INFLUENCE_LEVEL2 );
		AI_ONLADDER	= false;
		AI_JUMP		= false;
	}
	else
	{
		AI_CROUCH	= physicsObj.IsCrouching();
		AI_ONGROUND	= physicsObj.HasGroundContacts();
		AI_ONLADDER	= physicsObj.OnLadder();
		AI_JUMP		= physicsObj.HasJumped();
		
		// check if we're standing on top of a monster and give a push if we are
		idEntity* groundEnt = physicsObj.GetGroundEntity(); 
		if( groundEnt != NULL && groundEnt->IsType( idAI::Type ) )
		{
			idVec3 vel = physicsObj.GetLinearVelocity();
			if( vel.ToVec2().LengthSqr() < 0.1f )
			{
				vel.ToVec2() = physicsObj.GetOrigin().ToVec2() - groundEnt->GetPhysics()->GetAbsBounds().GetCenter().ToVec2();
				vel.ToVec2().NormalizeFast();
				
				if ( game->isVR )
				{
					vel.ToVec2() *= (pm_walkspeed.GetFloat() + vr_walkSpeedAdjust.GetFloat());
				}
				else
				{
					vel.ToVec2() *= pm_walkspeed.GetFloat();
				}
			}
			else
			{
				// give em a push in the direction they're going
				vel *= 1.1f;
			}
			physicsObj.SetLinearVelocity( vel );
		}
	}
	
	if( AI_JUMP )
	{
		// bounce the view weapon
		loggedAccel_t*	acc = &loggedAccel[currentLoggedAccel & ( NUM_LOGGED_ACCELS - 1 )];
		currentLoggedAccel++;
		acc->time = gameLocal.time;
		acc->dir[2] = 200;
		acc->dir[0] = acc->dir[1] = 0;
	}
	
	if( AI_ONLADDER )
	{
		int old_rung = oldOrigin.z / LADDER_RUNG_DISTANCE;
		int new_rung = physicsObj.GetOrigin().z / LADDER_RUNG_DISTANCE;
		
		if( old_rung != new_rung )
		{
			StartSound( "snd_stepladder", SND_CHANNEL_ANY, 0, false, NULL );
		}
	}
	
	BobCycle( pushVelocity );
	// Carl: Motion sickness detection
	float distance = ((after - before) - commonVr->motionMoveDelta).LengthSqr();
	static float oldHeadHeightDiff = 0;
	float crouchDistance = EyeHeight() + commonVr->headHeightDiff - oldEyeOffset - oldHeadHeightDiff;
	oldHeadHeightDiff = commonVr->headHeightDiff;
	distance += crouchDistance * crouchDistance + viewBob.LengthSqr();
	blink = (distance > 0.005f);
	CrashLand( oldOrigin, oldVelocity );

	// Handling vr_comfortMode
	extern idCVar timescale;
	if (vr_motionSickness.IsModified() && !warpMove && !warpAim)
	{
		timescale.SetFloat(1);
		vr_motionSickness.ClearModified();
	}
	const int comfortMode = vr_motionSickness.GetInteger();
	//"	0 off | 2 = tunnel | 5 = tunnel + chaperone | 6 slow mo | 7 slow mo + chaperone | 8 tunnel + slow mo | 9 = tunnel + slow mo + chaperone
	if ( comfortMode < 2 || Flicksync_InCutscene || game->CheckInCinematic() ) 
	{
		this->playerView.EnableVrComfortVision( false );
		commonVr->thirdPersonMovement = false;
		return;
	}

	float speed = physicsObj.GetLinearVelocity().LengthFast();
	if ( comfortMode == 10 && speed == 0 && usercmd.forwardmove == 0 && usercmd.rightmove == 0 )
		commonVr->thirdPersonMovement = false;

	if (( comfortMode == 2 ) || ( comfortMode == 5 ) || ( comfortMode == 8 ) || ( comfortMode == 9 ))
	{
		if (speed == 0 && !blink)
		{
			this->playerView.EnableVrComfortVision(false);
		}
		else
		{
			this->playerView.EnableVrComfortVision(true);
		}
	}
	else
		this->playerView.EnableVrComfortVision(false);

	if ((( comfortMode == 6 ) || ( comfortMode == 7 ) || ( comfortMode == 8 ) || ( comfortMode == 9 )) && !warpAim && !warpMove )
	{
		float speedFactor = (( pm_runspeed.GetFloat() - speed ) / pm_runspeed.GetFloat());
		if ( speedFactor < 0 )
		{
			speedFactor = 0;
		}
		timescale.SetFloat( 0.5f + 0.5f * speedFactor );
	}
}

/*
========================
idPlayer::AllowClientAuthPhysics
========================
*/
bool idPlayer::AllowClientAuthPhysics()
{
	// note respawn count > 1: respawn should be called twice - once for initial spawn and once for actual respawn by game mode
	// TODO: I don't think doom 3 will need to care about the respawn count.
	return ( usercmd.serverGameMilliseconds > serverOverridePositionTime && commonLocal.GetUCmdMgr().HasUserCmdForPlayer( entityNumber ) );
}

/*
========================
idPlayer::RunPhysics_RemoteClientCorrection
========================
*/
void idPlayer::RunPhysics_RemoteClientCorrection()
{

	if( !AllowClientAuthPhysics() )
	{
		// We are still overriding client's position
		if( pm_clientAuthoritative_debug.GetBool() )
		{
			//clientGame->renderWorld->DebugPoint( idColor::colorRed, GetOrigin() );
			idLib::Printf( "[%d]Ignoring client auth:  cmd.serverTime: %d  overrideTime: %d \n", entityNumber,  usercmd.serverGameMilliseconds, serverOverridePositionTime );
		}
		return;
	}
	
	
	
	// Client is on a pusher... ignore him so he doesn't lag behind
	bool becameUnlocked = false;
	if( physicsObj.ClientPusherLocked( becameUnlocked ) )
	{
	
		// Check and see how far we've diverged.
		idVec3 cmdPos( usercmd.pos[0], usercmd.pos[1], usercmd.pos[2] );
		idVec3 newOrigin = physicsObj.GetOrigin();
		
		idVec3 divergeVec = cmdPos - newOrigin;
		idLib::Printf( "Client Divergence: %s Length: %2f\n", divergeVec.ToString( 3 ), divergeVec.Length() );
		
		// if the client Diverges over a certain amount, snap him back
		if( divergeVec.Length() < pm_clientAuthoritative_Divergence.GetFloat() )
		{
			return;
		}
		
	}
	if( becameUnlocked )
	{
		// Client just got off of a mover, wait before listening to him
		serverOverridePositionTime = gameLocal.GetServerGameTimeMs();
		return;
	}
	
	
	// Correction
	{
		idVec3 newOrigin = physicsObj.GetOrigin();
		idVec3 cmdPos( usercmd.pos[0], usercmd.pos[1], usercmd.pos[2] );
		idVec3 desiredPos = cmdPos;
		
		float delta = ( desiredPos - newOrigin ).Length();
		// ignore small differences in Z: this can cause player to not have proper ground contacts which messes up
		// velocity/acceleration calculation. If this hack doesn't work out, will may need more precision for at least
		// the Z component of the client's origin.
		if( idMath::Fabs( desiredPos.z - newOrigin.z ) < pm_clientAuthoritative_minDistZ.GetFloat() )
		{
			if( pm_clientAuthoritative_debug.GetBool() )
			{
				//idLib::Printf("[%d]Remote client physics: ignore small z delta: %f\n", usercmd.clientGameFrame, ( desiredPos.z - newOrigin.z ) );
			}
			desiredPos.z = newOrigin.z;
		}
		
		// Origin
		if( delta > pm_clientAuthoritative_minDist.GetFloat() )
		{
		
			if( pm_clientAuthoritative_Lerp.GetFloat() > 0.0f )
			{
				desiredPos.x = idMath::LerpToWithScale( newOrigin.x, desiredPos.x, pm_clientAuthoritative_Lerp.GetFloat() );
				desiredPos.y = idMath::LerpToWithScale( newOrigin.y, desiredPos.y, pm_clientAuthoritative_Lerp.GetFloat() );
			}
			
			// Set corrected position immediately if non deferred
			physicsObj.SetOrigin( desiredPos );
			
			if( pm_clientAuthoritative_debug.GetBool() && delta > pm_clientAuthoritative_warnDist.GetFloat() )
			{
				idLib::Warning( "Remote client player physics: delta movement for frame was %f units", delta );
				gameRenderWorld->DebugLine( colorRed, newOrigin, desiredPos );
			}
		}
		if( pm_clientAuthoritative_debug.GetBool() )
		{
			//idLib::Printf( "[%d]Remote client player physics delta: %.2f. forward: %d pos <%.2f, %.2f, %.2f> \n", usercmd.clientGameFrame, delta, (int)usercmd.forwardmove, desiredPos.x, desiredPos.y, desiredPos.z );
			gameRenderWorld->DebugLine( colorRed, newOrigin, desiredPos );
			//gameRenderWorld->DebugPoint( colorBlue, cmdPos );
		}
		
		// Set velocity if significantly different than client.
		const float serverSpeedSquared = physicsObj.GetLinearVelocity().LengthSqr();
		const float clientSpeedSquared = usercmd.speedSquared;
		
		if( fabsf( serverSpeedSquared - clientSpeedSquared ) > pm_clientAuthoritative_minSpeedSquared.GetFloat() )
		{
			idVec3 normalizedVelocity = physicsObj.GetLinearVelocity();
			
			const float VELOCITY_EPSILON = 0.001f;
			if( normalizedVelocity.LengthSqr() > VELOCITY_EPSILON )
			{
				normalizedVelocity.Normalize();
			}
			
			physicsObj.SetLinearVelocity( normalizedVelocity * idMath::Sqrt( clientSpeedSquared ) );
		}
	}
}

/*
========================
idPlayer::GetPhysicsTimeStep

Uses the time from the usercmd in case the server is running at a slower engineHz
than the client.
========================
*/
int idPlayer::GetPhysicsTimeStep() const
{
	// if the ucDeltaMillisecond value looks wrong, use the game delta milliseconds
	// This can happen if the user brings up the pause menu in SP
	const int ucDeltaMilliseconds = usercmd.clientGameMilliseconds - oldCmd.clientGameMilliseconds;
	if( ucDeltaMilliseconds < 1 || ucDeltaMilliseconds > 20 )
	{
		return gameLocal.time - gameLocal.previousTime;
	}
	else
	{
		return ucDeltaMilliseconds;
	}
}

/*
==============
idPlayer::ShowRespawnHudMessage

Called once when the minimum respawn time has passed after a player has died
so that we can display a message to the user.
==============
*/
void idPlayer::ShowRespawnHudMessage()
{
	if( IsLocallyControlled() )
	{
		hud->ShowRespawnMessage( true );
	}
	else
	{
		// Clients show the hud message through a reliable message.
		idBitMsg	outMsg;
		byte		dummyData[1];
		outMsg.InitWrite( dummyData, sizeof( dummyData ) );
		outMsg.BeginWriting();
		outMsg.WriteByte( 0 );
		session->GetActingGameStateLobbyBase().SendReliableToLobbyUser( gameLocal.lobbyUserIDs[entityNumber], GAME_RELIABLE_MESSAGE_RESPAWN_AVAILABLE, outMsg );
	}
}

/*
==============
idPlayer::HideRespawnHudMessage

Called once when we should remove the respawn message from the hud,
for example, when a player does respawn.
==============
*/
void idPlayer::HideRespawnHudMessage()
{
	if( IsLocallyControlled() )
	{
		hud->ShowRespawnMessage( false );
	}
}

/*
==============
idPlayer::UpdateHud
==============
*/
void idPlayer::UpdateHud()
{
	idPlayer* aimed;
	
	if( !hud )
	{
		return;
	}
	
	if( !IsLocallyControlled() )
	{
		return;
	}
	

	if ( game->isVR )
	{
		UpdateVrHud();
	}


	int c = inventory.pickupItemNames.Num();
	if( c > 0 )
	{
		if( hud != NULL && hud->IsPickupListReady() )
		{
			if( inventory.nextItemPickup && gameLocal.time - inventory.nextItemPickup > 2000 )
			{
				inventory.nextItemNum = 1;
			}
			int i;
			
			int count = 5;
			bool showNewPickups = false;
			for( i = 0; i < count; i++ )    //_D3XP
			{
				if( i < c )
				{
					hud->UpdatePickupInfo( i, inventory.pickupItemNames[0] );
					inventory.nextItemNum++;
					showNewPickups = true;
				}
				else
				{
					hud->UpdatePickupInfo( i, "" );
					continue;
				}
				
				inventory.nextItemPickup = gameLocal.time + 2500;
				inventory.pickupItemNames.RemoveIndex( 0 );
			}
			
			if( showNewPickups )
			{
				hud->ShowPickups();
			}
		}
	}
	
	if( gameLocal.realClientTime == lastMPAimTime )
	{
		if( MPAim != -1 && gameLocal.mpGame.IsGametypeTeamBased()  /* CTF */
				&& gameLocal.entities[ MPAim ] && gameLocal.entities[ MPAim ]->IsType( idPlayer::Type )
				&& static_cast< idPlayer* >( gameLocal.entities[ MPAim ] )->team == team )
		{
			aimed = static_cast< idPlayer* >( gameLocal.entities[ MPAim ] );
			
			hud->TriggerHitTarget( true, session->GetActingGameStateLobbyBase().GetLobbyUserName( gameLocal.lobbyUserIDs[ MPAim ] ), aimed->team + 1 );
			MPAimHighlight = true;
			MPAimFadeTime = 0;	// no fade till loosing focus
		}
		else if( MPAimHighlight )
		{
			hud->TriggerHitTarget( false, "" );
			MPAimFadeTime = gameLocal.realClientTime;
			MPAimHighlight = false;
		}
	}
	if( MPAimFadeTime )
	{
		assert( !MPAimHighlight );
		if( gameLocal.realClientTime - MPAimFadeTime > 2000 )
		{
			MPAimFadeTime = 0;
		}
	}
	
	if( common->IsMultiplayer() && IsLocallyControlled() )
	{
	
		hud->ToggleLagged( isLagged );
		
		// TODO_SPARTY: what is this projectile stuff for
		//hud->SetStateInt( "g_showProjectilePct", g_showProjectilePct.GetInteger() );
		//if ( numProjectilesFired ) {
		//	hud->SetStateString( "projectilepct", va( "Hit %% %.1f", ( (float) numProjectileHits / numProjectilesFired ) * 100 ) );
		//} else {
		//	hud->SetStateString( "projectilepct", "Hit % 0.0" );
		//}
		
	}
}

/*
==============
idPlayer::UpdateDeathSkin
==============
*/
void idPlayer::UpdateDeathSkin( bool state_hitch )
{
	if( !( common->IsMultiplayer() || g_testDeath.GetBool() ) )
	{
		return;
	}
	if( health <= 0 )
	{
		if( !doingDeathSkin )
		{
			deathClearContentsTime = spawnArgs.GetInt( "deathSkinTime" );
			doingDeathSkin = true;
			renderEntity.noShadow = true;
			if( state_hitch )
			{
				renderEntity.shaderParms[ SHADERPARM_TIME_OF_DEATH ] = gameLocal.time * 0.001f - 2.0f;
			}
			else
			{
				renderEntity.shaderParms[ SHADERPARM_TIME_OF_DEATH ] = gameLocal.time * 0.001f;
			}
			UpdateVisuals();
		}
		
		// wait a bit before switching off the content
		if( deathClearContentsTime && gameLocal.time > deathClearContentsTime )
		{
			SetCombatContents( false );
			deathClearContentsTime = 0;
		}
	}
	else
	{
		renderEntity.noShadow = false;
		renderEntity.shaderParms[ SHADERPARM_TIME_OF_DEATH ] = 0.0f;
		UpdateVisuals();
		doingDeathSkin = false;
	}
}

/*
==============
idPlayer::StartFxOnBone
==============
*/
void idPlayer::StartFxOnBone( const char* fx, const char* bone )
{
	idVec3 offset;
	idMat3 axis;
	jointHandle_t jointHandle = GetAnimator()->GetJointHandle( bone );
	
	if( jointHandle == INVALID_JOINT )
	{
		gameLocal.Printf( "Cannot find bone %s\n", bone );
		return;
	}
	
	if( GetAnimator()->GetJointTransform( jointHandle, gameLocal.time, offset, axis ) )
	{
		offset = GetPhysics()->GetOrigin() + offset * GetPhysics()->GetAxis();
		axis = axis * GetPhysics()->GetAxis();
	}
	
	idEntityFx::StartFx( fx, &offset, &axis, this, true );
}

/*
==============
idPlayer::HandleGuiEvents
==============
*/
bool idPlayer::HandleGuiEvents( const sysEvent_t* ev )
{

	bool handled = false;
	
	if( hudManager != NULL && hudManager->IsActive() )
	{
		handled = hudManager->HandleGuiEvent( ev );
	}
	
	if ( pdaMenu != NULL && pdaMenu->IsActive() )
	{
		handled = pdaMenu->HandleGuiEvent( ev );
	}
	
	return handled;
}

// Carl:
#define TP_HAND_DISABLED 0
#define TP_HAND_GUNSIGHT 1
#define TP_HAND_RIGHT 2
#define TP_HAND_LEFT 3
#define TP_HAND_HEAD 4
bool idPlayer::GetHandOrHeadPositionWithHacks( int hand, idVec3& origin, idMat3& axis )
{
	int weaponHand;
	if( hand == TP_HAND_RIGHT )
		weaponHand = HAND_RIGHT;
	else if( hand == TP_HAND_LEFT )
		weaponHand = HAND_LEFT;
	else
		weaponHand = vr_weaponHand.GetInteger();

	idWeapon* weapon = hands[ weaponHand ].weapon.GetEntity();
	// In Multiplayer, weapon might not have been spawned yet.
	if (weapon == NULL || hand == TP_HAND_HEAD)
	{
		origin = commonVr->lastViewOrigin; // Koz fixme set the origin and axis to the players view
		axis = commonVr->lastViewAxis;
		return false;
	}
	weapon_t currentWeap = weapon->IdentifyWeapon();
	// Carl: weapon hand
	if ( hand == 1 || hand == TP_HAND_RIGHT + vr_weaponHand.GetInteger() )
	{
		switch ( currentWeap )
		{
			case WEAPON_NONE:
			case WEAPON_FISTS:
			case WEAPON_SOULCUBE:
			case WEAPON_PDA:
			case WEAPON_HANDGRENADE:
				origin = weapon->viewWeaponOrigin; // Koz fixme set the origin and axis to the weapon default
				axis = weapon->viewWeaponAxis;
				return false;
				break;

			default:
				return weapon->GetMuzzlePositionWithHacks( origin, axis ); 
				break;
		}
	}
	// Carl: flashlight hand
	else if ( commonVr->GetCurrentFlashlightMode() == FLASHLIGHT_HAND && weaponEnabled && !spectating && !gameLocal.world->spawnArgs.GetBool("no_Weapons") && !game->IsPDAOpen() && !commonVr->PDAforcetoggle && hands[0].currentWeapon != weapon_pda && hands[1].currentWeapon != weapon_pda )
	{
		weapon_t currentWeapon = flashlight->IdentifyWeapon();
		CalculateViewFlashlightPos( origin, axis, flashlightOffsets[ int( currentWeapon ) ] );
		return false;
	}
	// Carl: todo empty non-weapon hand (currently using head instead)
	else
	{
		origin = commonVr->lastViewOrigin; // Koz fixme set the origin and axis to the players view
		axis = commonVr->lastViewAxis;
		return false;
	}
}

/*
==============
idPlayer::SetupPDASlot
==============
*/
void idPlayer::SetupPDASlot( bool holsterPDA )
{
	const char * modelname;
	idRenderModel* renderModel;

	FreePDASlot();

	if( vr_slotDisable.GetBool() )
	{
		return;
	}

	if ( holsterPDA )
	{
		// we will holster the PDA
		modelname = "models/items/pda/pda_world.lwo";
		pdaHolsterAxis = (pdaAngle1.ToMat3() * pdaAngle2.ToMat3() * pdaAngle3.ToMat3()) * 0.6f;
	}
	else
	{
		// we will holster the flashlight if carrying it
		if ( commonVr->currentFlashlightMode == FLASHLIGHT_HAND && flashlight.GetEntity()->IsLinked() && !spectating && weaponEnabled && !hiddenWeapon && !gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) )
		{
			modelname = flashlight->weaponDef->dict.GetString("model");
			pdaHolsterAxis = idAngles(0, 90, 90).ToMat3();
		}
		else modelname = "";
	}

	memset( &pdaRenderEntity, 0, sizeof( pdaRenderEntity ) );

	// can we holster?
	if ( !(renderModel = renderModelManager->FindModel(modelname)) )
	{
		// can't holster, just unholster
		return;
	}

	pdaRenderEntity.hModel = renderModel;
	if (pdaRenderEntity.hModel)
	{
		pdaRenderEntity.hModel->Reset();
		pdaRenderEntity.bounds = pdaRenderEntity.hModel->Bounds( &pdaRenderEntity );
	}
	pdaRenderEntity.shaderParms[ SHADERPARM_RED ]	= 1.0f;
	pdaRenderEntity.shaderParms[ SHADERPARM_GREEN ] = 1.0f;
	pdaRenderEntity.shaderParms[ SHADERPARM_BLUE ]	= 1.0f;
	pdaRenderEntity.shaderParms[3] = 1.0f;
	pdaRenderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] = 0.0f;
	pdaRenderEntity.shaderParms[5] = 0.0f;
	pdaRenderEntity.shaderParms[6] = 0.0f;
	pdaRenderEntity.shaderParms[7] = 0.0f;
}

/*
==============
idPlayer::FreePDASlot
==============
*/
void idPlayer::FreePDASlot()
{
	if( pdaModelDefHandle != -1 )
	{
		gameRenderWorld->FreeEntityDef( pdaModelDefHandle );
		pdaModelDefHandle = -1;
	}
}

/*
==============
idPlayer::UpdatePDASlot
==============
*/
void idPlayer::UpdatePDASlot()
{
	if( vr_slotDisable.GetBool() )
	{
		return;
	}
	if( pdaRenderEntity.hModel ) // && inventory.pdas.Num()
	{
		pdaRenderEntity.timeGroup = timeGroup;

		pdaRenderEntity.entityNum = ENTITYNUM_NONE;

		pdaRenderEntity.axis = pdaHolsterAxis * waistAxis;
		idVec3 slotOrigin = slots[SLOT_PDA_HIP].origin;
		if (vr_weaponHand.GetInteger())
			slotOrigin.y *= -1.0f;
		pdaRenderEntity.origin = waistOrigin + slotOrigin * waistAxis;

		pdaRenderEntity.allowSurfaceInViewID = entityNumber + 1;
		pdaRenderEntity.weaponDepthHack = g_useWeaponDepthHack.GetBool();

		if( pdaModelDefHandle == -1 )
		{
			pdaModelDefHandle = gameRenderWorld->AddEntityDef( &pdaRenderEntity );
		}
		else
		{
			gameRenderWorld->UpdateEntityDef( pdaModelDefHandle, &pdaRenderEntity );
		}
	}
}

/*
==============
idPlayer::SetupHolsterSlot

stashed: -1 = switch weapons, 1 = empty holster of stashed weapon, 0 = stash current weapon in holster but don't switch
==============
*/
void idPlayer::SetupHolsterSlot( int hand, int stashed )
{
	// if there's nothing to stash because we were already using fists or PDA
	if ( stashed == 0 && (hands[hand].currentWeapon == weapon_pda || hands[ hand ].currentWeapon == weapon_fists) )
		return;
	// if we were using fists before activating pda, we didn't stash anything in our holster, so don't unstash anything
	if( stashed == 1 && hands[ hand ].previousWeapon == weapon_fists )
		return;
	// if we want to read or switch the current weapon but it's not ready
	if( !hands[ hand ].weapon.GetEntity()->IsReady() && stashed != 1 )
	{
		return;
	}

	const char * modelname;
	idRenderModel* renderModel;

	if ( stashed == 0 )
	{
		extraHolsteredWeapon = holsteredWeapon;
		if ( holsterRenderEntity.hModel )
			extraHolsteredWeaponModel = holsterRenderEntity.hModel->Name();
		else
			extraHolsteredWeaponModel = NULL;
	}

	FreeHolsterSlot();
	if( vr_slotDisable.GetBool() )
	{
		return;
	}

	if ( stashed == 1 )
	{
		modelname = extraHolsteredWeaponModel;
		extraHolsteredWeaponModel = NULL;
	}
	else
		modelname = hands[hand].weapon->weaponDef->dict.GetString("model");

	// can we holster?
	if( !modelname ||
		strcmp(modelname, "models/weapons/soulcube/w_soulcube.lwo") == 0 ||
		strcmp(modelname, "_DEFAULT") == 0 ||
		strcmp(modelname, "models/items/grenade_ammo/grenade.lwo") == 0 ||
		strcmp(modelname, "models/items/pda/pda_world.lwo") == 0 ||
		!(renderModel = renderModelManager->FindModel( modelname )) )
	{
		// can't holster, just unholster
		if( holsteredWeapon != weapon_fists )
		{
			if ( stashed < 0 )
				SelectWeapon(holsteredWeapon, false, true);
			holsteredWeapon = weapon_fists;
			memset(&holsterRenderEntity, 0, sizeof(holsterRenderEntity));
		}
		return;
	}

	// we can holster! so unholster or change weapons
	if (stashed < 0)
	{
		int prevWeapon = hands[hand].currentWeapon;
		hands[hand].SelectWeapon(holsteredWeapon, false, true);
		holsteredWeapon = prevWeapon;
	}
	else
	{
		if (stashed == 0) // stash current weapon, holstered weapon moves to invisible "extra" slot
		{
			holsteredWeapon = hands[hand].currentWeapon;
		}
		else // unstash holstered weapon, extra weapon moves back to holster
		{
			hands[ hand ].SelectWeapon(holsteredWeapon, true, true);
			holsteredWeapon = extraHolsteredWeapon;
			extraHolsteredWeapon = weapon_fists;
		}
	}

	memset( &holsterRenderEntity, 0, sizeof( holsterRenderEntity ) );

	holsterRenderEntity.hModel = renderModel;
	if( holsterRenderEntity.hModel )
	{
		holsterRenderEntity.hModel->Reset();
		holsterRenderEntity.bounds = holsterRenderEntity.hModel->Bounds( &holsterRenderEntity );
	}
	holsterRenderEntity.shaderParms[ SHADERPARM_RED ]	= 1.0f;
	holsterRenderEntity.shaderParms[ SHADERPARM_GREEN ] = 1.0f;
	holsterRenderEntity.shaderParms[ SHADERPARM_BLUE ]	= 1.0f;
	holsterRenderEntity.shaderParms[3] = 1.0f;
	holsterRenderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] = 0.0f;
	holsterRenderEntity.shaderParms[5] = 0.0f;
	holsterRenderEntity.shaderParms[6] = 0.0f;
	holsterRenderEntity.shaderParms[7] = 0.0f;

	if( strcmp(modelname, "models/weapons/pistol/w_pistol.lwo") == 0 )
	{
		holsterAxis = idAngles(90, 0, 0).ToMat3() * 0.75f;
	}
	else if( strcmp(modelname, "models/weapons/shotgun/w_shotgun2.lwo") == 0 ||
		strcmp(modelname, "models/weapons/bfg/bfg_world.lwo") == 0)
	{
		holsterAxis = idAngles(0, -90, -90).ToMat3();
	}
	else if( strcmp(modelname, "models/weapons/grabber/grabber_world.ase") == 0 )
	{
		holsterAxis = idAngles(-90, 180, 0).ToMat3() * 0.5f;
	}
	else if (strcmp(modelname, "models/weapons/machinegun/w_machinegun.lwo") == 0)
	{
		holsterAxis = idAngles(0, 90, 90).ToMat3() * 0.75f;
	}
	else if (strcmp(modelname, "models/weapons/plasmagun/plasmagun_world.lwo") == 0)
	{
		holsterAxis = idAngles(0, 90, 90).ToMat3() * 0.75f;
	}
	else if (strcmp(modelname, "models/weapons/chainsaw/w_chainsaw.lwo") == 0)
	{
		holsterAxis = idAngles(0, 90, 90).ToMat3() * 0.9f;
	}
	else if (strcmp(modelname, "models/weapons/chaingun/w_chaingun.lwo") == 0)
	{
		holsterAxis = idAngles(0, 90, 90).ToMat3() * 0.9f;
	}
	else
	{
		holsterAxis = idAngles(0, 90, 90).ToMat3();
	}
}

/*
==============
idPlayer::FreeHolsterSlot
==============
*/
void idPlayer::FreeHolsterSlot()
{
	if( holsterModelDefHandle != -1 )
	{
		gameRenderWorld->FreeEntityDef( holsterModelDefHandle );
		holsterModelDefHandle = -1;
	}
}

/*
==============
idPlayer::UpdateHolsterSlot
==============
*/
void idPlayer::UpdateHolsterSlot()
{
	if( vr_slotDisable.GetBool() )
	{
		FreeHolsterSlot();
		holsteredWeapon = weapon_fists;
		return;
	}
	if( holsterRenderEntity.hModel )
	{
		holsterRenderEntity.timeGroup = timeGroup;

		holsterRenderEntity.entityNum = ENTITYNUM_NONE;

		holsterRenderEntity.axis = holsterAxis * waistAxis;
		idVec3 slotOrigin = slots[SLOT_WEAPON_HIP].origin + idVec3(-5, 0, 0);
		if (vr_weaponHand.GetInteger())
			slotOrigin.y *= -1.0f;
		holsterRenderEntity.origin = waistOrigin + slotOrigin * waistAxis;

		holsterRenderEntity.allowSurfaceInViewID = entityNumber + 1;
		holsterRenderEntity.weaponDepthHack = g_useWeaponDepthHack.GetBool();

		if( holsterModelDefHandle == -1 )
		{
			holsterModelDefHandle = gameRenderWorld->AddEntityDef( &holsterRenderEntity );
		}
		else
		{
			gameRenderWorld->UpdateEntityDef( holsterModelDefHandle, &holsterRenderEntity );
		}
	}
}

/*
==============
idPlayer::UpdateLaserSight
==============
*/
idCVar	g_laserSightWidth( "g_laserSightWidth", "1.0", CVAR_FLOAT | CVAR_ARCHIVE, "laser sight beam width" ); // Koz default was 2, IMO too big in VR.
idCVar	g_laserSightLength( "g_laserSightLength", "1000", CVAR_FLOAT | CVAR_ARCHIVE, "laser sight beam length" ); // Koz default was 250, but was to short in VR.  Length will be clipped if object is hit, this is max length for the hit trace. 


void idPlayer::UpdateLaserSight( int hand )
{
	idVec3	muzzleOrigin;
	idMat3	muzzleAxis;

	idVec3 end, start;
	trace_t traceResults;
	
	float beamLength = g_laserSightLength.GetFloat(); // max length to run trace.
	
	int sightMode = vr_weaponSight.GetInteger();

	bool hideSight = false;

	bool traceHit = false;
	idWeapon* weapon = hands[ hand ].weapon.GetEntity();

	// In Multiplayer, weapon might not have been spawned yet.
	if( weapon ==  NULL )
	{
		return;
	}

	// Carl: teleport
	static bool oldTeleport = false;
	bool showTeleport = vr_teleport.GetInteger() == 1 && commonVr->VR_USE_MOTION_CONTROLS; // only show the teleport gun cursor if we're teleporting using the gun aim mode
	showTeleport = showTeleport && !AI_DEAD && !gameLocal.inCinematic && !Flicksync_InCutscene && !game->IsPDAOpen();

	// check if lasersight should be hidden
	if ( //!IsGameStereoRendered() ||
		!hands[hand].laserSightActive ||							// Koz allow user to toggle lasersight.
		sightMode == -1 ||
		!weapon->ShowCrosshair() ||		
		AI_DEAD ||
		weapon->IsHidden() ||												
		weapon->hideOffset != 0 ||						// Koz - turn off lasersight If gun is lowered ( in gui ).
		commonVr->handInGui ||							// turn off lasersight if hand is in gui.
		gameLocal.inCinematic ||
		game->IsPDAOpen() ||							// Koz - turn off laser sight if using pda.
		weapon->GetGrabberState() >= 2 ||	// Koz turn off laser sight if grabber is dragging an entity
		showTeleport || !weapon->GetMuzzlePositionWithHacks(muzzleOrigin, muzzleAxis)) // no lasersight for fists,grenades,soulcube etc

	{
		hideSight = !showTeleport;
	}
		
	if ( hideSight == true || ( sightMode != 0 && sightMode < 4 ) )
	{
		hands[hand].laserSightRenderEntity.allowSurfaceInViewID = -1;
		if( hands[hand].laserSightHandle == -1 )
		{
			hands[hand].laserSightHandle = gameRenderWorld->AddEntityDef( &hands[hand].laserSightRenderEntity );
		}
		else
		{
			gameRenderWorld->UpdateEntityDef( hands[hand].laserSightHandle, &hands[hand].laserSightRenderEntity );
		}
	}

	if ( !showTeleport && ( hideSight == true || sightMode == 0 ) )
	{
		hands[hand].crosshairEntity.allowSurfaceInViewID = -1;
		if ( hands[hand].crosshairHandle == -1 )
		{
			hands[hand].crosshairHandle = gameRenderWorld->AddEntityDef( &hands[hand].crosshairEntity );
		}
		else
		{
			gameRenderWorld->UpdateEntityDef( hands[hand].crosshairHandle, &hands[hand].crosshairEntity );
		}
	}


	if ( hideSight && !showTeleport ) return;

	if ( showTeleport )
	{
		GetHandOrHeadPositionWithHacks(vr_teleport.GetInteger(), muzzleOrigin, muzzleAxis);
	}

	// calculate the beam origin and length.
	start = muzzleOrigin - muzzleAxis[0] * 2.0f;
	
	if ( vr_laserSightUseOffset.GetBool() ) start += weapon->laserSightOffset * muzzleAxis;

	end = start + muzzleAxis[0] * beamLength;

	// Koz begin : Keep the lasersight from clipping through everything. 

	traceHit = gameLocal.clip.TracePoint( traceResults, start, end, MASK_SHOT_RENDERMODEL, this );
	if ( traceHit )
	{
		beamLength *= traceResults.fraction;
	}
	
	
	if ( (vr_weaponSight.GetInteger() == 0 || vr_weaponSight.GetInteger() > 3 )  && !showTeleport && !hideSight ) // using the lasersight

	{
		// only show in the player's view
		// Koz - changed show lasersight shows up in all views/reflections in VR
		hands[hand].laserSightRenderEntity.allowSurfaceInViewID = 0;// entityNumber + 1;
		hands[hand].laserSightRenderEntity.axis.Identity();
		hands[hand].laserSightRenderEntity.origin = start;
		

		// program the beam model
		idVec3&	target = *reinterpret_cast<idVec3*>( &hands[hand].laserSightRenderEntity.shaderParms[SHADERPARM_BEAM_END_X] );
		target = start + muzzleAxis[0] * beamLength;

		hands[hand].laserSightRenderEntity.shaderParms[SHADERPARM_BEAM_WIDTH] = g_laserSightWidth.GetFloat();

		if ( /*IsGameStereoRendered() &&*/ hands[hand].laserSightHandle == -1 )
		{
			hands[hand].laserSightHandle = gameRenderWorld->AddEntityDef( &hands[hand].laserSightRenderEntity );
		}
		else
		{
			gameRenderWorld->UpdateEntityDef( hands[hand].laserSightHandle, &hands[hand].laserSightRenderEntity );
		}
	}
	
	if ( vr_teleport.GetInteger() == 1 && commonVr->VR_USE_MOTION_CONTROLS && vr_weaponSight.GetInteger() == 0 )
		vr_weaponSight.SetInteger( 1 );

	sightMode = vr_weaponSight.GetInteger();
	vr_weaponSight.ClearModified();

	if ( !showTeleport && ( sightMode < 1 || hideSight )) return;

	// update the crosshair model
	// set the crosshair skin
	
	switch ( sightMode )
	{
		case 4:
		case 1:
			hands[hand].crosshairEntity.customSkin = skinCrosshairDot;
			break;

		case 5:
		case 2:
			hands[hand].crosshairEntity.customSkin = skinCrosshairCircleDot;
			break;

		case 6:
		case 3:
			hands[hand].crosshairEntity.customSkin = skinCrosshairCross;
			break;

		default:
			hands[hand].crosshairEntity.customSkin = skinCrosshairDot;

	}
		
	if ( showTeleport || sightMode > 0 ) hands[hand].crosshairEntity.allowSurfaceInViewID = entityNumber + 1;
	hands[hand].crosshairEntity.axis.Identity();
	
	static float muzscale = 0.0f ;

	muzscale = 1 + beamLength / 100;
	hands[hand].crosshairEntity.axis = muzzleAxis * muzscale;

	bool aimLadder = false, aimActor = false, aimElevator = false;
	
	static idAngles surfaceAngle = ang_zero;
	
	if ( traceHit )
	{
		muzscale = 1 + beamLength / 100;

		if ( showTeleport || vr_weaponSightToSurface.GetBool() )
		{
			aimLadder = traceResults.c.material && ( traceResults.c.material->GetSurfaceFlags() & SURF_LADDER );
			idEntity* aimEntity = gameLocal.GetTraceEntity(traceResults);
			if (aimEntity)
			{
				if (aimEntity->IsType(idActor::Type))
					aimActor = aimEntity->health > 0;
				else if (aimEntity->IsType(idElevator::Type))
					aimElevator = true;
				else if (aimEntity->IsType(idStaticEntity::Type) || aimEntity->IsType(idLight::Type))
				{
					renderEntity_t *rend = aimEntity->GetRenderEntity();
					if (rend)
					{
						idRenderModel *model = rend->hModel;
						aimElevator = (model && idStr::Cmp(model->Name(), "models/mapobjects/elevators/elevator.lwo") == 0);
					}
				}
			}

			// fake it till you make it. there must be a better way. Too bad my brain is broken.

			static idAngles muzzleAngle = ang_zero;
			static idAngles diffAngle = ang_zero;
			static float rollDiff = 0.0f;
			
			surfaceAngle = traceResults.c.normal.ToAngles().Normalize180();
			muzzleAngle = muzzleAxis.ToAngles().Normalize180();
			
			surfaceAngle.pitch *= -1;
			surfaceAngle.yaw += 180;
			surfaceAngle.Normalize180();
				
			diffAngle = idAngles( 0, 0, muzzleAngle.yaw - surfaceAngle.yaw ).Normalize180();
						
			rollDiff = diffAngle.roll * 1 / ( 90 / surfaceAngle.pitch );
						
			surfaceAngle.roll = muzzleAngle.roll - rollDiff;
			surfaceAngle.Normalize180();
		
			hands[hand].crosshairEntity.axis = surfaceAngle.ToMat3() * muzscale;
		}
		else
		{
			hands[hand].crosshairEntity.axis = muzzleAxis * muzscale;
		}
	}

	hands[hand].crosshairEntity.origin = start + muzzleAxis[0] * beamLength;
	

	
	

	// Carl: teleport
	if  ( showTeleport )
	{
		
		// teleportAimPoint is where you are actually aiming. teleportPoint is where AAS has nudged the teleport cursor to (so you can't teleport too close to a wall).
		// teleportAimPointPitch is the pitch of the surface you are aiming at, where 90 is the floor and 0 is the wall
		teleportAimPoint = hands[hand].crosshairEntity.origin;
		teleportAimPointPitch = surfaceAngle.pitch;		// if the elevator is moving up, we don't want to fall through the floor
		if ( aimElevator )
			teleportPoint = teleportAimPoint + idVec3(0, 0, 10);
		// 45 degrees is maximum slope you can walk up
		bool pitchValid = ( teleportAimPointPitch >= 45 && !aimActor ) || aimLadder; // -90 = ceiling, 0 = wall, 90 = floor
		// can always teleport into nearby elevator, otherwise we need to check
		aimValidForTeleport = pitchValid && (( aimElevator && beamLength <= 300 ) || CanReachPosition( teleportAimPoint, teleportPoint ));

		if ( aimValidForTeleport )
		{
			hands[hand].crosshairEntity.origin = teleportPoint;
			hands[hand].crosshairEntity.customSkin = skinCrosshairCircleDot;
		}
		else if ( pitchValid )
		{
			hands[hand].crosshairEntity.origin = teleportPoint;
			hands[hand].crosshairEntity.customSkin = skinCrosshairCross;
		}
		else if ( vr_teleport.GetInteger() == 1 && commonVr->VR_USE_MOTION_CONTROLS )
		{
			hands[hand].crosshairEntity.customSkin = skinCrosshairDot;
		}
		else
		{
			hands[hand].crosshairEntity.customSkin = skinCrosshairCross;
		}
	}
	else
	{
		aimValidForTeleport = false;
	}
	oldTeleport = showTeleport;

	if ( /*IsGameStereoRendered() &&*/ hands[hand].crosshairHandle == -1 )
	{
		hands[hand].crosshairHandle = gameRenderWorld->AddEntityDef( &hands[hand].crosshairEntity );
	}
	else
	{
		gameRenderWorld->UpdateEntityDef( hands[hand].crosshairHandle, &hands[hand].crosshairEntity );
	}

}


bool idPlayer::GetTeleportBeamOrigin( idVec3 &beamOrigin, idMat3 &beamAxis ) // returns true if the teleport beam should be displayed
{
	//const idVec3 beamOff[2] = { idVec3( 2.5f, 0.0f, 1.0f ), idVec3( 2.5f, 0.0f, 1.5f ) };
	const idVec3 beamOff[2] = { idVec3( 4.5f, 0.0f, 1.0f ), idVec3( 4.5f, 0.0f, 1.5f ) };

	if ( gameLocal.inCinematic || Flicksync_InCutscene || AI_DEAD || game->IsPDAOpen() )
	{
		return false;
	}

	int teleportHand, hand;
	teleportHand = vr_teleport.GetInteger();
	if( teleportHand == TP_HAND_RIGHT )
		hand = HAND_RIGHT;
	else if( teleportHand == TP_HAND_LEFT )
		hand = HAND_LEFT;
	else if( teleportHand == TP_HAND_HEAD )
		hand = vr_weaponHand.GetInteger();
	else if( teleportHand == 1 )
		hand = vr_weaponHand.GetInteger();
	else
		hand = 1 - vr_weaponHand.GetInteger();

	if ( teleportHand <= 0 || ( teleportHand == 1 && commonVr->VR_USE_MOTION_CONTROLS ) )// teleport aim mode is to use the standard weaponsight, so just return.
	{
		return false;
	}
	else if( teleportHand == TP_HAND_HEAD || !commonVr->VR_USE_MOTION_CONTROLS ) // beam originates from in front of the head
	{
		beamAxis = commonVr->lastHMDViewAxis;
		beamOrigin = commonVr->lastHMDViewOrigin + 12 * beamAxis[ 0 ];
		beamOrigin = beamOrigin + 5 * beamAxis[ 2 ];
	}
	else // teleport aim origin from the hand
	{
		if ( !hands[hand].weapon->ShowCrosshair() ||
			hands[hand].weapon->IsHidden() ||
			hands[ hand ].weapon->hideOffset != 0 ||						// Koz - turn off lasersight If gun is lowered ( in gui ).
			commonVr->handInGui ||							// turn off lasersight if hand is in gui.
			hands[ hand ].weapon.GetEntity()->GetGrabberState() >= 2 	// Koz turn off laser sight if grabber is dragging an entity
			)
		{
			return false;
		}

		if( hands[hand].holdingFlashlight() ) // flashlight is in the hand, so originate the beam slightly in front of the flashlight.
		{
			beamAxis = flashlight->GetRenderEntity()->axis;
			beamOrigin = flashlight->GetRenderEntity()->origin + 10 * beamAxis[ 0 ];
		}
		else if ( !hands[hand].controllingWeapon() ) // just send it from the hand.
		{
			if( animator.GetJointTransform( ik_hand[ hand ], gameLocal.time, beamOrigin, beamAxis ) )
			{
				beamAxis = ik_handCorrectAxis[ hand ][ 1 ].Inverse() * beamAxis;

				beamOrigin = beamOrigin * renderEntity.axis + renderEntity.origin;
				beamAxis = beamAxis * renderEntity.axis;
				beamOrigin += beamOff[ hand ] * beamAxis;
			}
			else
			{
				// we failed to get the joint for some reason, so just default to the weapon origin and axis
				beamOrigin = hands[ hand ].weapon->viewWeaponOrigin;
				beamAxis = hands[ hand ].weapon->viewWeaponAxis;
			}
		}
		else if ( !hands[ hand ].weapon->GetMuzzlePositionWithHacks( beamOrigin, beamAxis ) )
		{
			// weapon has no muzzle, so get the position and axis of the animated hand joint
			if ( animator.GetJointTransform( ik_hand[hand], gameLocal.time, beamOrigin, beamAxis ) )
			{
				beamAxis = ik_handCorrectAxis[hand][1].Inverse() * beamAxis;

				beamOrigin = beamOrigin * renderEntity.axis + renderEntity.origin;
				beamAxis = beamAxis * renderEntity.axis;
				beamOrigin += beamOff[hand] * beamAxis;
			}
			else
			{
				// we failed to get the joint for some reason, so just default to the weapon origin and axis
				beamOrigin = hands[ hand ].weapon->viewWeaponOrigin;
				beamAxis = hands[ hand ].weapon->viewWeaponAxis;
			}
		}
		else // had a valid muzzle;
		{
			beamOrigin -= 2 * beamAxis[1]; // if coming from the muzzle, move 2 in down, looks better when it doesn't interfere with the laser sight.
		}

		if ( hands[hand].weapon->IdentifyWeapon() == WEAPON_CHAINSAW )
		{
			beamOrigin += 6 * beamAxis[0]; // move the beam origin 6 inches forward
		}
		else
		{
			beamOrigin += 4 * beamAxis[0]; // move the beam origin 4 inches forward
		}
	}
	return true;
}

/*
Koz
idPlayer::UpdateTeleportAim

equation for parabola :	y = y0 + vy0 * t - .5 * g * t^2
						x = x0 + vx0 * t
*/

void idPlayer::UpdateTeleportAim()// idVec3 beamOrigin, idMat3 beamAxis )// idVec3 p0, idVec3 v0, idVec3 a, float dist, int points, idVec3 hitLocation, idVec3 hitNormal, float timeToHit )
{
	// teleport target is a .md5 model
	// model has 3 components:
	// the aiming beam comprised of a ribbon with 23 segments/24  ( joints teleportBeamJoint[ 0 - 23 ] )
	// the telepad itself (big circle with the fx and cylinder - joint teleportPadJoint )
	// the center aiming dot ( terminates the beam, shown whenever beam is on - joint teleportCenterPadJoint )
	// the origin of the model should be set to the starting point of the aiming beam
	// teleportBeamJoint[0] should also be set to the origin
	// teleportBeamJoint[1 - 22] trace the arc
	// teleportBeamJoint[23] and teleportCenterPadJoint should be set to the end position of the beam
	// teleportPadJoint should be set to the position of the teleport target ( can be different from beam if aim assist is active ) 

	const int slTime = 200; // 200ms, remove vr_teleportSlerpTime.GetFloat();

	const float grav = 9.81f * 39.3701f;

	static bool pleaseDuck = false; // Low Headroom Please Duck
		
	float numPoints = vr_teleportMaxPoints.GetFloat();// 24;
	float vel = vr_teleportVel.GetFloat();
	float dist = vr_teleportDist.GetFloat();

	trace_t traceResults;
	idVec3 last = vec3_zero;
	idVec3 next = vec3_zero;
	idVec3 endPos = vec3_zero;

	idVec3 up = idVec3( 0, 0, 1 );

	idMat3 padAxis = mat3_identity;
	float t = 0;
		
	idMat3 forward = mat3_identity;
	float beamAngle = 0.0f;
	float vx, vz = 0.0f;
	float z0 = 0.0f;
	float tDisX = 0.0f;
	float zDelt, xDelt = 0.0f;

	idVec2 st, en, df = vec2_zero;
	idVec3 jpos = vec3_zero;

	static bool isShowing = false;
	static bool wasShowing = false;

	static idVec3 beamOrigin = vec3_zero;
	static idMat3 beamAxis = mat3_identity;
	
	bool showTeleport = ( vr_teleport.GetInteger() > 1 || ( !commonVr->VR_USE_MOTION_CONTROLS && vr_teleport.GetInteger() > 0 ) ) && commonVr->teleportButtonCount != 0;
	static bool lastShowTeleport = false;

	if ( !lastShowTeleport )
	{
		isShowing = false;
	}

	lastShowTeleport = showTeleport;

	pleaseDuck = false;

	if ( !showTeleport || !GetTeleportBeamOrigin( beamOrigin, beamAxis ) )
	{
		if (vr_teleport.GetInteger() != 1)
			aimValidForTeleport = false;
		teleportTarget.GetEntity()->Hide();
		return;
	}
	
	aimValidForTeleport = false;

	forward = idAngles(0.0f, beamAxis.ToAngles().yaw, 0.0f).ToMat3();
	teleportTarget.GetEntity()->SetAxis( forward );
		
	beamAngle = idMath::ClampFloat( -65.0f, 65.0f, beamAxis.ToAngles().pitch );

	dist *= idMath::Cos( DEG2RAD( beamAngle ) ); // we want to be able to aim farther horizontally than vertically, so modify velocity and dist based on pitch.
	vel *= idMath::Cos( DEG2RAD( beamAngle ) );

	vx = vel * idMath::Cos( DEG2RAD( beamAngle ));
	vz = vel * idMath::Sin( DEG2RAD( beamAngle ));

	tDisX = dist / vel;
	
	last = beamOrigin;
	z0 = beamOrigin.z;
	
	for ( int i = 0; i < numPoints; i++ )
	{
		t += tDisX;
			
		zDelt = z0 - vz * t - 0.5 * grav * ( t * t );
		xDelt = vx * tDisX;

		next = last + forward[0] * xDelt;
		
		if ( z0 - zDelt >= vr_teleportMaxDrop.GetFloat() )
		{
			zDelt = z0 - vr_teleportMaxDrop.GetFloat();
			t = (idMath::Sqrt( (2 * grav * z0) - (2 * grav * zDelt) + (vz * vz) ) - vz) / grav;
			xDelt = vx * t;
			next = beamOrigin + forward[0] * xDelt;
			i = numPoints;
						
		}
		
		next.z = zDelt;
		teleportPoint = teleportAimPoint = next;

		padAxis = forward;
		
		if ( gameLocal.clip.TracePoint( traceResults, last, next, MASK_SHOT_RENDERMODEL, this ) )
		{
				
			const char * hitMat;
			
			hitMat = traceResults.c.material->GetName();
			float hitPitch = traceResults.c.normal.ToAngles().pitch;

			// handrails really make aiming suck, so skip any non floor hits that consist of these materials
			// really need to verify this doesn't break anything.
			// Carl: It does break teleporting onto handrails, which I intended to be able to do.

			
			if ( vr_teleportSkipHandrails.GetInteger() == 1 && hitPitch != -90 )
			{
				if ( idStr::FindText( hitMat, "base_trim" ) > -1 ||
					idStr::FindText( hitMat, "swatch" ) > -1 ||
					idStr::FindText( hitMat, "mchangar2" ) > -1 ||
					idStr::FindText( hitMat, "mchangar3" ) > -1 )
				{

					common->Printf( "Beam hit rejected: material %s hitpitch %f\n", hitMat,hitPitch );
					last = next;
					endPos = last;
					continue;
				} 

			}
								
			//common->Printf( "Beam hit material = %s\n", traceResults.c.material->GetName() );
			next = traceResults.c.point;
			endPos = next;
			
			
			//set the axis for the telepad to match the surface
			static idAngles surfaceAngle = ang_zero;

			static idQuat lastQ = idAngles( 0.0f, 0.0f, 90.0f ).ToQuat();
			static idQuat nextQ = lastQ;
			static idQuat lastSet = lastQ;

			static idMat3 lastAxis = mat3_zero;
			static idMat3 curAxis = mat3_zero;

			static int slerpEnd = Sys_Milliseconds() - 500;
			static idVec3 lastHitNormal = vec3_zero;

			idVec3 hitNormal = traceResults.c.normal;

			static idAngles muzzleAngle = ang_zero;
			static idAngles diffAngle = ang_zero;
			static float rollDiff = 0.0f;

			surfaceAngle = traceResults.c.normal.ToAngles().Normalize180();
			muzzleAngle = beamAxis.ToAngles().Normalize180();
			muzzleAngle.roll = 0;

			surfaceAngle.pitch *= -1;
			surfaceAngle.yaw += 180;
			surfaceAngle.Normalize180();

			diffAngle = idAngles( 0, 0, muzzleAngle.yaw - surfaceAngle.yaw ).Normalize180();

			rollDiff = diffAngle.roll * 1 / (90 / surfaceAngle.pitch);

			surfaceAngle.roll = muzzleAngle.roll - rollDiff;
			surfaceAngle.Normalize180();
			curAxis = surfaceAngle.ToMat3();
						
			if ( hitNormal != lastHitNormal )
			{

				lastHitNormal = hitNormal;
								
				if ( slerpEnd - Sys_Milliseconds() <= 0 )
				{
					lastQ = lastAxis.ToQuat();
				}
				else
				{
					lastQ = lastSet;
				}

				slerpEnd = Sys_Milliseconds() + slTime;
				nextQ = curAxis.ToQuat();

			}
			
			if ( slerpEnd - Sys_Milliseconds() <= 0 )
			{
				padAxis = curAxis;
			}
			else
			{
				float qt = (float)((float)(slTime + 1.0f) - (float)(slerpEnd - Sys_Milliseconds())) / (float)slTime;
				lastSet.Slerp( lastQ, nextQ, qt );
				padAxis = lastSet.ToMat3();
			}
						
			lastAxis = curAxis;
			
			bool aimLadder = false, aimActor = false, aimElevator = false;
			aimLadder = traceResults.c.material && (traceResults.c.material->GetSurfaceFlags() & SURF_LADDER);
			idEntity* aimEntity = gameLocal.GetTraceEntity(traceResults);
			if (aimEntity)
			{
				if (aimEntity->IsType(idActor::Type))
					aimActor = aimEntity->health > 0;
				else if (aimEntity->IsType(idElevator::Type))
					aimElevator = true;
				else if (aimEntity->IsType(idStaticEntity::Type) || aimEntity->IsType(idLight::Type))
				{
					renderEntity_t *rend = aimEntity->GetRenderEntity();
					if (rend)
					{
						idRenderModel *model = rend->hModel;
						aimElevator = (model && idStr::Cmp(model->Name(), "models/mapobjects/elevators/elevator.lwo") == 0);
					}
				}
			}

			teleportPoint = teleportAimPoint = traceResults.c.point;
			float beamLengthSquared = 0;
			if (aimElevator)
			{
				teleportPoint = teleportAimPoint + idVec3(0, 0, 10);
				beamLengthSquared = (teleportPoint - beamOrigin).LengthSqr();
			}
			if ((hitPitch >= -90.0f && hitPitch <= -45.0f && !aimActor) || aimLadder)
			{
				bool aimValid = (aimElevator && beamLengthSquared <= 300 * 300) || CanReachPosition(teleportAimPoint, teleportPoint);
				// check if we are teleporting OUT of an elevator
				if (!aimValid && !aimActor && fabs(teleportPoint.z - physicsObj.GetOrigin().z) <= 10 && (teleportPoint - beamOrigin).LengthSqr() <= 300 * 300)
				{
					// do a trace to see if we're in an elevator, and if so, set aimValid to true
					trace_t result;
					physicsObj.ClipTranslation(result, GetPhysics()->GetGravityNormal() * 10, NULL);
					if (result.fraction < 1.0f)
					{
						aimEntity = gameLocal.GetTraceEntity(result);
						if (aimEntity)
						{
							if (aimEntity->IsType(idElevator::Type))
								aimValid = true;
							else if (aimEntity->IsType(idStaticEntity::Type) || aimEntity->IsType(idLight::Type))
							{
								renderEntity_t *rend = aimEntity->GetRenderEntity();
								if (rend)
								{
									idRenderModel *model = rend->hModel;
									aimValid = (model && idStr::Cmp(model->Name(), "models/mapobjects/elevators/elevator.lwo") == 0);
								}
							}
						}
					}
				}
				if ( aimValid )
				{
										
					// pitch indicates a flat surface or <45 deg slope,
					// check to see if a clip test passes at the location AFTER
					// checking reachability. 
					// the clip test will prevent us from teleporting too close to walls.

					static trace_t trace;
					static idClipModel* clip;
					static idMat3 clipAxis;
					static idVec3 tracePt;
					tracePt = teleportPoint;

					clip = physicsObj.GetClipModel();
					clipAxis = physicsObj.GetClipModel()->GetAxis();

					gameLocal.clip.Translation(trace, tracePt, tracePt, clip, clipAxis, CONTENTS_SOLID, NULL);
					// Carl: check if we're on stairs
					if (trace.fraction < 1.0f)
					{
						tracePt.z += pm_stepsize.GetFloat();
						gameLocal.clip.Translation(trace, tracePt, tracePt, clip, clipAxis, CONTENTS_SOLID, NULL);
					}

					if (trace.fraction < 1.0f)
					{
						aimValidForTeleport = false;
						isShowing = false;
						// Koz
						//please duck sometimes shows incorrectly if a moveable object is in the way.
						//add a clip check at the crouch height, and if it passes, show the please duck graphic.
						//otherwise something is in the way.

						if ( clip->GetBounds()[1][2] == pm_crouchheight.GetFloat() ) // player is already crouching and we failed the clip test
						{
							pleaseDuck = false;
						}
						else
						{
							
							// change bounds to crouch height and check again.
							idBounds bounds = clip->GetBounds();
							bounds[1][2] = pm_crouchheight.GetFloat();
							if ( pm_usecylinder.GetBool() )
							{
								clip->LoadModel( idTraceModel( bounds, 8 ) );
							}
							else
							{
								clip->LoadModel( idTraceModel( bounds ) );
							}

							gameLocal.clip.Translation( trace, tracePt, tracePt, clip, clipAxis, CONTENTS_SOLID, NULL );
							
							pleaseDuck = trace.fraction >= 1.0f ? true : false;
							
							//reset bounds
							bounds[1][2] = pm_normalheight.GetFloat();
							if ( pm_usecylinder.GetBool() )
							{
								clip->LoadModel( idTraceModel( bounds, 8 ) );
							}
							else
							{
								clip->LoadModel( idTraceModel( bounds ) );
							}
						}
					}
					else
					{
						aimValidForTeleport = true;
						if ( !isShowing )
						{
							slerpEnd = Sys_Milliseconds() - 100;
							padAxis = curAxis;
							lastAxis = curAxis;
						}
												
						teleportTarget.GetEntity()->GetRenderEntity()->weaponDepthHack = false;
						isShowing = true;
					}
				}
			}
			
			const idMat3 correct = idAngles( -90.0f, 0.0f, 90.0f ).ToMat3();
			padAxis *= forward.Inverse();
			padAxis = correct * padAxis;
			
			break;
		}
	
		last = next;
		endPos = last;
	}
		
	teleportTarget.GetEntity()->SetOrigin( beamOrigin );

	// an approximation of a parabola has been scanned for surface hits and the endpoint has been calculated.
	// update the beam model by setting the joint positions along the beam to trace the arc.
	// joint 0 should be set to the origin, and the origin of the model is set to the origin of the beam in worldspace.
	
	teleportTargetAnimator->SetJointPos( teleportBeamJoint[0], JOINTMOD_WORLD_OVERRIDE, vec3_zero ); // joint 0 always the origin.
		
	st = beamOrigin.ToVec2();
	en = endPos.ToVec2();
		
	tDisX = ( (en - st).Length() / vx ) / 23.0f; // time for each segment of arc at velocity vx.
	t = 0.0f;
	
	next = beamOrigin;
	
	idMat3 forwardInv = forward.Inverse();

	for ( int i = 1; i < 23; i++ )
	{
		t += tDisX;
	
		zDelt = z0 - vz * t - 0.5f * grav * (t * t);
		xDelt = vx * tDisX;

		next = next + forward[0] * xDelt;
		next.z = zDelt;

		jpos = ( next - beamOrigin ) * forwardInv;
		teleportTargetAnimator->SetJointPos( teleportBeamJoint[i], JOINTMOD_WORLD_OVERRIDE, jpos );
	}
		
	jpos = ( endPos - beamOrigin ) * forwardInv;
	teleportTargetAnimator->SetJointPos( teleportBeamJoint[23], JOINTMOD_WORLD_OVERRIDE, jpos );
	//teleportCenterPadJoint
	
	//set the center aiming point
	jpos = (teleportAimPoint - beamOrigin) * forwardInv;
	teleportTargetAnimator->SetJointPos( teleportCenterPadJoint, JOINTMOD_WORLD_OVERRIDE, jpos );
	teleportTargetAnimator->SetJointAxis( teleportCenterPadJoint, JOINTMOD_WORLD_OVERRIDE, padAxis );
	
	if ( vr_teleportShowAimAssist.GetInteger() )
	{
		// if we want to have the telepad reflect the aim assist, update the joint for the telepad
		//otherwise it will use the same origin as the aiming point
		jpos = (teleportPoint - beamOrigin) * forwardInv;
	}
			
	teleportTargetAnimator->SetJointPos( teleportPadJoint, JOINTMOD_WORLD_OVERRIDE, jpos );
	teleportTargetAnimator->SetJointAxis( teleportPadJoint, JOINTMOD_WORLD_OVERRIDE, padAxis );
	
	if ( !aimValidForTeleport )
	{
		// this will show the beam, but hide the teleport target
		
		if ( pleaseDuck )
		{
			teleportTarget.GetEntity()->GetRenderEntity()->shaderParms[0] = 1;
			teleportTarget.GetEntity()->GetRenderEntity()->shaderParms[1] = 1;
			teleportTarget.GetEntity()->Show();
			teleportTarget.GetEntity()->GetRenderEntity()->customSkin = skinTelepadCrouch;
			isShowing = true;

			if ( !vr_teleportHint.GetBool() )
			{
				ShowTip( "Duck! Low Headroom!", "If the teleport target turns red, there is limited headroom at the teleport destination. You must crouch before you can teleport to this location.", false );
				vr_teleportHint.SetBool( true );
			}
			//gameRenderWorld->DrawText( "Low Headroom\nPlease Duck", teleportPoint + idVec3( 0, 0, 18 ), 0.2f, colorOrange, viewAngles.ToMat3() );
		}
		else
		{
			teleportTarget.GetEntity()->GetRenderEntity()->customSkin = NULL;
			teleportTarget.GetEntity()->GetRenderEntity()->shaderParms[0] = 0;
			teleportTarget.GetEntity()->GetRenderEntity()->shaderParms[1] = 0;
			teleportTarget.GetEntity()->Show();
			isShowing = false;
		}
	}
	else
	{
		teleportTarget.GetEntity()->GetRenderEntity()->customSkin = NULL;
		teleportDir = ( physicsObj.GetOrigin() - teleportPoint );
		teleportDir.Normalize();
		teleportTarget.GetEntity()->GetRenderEntity()->shaderParms[0] = 255;
		teleportTarget.GetEntity()->GetRenderEntity()->shaderParms[1] = 1;
		teleportTarget.GetEntity()->Show();
		isShowing = true;
	}
	
	teleportTarget.GetEntity()->Present();
	
	return;
}

/*
Koz
idPlayer::UpdateHeadingBeam
*/
void idPlayer::UpdateHeadingBeam()
{
	
	if ( vr_headingBeamMode.IsModified() )
	{
		int mode = vr_headingBeamMode.GetInteger();
		vr_headingBeamMode.ClearModified();


		switch ( mode )
		{

			case 0:
				headingBeamActive = false;
				break;

			case 1:
				headingBeamEntity.customSkin = skinHeadingSolid;
				headingBeamActive = true;
				break;

			case 2:
				headingBeamEntity.customSkin = skinHeadingArrows;
				headingBeamActive = true;
				break;

			case 3:
				headingBeamEntity.customSkin = skinHeadingArrowsScroll;
				headingBeamActive = true;
				break;

			default:
				headingBeamEntity.customSkin = skinHeadingArrowsScroll;
				headingBeamActive = true;

		}
	}

	if ( !headingBeamActive  )
	{
		// hide it
		headingBeamEntity.allowSurfaceInViewID = -1;
			
	}
	else 
	{
		idVec3 beamOrigin = GetEyePosition();
		idMat3 beamAxis = idAngles( 0.0f, viewAngles.yaw, 0.0f ).ToMat3();
		
		beamOrigin.z -= (pm_normalviewheight.GetFloat());
		
		headingBeamEntity.axis = beamAxis;
		headingBeamEntity.origin = beamOrigin;
		headingBeamEntity.bounds.Zero();

		headingBeamEntity.allowSurfaceInViewID = entityNumber + 1 ;
				
	}
	
	// make sure the entitydefs are updated
	if ( headingBeamHandle == -1 )
	{
		headingBeamHandle = gameRenderWorld->AddEntityDef( &headingBeamEntity );
	}
	else
	{
		gameRenderWorld->UpdateEntityDef( headingBeamHandle, &headingBeamEntity );
	}

}

/*
==============
Koz
idPlayer::UpdateVrHud
==============
*/
void idPlayer::UpdateVrHud()
{
	static idVec3 hudOrigin;
	static idMat3 hudAxis;
	float hudPitch;

	// update the hud model
	if ( ( !hudActive && ( vr_hudLowHealth.GetInteger() == 0 ) ) || commonVr->PDAforced || game->IsPDAOpen() )
	{
		// hide it
		hudEntity.allowSurfaceInViewID = -1;
	}
	else
	{
		// always show HUD if in flicksync
		if ( vr_flicksyncCharacter.GetInteger() )
			hudEntity.allowSurfaceInViewID = 0;
		else
			hudEntity.allowSurfaceInViewID = entityNumber + 1;
		
		if ( vr_hudPosLock.GetInteger() == 1 && !commonVr->thirdPersonMovement ) // hud in fixed position in space, except if running in third person, then attach to face.
		{
			hudPitch = vr_hudType.GetInteger() == VR_HUD_LOOK_DOWN ? vr_hudPosAngle.GetFloat() : 10.0f;
			
			float yaw;
			if( gameLocal.inCinematic )
			{
				yaw = commonVr->lastHMDViewAxis.ToAngles().yaw;
				hudOrigin = commonVr->lastHMDViewOrigin;
			}
			else
			{
				GetViewPos( hudOrigin, hudAxis );
				yaw = viewAngles.yaw;
			}
			hudAxis = idAngles( hudPitch, yaw, 0.0f ).ToMat3();

			hudOrigin += hudAxis[0] * vr_hudPosDis.GetFloat();
			hudOrigin += hudAxis[1] * vr_hudPosHor.GetFloat();
			hudOrigin.z += vr_hudPosVer.GetFloat();
		}
		else // hud locked to face
		{
			if ( commonVr->thirdPersonMovement )
			{
				hudAxis = commonVr->thirdPersonHudAxis;
				hudOrigin = commonVr->thirdPersonHudPos;
			}
			else
			{
				hudAxis = commonVr->lastHMDViewAxis;
				hudOrigin = commonVr->lastHMDViewOrigin;
			}
			hudOrigin += hudAxis[0] * vr_hudPosDis.GetFloat(); 
			hudOrigin += hudAxis[1] * vr_hudPosHor.GetFloat();
			hudOrigin += hudAxis[2] * vr_hudPosVer.GetFloat();
			
		}
		
		hudAxis *= vr_hudScale.GetFloat();

		hudEntity.axis = hudAxis;
		hudEntity.origin = hudOrigin;
		hudEntity.weaponDepthHack = vr_hudOcclusion.GetBool();

	}

	if ( hudHandle == -1 )
	{
		hudHandle = gameRenderWorld->AddEntityDef( &hudEntity );
	}
	else
	{
		gameRenderWorld->UpdateEntityDef( hudHandle, &hudEntity );
	}
}

/*
==============
idPlayer::SetFlashHandPose() // Call set flashlight hand pose script function
Updates the pose of the player model flashlight hand
======
*/
void idPlayer::SetFlashHandPose() // Call set flashlight hand pose script function
{

	const function_t* func;

	func = scriptObject.GetFunction( "SetFlashHandPose" ); // Set flashlight hand pose
	if ( func )
	{
		// use the frameCommandThread since it's safe to use outside of framecommands
		// Koz debug common->Printf( "Calling SetFlashHandPose\n" ); // Set flashlight hand pose
		gameLocal.frameCommandThread->CallFunction( this, func, true );
		gameLocal.frameCommandThread->Execute();

	}
	else
	{
		common->Warning( "Can't find function 'SetFlashHandPose' in object '%s'", scriptObject.GetTypeName() ); // Set flashlight hand pose
		return;
	}
}

/*
==============
idPlayer::SetWeaponHandPose()
Updates the pose of the player model weapon hand
======
*/
void idPlayer::SetWeaponHandPose()
{
	const function_t* func;
	func = scriptObject.GetFunction( "SetWeaponHandPose" );
	if ( func )
	{
		// use the frameCommandThread since it's safe to use outside of framecommands
		//common->Printf( "Calling SetWeaponHandPose\n" );g
		gameLocal.frameCommandThread->CallFunction( this, func, true );
		gameLocal.frameCommandThread->Execute();

	}
	else
	{
		common->Warning( "Can't find function 'SetWeaponHandPose' in object '%s'", scriptObject.GetTypeName() );
		return;
	}
}

/*
==============
idPlayer::UpdatePlayerSkinsPoses()
Updates the skins of the weapon and flashlight to hide/show arms/watch and updates poses of player model hands.
======
*/
void idPlayer::UpdatePlayerSkinsPoses()
{
	for( int h = 0; h < 2; h++ )
	{
		if( hands[ h ].weapon.GetEntity() )
			hands[ h ].weapon->UpdateSkin();
	}
	if ( flashlight )
	{
		flashlight->UpdateSkin();
	}
	SetFlashHandPose(); // Call set flashlight hand pose script function
	SetWeaponHandPose();
}
/*
==============
idPlayer::Think

Called every tic for each player
==============
*/
void idPlayer::Think()
{
	playedTimeResidual += ( gameLocal.time - gameLocal.previousTime );
	playedTimeSecs += playedTimeResidual / 1000;
	playedTimeResidual = playedTimeResidual % 1000;
	
	aimAssist.Update();
	
	UpdatePlayerIcons();
	
	UpdateSkinSetup();
			
	buttonMask &= usercmd.buttons;
	usercmd.buttons &= ~buttonMask;
	
	// clear the ik before we do anything else so the skeleton doesn't get updated twice
	walkIK.ClearJointMods();

	// Koz
	armIK.ClearJointMods();
	
	// if this is the very first frame of the map, set the delta view angles
	// based on the usercmd angles
	if( !spawnAnglesSet && ( gameLocal.GameState() != GAMESTATE_STARTUP ) )
	{
		spawnAnglesSet = true;
		SetViewAngles( spawnAngles );
		oldImpulseSequence = usercmd.impulseSequence;
		// Koz fixme recenter ovr_RecenterTrackingOrigin( commonVr->hmdSession ); // Koz reset hmd orientation  Koz fixme check if still appropriate here.


	}
	
	if( mountedObject )
	{
		usercmd.forwardmove = 0;
		usercmd.rightmove = 0;
		usercmd.buttons &= ~( BUTTON_JUMP | BUTTON_CROUCH );
	}
	
	if( objectiveSystemOpen || gameLocal.inCinematic || Flicksync_InCutscene || influenceActive )
	{
		if( objectiveSystemOpen && AI_PAIN )
		{
			common->Printf( "idPlayer::Think calling TogglePDA\n" );
			TogglePDA( 1 - vr_weaponHand.GetInteger() );
		}
		usercmd.forwardmove = 0;
		usercmd.rightmove = 0;
		usercmd.buttons &= ~( BUTTON_JUMP | BUTTON_CROUCH );
	}
	
	// log movement changes for weapon bobbing effects
	if( usercmd.forwardmove != oldCmd.forwardmove )
	{
		loggedAccel_t*	acc = &loggedAccel[currentLoggedAccel & ( NUM_LOGGED_ACCELS - 1 )];
		currentLoggedAccel++;
		acc->time = gameLocal.time;
		acc->dir[0] = usercmd.forwardmove - oldCmd.forwardmove;
		acc->dir[1] = acc->dir[2] = 0;
	}
	
	if( usercmd.rightmove != oldCmd.rightmove )
	{
		loggedAccel_t*	acc = &loggedAccel[currentLoggedAccel & ( NUM_LOGGED_ACCELS - 1 )];
		currentLoggedAccel++;
		acc->time = gameLocal.time;
		acc->dir[1] = usercmd.rightmove - oldCmd.rightmove;
		acc->dir[0] = acc->dir[2] = 0;
	}
	
	// Koz, no zoom in VR.
	/*
	
		// zooming
		if( ( usercmd.buttons ^ oldCmd.buttons ) & BUTTON_ZOOM )
		{
			if( ( usercmd.buttons & BUTTON_ZOOM ) && weapon.GetEntity() )
			{
				zoomFov.Init( gameLocal.time, 200.0f, CalcFov( false ), weapon.GetEntity()->GetZoomFov() );
			}
			else
			{
				zoomFov.Init( gameLocal.time, 200.0f, zoomFov.GetCurrentValue( gameLocal.time ), DefaultFov() );
			}
		}
	*/

	// if we have an active gui, we will unrotate the view angles as
	// we turn the mouse movements into gui events
	idUserInterface* gui = ActiveGui();
	if( gui && gui != focusUI )
	{
		RouteGuiMouse( gui );
	}
	
	// set the push velocity on the weapons before running the physics
	for( int h = 0; h < 2; h++ )
	{
		if( hands[h].weapon.GetEntity() )
			hands[h].weapon.GetEntity()->SetPushVelocity( physicsObj.GetPushedLinearVelocity() );
	}
	
	EvaluateControls();
	
	if( !af.IsActive() )
	{
		AdjustBodyAngles();
		CopyJointsFromBodyToHead();
	}
	
	if( IsLocallyControlled() )
	{
		// Local player on the server. Do normal movement.
		Move();
	//	common->Printf( "HideOffset = %f\n", weapon->hideOffset );
		
		SetWeaponHandPose();
		SetFlashHandPose(); // Call set flashlight hand pose script function
		
	}
	else
	{
		// Server is processing a client. Run client's commands like normal...
		Move();
		
		// ...then correct if needed.
		RunPhysics_RemoteClientCorrection();
	}

	if ( !g_stopTime.GetBool() ) // && !commonVr->VR_GAME_PAUSED )  // Koz pause vr
	{
	
		if( !noclip && !spectating && ( health > 0 ) && !IsHidden() )
		{
			TouchTriggers();
		}
		
		// not done on clients for various reasons. don't do it on server and save the sound channel for other things
		if( !common->IsMultiplayer() )
		{
			SetCurrentHeartRate();
			float scale = new_g_damageScale;
			if( g_useDynamicProtection.GetBool() && scale < 1.0f && gameLocal.time - lastDmgTime > 500 )
			{
				if( scale < 1.0f )
				{
					scale += 0.05f;
				}
				if( scale > 1.0f )
				{
					scale = 1.0f;
				}
				new_g_damageScale = scale;
			}
		}
		
		// update GUIs, Items, and character interactions
		
		// UpdateFocus(); // Koz moved to just before update flashlight.
				
		UpdateLocation();
		
		// update player script
		UpdateScript(); 
		
		// service animations
		if( !spectating && !af.IsActive() && !gameLocal.inCinematic )
		{
			UpdateConditions();
			UpdateAnimState();
			CheckBlink();
		}
		
		// clear out our pain flag so we can tell if we recieve any damage between now and the next time we think
		AI_PAIN = false;
	}
		
	// calculate the exact bobbed view position, which is used to
	// position the view weapon, among other things
	CalculateFirstPersonView();
	
	// this may use firstPersonView, or a thirdPeroson / camera view
	CalculateRenderView();
	
	inventory.UpdateArmor();
	
	if( spectating )
	{
		UpdateSpectating();
	}
	else if( health > 0 )
	{
		UpdateWeapon();
	}
	
	// Koz
	UpdateNeckPose();
	
	UpdateFocus(); // Koz move here update GUIs, Items, and character interactions.

	UpdateFlashlight();
	
	UpdateAir();
	
	UpdatePowerupHud();
	
	UpdateHud();
	
	UpdatePowerUps();
	
	UpdateDeathSkin( false );
	
	if( common->IsMultiplayer() )
	{
		DrawPlayerIcons();
		
		if( enviroSuitLight.IsValid() )
		{
			idAngles lightAng = firstPersonViewAxis.ToAngles();
			idVec3 lightOrg = firstPersonViewOrigin;
			const idDict* lightDef = gameLocal.FindEntityDefDict( "envirosuit_light", false );
			
			idVec3 enviroOffset = lightDef->GetVector( "enviro_offset" );
			idVec3 enviroAngleOffset = lightDef->GetVector( "enviro_angle_offset" );
			
			lightOrg += ( enviroOffset.x * firstPersonViewAxis[0] );
			lightOrg += ( enviroOffset.y * firstPersonViewAxis[1] );
			lightOrg += ( enviroOffset.z * firstPersonViewAxis[2] );
			lightAng.pitch += enviroAngleOffset.x;
			lightAng.yaw += enviroAngleOffset.y;
			lightAng.roll += enviroAngleOffset.z;
			
			enviroSuitLight.GetEntity()->GetPhysics()->SetOrigin( lightOrg );
			enviroSuitLight.GetEntity()->GetPhysics()->SetAxis( lightAng.ToMat3() );
			enviroSuitLight.GetEntity()->UpdateVisuals();
			enviroSuitLight.GetEntity()->Present();
		}
	}
	
	
	
	static int lastFlashlightMode = commonVr->GetCurrentFlashlightMode();
	//static bool lastViewArms = vr_viewModelArms.GetBool();
	static bool lastFists = false;
	static bool lastHandInGui = false;

	


	// Koz check for forced standard controller
	if( commonVr->VR_USE_MOTION_CONTROLS && vr_controllerStandard.GetInteger() )
		commonVr->VR_USE_MOTION_CONTROLS = false;

	// Koz turn body on or off in vr, update hand poses/skins if body or weapon hand changes.
	if ( game->isVR )
	{
				
		if ( vr_weaponHand.IsModified()  )
		{
			UpdatePlayerSkinsPoses();
			vr_weaponHand.ClearModified();

		}

		if ( vr_flashlightMode.IsModified() || lastFlashlightMode != commonVr->GetCurrentFlashlightMode() )
		{
			
			if ( vr_flashlightMode.IsModified() )
			{
				commonVr->currentFlashlightMode = vr_flashlightMode.GetInteger();
				vr_flashlightMode.ClearModified();
			}

			lastFlashlightMode = commonVr->GetCurrentFlashlightMode();
			UpdatePlayerSkinsPoses();
		}
	}
	// Koz end

	renderEntity_t* headRenderEnt = NULL;
	if( head.GetEntity() )
	{
		headRenderEnt = head.GetEntity()->GetRenderEntity();
	}
	if( headRenderEnt )
	{
		if( influenceSkin )
		{
			headRenderEnt->customSkin = influenceSkin;
		}
		else
		{
			headRenderEnt->customSkin = NULL;
		}

		// Koz show the head if the player is using third person movement mode && the model has moved more than 8 inches.
		if ( commonVr->thirdPersonMovement && commonVr->thirdPersonDelta > 45.0f )
		{
			headRenderEnt->suppressSurfaceInViewID = 0; // show head
			headRenderEnt->allowSurfaceInViewID = 0;
		}
		else
		{
			if ( vr_playerBodyMode.GetInteger() != 0 ) // not showing the body
			{
				headRenderEnt->allowSurfaceInViewID = -1; // hide the head, even in mirror views.
			}
			else
			{
				//headRenderEnt->suppressSurfaceInViewID = entityNumber + 1;
				headRenderEnt->allowSurfaceInViewID = 0; // show the head.
			}
		}
	}
	
	if( common->IsMultiplayer() || g_showPlayerShadow.GetBool() )
	{
		renderEntity.suppressShadowInViewID	= 0;
		if( headRenderEnt )
		{
			headRenderEnt->suppressShadowInViewID = 0;
		}
	}
	else
	{
		renderEntity.suppressShadowInViewID	= entityNumber + 1;
		if( headRenderEnt )
		{
			// Koz begin
			if ( game->isVR && vr_playerBodyMode.GetInteger() == 0 ) // show the head shadow
			{
				headRenderEnt->suppressShadowInViewID = 0; 
			}
			else
			{
				headRenderEnt->suppressShadowInViewID = entityNumber + 1;
			}
			// Koz end
		}
	}
	
	// never cast shadows from our first-person muzzle flashes
	// Koz fixme shadows
	renderEntity.suppressShadowInLightID = LIGHTID_VIEW_MUZZLE_FLASH + entityNumber;
	if( headRenderEnt )
	{
		headRenderEnt->suppressShadowInLightID = LIGHTID_VIEW_MUZZLE_FLASH + entityNumber;
	}
	

	if ( !g_stopTime.GetBool() ) //&& !commonVr->VR_GAME_PAUSED )  // Koz fixme pause in VR
	{
		
		if ( armIK.IsInitialized() ) armIK.Evaluate();
		
		UpdateAnimation();
		
		Present();
		
		UpdateDamageEffects();
		
		LinkCombat();
		
		playerView.CalculateShake();
	}
	
	if( !( thinkFlags & TH_THINK ) )
	{
		gameLocal.Printf( "player %d not thinking?\n", entityNumber );
	}
	
	if( g_showEnemies.GetBool() )
	{
		idActor* ent;
		int num = 0;
		for( ent = enemyList.Next(); ent != NULL; ent = ent->enemyNode.Next() )
		{
			gameLocal.Printf( "enemy (%d)'%s'\n", ent->entityNumber, ent->name.c_str() );
			gameRenderWorld->DebugBounds( colorRed, ent->GetPhysics()->GetBounds().Expand( 2 ), ent->GetPhysics()->GetOrigin() );
			num++;
		}
		gameLocal.Printf( "%d: enemies\n", num );
	}
	
	inventory.RechargeAmmo( this );
	
	if( healthRecharge )
	{
		int elapsed = gameLocal.time - lastHealthRechargeTime;
		if( elapsed >= rechargeSpeed )
		{
			int intervals = ( gameLocal.time - lastHealthRechargeTime ) / rechargeSpeed;
			Give( "health", va( "%d", intervals ), ITEM_GIVE_FEEDBACK | ITEM_GIVE_UPDATE_STATE, -1 );
			lastHealthRechargeTime += intervals * rechargeSpeed;
		}
	}
	
	// determine if portal sky is in pvs
	gameLocal.portalSkyActive = gameLocal.pvs.CheckAreasForPortalSky( gameLocal.GetPlayerPVS(), GetPhysics()->GetOrigin() );
	
	// stereo rendering laser sight that replaces the crosshair
	for( int h = 0; h < 2; h++ )
	{
		UpdateLaserSight( h );
	}
	UpdateTeleportAim();

	if ( vr_teleportMode.GetInteger() != 0 )
	{
		if  (warpMove ) {
			if ( gameLocal.time > warpTime )
			{
				extern idCVar timescale;
				warpTime = 0;
				noclip = false;
				warpMove = false;
				warpVel = vec3_origin;
				timescale.SetFloat( 1.0f );
				//playerView.EnableBFGVision(false);
				Teleport( warpDest, viewAngles, NULL ); //Carl: get the destination exact
			}
			physicsObj.SetLinearVelocity( warpVel );
		}

		if ( jetMove ) {
			
			if ( gameLocal.time > jetMoveTime )
			{
				jetMoveTime = 0;
				jetMove = false;
				jetMoveVel = vec3_origin;
				jetMoveCoolDownTime = gameLocal.time + 30;
			}
			
			physicsObj.SetLinearVelocity( jetMoveVel );

		}
	}

	if ( game->isVR ) UpdateHeadingBeam(); // Koz

	// Show the respawn hud message if necessary.
	if( common->IsMultiplayer() && ( minRespawnTime != maxRespawnTime ) )
	{
		if( gameLocal.previousTime < minRespawnTime && minRespawnTime <= gameLocal.time )
		{
			// Server will show the hud message directly.
			ShowRespawnHudMessage();
		}
	}
	
	// Make sure voice groups are set to the right team
	if( common->IsMultiplayer() && session->GetState() >= idSession::INGAME && entityNumber < MAX_CLIENTS )  		// The entityNumber < MAX_CLIENTS seems to quiet the static analyzer
	{
		// Make sure we're on the right team (at the lobby level)
		const int voiceTeam = spectating ? LOBBY_SPECTATE_TEAM_FOR_VOICE_CHAT : team;
		
		//idLib::Printf( "SERVER: Sending voice %i / %i\n", entityNumber, voiceTeam );
		
		// Update lobby team
		session->GetActingGameStateLobbyBase().SetLobbyUserTeam( gameLocal.lobbyUserIDs[ entityNumber ], voiceTeam );
		
		// Update voice groups to match in case something changed
		session->SetVoiceGroupsToTeams();
	}
	UpdatePDASlot();
	UpdateHolsterSlot();

	if( vr_slotDebug.GetBool() )
	{
		for( int i = 0; i < SLOT_COUNT; i++ )
		{
			idVec3 slotOrigin = slots[i].origin;
			if ( vr_weaponHand.GetInteger() && i != SLOT_FLASHLIGHT_SHOULDER )
				slotOrigin.y *= -1;
			idVec3 origin = waistOrigin + slotOrigin * waistAxis;
			idSphere tempSphere( origin, sqrtf(slots[i].radiusSq) );
			gameRenderWorld->DebugSphere( colorWhite, tempSphere, 18, true );
		}
	}
}

/*
=================
idPlayer::StartHealthRecharge
=================
*/
void idPlayer::StartHealthRecharge( int speed )
{
	lastHealthRechargeTime = gameLocal.time;
	healthRecharge = true;
	rechargeSpeed = speed;
}

/*
=================
idPlayer::StopHealthRecharge
=================
*/
void idPlayer::StopHealthRecharge()
{
	healthRecharge = false;
}

/*
=================
idPlayer::GetCurrentWeapon
=================
*/
idStr idPlayer::GetCurrentWeapon()
{
	return hands[ vr_weaponHand.GetInteger() ].GetCurrentWeaponString();
}

/*
=================
idPlayerHand::GetCurrentWeaponString
=================
*/
idStr idPlayerHand::GetCurrentWeaponString()
{
	const char* weapon;

	if( currentWeapon >= 0 && owner )
	{
		weapon = owner->spawnArgs.GetString( va( "def_weapon%d", currentWeapon ) );
		return weapon;
	}
	else
	{
		return "";
	}
}

/*
=================
idPlayer::CanGive
=================
*/
bool idPlayer::CanGive( const char* statname, const char* value )
{
	if( AI_DEAD )
	{
		return false;
	}
	
	if( !idStr::Icmp( statname, "health" ) )
	{
		if( health >= inventory.maxHealth )
		{
			return false;
		}
		return true;
	}
	else if( !idStr::Icmp( statname, "stamina" ) )
	{
		if( stamina >= 100 )
		{
			return false;
		}
		return true;
		
	}
	else if( !idStr::Icmp( statname, "heartRate" ) )
	{
		return true;
		
	}
	else if( !idStr::Icmp( statname, "air" ) )
	{
		if( airMsec >= pm_airMsec.GetInteger() )
		{
			return false;
		}
		return true;
	}
	else
	{
		return inventory.CanGive( this, spawnArgs, statname, value );
	}
}

/*
=================
idPlayer::StopHelltime

provides a quick non-ramping way of stopping helltime
=================
*/
void idPlayer::StopHelltime( bool quick )
{
	if( !PowerUpActive( HELLTIME ) )
	{
		return;
	}
	
	// take away the powerups
	if( PowerUpActive( INVULNERABILITY ) )
	{
		ClearPowerup( INVULNERABILITY );
	}
	
	if( PowerUpActive( BERSERK ) )
	{
		ClearPowerup( BERSERK );
	}
	
	if( PowerUpActive( HELLTIME ) )
	{
		ClearPowerup( HELLTIME );
	}
	
	// stop the looping sound
	StopSound( SND_CHANNEL_DEMONIC, false );
	
	// reset the game vars
	if( quick )
	{
		gameLocal.QuickSlowmoReset();
	}
}

/*
=================
idPlayer::Event_ToggleBloom
=================
*/
void idPlayer::Event_ToggleBloom( int on )
{
	if( on )
	{
		bloomEnabled = true;
	}
	else
	{
		bloomEnabled = false;
	}
}

/*
=================
idPlayer::Event_SetBloomParms
=================
*/
void idPlayer::Event_SetBloomParms( float speed, float intensity )
{
	bloomSpeed = speed;
	bloomIntensity = intensity;
}

/*
=================
idPlayer::PlayHelltimeStopSound
=================
*/
void idPlayer::PlayHelltimeStopSound()
{
	const char* sound;
	
	if( spawnArgs.GetString( "snd_helltime_stop", "", &sound ) )
	{
		PostEventMS( &EV_StartSoundShader, 0, sound, SND_CHANNEL_ANY );
	}
}

/*
=================
idPlayer::RouteGuiMouse
=================
*/
void idPlayer::RouteGuiMouse( idUserInterface* gui )
{
	sysEvent_t ev;
	const char* command;
	
	if( usercmd.mx != oldMouseX || usercmd.my != oldMouseY )
	{
		ev = sys->GenerateMouseMoveEvent( usercmd.mx - oldMouseX, usercmd.my - oldMouseY );
		command = gui->HandleEvent( &ev, gameLocal.time );
		oldMouseX = usercmd.mx;
		oldMouseY = usercmd.my;
	}
}

/*
==================
idPlayer::LookAtKiller
==================
*/
void idPlayer::LookAtKiller( idEntity* inflictor, idEntity* attacker )
{
	idVec3 dir;
	
	if( attacker && attacker != this )
	{
		dir = attacker->GetPhysics()->GetOrigin() - GetPhysics()->GetOrigin();
	}
	else if( inflictor && inflictor != this )
	{
		dir = inflictor->GetPhysics()->GetOrigin() - GetPhysics()->GetOrigin();
	}
	else
	{
		dir = viewAxis[ 0 ];
	}
	
	idAngles ang( 0, dir.ToYaw(), 0 );
	SetViewAngles( ang );
}

/*
==============
idPlayer::Kill
==============
*/
void idPlayer::Kill( bool delayRespawn, bool nodamage )
{
	if( spectating )
	{
		SpectateFreeFly( false );
	}
	else if( health > 0 )
	{
		godmode = false;
		if( nodamage )
		{
			ServerSpectate( true );
			idLib::Printf( "TOURNEY Kill :> Player %d On Deck \n", entityNumber );
			forceRespawn = true;
		}
		else
		{
			Damage( this, this, vec3_origin, "damage_suicide", 1.0f, INVALID_JOINT );
			if( delayRespawn )
			{
				forceRespawn = false;
				int delay = spawnArgs.GetFloat( "respawn_delay" );
				minRespawnTime = gameLocal.time + SEC2MS( delay );
				maxRespawnTime = minRespawnTime + MAX_RESPAWN_TIME;
			}
		}
	}
}

/*
==================
idPlayer::Killed
==================
*/
void idPlayer::Killed( idEntity* inflictor, idEntity* attacker, int damage, const idVec3& dir, int location )
{
	float delay;
	
	assert( !common->IsClient() );
	
	// stop taking knockback once dead
	fl.noknockback = true;
	if( health < -999 )
	{
		health = -999;
	}
	
	if( AI_DEAD )
	{
		AI_PAIN = true;
		return;
	}
	
	heartInfo.Init( 0, 0, 0, BASE_HEARTRATE );
	AdjustHeartRate( DEAD_HEARTRATE, 10.0f, 0.0f, true );
	
	if( !g_testDeath.GetBool() && !common->IsMultiplayer() )
	{
		playerView.Fade( colorBlack, 3000 );
	}
	
	AI_DEAD = true;
	SetAnimState( ANIMCHANNEL_LEGS, "Legs_Death", 4 );
	SetAnimState( ANIMCHANNEL_TORSO, "Torso_Death", 4 );
	SetWaitState( "" );
	
	animator.ClearAllJoints();
	
	if( StartRagdoll() )
	{
		pm_modelView.SetInteger( 0 );
		minRespawnTime = gameLocal.time + RAGDOLL_DEATH_TIME;
		maxRespawnTime = minRespawnTime + MAX_RESPAWN_TIME;
	}
	else
	{
		// don't allow respawn until the death anim is done
		// g_forcerespawn may force spawning at some later time
		delay = spawnArgs.GetFloat( "respawn_delay" );
		minRespawnTime = gameLocal.time + SEC2MS( delay );
		maxRespawnTime = minRespawnTime + MAX_RESPAWN_TIME;
	}
	
	physicsObj.SetMovementType( PM_DEAD );
	StartSound( "snd_death", SND_CHANNEL_VOICE, 0, false, NULL );
	StopSound( SND_CHANNEL_BODY2, false );
	
	fl.takedamage = true;		// can still be gibbed
	
	// get rid of weapons
	for( int h = 0; h < 2; h++ )
		hands[ h ].weapon.GetEntity()->OwnerDied();
	
	// In multiplayer, get rid of the flashlight, or other players
	// will see it floating after the player is dead.
	if( common->IsMultiplayer() )
	{
		FlashlightOff();
		if( flashlight.GetEntity() )
		{
			flashlight.GetEntity()->OwnerDied();
		}
	}
	
	// drop the weapons as items
	DropWeapons( true );
	
	// drop the flag if player was carrying it
	if( common->IsMultiplayer() && gameLocal.mpGame.IsGametypeFlagBased() && carryingFlag )
	{
		DropFlag();
	}
	
	if( !g_testDeath.GetBool() )
	{
		LookAtKiller( inflictor, attacker );
	}
	
	if( common->IsMultiplayer() || g_testDeath.GetBool() )
	{
		idPlayer* killer = NULL;
		// no gibbing in MP. Event_Gib will early out in MP
		if( attacker->IsType( idPlayer::Type ) )
		{
			killer = static_cast<idPlayer*>( attacker );
			if( health < -20 || killer->PowerUpActive( BERSERK ) )
			{
				gibDeath = true;
				gibsDir = dir;
				gibsLaunched = false;
			}
		}
		gameLocal.mpGame.PlayerDeath( this, killer, isTelefragged );
	}
	else
	{
		physicsObj.SetContents( CONTENTS_CORPSE | CONTENTS_MONSTERCLIP );
	}
	
	ClearPowerUps();
	
	UpdateVisuals();
}

/*
=====================
idPlayer::GetAIAimTargets

Returns positions for the AI to aim at.
=====================
*/
void idPlayer::GetAIAimTargets( const idVec3& lastSightPos, idVec3& headPos, idVec3& chestPos )
{
	idVec3 offset;
	idMat3 axis;
	idVec3 origin;
	
	origin = lastSightPos - physicsObj.GetOrigin();
	
	GetJointWorldTransform( chestJoint, gameLocal.time, offset, axis );
	headPos = offset + origin;
	
	GetJointWorldTransform( headJoint, gameLocal.time, offset, axis );
	chestPos = offset + origin;
}

/*
================
idPlayer::DamageFeedback

callback function for when another entity received damage from this entity.  damage can be adjusted and returned to the caller.
================
*/
void idPlayer::DamageFeedback( idEntity* victim, idEntity* inflictor, int& damage )
{
	// Since we're predicting projectiles on the client now, we might actually get here
	// (used be an assert for clients).
	if( common->IsClient() )
	{
		return;
	}
	
	damage *= PowerUpModifier( BERSERK );
	if( damage && ( victim != this ) && ( victim->IsType( idActor::Type ) || victim->IsType( idDamagable::Type ) ) )
	{
	
		idPlayer* victimPlayer = NULL;
		
		/* No damage feedback sound for hitting friendlies in CTF */
		if( victim->IsType( idPlayer::Type ) )
		{
			victimPlayer = static_cast<idPlayer*>( victim );
		}
		
		if( gameLocal.mpGame.IsGametypeFlagBased() && victimPlayer && this->team == victimPlayer->team )
		{
			/* Do nothing ... */
		}
		else
		{
			SetLastHitTime( gameLocal.time );
		}
	}
}

/*
=================
idPlayer::CalcDamagePoints

Calculates how many health and armor points will be inflicted, but
doesn't actually do anything with them.  This is used to tell when an attack
would have killed the player, possibly allowing a "saving throw"
=================
*/
void idPlayer::CalcDamagePoints( idEntity* inflictor, idEntity* attacker, const idDict* damageDef,
								 const float damageScale, const int location, int* health, int* armor )
{
	int		damage;
	int		armorSave;
	
	damageDef->GetInt( "damage", "20", damage );
	damage = GetDamageForLocation( damage, location );
	
	idPlayer* player = attacker->IsType( idPlayer::Type ) ? static_cast<idPlayer*>( attacker ) : NULL;
	if( !common->IsMultiplayer() )
	{
		if( inflictor != gameLocal.world )
		{
			switch( g_skill.GetInteger() )
			{
				case 0:
					damage *= 0.50f;
					if( damage < 1 )
					{
						damage = 1;
					}
					break;
				case 2:
					damage *= 1.70f;
					break;
				case 3:
					damage *= 3.5f;
					break;
				default:
					break;
			}
		}
	}
	
	damage *= damageScale;
	
	// always give half damage if hurting self
	if( attacker == this )
	{
		if( common->IsMultiplayer() )
		{
			// only do this in mp so single player plasma and rocket splash is very dangerous in close quarters
			damage *= damageDef->GetFloat( "selfDamageScale", "0.5" );
		}
		else
		{
			damage *= damageDef->GetFloat( "selfDamageScale", "1" );
		}
	}
	
	// check for completely getting out of the damage
	if( !damageDef->GetBool( "noGod" ) )
	{
		// check for godmode
		if( godmode )
		{
			damage = 0;
		}
		//Invulnerability is just like god mode
		if( PowerUpActive( INVULNERABILITY ) )
		{
			damage = 0;
		}
	}
	
	// inform the attacker that they hit someone
	attacker->DamageFeedback( this, inflictor, damage );
	
	// save some from armor
	if( !damageDef->GetBool( "noArmor" ) )
	{
		float armor_protection;
		
		armor_protection = ( common->IsMultiplayer() ) ? g_armorProtectionMP.GetFloat() : g_armorProtection.GetFloat();
		
		armorSave = ceil( damage * armor_protection );
		if( armorSave >= inventory.armor )
		{
			armorSave = inventory.armor;
		}
		
		if( !damage )
		{
			armorSave = 0;
		}
		else if( armorSave >= damage )
		{
			armorSave = damage - 1;
			damage = 1;
		}
		else
		{
			damage -= armorSave;
		}
	}
	else
	{
		armorSave = 0;
	}
	
	// check for team damage
	if( gameLocal.mpGame.IsGametypeTeamBased()  /* CTF */
			&& !gameLocal.serverInfo.GetBool( "si_teamDamage" )
			&& !damageDef->GetBool( "noTeam" )
			&& player
			&& player != this		// you get self damage no matter what
			&& player->team == team )
	{
		damage = 0;
	}
	
	*health = damage;
	*armor = armorSave;
}

/*
============
idPlayer::ControllerShakeFromDamage
============
*/
void idPlayer::ControllerShakeFromDamage( int damage )
{

	// If the player is local. SHAkkkkkkeeee!
	if( common->IsMultiplayer() && IsLocallyControlled() )
	{
	
		int maxMagScale = pm_controllerShake_damageMaxMag.GetFloat();
		int maxDurScale = pm_controllerShake_damageMaxDur.GetFloat();
		
		// determine rumble
		// >= 100 damage - will be 300 Mag
		float highMag = ( Max( damage, 100 ) / 100.0f ) * maxMagScale;
		int highDuration = idMath::Ftoi( ( Max( damage, 100 ) / 100.0f ) * maxDurScale );
		float lowMag = highMag * 0.75f;
		int lowDuration = idMath::Ftoi( highDuration );
		
		// Multiplayer damage from an unknown source to an unknown location. So shake both hands.
		for( int h = 0; h < 2; h++ )
			hands[h].SetControllerShake( highMag, highDuration, lowMag, lowDuration );
	}
	
}

/*
============
idPlayer::ControllerShakeFromDamage
============
*/
void idPlayer::ControllerShakeFromDamage( int damage, const idVec3 &dir )
{

	// If the player is local. SHAkkkkkkeeee!
	if( common->IsMultiplayer() && IsLocallyControlled() )
	{
	
		int maxMagScale = pm_controllerShake_damageMaxMag.GetFloat();
		int maxDurScale = pm_controllerShake_damageMaxDur.GetFloat();
		
		// determine rumble
		// >= 100 damage - will be 300 Mag
		float highMag = ( Max( damage, 100 ) / 100.0f ) * maxMagScale;
		int highDuration = idMath::Ftoi( ( Max( damage, 100 ) / 100.0f ) * maxDurScale );
		
		if( commonVr->hasHMD )
		{
			SetControllerShake( highMag, highDuration, dir );
		}
		else
		{
			float lowMag = highMag * 0.75f;
			int lowDuration = idMath::Ftoi( highDuration );
			hands[HAND_LEFT].SetControllerShake( highMag, highDuration, lowMag, lowDuration );
			hands[HAND_RIGHT].SetControllerShake( highMag, highDuration, lowMag, lowDuration );
		}
	}
	
}

/*
============
AdjustDamageAmount

Modifies the previously calculated damage to adjust for more factors.
============
*/
int idPlayer::AdjustDamageAmount( const int inputDamage )
{
	int outputDamage = inputDamage;
	
	if( inputDamage > 0 )
	{
	
		if( !common->IsMultiplayer() )
		{
			float scale = new_g_damageScale;
			if( g_useDynamicProtection.GetBool() && g_skill.GetInteger() < 2 )
			{
				if( gameLocal.time > lastDmgTime + 500 && scale > 0.25f )
				{
					scale -= 0.05f;
					new_g_damageScale = scale;
				}
			}
			
			if( scale > 0.0f )
			{
				outputDamage *= scale;
			}
		}
		
		if( g_demoMode.GetBool() )
		{
			outputDamage /= 2;
		}
		
		if( outputDamage < 1 )
		{
			outputDamage = 1;
		}
	}
	
	return outputDamage;
}


/*
============
ServerDealDamage

Only called on the server and in singleplayer. This is where
the player's health is actually modified, but the visual and
sound effects happen elsewhere so that clients can get instant
feedback and hide lag.
============
*/
void idPlayer::ServerDealDamage( int damage, idEntity& inflictor, idEntity& attacker, const idVec3& dir, const char* damageDefName, const int location )
{
	assert( !common->IsClient() );
	
	const idDeclEntityDef* damageDef = gameLocal.FindEntityDef( damageDefName, false );
	if( !damageDef )
	{
		gameLocal.Warning( "Unknown damageDef '%s'", damageDefName );
		return;
	}
	
	// move the world direction vector to local coordinates
	idVec3		damage_from;
	idVec3		localDamageVector;
	
	damage_from = dir;
	damage_from.Normalize();
	
	viewAxis.ProjectVector( damage_from, localDamageVector );
	
	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if( health > 0 )
	{
		playerView.DamageImpulse( localDamageVector, &damageDef->dict );
	}
	
	// do the damage
	if( damage > 0 )
	{
		GetAchievementManager().SetPlayerTookDamage( true );
		
		int oldHealth = health;
		health -= damage;
		
		if( health <= 0 )
		{
		
			if( health < -999 )
			{
				health = -999;
			}
			
			// HACK - A - LICIOUS - Check to see if we are being damaged by the frag chamber.
			if( oldHealth > 0 && strcmp( gameLocal.GetMapName(), "maps/game/mp/d3dm3.map" ) == 0 && strcmp( damageDefName, "damage_triggerhurt_1000_chamber" ) == 0 )
			{
				idPlayer* fragChamberActivator = gameLocal.playerActivateFragChamber;
				if( fragChamberActivator != NULL )
				{
					fragChamberActivator->GetAchievementManager().EventCompletesAchievement( ACHIEVEMENT_MP_CATCH_ENEMY_IN_ROFC );
				}
				gameLocal.playerActivateFragChamber = NULL;
			}
			
			isTelefragged = damageDef->dict.GetBool( "telefrag" );
			
			lastDmgTime = gameLocal.time;
			Killed( &inflictor, &attacker, damage, dir, location );
		}
		else
		{
			if( !g_testDeath.GetBool() )
			{
				lastDmgTime = gameLocal.time;
			}
		}
	}
	else
	{
		// don't accumulate impulses
		if( af.IsLoaded() )
		{
			// clear impacts
			af.Rest();
			
			// physics is turned off by calling af.Rest()
			BecomeActive( TH_PHYSICS );
		}
	}

}

/*
============
Damage

this		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: this=monster, inflictor=rocket, attacker=player

dir			direction of the attack for knockback in global space

damageDef	an idDict with all the options for damage effects

inflictor, attacker, dir, and point can be NULL for environmental effects
============
*/
void idPlayer::Damage( idEntity* inflictor, idEntity* attacker, const idVec3& dir,
					   const char* damageDefName, const float damageScale, const int location )
{
	idVec3		kick;
	int			damage;
	int			armorSave;
	
	SetTimeState ts( timeGroup );
	
	if( !fl.takedamage || noclip || spectating || gameLocal.inCinematic || Flicksync_InCutscene )
	{
		return;
	}
	
	if( !inflictor )
	{
		inflictor = gameLocal.world;
	}
	if( !attacker )
	{
		attacker = gameLocal.world;
	}
	
	if( attacker->IsType( idAI::Type ) )
	{
		if( PowerUpActive( BERSERK ) )
		{
			return;
		}
		// don't take damage from monsters during influences
		if( influenceActive != 0 )
		{
			return;
		}
	}
	
	const idDeclEntityDef* damageDef = gameLocal.FindEntityDef( damageDefName, false );
	if( !damageDef )
	{
		gameLocal.Warning( "Unknown damageDef '%s'", damageDefName );
		return;
	}
	
	if( damageDef->dict.GetBool( "ignore_player" ) )
	{
		return;
	}
	
	// determine knockback
	if ( !game->isVR || (game->isVR && vr_knockBack.GetBool()) ) // Koz disable damage knockback in VR if desired.
	{
		int knockback = 0;
		damageDef->dict.GetInt( "knockback", "20", knockback );

		common->Printf( "Knocking back\n" );

		if ( knockback != 0 && !fl.noknockback )
		{
			float attackerPushScale = 0.0f;

			if ( attacker == this )
			{
				damageDef->dict.GetFloat( "attackerPushScale", "0", attackerPushScale );
			}
			else
			{
				attackerPushScale = 1.0f;
			}

			idVec3 kick = dir;
			kick.Normalize();
			kick *= g_knockback.GetFloat() * knockback * attackerPushScale / 200.0f;
			physicsObj.SetLinearVelocity( physicsObj.GetLinearVelocity() + kick );

			// set the timer so that the player can't cancel out the movement immediately
			physicsObj.SetKnockBack( idMath::ClampInt( 50, 200, knockback * 2 ) );

			if ( common->IsServer() )
			{
				idBitMsg	msg;
				byte		msgBuf[MAX_EVENT_PARAM_SIZE];

				msg.InitWrite( msgBuf, sizeof( msgBuf ) );
				msg.WriteFloat( physicsObj.GetLinearVelocity()[0] );
				msg.WriteFloat( physicsObj.GetLinearVelocity()[1] );
				msg.WriteFloat( physicsObj.GetLinearVelocity()[2] );
				msg.WriteByte( idMath::ClampInt( 50, 200, knockback * 2 ) );
				ServerSendEvent( idPlayer::EVENT_KNOCKBACK, &msg, false );
			}
		}
	}
	// If this is a locally controlled MP client, don't apply damage effects predictively here.
	// Local clients will see the damage feedback (view kick, etc) when their health changes
	// in a snapshot. This ensures that any feedback the local player sees is in sync with
	// his actual health reported by the server.
	if( common->IsMultiplayer() && common->IsClient() && IsLocallyControlled() )
	{
		return;
	}
	
	CalcDamagePoints( inflictor, attacker, &damageDef->dict, damageScale, location, &damage, &armorSave );
	
	// give feedback on the player view and audibly when armor is helping
	if( armorSave )
	{
		inventory.armor -= armorSave;
		
		if( gameLocal.time > lastArmorPulse + 200 )
		{
			StartSound( "snd_hitArmor", SND_CHANNEL_ITEM, 0, false, NULL );
		}
		lastArmorPulse = gameLocal.time;
	}
	
	if( damageDef->dict.GetBool( "burn" ) )
	{
		StartSound( "snd_burn", SND_CHANNEL_BODY3, 0, false, NULL );
	}
	else if( damageDef->dict.GetBool( "no_air" ) )
	{
		if( !armorSave && health > 0 )
		{
			StartSound( "snd_airGasp", SND_CHANNEL_ITEM, 0, false, NULL );
		}
	}
	
	if( g_debugDamage.GetInteger() )
	{
		gameLocal.Printf( "client:%02d\tdamage type:%s\t\thealth:%03d\tdamage:%03d\tarmor:%03d\n", entityNumber, damageDef->GetName(), health, damage, armorSave );
	}
	
	if( common->IsMultiplayer() && IsLocallyControlled() )
	{
		ControllerShakeFromDamage( damage, dir );
	}
	
	// The client needs to know the final damage amount for predictive pain animations.
	const int finalDamage = AdjustDamageAmount( damage );
	
	if( health > 0 )
	{
		// force a blink
		blink_time = 0;
		
		// let the anim script know we took damage
		AI_PAIN = Pain( inflictor, attacker, damage, dir, location );
	}
	
	// Only actually deal the damage here in singleplayer and for locally controlled servers.
	if( !common->IsMultiplayer() || common->IsServer() )
	{
		// Server will deal his damage normally
		ServerDealDamage( finalDamage, *inflictor, *attacker, dir, damageDefName, location );
	}
	else if( attacker->GetEntityNumber() == gameLocal.GetLocalClientNum() )
	{
		// Clients send a reliable message to the server with the parameters of the hit. The
		// server should make sure the client still has line-of-sight to its target before
		// actually applying the damage.
		
		byte		msgBuffer[MAX_GAME_MESSAGE_SIZE];
		idBitMsg	msg;
		
		msg.InitWrite( msgBuffer, sizeof( msgBuffer ) );
		msg.BeginWriting();
		
		msg.WriteShort( attacker->GetEntityNumber() );
		msg.WriteShort( GetEntityNumber() );		// victim
		msg.WriteVectorFloat( dir );
		msg.WriteLong( damageDef->Index() );
		msg.WriteFloat( damageScale );
		msg.WriteLong( location );
		
		idLobbyBase& lobby = session->GetActingGameStateLobbyBase();
		lobby.SendReliableToHost( GAME_RELIABLE_MESSAGE_CLIENT_HITSCAN_HIT, msg );
	}
	
	lastDamageDef = damageDef->Index();
	lastDamageDir = dir;
	lastDamageDir.Normalize();
	lastDamageLocation = location;
}

/*
===========
idPlayer::Teleport
============
*/
void idPlayer::Teleport( const idVec3& origin, const idAngles& angles, idEntity* destination )
{
	idVec3 org;
	
	for( int h = 0; h < 2; h++ )
	{
		if( hands[ h ].weapon.GetEntity() )
			hands[ h ].weapon.GetEntity()->LowerWeapon();
	}
	
	SetOrigin( origin + idVec3( 0, 0, CM_CLIP_EPSILON ) );
	if( !common->IsMultiplayer() && GetFloorPos( 16.0f, org ) )
	{
		SetOrigin( org );
	}
	
	// clear the ik heights so model doesn't appear in the wrong place
	walkIK.EnableAll();
	
	GetPhysics()->SetLinearVelocity( vec3_origin );
	
	SetViewAngles( angles );
	
	legsYaw = 0.0f;
	idealLegsYaw = 0.0f;
	oldViewYaw = viewAngles.yaw;
	
	if( common->IsMultiplayer() )
	{
		playerView.Flash( colorWhite, 140 );
	}
	
	UpdateVisuals();
	
	teleportEntity = destination;
	
	if( !common->IsClient() && !noclip )
	{
		if( common->IsMultiplayer() )
		{
			// kill anything at the new position or mark for kill depending on immediate or delayed teleport
			gameLocal.KillBox( this, destination != NULL );
		}
		else
		{
			// kill anything at the new position
			gameLocal.KillBox( this, true );
		}
	}
	
	if( PowerUpActive( HELLTIME ) )
	{
		StopHelltime();
	}
}

/* Carl: TouchTriggers() at every point along straight line from start to end
====================
idPlayer::TeleportPathSegment
====================
*/
bool idPlayer::TeleportPathSegment( const idVec3& start, const idVec3& end, idVec3& lastPos )
{
	idVec3 total = end - start;
	float length = total.Length();
	if ( length >= 0.1f )
	{
		const float stepSize = 8.0f;
		int steps = (int)(length / stepSize);
		if (steps <= 0) steps = 1;
		idVec3 step = total / steps;
		idVec3 pos = start;
		for (int i = 0; i < steps; i++)
		{
			bool blocked = false;
			physicsObj.SetOrigin(pos);
			// Like TouchTriggers() but also checks for locked doors
			{
				int				i, numClipModels;
				idClipModel* 	cm;
				idClipModel* 	clipModels[MAX_GENTITIES];
				idEntity* 		ent;
				trace_t			trace;

				memset(&trace, 0, sizeof(trace));
				trace.endpos = pos;
				trace.endAxis = GetPhysics()->GetAxis();

				numClipModels = gameLocal.clip.ClipModelsTouchingBounds(GetPhysics()->GetAbsBounds(), CONTENTS_TRIGGER | CONTENTS_SOLID, clipModels, MAX_GENTITIES);

				for (i = 0; i < numClipModels; i++)
				{
					cm = clipModels[i];

					// don't touch it if we're the owner
					if (cm->GetOwner() == this)
					{
						continue;
					}

					ent = cm->GetEntity();

					if (!blocked && ent->IsType(idDoor::Type))
					{
						idDoor *door = (idDoor *)ent;
						if (door->IsLocked() || ( !vr_teleportThroughDoors.GetBool() && (cm->GetContents() & CONTENTS_SOLID) ))
						{
							// check if we're moving toward the door
							idVec3 away = door->GetPhysics()->GetOrigin() - pos;
							away.z = 0;
							float dist = away.Length();
							if (dist < 60.0f)
							{
								away /= dist;
								idVec3 my_dir = step;
								my_dir.Normalize();
								float angle = idMath::ACos(away * my_dir);
								if (angle < DEG2RAD(45) || (angle < DEG2RAD(90) && dist < 20))
									blocked = true;
								if (blocked && door->IsLocked())
								{
									// Trigger the door to make the locked sound, if we're not close enough to happen naturally
									if (dist > 30)
									{
										physicsObj.SetOrigin(pos + (away * (dist - 30)));
										TouchTriggers();
									}
								}
							}
						}
					}

					if (!ent->RespondsTo(EV_Touch) && !ent->HasSignal(SIG_TOUCH))
					{
						continue;
					}

					if (!GetPhysics()->ClipContents(cm))
					{
						continue;
					}

					SetTimeState ts(ent->timeGroup);

					trace.c.contents = cm->GetContents();
					trace.c.entityNum = cm->GetEntity()->entityNumber;
					trace.c.id = cm->GetId();

					ent->Signal(SIG_TOUCH);
					ent->ProcessEvent(&EV_Touch, this, &trace);
				}
			}

			if (blocked)
				return false;
			lastPos = pos;
			pos += step;
		}
		// we don't call TouchTriggers after the final step because it's either
		// the start of the next path segment, or the teleport destination
	}
	return true;
}

/* Carl: TouchTriggers() at every point in a pathfinding walk from the player's position to target, then teleport to target.
   It does so even if there is no path, or the current position and/or target aren't valid.
====================
idPlayer::TeleportPath
====================
*/
void idPlayer::TeleportPath( const idVec3& target )
{
	aasPath_t	path;
	int	originAreaNum, toAreaNum;
	idVec3 origin = physicsObj.GetOrigin();
	idVec3 trueOrigin = physicsObj.GetOrigin();
	idVec3 toPoint = target;
	idVec3 lastPos = origin;
	bool blocked = false;
	// Find path start and end areas and points
	originAreaNum = PointReachableAreaNum( origin );
	if ( aas )
		aas->PushPointIntoAreaNum( originAreaNum, origin );
	toAreaNum = PointReachableAreaNum( toPoint );
	if ( aas )
		aas->PushPointIntoAreaNum( toAreaNum, toPoint );
	// if there's no path, just go in a straight line (or should we just teleport straight there?)
	if ( !aas || !originAreaNum || !toAreaNum || !aas->WalkPathToGoal( path, originAreaNum, origin, toAreaNum, toPoint, travelFlags ) )
	{
		blocked = !TeleportPathSegment( physicsObj.GetOrigin(), target, lastPos );
	}
	else
	{
		// move from actual position to start of path
		blocked = !TeleportPathSegment( physicsObj.GetOrigin(), origin, lastPos );
		idVec3 currentPos = origin;
		int currentArea = originAreaNum;
		// Move along path
		while ( !blocked && currentArea && currentArea != toAreaNum )
		{
			if ( !TeleportPathSegment( currentPos, path.moveGoal, lastPos ) )
			{
				blocked = true;
				break;
			}
			currentPos = path.moveGoal;
			currentArea = path.moveAreaNum;
			// Find next path segment. Sometimes it tells us to go to the current location and gets stuck in a loop, so check for that.
			// TODO: Work out why it gets stuck in a loop, and fix it. Currently we just go in a straight line from stuck point to destination.
			if ( !aas->WalkPathToGoal( path, currentArea, currentPos, toAreaNum, toPoint, travelFlags ) || ( path.moveAreaNum == currentArea && path.moveGoal == currentPos ) )
			{
				path.moveGoal = toPoint;
				path.moveAreaNum = toAreaNum;
			}
		}
		// Is this needed? Doesn't hurt.
		blocked = blocked || !TeleportPathSegment( currentPos, toPoint, lastPos );
		// move from end of path to actual target
		blocked = blocked || !TeleportPathSegment( toPoint, target, lastPos );
	}

	if ( !blocked )
	{
		lastPos = target;
		// Check we didn't teleport inside a door that's not open.
		// It's OK to teleport THROUGH a closed but unlocked door, but we can't end up inside it.
		int				i, numClipModels;
		idClipModel* 	cm;
		idClipModel* 	clipModels[MAX_GENTITIES];
		idEntity* 		ent;
		trace_t			trace;

		memset( &trace, 0, sizeof(trace) );
		trace.endpos = target;
		trace.endAxis = GetPhysics()->GetAxis();

		numClipModels = gameLocal.clip.ClipModelsTouchingBounds( GetPhysics()->GetAbsBounds(), CONTENTS_SOLID, clipModels, MAX_GENTITIES );

		for ( i = 0; i < numClipModels; i++ )
		{
			cm = clipModels[i];

			// don't touch it if we're the owner
			if (cm->GetOwner() == this)
				continue;

			ent = cm->GetEntity();

			if ( !blocked && ent->IsType(idDoor::Type) )
			{
				idDoor *door = (idDoor *)ent;
				// A door that is in the process of opening falsely registers as open.
				// But we can rely on the fact that we're touching it, to know it's still partly closed.
				//if ( !door->IsOpen() )
				{
					idVec3 away = door->GetPhysics()->GetOrigin() - target;
					away.z = 0;
					float dist = away.Length();
					if (dist < 50.0f)
					{
						away /= dist;
						lastPos = target + (away * (dist - 50));
					}
				}
			}
		}
	}
	physicsObj.SetOrigin( trueOrigin );
	// Actually teleport

	if ( vr_teleportMode.GetInteger() == 0 ) 
	{
		Teleport( lastPos, viewAngles, NULL );
	}
	else 
	{
		extern idCVar timescale;
		warpMove = true;
		noclip = true;
		warpDest = lastPos;
		//warpDest.z += 1;
		warpVel = ( warpDest - trueOrigin ) / 0.075f;  // 75 ms
		//warpVel[2] = warpVel[2] + 50; // add a small fixed upwards velocity to handle noclip problem
		warpTime = gameLocal.time + 75;
		timescale.SetFloat( 0.5f );
		//playerView.EnableBFGVision(true);
	}
}

bool idPlayer::CheckTeleportPathSegment(const idVec3& start, const idVec3& end, idVec3& lastPos)
{
	idVec3 total = end - start;
	float length = total.Length();
	if ( length >= 0.1f )
	{
		const float stepSize = 15.0f; // We have a radius of 16, so this should catch everything
		int steps = (int)( length / stepSize );
		if ( steps <= 0 ) steps = 1;
		idVec3 step = total / steps;
		idVec3 pos = start;
		for ( int i = 0; i < steps; i++ )
		{
			physicsObj.SetOrigin( pos );
			// Check for doors
			{
				int				i, numClipModels;
				idClipModel* 	cm;
				idClipModel* 	clipModels[MAX_GENTITIES];
				idEntity* 		ent;
				trace_t			trace;

				memset( &trace, 0, sizeof(trace) );
				trace.endpos = pos;
				trace.endAxis = GetPhysics()->GetAxis();

				numClipModels = gameLocal.clip.ClipModelsTouchingBounds( GetPhysics()->GetAbsBounds(), CONTENTS_SOLID, clipModels, MAX_GENTITIES );

				for ( i = 0; i < numClipModels; i++ )
				{
					cm = clipModels[i];

					// don't touch it if we're the owner
					if (cm->GetOwner() == this)
					{
						continue;
					}

					ent = cm->GetEntity();

					// check if it's a closed or locked door 
					if ( ent->IsType(idDoor::Type) )
					{
						idDoor *door = (idDoor *)ent;
						if ( door->IsLocked() || ( !vr_teleportThroughDoors.GetBool() && ( cm->GetContents() & CONTENTS_SOLID ) ) )
						{
							// check if we're moving toward the door
							idVec3 away = door->GetPhysics()->GetOrigin() - pos;
							away.z = 0;
							float dist = away.Length();
							if ( dist < 60.0f )
							{
								away /= dist;
								idVec3 my_dir = step;
								my_dir.Normalize();
								float angle = idMath::ACos(away * my_dir);
								if ( angle < DEG2RAD( 45 ) || ( angle < DEG2RAD( 90 ) && dist < 20 ) )
									return false;
							}
						}
					}
					// Check if it's a glass window. func_static with textures/glass/glass2
					else if ( ent->IsType( idStaticEntity::Type ) )
					{
						renderEntity_t *rent = ent->GetRenderEntity();
						if ( rent )
						{
							const idMaterial *mat = rent->customShader;
							if ( !mat )
								mat = rent->referenceShader;
							if ( !mat && rent->hModel )
							{
								for ( int i = 0; i < rent->hModel->NumSurfaces(); i++ )
									if ( rent->hModel->Surface(i)->shader )
									{
										mat = rent->hModel->Surface(i)->shader;
										break;
									}
							}
							if ( mat )
							{
								const char* name = mat->GetName();
								// trying to teleport through glass: textures/glass/glass2 or textures/glass/glass1
								if ( name && idStr::Cmpn( name, "textures/glass/glass", 20 ) == 0 )
									return false;
								//else if (name)
								//	common->Printf("teleporting through \"%s\"\n", name);
								//else
								//	common->Printf("teleporting through NULL\n");
							}
							else if ( ent->name )
							{
								//common->Printf("teleporting through entity %s", ent->name);
							}
							
						}
					}
				}
			}

			lastPos = pos;
			pos += step;
		}
	}
	return true;
}

/* Carl: Check if we are trying to teleport through a locked or closed door
====================
idPlayer::CheckTeleportPath
====================
*/
bool idPlayer::CheckTeleportPath(const idVec3& target, int toAreaNum)
{
	aasPath_t	path;
	int	originAreaNum;
	idVec3 origin = physicsObj.GetOrigin();
	idVec3 trueOrigin = origin;
	idVec3 toPoint = target;
	idVec3 lastPos = origin;
	bool blocked = false;
	// Find path start and end areas and points
	originAreaNum = PointReachableAreaNum( origin );
	if ( aas )
		aas->PushPointIntoAreaNum( originAreaNum, origin );
	if ( !toAreaNum )
		toAreaNum = PointReachableAreaNum( toPoint );
	if ( aas )
		aas->PushPointIntoAreaNum( toAreaNum, toPoint );
	// if there's no path, just go in a straight line
	if ( !aas || !originAreaNum || !toAreaNum || !aas->WalkPathToGoal(path, originAreaNum, origin, toAreaNum, toPoint, travelFlags ) )
	{
		if ( !CheckTeleportPathSegment( physicsObj.GetOrigin(), target, lastPos ) )
		{
			physicsObj.SetOrigin( trueOrigin );
			return false;
		}
	}
	else
	{
		// move from actual position to start of path
		if ( !CheckTeleportPathSegment( trueOrigin, origin, lastPos ) )
		{
			physicsObj.SetOrigin( trueOrigin );
			return false;
		}
		idVec3 currentPos = origin;
		int currentArea = originAreaNum;

		int lastAreas[4], lastAreaIndex;
		lastAreas[0] = lastAreas[1] = lastAreas[2] = lastAreas[3] = currentArea;
		lastAreaIndex = 0;


		// Move along path
		while ( currentArea && currentArea != toAreaNum )
		{
			if ( !CheckTeleportPathSegment( currentPos, path.moveGoal, lastPos ) )
			{
				physicsObj.SetOrigin( trueOrigin );
				return false;
			}

			lastAreas[lastAreaIndex] = currentArea;
			lastAreaIndex = ( lastAreaIndex + 1 ) & 3;

			currentPos = path.moveGoal;
			currentArea = path.moveAreaNum;

			// Sometimes it tells us to go to the current location and gets stuck in a loop, so check for that.
			// TODO: Work out why it gets stuck in a loop, and fix it. Currently we just go in a straight line from stuck point to destination.
			if ( currentArea == lastAreas[0] || currentArea == lastAreas[1] ||
				currentArea == lastAreas[2] || currentArea == lastAreas[3] )
			{
				common->Warning( "CheckTeleportPath: local routing minimum going from area %d to area %d", currentArea, toAreaNum );
				if ( !CheckTeleportPathSegment( currentPos, toPoint, lastPos ) )
				{
					physicsObj.SetOrigin( trueOrigin );
					return false;
				}
				currentPos = toPoint;
				currentArea = toAreaNum;
				break;
			}

			// Find next path segment.
			if ( !aas->WalkPathToGoal(path, currentArea, currentPos, toAreaNum, toPoint, travelFlags) )
			{
				path.moveGoal = toPoint;
				path.moveAreaNum = toAreaNum;
				currentPos = toPoint;
				currentArea = toAreaNum;
			}
		}
		// Is this needed? Doesn't hurt.
		if ( !CheckTeleportPathSegment( currentPos, toPoint, lastPos ) )
		{
			physicsObj.SetOrigin( trueOrigin );
			return false;
		}
		// move from end of path to actual target
		if ( !CheckTeleportPathSegment( toPoint, target, lastPos ) )
		{
			physicsObj.SetOrigin( trueOrigin );
			return false;
		}
	}

	physicsObj.SetOrigin( trueOrigin );
	return true;
}

/*
====================
idPlayer::SetPrivateCameraView
====================
*/
void idPlayer::SetPrivateCameraView( idCamera* camView )
{
	privateCameraView = camView;
	if( camView )
	{
		StopFiring();
		Hide();
	}
	else
	{
		if( !spectating )
		{
			Show();
		}
	}
}

/*
====================
idPlayer::DefaultFov

Returns the base FOV
====================
*/
float idPlayer::DefaultFov() const
{
	float fov;

	if ( game->isVR ) // Koz fixme report HMD fov in VR. 
	{
		fov = commonVr->hmdFovX;
	}
	else
	{
		fov = g_fov.GetFloat();
		if ( common->IsMultiplayer() )
		{
			if ( fov < 80.0f )
			{
				return 80.0f;
			}
			else if ( fov > 120.0f )
			{
				return 120.0f;
			}
		}
	}

	return fov;
}

/*
====================
idPlayer::CalcFov

Fixed fov at intermissions, otherwise account for fov variable and zooms.
====================
*/
float idPlayer::CalcFov( bool honorZoom )
{
	float fov;
/*
	if ( game->isVR ) // Koz only use HMD fov in VR
	{
		return DefaultFov();
	}
	else
	{ */
		if ( fxFov )
		{
			return DefaultFov() + 10.0f + cos( (gameLocal.time + 2000) * 0.01 ) * 10.0f;
		}

		if ( influenceFov )
		{
			return influenceFov;
		}

		// Koz, no zoom in VR.

		/*
		if ( zoomFov.IsDone( gameLocal.time ) )
		{
			fov = (honorZoom && usercmd.buttons & BUTTON_ZOOM) && weapon.GetEntity() ? weapon.GetEntity()->GetZoomFov() : DefaultFov();
		}
		else
		{
			fov = zoomFov.GetCurrentValue( gameLocal.time );
		}
		*/

		fov = DefaultFov();

		// bound normal viewsize
		if ( fov < 1 )
		{
			fov = 1;
		}
		else if ( fov > 179 )
		{
			fov = 179;
		}
//	}
	return fov;
}

/*
==============
idPlayer::GunTurningOffset

generate a rotational offset for the gun based on the view angle
history in loggedViewAngles
==============
*/
idAngles idPlayer::GunTurningOffset()
{
	idAngles	a;
	
	a.Zero();
	
	if( gameLocal.framenum < NUM_LOGGED_VIEW_ANGLES )
	{
		return a;
	}
	
	idAngles current = loggedViewAngles[ gameLocal.framenum & ( NUM_LOGGED_VIEW_ANGLES - 1 ) ];
	
	idAngles	av, base;
	int weaponAngleOffsetAverages;
	float weaponAngleOffsetScale, weaponAngleOffsetMax;
	
	// Carl: todo dual wielding
	hands[vr_weaponHand.GetInteger()].weapon.GetEntity()->GetWeaponAngleOffsets( &weaponAngleOffsetAverages, &weaponAngleOffsetScale, &weaponAngleOffsetMax );
	
	av = current;
	
	// calcualte this so the wrap arounds work properly
	for( int j = 1 ; j < weaponAngleOffsetAverages ; j++ )
	{
		idAngles a2 = loggedViewAngles[( gameLocal.framenum - j ) & ( NUM_LOGGED_VIEW_ANGLES - 1 ) ];
		
		idAngles delta = a2 - current;
		
		if( delta[1] > 180 )
		{
			delta[1] -= 360;
		}
		else if( delta[1] < -180 )
		{
			delta[1] += 360;
		}
		
		av += delta * ( 1.0f / weaponAngleOffsetAverages );
	}
	
	a = ( av - current ) * weaponAngleOffsetScale;
	
	for( int i = 0 ; i < 3 ; i++ )
	{
		if( a[i] < -weaponAngleOffsetMax )
		{
			a[i] = -weaponAngleOffsetMax;
		}
		else if( a[i] > weaponAngleOffsetMax )
		{
			a[i] = weaponAngleOffsetMax;
		}
	}
	
	return a;
}

/*
==============
idPlayer::GunAcceleratingOffset

generate a positional offset for the gun based on the movement
history in loggedAccelerations
==============
*/
idVec3	idPlayer::GunAcceleratingOffset()
{
	idVec3	ofs;
	
	float weaponOffsetTime, weaponOffsetScale;
	
	ofs.Zero();
	
	// Carl: todo dual wielding
	hands[vr_weaponHand.GetInteger()].weapon.GetEntity()->GetWeaponTimeOffsets( &weaponOffsetTime, &weaponOffsetScale );
	
	int stop = currentLoggedAccel - NUM_LOGGED_ACCELS;
	if( stop < 0 )
	{
		stop = 0;
	}
	for( int i = currentLoggedAccel - 1 ; i > stop ; i-- )
	{
		loggedAccel_t*	acc = &loggedAccel[i & ( NUM_LOGGED_ACCELS - 1 )];
		
		float	f;
		float	t = gameLocal.time - acc->time;
		if( t >= weaponOffsetTime )
		{
			break;	// remainder are too old to care about
		}
		
		f = t / weaponOffsetTime;
		f = ( cos( f * 2.0f * idMath::PI ) - 1.0f ) * 0.5f;
		ofs += f * weaponOffsetScale * acc->dir;
	}
	
	return ofs;
}


/*
==============
idPlayer::CalculateViewWeaponPos

Calculate the bobbing position of the view weapon
==============
*/

void idPlayer::CalculateViewWeaponPos( int hand, idVec3& origin, idMat3& axis )
{
	
	if ( game->isVR || true )
	{
		CalculateViewWeaponPosVR( hand, origin, axis );
		return;
	}
	
	float		scale;
	float		fracsin;
	idAngles	angles;
	int			delta;
	
	// CalculateRenderView must have been called first
	const idVec3& viewOrigin = firstPersonViewOrigin;
	const idMat3& viewAxis = firstPersonViewAxis;
	
	// these cvars are just for hand tweaking before moving a value to the weapon def
	idVec3	gunpos( g_gun_x.GetFloat(), g_gun_y.GetFloat(), g_gun_z.GetFloat() );

	// as the player changes direction, the gun(s) will take a small lag
	idVec3	gunOfs = GunAcceleratingOffset();
	origin = viewOrigin + ( gunpos + gunOfs ) * viewAxis;
	
	// on odd legs, invert some angles
	if( bobCycle & 128 )
	{
		scale = -xyspeed;
	}
	else
	{
		scale = xyspeed;
	}
	
	// gun angles from bobbing
	angles.roll		= scale * bobfracsin * 0.005f;
	angles.yaw		= scale * bobfracsin * 0.01f;
	angles.pitch	= xyspeed * bobfracsin * 0.005f;
	
	// gun angles from turning
	if( common->IsMultiplayer() )
	{
		idAngles offset = GunTurningOffset();
		offset *= g_mpWeaponAngleScale.GetFloat();
		angles += offset;
	}
	else
	{
		angles += GunTurningOffset();
	}
	
	idVec3 gravity = physicsObj.GetGravityNormal();
	
	// drop the weapon(s) when landing after a jump / fall
	delta = gameLocal.time - landTime;
	if( delta < LAND_DEFLECT_TIME )
	{
		origin -= gravity * ( landChange * 0.25f * delta / LAND_DEFLECT_TIME );
	}
	else if( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME )
	{
		origin -= gravity * ( landChange * 0.25f * ( LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta ) / LAND_RETURN_TIME );
	}
	
	// speed sensitive idle drift
	scale = xyspeed + 40.0f;
	fracsin = scale * sin( MS2SEC( gameLocal.time ) ) * 0.01f;
	angles.roll		+= fracsin;
	angles.yaw		+= fracsin;
	angles.pitch	+= fracsin;
		
	const idMat3	anglesMat = angles.ToMat3();
	const idMat3	scaledMat = anglesMat * g_gunScale.GetFloat();
	
	axis = scaledMat * viewAxis;

	hands[ hand ].weapon->CalculateHideRise( origin, axis ); // Koz
}

void DebugCross( idVec3 origin, idMat3 axis, idVec4 color )
{
	
	return;
	gameRenderWorld->DebugLine( color, origin - 3 * axis[0], origin + 3 * axis[0], 20 );
	gameRenderWorld->DebugLine( color, origin - 3 * axis[1], origin + 3 * axis[1], 20 );
	gameRenderWorld->DebugLine( color, origin - 3 * axis[2], origin + 3 * axis[2], 20 );
}

// Carl: TODO Dual wielding
void idPlayer::CalculateViewWeaponPosVR( int hand, idVec3 &origin, idMat3 &axis )
{
	
	weapon_t currentWeaponEnum = WEAPON_NONE;
	idVec3	gunOrigin;
	idVec3	originOffset = vec3_zero;
	idAngles hmdAngles;
	idVec3	headPositionDelta;
	idVec3	bodyPositionDelta;
	idVec3	absolutePosition;
	idQuat	weaponPitch;
	
	currentWeaponEnum = hands[ hand ].weapon->IdentifyWeapon();
	idWeapon* weapon = hands[ hand ].weapon;
	int currentWeaponIndex = hands[ hand ].currentWeapon;

	if ( hands[hand].holdingFlashlight() || weapon->isPlayerFlashlight ) return;

	gunOrigin = GetEyePosition();

	if ( game->isVR && commonVr->VR_USE_MOTION_CONTROLS ) gunOrigin += commonVr->leanOffset;
		
	// direction the player body is facing.
	idMat3		bodyAxis = idAngles( 0.0, viewAngles.yaw, 0.0f ).ToMat3();
	idVec3		gravity = physicsObj.GetGravityNormal();
		

	if ( currentWeaponEnum != WEAPON_PDA )
	{
		hands[hand].PDAfixed = false; // release the PDA if weapon has been switched.
	}
	
	if ( !commonVr->VR_USE_MOTION_CONTROLS || ( vr_PDAfixLocation.GetBool() && currentWeaponEnum == WEAPON_PDA ) ) // non-motion control & fixed pda positioning.
	{ 
		
		idMat3 pdaPitch = idAngles( vr_pdaPitch.GetFloat(), 0.0f, 0.0f ).ToMat3();
		
		axis = bodyAxis;
		origin = gunOrigin; 

		if ( currentWeaponEnum == WEAPON_PDA ) //&& weapon->GetStatus() == WP_READY )
		{ 
						
			if ( hands[ hand ].PDAfixed )
			{ // pda has already been locked in space, use stored values
						
				origin = PDAorigin;
				axis = PDAaxis;
								
			
				//if the player has moved ( or been moved, if on an elevator or lift )
				//move the PDA to maintain a constant relative position
				idVec3 curPlayerPos = physicsObj.GetOrigin();
				origin -= ( hands[hand].playerPdaPos - curPlayerPos ) + commonVr->fixedPDAMoveDelta;
				//common->Printf( "playerPDA x %f y %f  currentPlay x %f y %f  fixMoveDel x %f y %f\n", playerPdaPos.x, playerPdaPos.y, curPlayerPos.x, curPlayerPos.y, commonVr->fixedPDAMoveDelta.x, commonVr->fixedPDAMoveDelta.y );

				SetHandIKPos( hand, origin, axis, pdaPitch.ToQuat() , false );
				originOffset = weapon->weaponHandDefaultPos[hand];
				origin -= originOffset * axis;
				origin += handWeaponAttacherToDefaultOffset[hand][currentWeaponIndex] * axis; // add the attacher offsets
			}
			else
			{ // fix the PDA in space, set flag and store position
				
				hands[ hand ].playerPdaPos = physicsObj.GetOrigin();

				origin = gunOrigin;
				origin += vr_pdaPosX.GetFloat() * bodyAxis[0] + vr_pdaPosY.GetFloat() *  bodyAxis[1] + vr_pdaPosZ.GetFloat() * bodyAxis[2];
				PDAorigin = origin;
				PDAaxis = pdaPitch * bodyAxis; 
				axis = PDAaxis;
				hands[ hand ].PDAfixed = true;

				SetHandIKPos( hand, origin, axis, pdaPitch.ToQuat(), false );
				originOffset = weapon->weaponHandDefaultPos[hand];
				origin -= originOffset * axis;
				origin += handWeaponAttacherToDefaultOffset[hand][currentWeaponIndex] * axis; // add the attacher offsets
				
				
				// the non weapon hand was set to the PDA fixed location, now fall thru and normal motion controls will place the pointer hand location
				
			}
		}
		//else
		{
			// non motion control weapon positioning for everything except PDA.

			static idQuat angQuat;
			static idVec3 gunpos;

			static idVec3 newOrg;
			static idMat3 newAx;

			static float origWeapPitch;
			static float origWeapYaw;

			if ( ( hands[ hand ].PDAfixed != hands[ hand ].lastPdaFixed ) && !commonVr->VR_USE_MOTION_CONTROLS )
			{
				hands[ hand ].lastPdaFixed = hands[ hand ].PDAfixed;
				if ( hands[ hand ].PDAfixed )
				{
					origWeapPitch = commonVr->independentWeaponPitch;
					origWeapYaw = commonVr->independentWeaponYaw;
					commonVr->independentWeaponPitch = 30;
					commonVr->independentWeaponYaw = 0;
				}
				else
				{
					commonVr->independentWeaponPitch = origWeapPitch;
					commonVr->independentWeaponYaw = origWeapYaw;
				}
			}

			newOrg = origin;
			newAx = axis;
					
			angQuat = idAngles( commonVr->independentWeaponPitch, commonVr->independentWeaponYaw, 0 ).ToQuat();
				
			idQuat gunAxis = angQuat;
			if( game->isVR || in_independentAim.GetInteger() )
			{
				gunAxis *= bodyAxis.ToQuat();
			}
			else
			{
				idMat3 headAxis = idAngles( viewAngles.pitch, viewAngles.yaw, 0.0f ).ToMat3();
				gunAxis *= headAxis.ToQuat();
				commonVr->independentWeaponPitch = 0;
				commonVr->independentWeaponYaw = 0;
			}
			newAx = gunAxis.ToMat3();
			
			int flip = vr_weaponHand.GetInteger() == 0 ? 1 : -1;
			
			gunpos = idVec3( vr_weaponPivotOffsetForward.GetFloat(), vr_weaponPivotOffsetHorizontal.GetFloat() * flip, vr_weaponPivotOffsetVertical.GetFloat() );
			gunOrigin += gunpos * bodyAxis;			// Koz move the gun to the hand position
						
			idVec3 forearm = idVec3( vr_weaponPivotForearmLength.GetFloat(), 0.0f, 0.0f );
			newOrg = gunOrigin + forearm * newAx;
						
			
						
			weapon->CalculateHideRise( newOrg, newAx );// Koz
			
			SetHandIKPos( hand, newOrg, newAx, angQuat, false );
			originOffset = weapon->weaponHandDefaultPos[hand];
			
			if ( hands[ hand ].PDAfixed ) return;
					
			origin = newOrg;
			axis = newAx;

			origin -= originOffset * axis;
			origin += handWeaponAttacherToDefaultOffset[hand][currentWeaponIndex] * axis; // add the attacher offsets
		}
		
		// drop the weapon when landing after a jump / fall
		int delta = gameLocal.time - landTime;
		if ( delta < LAND_DEFLECT_TIME ) {
			origin -= gravity * (landChange*0.25f * delta / LAND_DEFLECT_TIME);
		}
		else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
			origin -= gravity * (landChange*0.25f * (LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta) / LAND_RETURN_TIME);
		}
					
	}

	if (commonVr->VR_USE_MOTION_CONTROLS )
	{
		// motion control weapon positioning.
		//-----------------------------------

		idVec3 weapOrigin = vec3_zero;
		idMat3 weapAxis = mat3_identity;

		// idVec3 fixPosVec = idVec3( -17.0f, 6.0f, 0.0f );
		// idVec3 fixPos = fixPosVec;
		// idQuat fixRot = idAngles( 40.0f, -40.0f, 20.0f ).ToQuat();
		idVec3 attacherToDefault = vec3_zero;
		// idMat3 rot180 = idAngles( 0.0f, 180.0f, 0.0f ).ToMat3();
		
		if ( !hands[ hand ].PDAfixed && currentWeaponEnum == WEAPON_PDA )
		{
			// do the non-PDA hand first (the hand with the pointy finger)
			int fingerHand = 1 - hand;

			attacherToDefault = handWeaponAttacherToDefaultOffset[fingerHand][currentWeaponIndex];
			originOffset = weapon->weaponHandDefaultPos[fingerHand];
			commonVr->MotionControlGetHand( fingerHand, hands[fingerHand].motionPosition, hands[fingerHand].motionRotation );

			weaponPitch = idAngles( vr_motionWeaponPitchAdj.GetFloat(), 0.f, 0.0f ).ToQuat();
			hands[fingerHand].motionRotation = weaponPitch * hands[fingerHand].motionRotation;

			GetViewPos( weapOrigin, weapAxis );

			weapOrigin += commonVr->leanOffset;
			//commonVr->HMDGetOrientation( hmdAngles, headPositionDelta, bodyPositionDelta, absolutePosition, false );// gameLocal.inCinematic );
			
			hmdAngles = commonVr->poseHmdAngles;
			headPositionDelta = commonVr->poseHmdHeadPositionDelta;
			bodyPositionDelta = commonVr->poseHmdBodyPositionDelta;
			absolutePosition = commonVr->poseHmdAbsolutePosition;



			weapAxis = idAngles( 0.0, weapAxis.ToAngles().yaw - commonVr->bodyYawOffset, 0.0f ).ToMat3();
			
			weapOrigin += weapAxis[0] * headPositionDelta.x + weapAxis[1] * headPositionDelta.y + weapAxis[2] * headPositionDelta.z;

			weapOrigin += hands[fingerHand].motionPosition * weapAxis;
			weapAxis = hands[fingerHand].motionRotation.ToMat3() * weapAxis;
		
			//weapon->CalculateHideRise( weapOrigin, weapAxis );

			idAngles motRot = hands[fingerHand].motionRotation.ToAngles();
			motRot.yaw -= commonVr->bodyYawOffset;
			motRot.Normalize180();
			hands[fingerHand].motionRotation = motRot.ToQuat();
		
			SetHandIKPos( fingerHand, weapOrigin, weapAxis, hands[fingerHand].motionRotation, false );
				
			// now switch hands and fall through again.,
		}
		// NOT the PDA

		attacherToDefault = handWeaponAttacherToDefaultOffset[hand][currentWeaponIndex];
		originOffset = weapon->weaponHandDefaultPos[hand];
				
		commonVr->MotionControlGetHand( hand, hands[ hand ].motionPosition, hands[ hand ].motionRotation );

		weaponPitch = idAngles( vr_motionWeaponPitchAdj.GetFloat(), 0.0f, 0.0f ).ToQuat();
		hands[ hand ].motionRotation = weaponPitch * hands[ hand ].motionRotation;
		
		GetViewPos( weapOrigin, weapAxis );

		weapOrigin += commonVr->leanOffset;
				
		//commonVr->HMDGetOrientation( hmdAngles, headPositionDelta, bodyPositionDelta, absolutePosition, false );// gameLocal.inCinematic );

		hmdAngles = commonVr->poseHmdAngles;
		headPositionDelta = commonVr->poseHmdHeadPositionDelta;
		bodyPositionDelta = commonVr->poseHmdBodyPositionDelta;
		absolutePosition = commonVr->poseHmdAbsolutePosition;


		weapAxis = idAngles( 0.0f, weapAxis.ToAngles().yaw - commonVr->bodyYawOffset, 0.0f ).ToMat3();

		weapOrigin += weapAxis[0] * headPositionDelta.x + weapAxis[1] * headPositionDelta.y + weapAxis[2] * headPositionDelta.z;
		
		weapOrigin += hands[ hand ].motionPosition * weapAxis;
	
		if ( currentWeaponEnum != WEAPON_ARTIFACT && currentWeaponEnum != WEAPON_SOULCUBE )
		{
			weapAxis = hands[ hand ].motionRotation.ToMat3() * weapAxis;
		}
		else
		{
			weapAxis = idAngles( 0.0f ,viewAngles.yaw , 0.0f).ToMat3();
		}

		//DebugCross( weapOrigin, weapAxis, colorYellow );

		if ( currentWeaponEnum != WEAPON_PDA )
		{
			hands[hand].TrackWeaponDirection( weapOrigin );
			hands[1 - hand].TrackWeaponDirection( weapOrigin );
			weapon->CalculateHideRise( weapOrigin, weapAxis );
			//check for melee hit?		
		}
		else if( !hands[ hand ].PDAfixed )
		{
			// Koz FIXME hack hack hack this is getting so ungodly ugly.
			// Lovely.  I forgot to correct the origin for the PDA model when I switched
			// to always showing the body, so now when holding the PDA it doesn't align with your controller.
			// will fix the assets later but for now hack this correction in.
			
			const idVec3 pdaHackOrigin[2] { idVec3( 5.0f, -1.0f, 0.0f ), idVec3( 5.0f, 1.0f, 0.0f ) };
			weapOrigin += pdaHackOrigin[hand] * weapAxis;
		}
	
		idAngles motRot = hands[ hand ].motionRotation.ToAngles();
		motRot.yaw -= commonVr->bodyYawOffset;
		motRot.Normalize180();
		hands[ hand ].motionRotation = motRot.ToQuat();
		
		SetHandIKPos( hand, weapOrigin, weapAxis, hands[ hand ].motionRotation, false );
				
		if ( hands[ hand ].PDAfixed ) return;

		if ( currentWeaponEnum == WEAPON_PDA )
		{
			
			PDAaxis = weapAxis;
			PDAorigin = weapOrigin;
			if ( hands[ hand ].wasPDA == false )
			{
				SetFlashHandPose(); // Call set flashlight hand pose script function
				SetWeaponHandPose();
				hands[ hand ].wasPDA = true;
			}
		}
		else
		{
			hands[hand].wasPDA = false;
		}

		axis = weapAxis;
		origin = weapOrigin;
		
		origin -= originOffset * weapAxis;
		origin += attacherToDefault  * weapAxis; // handWeaponAttacherToDefaultOffset[hand][currentWeaponIndex] * weapAxis; // add the attacher offsets
	}
}


/*
==============
Koz idPlayer::RecreateCopyJoints()
After restoring from a savegame with different player models, the copyjoints for the head are wrong.
This will recreate a working copyJoint list.
==============
*/
void idPlayer::RecreateCopyJoints()
{
	idEntity* headEnt = head.GetEntity();
	idAnimator* headAnimator;
	const idKeyValue* kv;
	idStr jointName;
	copyJoints_t	copyJoint;

	copyJoints.Clear();
	if ( headEnt )
	{
		headAnimator = headEnt->GetAnimator();
	}
	else
	{
		headAnimator = &animator;
	}

	if ( headEnt )
	{
		// set up the list of joints to copy to the head
		for ( kv = spawnArgs.MatchPrefix( "copy_joint", NULL ); kv != NULL; kv = spawnArgs.MatchPrefix( "copy_joint", kv ) )
		{
			if ( kv->GetValue() == "" )
			{
				// probably clearing out inherited key, so skip it
				continue;
			}

			jointName = kv->GetKey();
			if ( jointName.StripLeadingOnce( "copy_joint_world " ) )
			{
				copyJoint.mod = JOINTMOD_WORLD_OVERRIDE;
			}
			else
			{
				jointName.StripLeadingOnce( "copy_joint " );
				copyJoint.mod = JOINTMOD_LOCAL_OVERRIDE;
			}

			copyJoint.from = animator.GetJointHandle( jointName );
			if ( copyJoint.from == INVALID_JOINT )
			{
				gameLocal.Warning( "Unknown copy_joint '%s' on entity %s", jointName.c_str(), name.c_str() );
				continue;
			}

			jointName = kv->GetValue();
			copyJoint.to = headAnimator->GetJointHandle( jointName );
			if ( copyJoint.to == INVALID_JOINT )
			{
				gameLocal.Warning( "Unknown copy_joint '%s' on head of entity %s", jointName.c_str(), name.c_str() );
				continue;
			}

			copyJoints.Append( copyJoint );
		}
	}
}


/*
==============
Koz idPlayer::UpdateNeckPose
In Vr, if viewing the player body, update the neck joint with the orientation of the HMD.
==============
*/
void idPlayer::UpdateNeckPose()
{
	static idAngles headAngles, lastView = ang_zero;
			
	if ( !game->isVR ) return;
		
	// if showing the player body, move the head/neck based on HMD 
	lastView = commonVr->lastHMDViewAxis.ToAngles();
	headAngles.roll = lastView.pitch;
	headAngles.pitch = commonVr->lastHMDYaw - commonVr->bodyYawOffset;
	headAngles.yaw = lastView.roll;
	headAngles.Normalize360();
	animator.SetJointAxis( neckJoint, JOINTMOD_LOCAL, headAngles.ToMat3() );
}

/*
==============
Koz idPlayerHand::TrackWeaponDirection
keep track of weapon movement to determine direction of motion
==============
*/
void idPlayerHand::TrackWeaponDirection( idVec3 origin )
{
	frameNum += 1;
	if ( frameNum > 9 ) frameNum = 0;
	frameTime[frameNum] = gameLocal.GetTime();
	position[frameNum] = origin;

	startFrameNum = frameNum - 5;
	if ( startFrameNum < 0 ) startFrameNum = 9 - frameNum;

	timeDelta = frameTime[frameNum] - frameTime[startFrameNum];
	if ( timeDelta == 0 ) timeDelta = 1;

	throwDirection = position[frameNum] - position[startFrameNum];
	throwVelocity = ( throwDirection.Length() / timeDelta ) * 1000;
}


/*
==============
Koz idPlayer::SetHandIKPos
Set the position for the hand based on weapon origin
==============
Carl: TODO Dual wielding
*/
void idPlayer::SetHandIKPos( int hand, idVec3 handOrigin, idMat3 handAxis, idQuat rotation, bool isFlashlight )
{
	// this is for arm IK when viewing player body	
	// the position for the player hand joint is modified 
	// to reflect the position of the viewmodel.  
	// armIK / reach_ik then performs crude IK on arm using new positon.
	
	jointHandle_t weaponHandAttachJoint = INVALID_JOINT; // this is the joint on the WEAPON the hand should meet
	jointHandle_t handWeaponAttachJoint = INVALID_JOINT; // the is the joint on the HAND the weapon should meet

	static idVec3 weaponHandAttachJointPositionLocal = vec3_zero;
	static idMat3 weaponHandAttachJointAxisLocal = mat3_identity;
	//idVec3 weaponHandAttachJointPositionGlobal = vec3_zero;
	//idMat3 weaponHandAttachJointAxisGlobal = mat3_identity;

	idVec3 weaponHandAttachJointDefaultPositionLocal = vec3_zero;

	//idVec3 handWeaponAttachJointPositionLocal = vec3_zero;
	//idMat3 handWeaponAttachJointAxisLocal = mat3_identity;
	//idVec3 handWeaponAttachJointPositionGlobal = vec3_zero;
	//idMat3 handWeaponAttachJointAxisGlobal = mat3_identity;

	idVec3 weaponAttachDelta = vec3_zero;

	idVec3 handAttacherPositionLocal = vec3_zero;
	idVec3 handAttacherPositionGlobal = vec3_zero;
	idMat3 handmat = mat3_identity;
	//idVec3 handDelta = vec3_zero;

	//idMat3 rot180 = idAngles( 0.0f, 180.0f, 0.0f ).ToMat3();


	commonVr->currentHandWorldPosition[hand] = handOrigin;

	hands[hand].handOrigin = handOrigin;
	hands[hand].handAxis = handAxis;

	weapon_t currentWeaponEnum = hands[hand].weapon->IdentifyWeapon();
	idEntityPtr<idWeapon> curEntity;
	int activeWeapon;
	if( isFlashlight && commonVr->currentFlashlightPosition == FLASHLIGHT_HAND && flashlight.IsValid() )
	{
		curEntity = flashlight;
		activeWeapon = weapon_flashlight;
	}
	else
	{
		curEntity = hands[hand].weapon;
		activeWeapon = hands[hand].currentWeapon;
	}

	handWeaponAttachJoint = ik_handAttacher[hand]; // joint on the hand the weapon attaches to
	weaponHandAttachJoint = curEntity->weaponHandAttacher[hand]; // joint on the weapon the hand should attach to
	weaponHandAttachJointDefaultPositionLocal = curEntity->weaponHandDefaultPos[hand];// the default position of the attacher on the weapon. used to calc movement deltas from anims

	
	//get the local and global orientations for the hand and weapon attacher joints

	//weapon hand attacher - the joint on the weapon the hand should align to
	curEntity->GetAnimator()->GetJointTransform( weaponHandAttachJoint, gameLocal.time, weaponHandAttachJointPositionLocal, weaponHandAttachJointAxisLocal );
	
	/*
	// for debugging
	weaponHandAttachJointPositionGlobal = weaponHandAttachJointPositionLocal * curEntity->GetRenderEntity()->axis + curEntity->GetRenderEntity()->origin;
	weaponHandAttachJointAxisGlobal = weaponHandAttachJointAxisLocal * curEntity->GetRenderEntity()->axis;
	DebugCross( weaponHandAttachJointPositionGlobal, weaponHandAttachJointAxisGlobal, colorBlue );
	*/

	// calculate the delta between the weaponAttach joint default position and the animated position for this frame
	// this is in model space
	weaponAttachDelta = weaponHandAttachJointPositionLocal - weaponHandAttachJointDefaultPositionLocal;

	if ( activeWeapon == weapon_pda && hand == vr_weaponHand.GetInteger() )
	{
		//the PDA is actually being held in the off hand,
		//so don't let the weapon animation adjust the hand position.
		weaponHandAttachJointAxisLocal = mat3_identity;
		weaponAttachDelta = vec3_zero;
	}

	//handOrigin and handAxis are the points in world space where the attacher joint on the hand model should be moved to.
	//this location is updated by weaponAttachDelta so the weapon animation can drive the hand.

	handOrigin += handWeaponAttacherToDefaultOffset[hand][activeWeapon] * handAxis;

	handAttacherPositionGlobal = handOrigin + ( weaponAttachDelta * handAxis );

	// for debugging
	//DebugCross( handAttacherPositionGlobal, handAxis, colorRed );
	
	handAttacherPositionLocal = handAttacherPositionGlobal - renderEntity.origin;
	handAttacherPositionLocal *= renderEntity.axis.Inverse();

	handmat = weaponHandAttachJointAxisLocal * rotation.ToMat3();
	
	handAttacherPositionLocal -= handWeaponAttachertoWristJointOffset[hand][activeWeapon] * handmat;

	/*
	idVec3 debug1 = handAttacherPositionLocal * renderEntity.axis;
	debug1 += renderEntity.origin;
	DebugCross( debug1, handmat * renderEntity.axis, colorGreen );
	*/

	handmat = ik_handCorrectAxis[hand][activeWeapon] * handmat;
		
	GetAnimator()->SetJointPos( armIK.handJoints[hand], JOINTMOD_WORLD_OVERRIDE, handAttacherPositionLocal );
	GetAnimator()->SetJointAxis( armIK.handJoints[hand], JOINTMOD_WORLD_OVERRIDE, handmat );
	
	commonVr->handRoll[hand] = rotation.ToAngles().roll;

}

/*
==============
Koz idPlayer::CalculateViewFlashlightPos
Calculate the flashlight orientation
==============
*/
void idPlayer::CalculateViewFlashlightPos( idVec3 &origin, idMat3 &axis, idVec3 flashlightOffset )
{
	static idVec3 viewOrigin = vec3_zero;
	static idMat3 viewAxis = mat3_identity;
	static bool setLeftHand = false;
	static weapon_t curWeap = WEAPON_NONE;
	static idMat3 swapMat = idAngles( 180.0f, 0.0f, 180.0f ).ToMat3();
	
	origin = GetEyePosition();

	if ( game->isVR && commonVr->VR_USE_MOTION_CONTROLS ) origin += commonVr->leanOffset;
		
	axis = idAngles( 0.0, viewAngles.yaw, 0.0f ).ToMat3();
	if ( game->isVR ) axis = idAngles( 0.0, viewAngles.yaw - commonVr->bodyYawOffset, 0.0f ).ToMat3();
	
	int flashlightMode = game->isVR ? commonVr->GetCurrentFlashlightMode() : FLASHLIGHT_BODY;
	
	setLeftHand = false;
	//move the flashlight to alternate location for items with no mount
		
	if ( spectating || !weaponEnabled || gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) ) flashlightMode = FLASHLIGHT_BODY;
	
	if ( flashlightMode == FLASHLIGHT_HAND )
	{
		if ( game->IsPDAOpen() || commonVr->PDAforcetoggle || hands[ 0 ].currentWeapon == weapon_pda || hands[ 1 ].currentWeapon == weapon_pda || !commonVr->VR_USE_MOTION_CONTROLS  || (commonVr->handInGui && flashlightMode == FLASHLIGHT_GUN) )
		{
			flashlightMode = FLASHLIGHT_HEAD;
		}
	}
	
	idWeapon* weaponWithFlashlightMounted = NULL;

	if( flashlightMode == FLASHLIGHT_GUN )
	{
		weaponWithFlashlightMounted = GetWeaponWithMountedFlashlight();
		if( !weaponWithFlashlightMounted || !weaponWithFlashlightMounted->GetMuzzlePositionWithHacks( origin, axis ) || commonVr->handInGui )
		{
			idAngles flashlightAx = axis.ToAngles();
			flashlightMode = FLASHLIGHT_HEAD;
			if( game->isVR ) axis = idAngles( flashlightAx.pitch, flashlightAx.yaw - commonVr->bodyYawOffset, flashlightAx.roll ).ToMat3();

		}
	}
	else if( flashlightMode == FLASHLIGHT_PISTOL )
	{
		if( hands[0].currentWeapon == weapon_pistol && !hands[0].isTheDuplicate )
			weaponWithFlashlightMounted = hands[0].weapon.GetEntity();
		else if( hands[1].currentWeapon == weapon_pistol && !hands[1].isTheDuplicate )
			weaponWithFlashlightMounted = hands[1].weapon.GetEntity();
		else
			weaponWithFlashlightMounted = NULL;

		if( !weaponWithFlashlightMounted || !weaponWithFlashlightMounted->GetMuzzlePositionWithHacks( origin, axis ) || commonVr->handInGui )
		{
			idAngles flashlightAx = axis.ToAngles();
			flashlightMode = FLASHLIGHT_INVENTORY;
			if( game->isVR ) axis = idAngles( flashlightAx.pitch, flashlightAx.yaw - commonVr->bodyYawOffset, flashlightAx.roll ).ToMat3();
		}
		else
			flashlightMode = FLASHLIGHT_GUN;
	}

	int oldFlashlightPosition = commonVr->currentFlashlightPosition;
	commonVr->currentFlashlightPosition = flashlightMode;

	switch ( flashlightMode )
	{
	
		case FLASHLIGHT_GUN:
			// move the flashlight to the weapon
			
			/* was for adjusting
			flashlightOffset.x += ftx.GetFloat();
			flashlightOffset.y += fty.GetFloat();
			flashlightOffset.z += ftz.GetFloat();
			*/

			origin += flashlightOffset.x * axis[1] + flashlightOffset.y * axis[0] + flashlightOffset.z * axis[2];
		
			curWeap = weaponWithFlashlightMounted->IdentifyWeapon();
			if ( curWeap == WEAPON_SHOTGUN_DOUBLE || curWeap == WEAPON_ROCKETLAUNCHER )
			{
				//hack was already present in the code to fix borked alignments for these weapons,
				//we need to put them back
				//std::swap( axis[0], axis[2] );
				axis = -1 * axis;
				//axis = idAngles( 180.0f, 0.0f, 180.0f ).ToMat3() * axis;
				axis = swapMat * axis;
			}
			
			flashlight->GetRenderEntity()->allowSurfaceInViewID = 0;
			flashlight->GetRenderEntity()->suppressShadowInViewID = 0;
			// if we just got the flashlight/weapon out, start with the flashlight turned on
			if( oldFlashlightPosition == FLASHLIGHT_INVENTORY || oldFlashlightPosition == FLASHLIGHT_NONE )
				FlashlightOn();
			setLeftHand = true;
			break;
				
		case FLASHLIGHT_HEAD:
			// Flashlight on helmet 
			origin = commonVr->lastViewOrigin;
			axis = commonVr->lastViewAxis;
		
			origin += vr_flashlightHelmetPosY.GetFloat() * axis[1] + vr_flashlightHelmetPosZ.GetFloat() * axis[0] + vr_flashlightHelmetPosX.GetFloat() * axis[2];
					
			flashlight->GetRenderEntity()->allowSurfaceInViewID = -1;
			flashlight->GetRenderEntity()->suppressShadowInViewID = entityNumber + 1;
			
			if ( curWeap == WEAPON_PDA && commonVr->VR_USE_MOTION_CONTROLS ) return;
			
			setLeftHand = true;
			break;
			
		case FLASHLIGHT_INVENTORY:
		case FLASHLIGHT_NONE:
			// don't draw the flashlight or its shadow
			flashlight->GetRenderEntity()->allowSurfaceInViewID = -1;
			flashlight->GetRenderEntity()->suppressShadowInViewID = entityNumber + 1;
			// if we just put the flashlight/weapon away, turn it off
			if( oldFlashlightPosition != FLASHLIGHT_INVENTORY && oldFlashlightPosition != FLASHLIGHT_NONE )
				FlashlightOff();
			setLeftHand = true;
			break;

		case FLASHLIGHT_HAND:
		
			if ( !commonVr->VR_USE_MOTION_CONTROLS )
			{
				static idQuat flashlightRot = idAngles( 0, 0.0f, 0.0f ).ToQuat();
				static idVec3 originOffset = vec3_zero;
				static int currentHand = 0;
				static int flip = 1;
				
				currentHand = 1 - vr_weaponHand.GetInteger();
				originOffset = flashlight->weaponHandDefaultPos[currentHand];

				origin = GetEyePosition();
				axis = idAngles( 0.0, viewAngles.yaw, 0.0f ).ToMat3();
				
				flip = currentHand == 0 ? 1 : -1;
				
				if ( commonVr->thirdPersonMovement )// if running in third person, make sure the hand is somewhere sane.
				{
					origin += idVec3( 4.0f, -10.0f * flip, -20.0f );
				}
				else
				{
					origin += idVec3( vr_offHandPosX.GetFloat(), vr_offHandPosY.GetFloat() * flip, vr_offHandPosZ.GetFloat() ) * axis;
				}
				idMat3 fr = flashlightRot.ToMat3() * axis;
				
				SetHandIKPos( currentHand, origin, fr , flashlightRot, true );

				origin -= originOffset * axis;
				origin += handWeaponAttacherToDefaultOffset[currentHand][weapon_flashlight] * axis;
				
				flashlight->GetRenderEntity()->allowSurfaceInViewID = 0;
				flashlight->GetRenderEntity()->suppressShadowInViewID = 0;
				break;
			}

		case FLASHLIGHT_BODY:
		default: // this is the original body mount code.
			{
				idWeapon* weapon = GetMainWeapon();
				origin = weapon->playerViewOrigin;
				axis = weapon->playerViewAxis;
				float fraccos = cos( (gameLocal.framenum & 255) / 127.0f * idMath::PI );
				static unsigned int divisor = 32;
				unsigned int val = (gameLocal.framenum + gameLocal.framenum / divisor) & 255;
				float fraccos2 = cos( val / 127.0f * idMath::PI );
				static idVec3 baseAdjustPos = idVec3( -8.0f, -20.0f, -10.0f ); // rt, fwd, up
				//static idVec3 baseAdjustPos = idVec3( 0, 0, 0 ); // rt, fwd, up
			
				if ( game->isVR )
				{
					baseAdjustPos.x = vr_flashlightBodyPosX.GetFloat();
					baseAdjustPos.y = vr_flashlightBodyPosY.GetFloat();
					baseAdjustPos.z = vr_flashlightBodyPosZ.GetFloat();
				}
			
				static float pscale = 0.5f;
				static float yscale = 0.125f;
				idVec3 adjustPos = baseAdjustPos;// + ( idVec3( fraccos, 0.0f, fraccos2 ) * scale );
				origin += adjustPos.x * axis[1] + adjustPos.y * axis[0] + adjustPos.z * axis[2];
				//viewWeaponOrigin += owner->viewBob;
			//	static idAngles baseAdjustAng = idAngles( 88.0f, 10.0f, 0.0f ); //
				static idAngles baseAdjustAng = idAngles( 0.0f,10.0f, 0.0f ); //
				idAngles adjustAng = baseAdjustAng + idAngles( fraccos * pscale, fraccos2 * yscale, 0.0f );
				// adjustAng += owner->GetViewBobAngles();
				axis = adjustAng.ToMat3() * axis;
				flashlight->GetRenderEntity()->allowSurfaceInViewID = -1;
				// Carl: This fixes the flashlight shadow in Mars City 1.
				// I could check if ( spectating || !weaponEnabled || gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) )
				// but I don't think an armor-mounted light should ever cast a flashlight-shaped shadow
				flashlight->GetRenderEntity()->suppressShadowInViewID = entityNumber + 1;
				setLeftHand = true;
			}
	}

	
	// Koz fixme this is where we set the left hand position. Yes it's a stupid place to do it move later
	
	if ( game->IsPDAOpen() || commonVr->PDAforcetoggle || hands[0].currentWeapon == weapon_pda || hands[1].currentWeapon == weapon_pda ) return; //dont dont anything with the left hand if motion controlling the PDA, only if fixed.
	
	if ( commonVr->VR_USE_MOTION_CONTROLS ) // && ( !game->IsPDAOpen() || commonVr->PDAforcetoggle || currentWeapon == weapon_pda ) )
	{
		
		static idVec3 motionPosition = vec3_zero;
		static idQuat motionRotation;
		static idVec3 originOffset = vec3_zero;
		static int currentHand = 0;
		static idAngles hmdAngles;
		static idVec3 headPositionDelta;
		static idVec3 bodyPositionDelta;
		static idVec3 absolutePosition;
		static idQuat flashlightPitch;
		static bool isFlashlight = false;
				
		currentHand = 1 - vr_weaponHand.GetInteger();
		originOffset = flashlight->weaponHandDefaultPos[currentHand];
		flashlightPitch = idAngles( vr_motionFlashPitchAdj.GetFloat(), 0.f, 0.0f ).ToQuat();
		isFlashlight = true;
		
		commonVr->MotionControlGetHand( currentHand, motionPosition, motionRotation );
		
		motionRotation = flashlightPitch * motionRotation;
		
		GetViewPos( viewOrigin, viewAxis );  //GetEyePosition();
	
		viewOrigin += commonVr->leanOffset;

		//commonVr->HMDGetOrientation( hmdAngles, headPositionDelta, bodyPositionDelta, absolutePosition, false );
	
		hmdAngles = commonVr->poseHmdAngles;
		headPositionDelta = commonVr->poseHmdHeadPositionDelta;
		bodyPositionDelta = commonVr->poseHmdBodyPositionDelta;
		absolutePosition = commonVr->poseHmdAbsolutePosition;

		viewAxis = idAngles( 0.0, viewAxis.ToAngles().yaw - commonVr->bodyYawOffset, 0.0f ).ToMat3();
		viewOrigin += viewAxis[0] * headPositionDelta.x + viewAxis[1] * headPositionDelta.y + viewAxis[2] * headPositionDelta.z;
		
		viewOrigin += motionPosition * viewAxis;
		viewAxis = motionRotation.ToMat3() * viewAxis;
		
		idAngles motRot = motionRotation.ToAngles();
		motRot.yaw -= commonVr->bodyYawOffset;
		motRot.Normalize180();
		motionRotation = motRot.ToQuat();
		

		// Koz fixme:
		// Koz hack , the alignment isn't quite right, so do a quick hack here so the hand and flashlight align 
		// better with the controllers.
		// need to really fix this right, the whole body/viewweapon pose attacher code is a complete trainwreck now.
		
		const idVec3 flashlightPosHack[2] = { idVec3( 0.0f, -1.0f, 0.5f ), idVec3( 0.0f, 0.85f, 0.5f ) };
		viewOrigin += flashlightPosHack[currentHand] * viewAxis;
		
		//DebugCross( viewOrigin, viewAxis, colorYellow );
		SetHandIKPos( currentHand , viewOrigin, viewAxis, motionRotation, isFlashlight );
		
		if ( flashlightMode == FLASHLIGHT_HAND   )
		{
			origin = viewOrigin;
			origin -= originOffset * viewAxis;
			
			int wepn = (hands[0].weapon->IdentifyWeapon() == WEAPON_PDA || hands[ 1 ].weapon->IdentifyWeapon() == WEAPON_PDA ) ? weapon_pda : weapon_flashlight;
			origin += handWeaponAttacherToDefaultOffset[currentHand][wepn/*weapon_flashlight*/] * viewAxis;
			axis = viewAxis;

			//DebugCross( origin, axis, colorOrange );

			flashlight->GetRenderEntity()->allowSurfaceInViewID = 0;
			flashlight->GetRenderEntity()->suppressShadowInViewID = 0;
		}
		
	}
	else if ( setLeftHand == true  )
	{
			// if the flashlight is not in the left hand, set the player model hand location anyway.
			static idQuat flashlightRot = idAngles( 0, 0.0f, 0.0f ).ToQuat();
			static idVec3 originOffset = vec3_zero;
			static int currentHand = 0;
			static int flip = 1;
			static idVec3 handLoc = vec3_zero;
			static idMat3 handOrg = mat3_identity;

			currentHand = 1 - vr_weaponHand.GetInteger();
			originOffset = flashlight->weaponHandDefaultPos[currentHand];

			handLoc = GetEyePosition();

			handLoc += commonVr->leanOffset;

			handOrg = idAngles( 0.0, viewAngles.yaw, 0.0f ).ToMat3();

			flip = currentHand == 0 ? 1 : -1;
			
			if ( commonVr->thirdPersonMovement )// if running in third person, make sure the hand is somewhere sane.
			{
				handLoc += idVec3( 4.0f, -10.0f * flip, -20.0f ) * handOrg;
			}
			else
			{
				handLoc += idVec3( vr_offHandPosX.GetFloat(), vr_offHandPosY.GetFloat() * flip, vr_offHandPosZ.GetFloat() ) * handOrg;
			}

			idMat3 fr = flashlightRot.ToMat3() * handOrg;

			SetHandIKPos( currentHand, handLoc, fr, flashlightRot, true );
	}

	if ( game->CheckInCinematic() )
	{
		flashlight->GetRenderEntity()->allowSurfaceInViewID = -1;
		flashlight->GetRenderEntity()->suppressShadowInViewID = entityNumber + 1;
	}
}
	

/*
===============
idPlayer::OffsetThirdPersonView
===============
*/
void idPlayer::OffsetThirdPersonView( float angle, float range, float height, bool clip )
{
	idVec3			view;
	idVec3			focusAngles;
	trace_t			trace;
	idVec3			focusPoint;
	float			focusDist;
	float			forwardScale, sideScale;
	idVec3			origin;
	idAngles		angles;
	idMat3			axis;
	idBounds		bounds;
	
	angles = viewAngles;
	GetViewPos( origin, axis );
	
	if( angle )
	{
		angles.pitch = 0.0f;
	}
	
	if( angles.pitch > 45.0f )
	{
		angles.pitch = 45.0f;		// don't go too far overhead
	}
	
	focusPoint = origin + angles.ToForward() * THIRD_PERSON_FOCUS_DISTANCE;
	focusPoint.z += height;
	view = origin;
	view.z += 8 + height;
	
	angles.pitch *= 0.5f;
	renderView->viewaxis = angles.ToMat3() * physicsObj.GetGravityAxis();
	
	idMath::SinCos( DEG2RAD( angle ), sideScale, forwardScale );
	view -= range * forwardScale * renderView->viewaxis[ 0 ];
	view += range * sideScale * renderView->viewaxis[ 1 ];
	
	if( clip )
	{
		// trace a ray from the origin to the viewpoint to make sure the view isn't
		// in a solid block.  Use an 8 by 8 block to prevent the view from near clipping anything
		bounds = idBounds( idVec3( -4, -4, -4 ), idVec3( 4, 4, 4 ) );
		gameLocal.clip.TraceBounds( trace, origin, view, bounds, MASK_SOLID, this );
		if( trace.fraction != 1.0f )
		{
			view = trace.endpos;
			view.z += ( 1.0f - trace.fraction ) * 32.0f;
			
			// try another trace to this position, because a tunnel may have the ceiling
			// close enough that this is poking out
			gameLocal.clip.TraceBounds( trace, origin, view, bounds, MASK_SOLID, this );
			view = trace.endpos;
		}
	}
	
	// select pitch to look at focus point from vieword
	focusPoint -= view;
	focusDist = idMath::Sqrt( focusPoint[0] * focusPoint[0] + focusPoint[1] * focusPoint[1] );
	if( focusDist < 1.0f )
	{
		focusDist = 1.0f;	// should never happen
	}
	
	angles.pitch = - RAD2DEG( atan2( focusPoint.z, focusDist ) );
	angles.yaw -= angle;
	
	renderView->vieworg = view;
	renderView->viewaxis = angles.ToMat3() * physicsObj.GetGravityAxis();
	renderView->viewID = 0;
}

/*
===============
idPlayer::GetEyePosition
===============
*/
idVec3 idPlayer::GetEyePosition() const
{
	idVec3 org;
	
	// use the smoothed origin if spectating another player in multiplayer
	if( common->IsClient() && !IsLocallyControlled() )
	{
		org = smoothedOrigin;
	}
	else
	{
		org = GetPhysics()->GetOrigin();
	}
	return org + (GetPhysics()->GetGravityNormal() * -eyeOffset.z) + idVec3(0, 0, commonVr->headHeightDiff);
}

/*
===============
idPlayer::GetViewPos
===============
*/
void idPlayer::GetViewPos( idVec3& origin, idMat3& axis ) const
{
	idAngles angles;

	if ( game->isVR )
	{
		GetViewPosVR( origin, axis );
		return;
	}

	// if dead, fix the angle and don't add any kick
	if ( health <= 0 )
	{
		if ( game->isVR ) // Koz 
		{
			angles = viewAngles; 
		}
		else
		{
			angles.yaw = viewAngles.yaw;
			angles.roll = 40;
			angles.pitch = -15;
		}

		axis = angles.ToMat3();
		origin = GetEyePosition();

	}
	else
	{
		origin = GetEyePosition() + viewBob;
		angles = viewAngles + viewBobAngles + playerView.AngleOffset();

		axis = angles.ToMat3() * physicsObj.GetGravityAxis();

		// Move pivot point down so looking straight ahead is a no-op on the Z
		const idVec3& gravityVector = physicsObj.GetGravityNormal();
		origin += gravityVector * g_viewNodalZ.GetFloat();

		// adjust the origin based on the camera nodal distance (eye distance from neck)
		origin += axis[0] * g_viewNodalX.GetFloat() + axis[2] * g_viewNodalZ.GetFloat();
	}
}
/*
===============
idPlayer::GetViewPosVR
===============
*/
void idPlayer::GetViewPosVR( idVec3 &origin, idMat3 &axis ) const {
	
	idAngles angles;

	// if dead, fix the angle and don't add any kick
	if ( health <= 0 )
	{
		angles = viewAngles;
		axis = angles.ToMat3();
		origin = GetEyePosition();
		return;
	}

	//Carl: Use head and neck rotation model
	float eyeHeightAboveRotationPoint;
	float eyeShiftRight = 0;
	
	eyeHeightAboveRotationPoint = 5;//
	
	origin = GetEyePosition(); // +viewBob;
	// Carl: No view bobbing unless knockback is enabled. This isn't strictly a knockback, but close enough.
	// This is the bounce when you land after jumping

	// re-enabling this until a better method is implemented, as headbob is how vertical smoothing is implemented when going over stairs/bumps
	// bobbing can be disabled via the walkbob and runbob cvars until a better method devised.
	// if (vr_knockBack.GetBool()) 


	origin += viewBob;
	angles = viewAngles; // NO VIEW KICKING  +playerView.AngleOffset();
	axis = angles.ToMat3();// *physicsObj.GetGravityAxis();

	// Move pivot point down so looking straight ahead is a no-op on the Z
//		const idVec3 & gravityVector = physicsObj.GetGravityNormal();
	//origin += gravityVector * g_viewNodalZ.GetFloat();


	// adjust the origin based on the camera nodal distance (eye distance from neck)
	//origin += axis[0] * g_viewNodalX.GetFloat() - axis[1] * eyeShiftRight + axis[2] * eyeHeightAboveRotationPoint;
	//origin +=  axis[1] * -eyeShiftRight + axis[2] * eyeHeightAboveRotationPoint;
	origin += axis[2] * eyeHeightAboveRotationPoint;

	//common->Printf( "GetViewPosVr returning %s\n", origin.ToString() );

}

/*
===============
idPlayer::CalculateFirstPersonView
===============
*/
void idPlayer::CalculateFirstPersonView()
{
	if( ( pm_modelView.GetInteger() == 1 ) || ( ( pm_modelView.GetInteger() == 2 ) && ( health <= 0 ) ) )
	{
		//	Displays the view from the point of view of the "camera" joint in the player model
		
		idMat3 axis;
		idVec3 origin;
		idAngles ang;
		
		ang = viewBobAngles + playerView.AngleOffset();
		ang.yaw += viewAxis[ 0 ].ToYaw();
		
		jointHandle_t joint = animator.GetJointHandle( "camera" );
		animator.GetJointTransform( joint, gameLocal.time, origin, axis );
		firstPersonViewOrigin = ( origin + modelOffset ) * ( viewAxis * physicsObj.GetGravityAxis() ) + physicsObj.GetOrigin() + viewBob;
		firstPersonViewAxis = axis * ang.ToMat3() * physicsObj.GetGravityAxis();
	}
	else
	{
		// offset for local bobbing and kicks
		GetViewPos( firstPersonViewOrigin, firstPersonViewAxis );
#if 0
		// shakefrom sound stuff only happens in first person
		firstPersonViewAxis = firstPersonViewAxis * playerView.ShakeAxis();
#endif
	}
	if ( commonVr->hasHMD )
	{
		CalculateLeftHand();
		CalculateRightHand();
		CalculateWaist();
	}
}

void idPlayer::CalculateWaist()
{
	idMat3 & hmdAxis = commonVr->lastHMDViewAxis;

	waistOrigin = hmdAxis * neckOffset + commonVr->lastHMDViewOrigin;
	waistOrigin.z += waistZ;

	if ( hmdAxis[0].z < 0 ) // looking down
	{
		if ( hmdAxis[2].z > 0 )
		{
			// use a point between head forward and upward
			float h = hmdAxis[2].z - hmdAxis[0].z;
			float x = -hmdAxis[0].z / h;
			float y = hmdAxis[2].z / h;
			idVec3 i = hmdAxis[0] * y + hmdAxis[2] * x;
			float yaw = atan2( i.y, i.x ) * idMath::M_RAD2DEG;
			waistAxis = idAngles( 0, yaw, 0 ).ToMat3();
		}
		else
		{
			// use a point between head backward and upward
			float h = -hmdAxis[2].z - hmdAxis[0].z;
			float x = -hmdAxis[0].z / h;
			float y = hmdAxis[2].z / h;
			idVec3 i = hmdAxis[0] * y + hmdAxis[2] * x;
			float yaw = atan2( i.y, i.x ) * idMath::M_RAD2DEG;
			waistAxis = idAngles( 0, yaw, 0 ).ToMat3();
		}
	}
	else // fallback
	{
		waistAxis = idAngles( 0, hmdAxis.ToAngles().yaw, 0 ).ToMat3();
	}
}

void idPlayer::CalculateLeftHand()
{
	slotIndex_t oldSlot = hands[HAND_LEFT].handSlot;
	slotIndex_t slot = SLOT_NONE;
	if ( commonVr->hasHMD )
	{
		// remove pitch
		idMat3 axis = firstPersonViewAxis;
		//float pitch = idMath::M_RAD2DEG * asin(axis[0][2]);
		//idAngles angles(pitch, 0, 0);
		//axis = angles.ToMat3() * axis;
		//leftHandOrigin = hmdOrigin + (usercmd.vrLeftControllerOrigin - usercmd.vrHeadOrigin) * vrFaceForward * axis;
		//leftHandAxis = usercmd.vrLeftControllerAxis * vrFaceForward * axis;

		if( !vr_slotDisable.GetBool() )
		{
			for( int i = 0; i < SLOT_COUNT; i++ )
			{
				idVec3 slotOrigin = slots[i].origin;
				if ( vr_weaponHand.GetInteger() && i != SLOT_FLASHLIGHT_SHOULDER )
					slotOrigin.y *= -1;
				idVec3 origin = waistOrigin + slotOrigin * waistAxis;
				if( ( hands[HAND_LEFT].handOrigin - origin ).LengthSqr() < slots[i].radiusSq )
				{
					slot = (slotIndex_t)i;
					break;
				}
			}
		}
	}
	else
	{
		//hands[HAND_LEFT].handOrigin = hmdOrigin + hmdAxis[2] * -5;
		//hands[HAND_LEFT].handAxis = hmdAxis;
	}
	if( oldSlot != slot )
	{
		hands[HAND_LEFT].SetControllerShake( vr_slotMag.GetFloat(), vr_slotDur.GetInteger(), vr_slotMag.GetFloat(), vr_slotDur.GetInteger() );
	}
	hands[HAND_LEFT].handSlot = slot;
}

void idPlayer::CalculateRightHand()
{
	slotIndex_t oldSlot = hands[HAND_RIGHT].handSlot;
	slotIndex_t slot = SLOT_NONE;
	if ( commonVr->hasHMD )
	{
		// remove pitch
		idMat3 axis = firstPersonViewAxis;
		//float pitch = idMath::M_RAD2DEG * asin(axis[0][2]);
		//idAngles angles(pitch, 0, 0);
		//axis = angles.ToMat3() * axis;
		//rightHandOrigin = hmdOrigin + (usercmd.vrRightControllerOrigin - usercmd.vrHeadOrigin) * vrFaceForward * axis;
		//rightHandAxis = usercmd.vrRightControllerAxis * vrFaceForward * axis;

		if( !vr_slotDisable.GetBool() )
		{
			for( int i = 0; i < SLOT_COUNT; i++ )
			{
				idVec3 slotOrigin = slots[i].origin;
				if ( vr_weaponHand.GetInteger() && i != SLOT_FLASHLIGHT_SHOULDER )
					slotOrigin.y *= -1;
				idVec3 origin = waistOrigin + slotOrigin * waistAxis;
				if( (hands[HAND_RIGHT].handOrigin - origin).LengthSqr() < slots[i].radiusSq )
				{
					slot = (slotIndex_t)i;
					break;
				}
			}
		}
	}
	else
	{
		//rightHandOrigin = hmdOrigin + hmdAxis[2] * -5;
		//rightHandAxis = hmdAxis;
	}
	if( oldSlot != slot )
	{
		hands[HAND_RIGHT].SetControllerShake(vr_slotMag.GetFloat(), vr_slotDur.GetInteger(), vr_slotMag.GetFloat(), vr_slotDur.GetInteger() );
	}
	hands[HAND_RIGHT].handSlot = slot;
}

/*
==================
idPlayer::ShouldBlink

Returns true if the view needs to be darkened
==================
*/
bool idPlayer::ShouldBlink()
{
	return ( blink || commonVr->leanBlank );
}

/*
==================
idPlayer::GetRenderView

Returns the renderView that was calculated for this tic
==================
*/
renderView_t* idPlayer::GetRenderView()
{
	return renderView;
}

/*
==================
idPlayer::CalculateRenderView

create the renderView for the current tic
==================
*/
void idPlayer::CalculateRenderView()
{
	// Koz add headtracking
	static idAngles hmdAngles( 0.0, 0.0, 0.0 );
	static idVec3 lastValidHmdTranslation = vec3_zero;
	static idVec3 headPositionDelta = vec3_zero;
	static idVec3 bodyPositionDelta = vec3_zero;

	static bool wasCinematic = false;
	static idVec3 cinematicOffset = vec3_zero;
	static float cineYawOffset = 0.0f;

	static bool wasThirdPerson = false;
	static idVec3 thirdPersonOffset = vec3_zero;
	static idVec3 thirdPersonOrigin = vec3_zero;
	static idMat3 thirdPersonAxis = mat3_identity;
	static float thirdPersonBodyYawOffset = 0.0f;


	int i;
	float range;

	if ( !renderView )
	{
		renderView = new(TAG_ENTITY)renderView_t;
	}
	memset( renderView, 0, sizeof( *renderView ) );

	// copy global shader parms
	for ( i = 0; i < MAX_GLOBAL_SHADER_PARMS; i++ )
	{
		renderView->shaderParms[i] = gameLocal.globalShaderParms[i];
	}
	renderView->globalMaterial = gameLocal.GetGlobalMaterial();

	renderView->time[0] = gameLocal.slow.time;
	renderView->time[1] = gameLocal.fast.time;

	renderView->viewID = 0;

	// check if we should be drawing from a camera's POV
	if ( !noclip && (gameLocal.GetCamera() || privateCameraView) )
	{
		
		
		// get origin, axis, and fov
		if ( privateCameraView )
		{
								
			privateCameraView->GetViewParms( renderView );
		}
		else
		{
			// Koz fixme this was in tmeks renderView->viewaxis = firstPersonViewAxis; shouldnt be needed, verify.
			gameLocal.GetCamera()->GetViewParms( renderView );
		}
		
	}
	else
	{
		if ( g_stopTime.GetBool() || commonVr->VR_GAME_PAUSED ) // Koz 
		{
			renderView->vieworg = firstPersonViewOrigin;
			renderView->viewaxis = firstPersonViewAxis;

			if ( !pm_thirdPerson.GetBool() )
			{
				// set the viewID to the clientNum + 1, so we can suppress the right player bodies and
				// allow the right player view weapons
				renderView->viewID = entityNumber + 1;
			}
		}
		else if ( pm_thirdPerson.GetBool() )
		{
			OffsetThirdPersonView( pm_thirdPersonAngle.GetFloat(), pm_thirdPersonRange.GetFloat(), pm_thirdPersonHeight.GetFloat(), pm_thirdPersonClip.GetBool() );
		}
		else if ( pm_thirdPersonDeath.GetBool() )
		{
			range = gameLocal.time < minRespawnTime ? (gameLocal.time + RAGDOLL_DEATH_TIME - minRespawnTime) * (120.0f / RAGDOLL_DEATH_TIME) : 120.0f;
			OffsetThirdPersonView( 0.0f, 20.0f + range, 0.0f, false );
		}
		else
		{
			renderView->vieworg = firstPersonViewOrigin;
			renderView->viewaxis = firstPersonViewAxis;

			// set the viewID to the clientNum + 1, so we can suppress the right player bodies and
			// allow the right player view weapons
			renderView->viewID = entityNumber + 1;
		}

		gameLocal.CalcFov( CalcFov( true ), renderView->fov_x, renderView->fov_y );
	}

	if ( renderView->fov_y == 0 )
	{
		common->Error( "renderView->fov_y == 0" );
	}

	if ( g_showviewpos.GetBool() )
	{
		gameLocal.Printf( "%s : %s\n", renderView->vieworg.ToString(), renderView->viewaxis.ToAngles().ToString() );
	}

	if ( game->isVR )
	{
				
		// Koz headtracker does not modify the model rotations
		// offsets to body rotation added here

		// body position based on neck model
		// Koz fixme fix this.

		// Koz begin : Add headtracking
		static idVec3 absolutePosition;
		
		hmdAngles = commonVr->poseHmdAngles;
		headPositionDelta = commonVr->poseHmdHeadPositionDelta;
		bodyPositionDelta = commonVr->poseHmdBodyPositionDelta;
		absolutePosition = commonVr->poseHmdAbsolutePosition;

		idVec3 origin = renderView->vieworg;
		idAngles angles = renderView->viewaxis.ToAngles();
		idMat3 axis = renderView->viewaxis;
		float yawOffset = commonVr->bodyYawOffset;





		
		// Koz
		// cant believe I did this it's so ugly.
		// In the final level, the player aas file for the map doesn't allow
		// teleporting past the sarcophagus.
		// This means the player can't use teleportation in the final battle.
		// The standard AAS48 file allows teleportation around the hellhole / final
		// fight, so check how far the player has descended in the level, and
		// switch AAS files if below threshold.

		// There is still a section of the level where the player will have to walk - 
		// the path from the sarcophagus to where the floor drops, but this
		// is the best I can do until I can figure out how to compile a better AAS
		// file for the level and get rid of this mess.
		
		if ( strstr( commonLocal.GetCurrentMapName(), "game/hellhole" ) != 0 )
		{
			float pz = physicsObj.GetOrigin().z;

			if ( pz < 1138.0f && aasState != 2 )
			{
				common->Printf( "Updating player AAS to aas48\n" );
				SetAAS( true );
				InitAASLocation();
				aasState = 2;
			}
			else if ( aasState != 1 )
			{
				common->Printf( "Updating player AAS to aas_player\n" );
				SetAAS();
				InitAASLocation();
				aasState = 1;
			}
		}


	
		if ( gameLocal.inCinematic || privateCameraView )
		{
			if ( wasCinematic == false )
			{
				wasCinematic = true;
								
				commonVr->cinematicStartViewYaw = hmdAngles.yaw + commonVr->trackingOriginYawOffset;
//#ifdef USE_OVR //TODO: ovr only?
				commonVr->cinematicStartPosition = absolutePosition + (commonVr->trackingOriginOffset * idAngles( 0.0f, commonVr->trackingOriginYawOffset, 0.0f ).ToMat3());
//#endif
				cineYawOffset = hmdAngles.yaw - yawOffset;
				//commonVr->cinematicStartPosition.x = -commonVr->hmdTrackingState.HeadPose.ThePose.Position.z;
				//commonVr->cinematicStartPosition.y = -commonVr->hmdTrackingState.HeadPose.ThePose.Position.x;
				//commonVr->cinematicStartPosition.z = commonVr->hmdTrackingState.HeadPose.ThePose.Position.y;
				
				playerView.Flash(colorWhite, 300);
				
				if ( vr_cinematics.GetInteger() == 2 && vr_flicksyncCharacter.GetInteger() == 0)
				{
					cinematicOffset = vec3_zero;
				}
				else
				{
					cinematicOffset = absolutePosition;
				}
			}

			if (vr_cinematics.GetInteger() == 2 && vr_flicksyncCharacter.GetInteger() == 0)
			{
				headPositionDelta = bodyPositionDelta = vec3_zero;
			}
			else
			{
				headPositionDelta = absolutePosition - cinematicOffset;
				bodyPositionDelta = vec3_zero;
			}
		}
		else
		{
			wasCinematic = false;
			cineYawOffset = 0.0f;
		}

		// if we're not in a projected cutscene
		if (!(gameLocal.inCinematic && vr_cinematics.GetInteger() == 2 && vr_flicksyncCharacter.GetInteger() == 0))
		{

			//move the head in relation to the body. 
			//bodyYawOffsets are external rotations of the body where the head remains looking in the same direction
			//e.g. when using movepoint and snapping the body to the view.

			idAngles bodyAng = axis.ToAngles();
			idMat3 bodyAx = idAngles( bodyAng.pitch, bodyAng.yaw - yawOffset, bodyAng.roll ).Normalize180().ToMat3();
			origin = origin + bodyAx[0] * headPositionDelta.x + bodyAx[1] * headPositionDelta.y + bodyAx[2] * headPositionDelta.z;

			origin += commonVr->leanOffset;
				
			// Koz to do clean up later - added to allow cropped cinematics with original camera movements.
			idQuat q1, q2;

			q1 = angles.ToQuat();
			q2 = idAngles( hmdAngles.pitch, ( hmdAngles.yaw - yawOffset ) - cineYawOffset, hmdAngles.roll ).ToQuat();

			angles = ( q2 * q1 ).ToAngles();

			angles.Normalize180();

			commonVr->lastHMDYaw = hmdAngles.yaw;
			commonVr->lastHMDPitch = hmdAngles.pitch;
			commonVr->lastHMDRoll = hmdAngles.roll;

			axis = angles.ToMat3(); // this sets the actual view axis, separate from the body axis.

			commonVr->lastHMDViewOrigin = origin;
			commonVr->lastHMDViewAxis = axis;

			commonVr->uncrouchedHMDViewOrigin = origin;
			commonVr->uncrouchedHMDViewOrigin.z -= commonVr->headHeightDiff;


			if (commonVr->thirdPersonMovement)
			{
				if (wasThirdPerson == false)
				{
					wasThirdPerson = true;
					thirdPersonOffset = absolutePosition;
					thirdPersonOrigin = commonVr->lastHMDViewOrigin;//origin;
					thirdPersonAxis = idAngles(0.0f, commonVr->lastHMDViewAxis.ToAngles().yaw, 0.0f).ToMat3();//axis;
					thirdPersonBodyYawOffset = hmdAngles.yaw;// -yawOffset;

				}
				origin = thirdPersonOrigin;
				axis = thirdPersonAxis;
				yawOffset = thirdPersonBodyYawOffset;
				angles = thirdPersonAxis.ToAngles();
				headPositionDelta = absolutePosition - thirdPersonOffset;
				bodyPositionDelta = vec3_zero;

				idAngles bodyAng = thirdPersonAxis.ToAngles();
				idMat3 bodyAx = idAngles(bodyAng.pitch, bodyAng.yaw - yawOffset, bodyAng.roll).Normalize180().ToMat3();
				origin = thirdPersonOrigin + bodyAx[0] * headPositionDelta.x + bodyAx[1] * headPositionDelta.y + bodyAx[2] * headPositionDelta.z;

				origin += commonVr->leanOffset;

				angles.yaw += hmdAngles.yaw - thirdPersonBodyYawOffset;    // add the current hmd orientation
				angles.pitch += hmdAngles.pitch;
				angles.roll += hmdAngles.roll;
				angles.Normalize180();
				axis = angles.ToMat3();
				commonVr->thirdPersonHudAxis = axis;
				commonVr->thirdPersonHudPos = origin;

				commonVr->thirdPersonDelta = (origin - firstPersonViewOrigin).LengthSqr();

			}
			else
			{
				if (wasThirdPerson)
				{
					commonVr->thirdPersonDelta = 0.0f;;
					playerView.Flash(colorBlack, 140);
				}
				wasThirdPerson = false;
			}

		}
		renderView->vieworg = origin;
		renderView->viewaxis = axis;

		// if leaning, check if the eye is in a wall
		if ( commonVr->isLeaning )
		{
			idBounds bounds = idBounds( idVec3( 0, -1, -1 ), idVec3( 0, 1, 1 ) );
			trace_t trace;

			gameLocal.clip.TraceBounds( trace, origin, origin, bounds, MASK_SHOT_RENDERMODEL /*MASK_SOLID*/, this );
			if ( trace.fraction != 1.0f )
			{
				commonVr->leanBlank = true;
				commonVr->leanBlankOffset = commonVr->leanOffset;
				commonVr->leanBlankOffsetLengthSqr = commonVr->leanOffset.LengthSqr();
				
			}

			else
			{
				commonVr->leanBlank = false;
				commonVr->leanBlankOffset = vec3_zero;
				commonVr->leanBlankOffsetLengthSqr = 0.0f;
			}

		}
		
		
		

		// Koz fixme pause - handle the PDA model if game is paused
		// really really need to move this somewhere else,

		if ( !commonVr->PDAforcetoggle && commonVr->PDAforced && hands[ 0 ].weapon->IdentifyWeapon() != WEAPON_PDA && hands[ 1 ].weapon->IdentifyWeapon() != WEAPON_PDA ) // PDAforced cannot be valid if the weapon is not the PDA
		{
			commonVr->PDAforced = false;
			commonVr->VR_GAME_PAUSED = false;
			idPlayer* player = gameLocal.GetLocalPlayer();
			player->SetupPDASlot( true );
			player->SetupHolsterSlot( vr_weaponHand.GetInteger(), true );
		}

		if ( commonVr->PDAforcetoggle )
		{
			int pdahand = 1 - vr_weaponHand.GetInteger();
			if ( !commonVr->PDAforced )
			{
				if ( hands[ 0 ].weapon->IdentifyWeapon() != WEAPON_PDA && hands[ 1 ].weapon->IdentifyWeapon() != WEAPON_PDA )
				{
					//common->Printf( "idPlayer::CalculateRenderView calling SelectWeapon for PDA\nPDA Forced = %i, PDAForceToggle = %i\n",commonVr->PDAforced,commonVr->PDAforcetoggle );
					//common->Printf( "CRV3 Calling SetupHolsterSlot( %i, %i ) \n", 1 - pdahand, commonVr->PDAforced );
					
					idPlayer* player = gameLocal.GetLocalPlayer();
					player->SetupPDASlot( commonVr->PDAforced );
					player->SetupHolsterSlot( 1 - pdahand, commonVr->PDAforced );
										
					hands[ pdahand ].SelectWeapon( weapon_pda, true, false );
					SetWeaponHandPose();
				}
				else
				{

					if ( (hands[ 0 ].weapon->IdentifyWeapon() == WEAPON_PDA && hands[ 0 ].weapon->status == WP_READY) || ( hands[ 1 ].weapon->IdentifyWeapon() == WEAPON_PDA && hands[ 1 ].weapon->status == WP_READY ) )
					{
						commonVr->PDAforced = true;
						commonVr->PDAforcetoggle = false;
					}
				}
			}
			else
			{ // pda has been already been forced active, put it away.

				TogglePDA( pdahand );
				commonVr->PDAforcetoggle = false;
				commonVr->PDAforced = false;
			}

		}
	}
	else // Carl: not VR
	{
		idVec3 origin = renderView->vieworg;
		// if we're not in a cutscene
		if( !gameLocal.inCinematic )
		{
			// Used to force the user's head down when there's a low ceiling  
			commonVr->uncrouchedHMDViewOrigin = origin;
			commonVr->uncrouchedHMDViewOrigin.z -= commonVr->headHeightDiff;
		}
	}
}

/*
=============
idPlayer::AddAIKill
=============
*/
void idPlayer::AddAIKill()
{
	int max_souls;
	int ammo_souls;
	
	if( ( weapon_soulcube < 0 ) || ( inventory.weapons & ( 1 << weapon_soulcube ) ) == 0 )
	{
		return;
	}
	
	ammo_souls = idWeapon::GetAmmoNumForName( "ammo_souls" );
	max_souls = inventory.MaxAmmoForAmmoClass( this, "ammo_souls" );
	const int currentSoulAmmo = inventory.GetInventoryAmmoForType( ammo_souls );
	if( currentSoulAmmo < max_souls )
	{
		inventory.SetInventoryAmmoForType( ammo_souls, currentSoulAmmo + 1 );
		if( inventory.GetInventoryAmmoForType( ammo_souls ) >= max_souls )
		{
		
			if( hud )
			{
				hud->UpdateSoulCube( true );
			}
			StartSound( "snd_soulcube_ready", SND_CHANNEL_ANY, 0, false, NULL );
		}
	}
}

/*
=============
idPlayer::SetSoulCubeProjectile
=============
*/
void idPlayer::SetSoulCubeProjectile( idProjectile* projectile )
{
	soulCubeProjectile = projectile;
}

/*
=============
idPlayer::AddProjectilesFired
=============
*/
void idPlayer::AddProjectilesFired( int count )
{
	numProjectilesFired += count;
}

/*
=============
idPlayer::AddProjectileHites
=============
*/
void idPlayer::AddProjectileHits( int count )
{
	numProjectileHits += count;
}

/*
=============
idPlayer::SetLastHitTime
=============
*/
void idPlayer::SetLastHitTime( int time )
{
	idPlayer* aimed = NULL;
	
	if( time && lastHitTime != time )
	{
		lastHitToggle ^= 1;
	}
	lastHitTime = time;
	if( !time )
	{
		// level start and inits
		return;
	}
	if( common->IsMultiplayer() && ( time - lastSndHitTime ) > 10 )
	{
		lastSndHitTime = time;
		StartSound( "snd_hit_feedback", SND_CHANNEL_ANY, SSF_PRIVATE_SOUND, false, NULL );
	}
	
	if( hud )
	{
		hud->CombatCursorFlash();
	}
	
	if( MPAim != -1 )
	{
		if( gameLocal.entities[ MPAim ] && gameLocal.entities[ MPAim ]->IsType( idPlayer::Type ) )
		{
			aimed = static_cast< idPlayer* >( gameLocal.entities[ MPAim ] );
		}
		assert( aimed );
		// full highlight, no fade till loosing aim
		
		if( hud )
		{
			int color = 0;
			if( aimed )
			{
				color = aimed->team + 1;
			}
			hud->TriggerHitTarget( true, session->GetActingGameStateLobbyBase().GetLobbyUserName( gameLocal.lobbyUserIDs[ MPAim ] ), color );
		}
		MPAimHighlight = true;
		MPAimFadeTime = 0;
	}
	else if( lastMPAim != -1 )
	{
		if( gameLocal.entities[ lastMPAim ] && gameLocal.entities[ lastMPAim ]->IsType( idPlayer::Type ) )
		{
			aimed = static_cast< idPlayer* >( gameLocal.entities[ lastMPAim ] );
		}
		assert( aimed );
		// start fading right away
		if( hud )
		{
			int color = 0;
			if( aimed )
			{
				color = aimed->team + 1;
			}
			hud->TriggerHitTarget( true, session->GetActingGameStateLobbyBase().GetLobbyUserName( gameLocal.lobbyUserIDs[ lastMPAim ] ), color );
			hud->TriggerHitTarget( false, "" );
		}
		MPAimHighlight = false;
		MPAimFadeTime = gameLocal.realClientTime;
	}
}

/*
=============
idPlayer::SetInfluenceLevel
=============
*/
void idPlayer::SetInfluenceLevel( int level )
{
	if( level != influenceActive )
	{
		if( level )
		{
			for( idEntity* ent = gameLocal.spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next() )
			{
				if( ent->IsType( idProjectile::Type ) )
				{
					// remove all projectiles
					ent->PostEventMS( &EV_Remove, 0 );
				}
			}
			for( int h = 0; h < 2; h++ )
			{
				if( weaponEnabled && hands[ h ].weapon.GetEntity() )
					hands[ h ].weapon.GetEntity()->EnterCinematic();
			}
		}
		else
		{
			physicsObj.SetLinearVelocity( vec3_origin );
			for( int h = 0; h < 2; h++ )
			{
				if( weaponEnabled && hands[ h ].weapon.GetEntity() )
					hands[ h ].weapon.GetEntity()->ExitCinematic();
			}
		}
		influenceActive = level;
	}
}

/*
=============
idPlayer::SetInfluenceView
=============
*/
void idPlayer::SetInfluenceView( const char* mtr, const char* skinname, float radius, idEntity* ent )
{
	influenceMaterial = NULL;
	influenceEntity = NULL;
	influenceSkin = NULL;
	if( mtr && *mtr )
	{
		influenceMaterial = declManager->FindMaterial( mtr );
	}
	if( skinname && *skinname )
	{
		influenceSkin = declManager->FindSkin( skinname );
		if( head.GetEntity() )
		{
			head.GetEntity()->GetRenderEntity()->shaderParms[ SHADERPARM_TIMEOFFSET ] = -MS2SEC( gameLocal.time );
		}
		UpdateVisuals();
	}
	influenceRadius = radius;
	if( radius > 0.0f )
	{
		influenceEntity = ent;
	}
}

/*
=============
idPlayer::SetInfluenceFov
=============
*/
void idPlayer::SetInfluenceFov( float fov )
{
	influenceFov = fov;
}

/*
================
idPlayer::OnLadder
================
*/
bool idPlayer::OnLadder() const
{
	return physicsObj.OnLadder();
}

/*
==================
idPlayer::Event_GetButtons
==================
*/
void idPlayer::Event_GetButtons()
{
	idThread::ReturnInt( usercmd.buttons );
}

/*
==================
idPlayer::Event_GetMove
==================
*/
void idPlayer::Event_GetMove()
{
	int upmove = ( ( usercmd.buttons & BUTTON_JUMP ) ? 127 : 0 ) - ( ( usercmd.buttons & BUTTON_CROUCH ) ? 127 : 0 );
	idVec3 move( usercmd.forwardmove, usercmd.rightmove, upmove );
	idThread::ReturnVector( move );
}

/*
================
idPlayer::Event_GetViewAngles
================
*/
void idPlayer::Event_GetViewAngles()
{
	idThread::ReturnVector( idVec3( viewAngles[0], viewAngles[1], viewAngles[2] ) );
}

/*
==================
idPlayer::Event_StopFxFov
==================
*/
void idPlayer::Event_StopFxFov()
{
	fxFov = false;
}

/*
==================
idPlayer::StartFxFov
==================
*/
void idPlayer::StartFxFov( float duration )
{
	fxFov = true;
	PostEventSec( &EV_Player_StopFxFov, duration );
}

/*
==================
idPlayer::Event_EnableWeapon
==================
*/
void idPlayer::Event_EnableWeapon()
{
	hiddenWeapon = gameLocal.world->spawnArgs.GetBool( "no_Weapons" );
	weaponEnabled = true;
	for( int h = 0; h < 2; h++ )
	{
		if( hands[ h ].weapon.GetEntity() )
			hands[ h ].weapon.GetEntity()->ExitCinematic();
	}
}

/*
==================
idPlayer::Event_DisableWeapon
==================
*/
void idPlayer::Event_DisableWeapon()
{
	hiddenWeapon = gameLocal.world->spawnArgs.GetBool( "no_Weapons" );
	weaponEnabled = false;
	for( int h = 0; h < 2; h++ )
	{
		if( hands[ h ].weapon.GetEntity() )
			hands[ h ].weapon.GetEntity()->EnterCinematic();
	}
}

/*
==================
idPlayer::Event_GiveInventoryItem
==================
*/
void idPlayer::Event_GiveInventoryItem( const char* name )
{
	GiveInventoryItem( name );
}

/*
==================
idPlayer::Event_RemoveInventoryItem
==================
*/
void idPlayer::Event_RemoveInventoryItem( const char* name )
{
	RemoveInventoryItem( name );
}

/*
==================
idPlayer::Event_GetIdealWeapon
==================
*/
void idPlayer::Event_GetIdealWeapon()
{
	const char* weapon;
	
	if( hands[vr_weaponHand.GetInteger()].idealWeapon.Get() >= 0 )
	{
		weapon = spawnArgs.GetString( va( "def_weapon%d", hands[ vr_weaponHand.GetInteger() ].idealWeapon.Get() ) );
		idThread::ReturnString( weapon );
	}
	else
	{
		idThread::ReturnString( "" );
	}
}

/*
==================
idPlayer::Event_SetPowerupTime
==================
*/
void idPlayer::Event_SetPowerupTime( int powerup, int time )
{
	if( time > 0 )
	{
		GivePowerUp( powerup, time, ITEM_GIVE_FEEDBACK | ITEM_GIVE_UPDATE_STATE );
	}
	else
	{
		ClearPowerup( powerup );
	}
}

/*
==================
idPlayer::Event_IsPowerupActive
==================
*/
void idPlayer::Event_IsPowerupActive( int powerup )
{
	idThread::ReturnInt( this->PowerUpActive( powerup ) ? 1 : 0 );
}

/*
==================
idPlayer::Event_StartWarp
==================
*/
void idPlayer::Event_StartWarp()
{
	playerView.AddWarp( idVec3( 0, 0, 0 ), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 100, 1000 );
}

/*
==================
idPlayer::Event_StopHelltime
==================
*/
void idPlayer::Event_StopHelltime( int mode )
{
	if( mode == 1 )
	{
		StopHelltime( true );
	}
	else
	{
		StopHelltime( false );
	}
}

/*
==================
idPlayer::Event_WeaponAvailable
==================
*/
void idPlayer::Event_WeaponAvailable( const char* name )
{

	idThread::ReturnInt( WeaponAvailable( name ) ? 1 : 0 );
}

bool idPlayer::WeaponAvailable( const char* name )
{
	for( int i = 0; i < MAX_WEAPONS; i++ )
	{
		if( inventory.weapons & ( 1 << i ) )
		{
			const char* weap = spawnArgs.GetString( va( "def_weapon%d", i ) );
			if( !idStr::Cmp( weap, name ) )
			{
				return true;
			}
		}
	}
	return false;
}


/*
==================
idPlayer::Event_GetCurrentWeapon
==================
*/
void idPlayer::Event_GetCurrentWeapon()
{
	const char* weapon;
	
	if( hands[vr_weaponHand.GetInteger()].currentWeapon >= 0 )
	{
		weapon = spawnArgs.GetString( va( "def_weapon%d", hands[ vr_weaponHand.GetInteger() ].currentWeapon ) );
		idThread::ReturnString( weapon );
	}
	else
	{
		idThread::ReturnString( "" );
	}
}

/*
==================
idPlayer::Event_GetPreviousWeapon
Carl: Only called by pda and bloodstone script functions.
weapon_bloodstone_passive::Fire() and item_pda::Lower() use it to call owner.selectWeapon() with the result
(these are the only times selectWeapon() is ever called in the game)
weapon_bloodstone_active* use it to check which bloodstone we were using previously and assign an anim based on that

==================
*/
void idPlayer::Event_GetPreviousWeapon()
{
	const char* weapon;
	
	// if the bloodstone is active, it should return whatever bloodstone was used before it or what weapon was used before weapon_bloodstone_passive
	// if the PDA is active
	//   this should return whatever weapon was used before the PDA

	if( hands[ vr_weaponHand.GetInteger() ].previousWeapon >= 0 )
	{
		int pw = ( gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) ) ? 0 : hands[ vr_weaponHand.GetInteger() ].previousWeapon;
		weapon = spawnArgs.GetString( va( "def_weapon%d", pw ) );
		idThread::ReturnString( weapon );
	}
	else
	{
		idThread::ReturnString( spawnArgs.GetString( "def_weapon0" ) );
	}
}

/*
==================
idPlayer::Event_SelectWeapon
==================
*/
void idPlayer::Event_SelectWeapon( const char* weaponName )
{
	if( vr_debugHands.GetBool() )
	{
		common->Printf( "Before Event_SelectWeapon(\"%s\"):\n", weaponName );
		hands[HAND_LEFT].debugPrint();
		hands[HAND_RIGHT].debugPrint();
	}
	int i;
	int weaponNum;
	
	if( common->IsClient() )
	{
		gameLocal.Warning( "Cannot switch weapons from script in multiplayer" );
		return;
	}
	
	// Carl: This function is called by item_pda::Lower() with the value returned from Event_GetPreviousWeapon,
	// in an attempt to put away the PDA and return both hands to what they were holding before.
	// That won't work when dual wielding, so check if we are in the PDA (before we change weapon) and put it away properly.
	for( int h = 0; h < 2; h++ )
	{
		if( hands[h].holdingPDA() )
		{
			hands[h].idealWeapon = hands[h].previousWeapon;
			//if ( hands[1 - h].previousWeapon >= 0 && hands[1 - h].previousWeapon != weapon_fists )
			//	UpdateHudWeapon( 1 - h );
			//else
				UpdateHudWeapon( h );
			return;
		}
	}

	// Carl: We're probably being called by weapon_bloodstone_passive::Fire() to switch back to the previous weapon
	for( int h = 0; h < 2; h++ )
	{
		if( hands[h].currentWeapon == weapon_bloodstone || hands[h].idealWeapon.Get() == weapon_bloodstone )
		{
			hands[h].idealWeapon = hands[h].previousWeapon;
			UpdateHudWeapon( h );
			return;
		}
	}

	if( hiddenWeapon && gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) )
	{
		hands[ vr_weaponHand.GetInteger() ].idealWeapon = weapon_fists;
		for( int h = 0; h < 2; h++ )
			hands[ h ].weapon.GetEntity()->HideWeapon();
		return;
	}
	
	weaponNum = -1;
	for( i = 0; i < MAX_WEAPONS; i++ )
	{
		if( inventory.weapons & ( 1 << i ) )
		{
			const char* weap = spawnArgs.GetString( va( "def_weapon%d", i ) );
			if( !idStr::Cmp( weap, weaponName ) )
			{
				weaponNum = i;
				break;
			}
		}
	}
	
	if( weaponNum < 0 )
	{
		gameLocal.Warning( "%s is not carrying weapon '%s'", name.c_str(), weaponName );
		return;
	}
	
	hiddenWeapon = false;
	// Carl: check for dual wielding
	if( hands[ 1 - vr_weaponHand.GetInteger() ].idealWeapon != weaponNum )
	{
		hands[ vr_weaponHand.GetInteger() ].idealWeapon = weaponNum;
		UpdateHudWeapon( vr_weaponHand.GetInteger() );
	}
	else
	{
		UpdateHudWeapon( 1 - vr_weaponHand.GetInteger() );
	}
	if( vr_debugHands.GetBool() )
	{
		common->Printf( "After Event_SelectWeapon(\"%s\") (not BFG or artifact):\n", weaponName );
		hands[HAND_LEFT].debugPrint();
		hands[HAND_RIGHT].debugPrint();
	}
}

/*
==================
idPlayer::Event_GetWeaponEntity
Carl: Called by script functions: player::RaiseWeapon(), player::NetCatchup(), weapon_bloodstone_passive::Raise(), weapon_bloodstone_passive::Idle()
So return the hand holding a bloodstone, if there is one, or the raised weapon hand otherwise
TODO fix this for proper Dual Wielding
==================
*/
void idPlayer::Event_GetWeaponEntity()
{
	int h = risingWeaponHand;
	if ( h < 0 )
		h = vr_weaponHand.GetInteger();
	if( hands[ 1 - h ].weapon->IdentifyWeapon() == WEAPON_ARTIFACT )
		h = 1 - h;
	idThread::ReturnEntity( hands[h].weapon.GetEntity() );
}

// Koz begin
/*
==================
idPlayer::Event_GetWeaponHand
==================
*/
void idPlayer::Event_GetWeaponHand()
{
	//returns 0 for right hand 1 for left
	if ( vr_weaponHand.GetInteger() == 1 )
	{
		
		idThread::ReturnInt( 1 );
	}
	else
	{
		idThread::ReturnInt( 0 );
	}
}

/*
==================
idPlayer::Event_GetWeaponHandState
==================
*/
void idPlayer::Event_GetWeaponHandState()
{
	
	// weapon hand
	// not for animations like firing, this is to change hand pose if no weapon is present or if using guis interactively
	// 0 = no weapon
	// 1 = normal (weapon in hand)
	// 2 = pointy gui finger 
	
	int handState = 0;
	int fingerPose = 0;
	int hand = vr_weaponHand.GetInteger();
	if ( commonVr->handInGui || commonVr->PDAforcetoggle || hands[ 1 - hand ].currentWeapon == weapon_pda ) 
	{
		handState = 2 ;
	}
	else if ( !spectating && weaponEnabled &&!hiddenWeapon && !gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) )
	{

		handState = 1 ;
	}
	
	
	if ( handState != 1 && vr_useHandPoses.GetBool() && !commonVr->PDAforcetoggle && !commonVr->PDAforced )
	{
		fingerPose = vr_weaponHand.GetInteger() == 0 ? commonVr->fingerPose[HAND_RIGHT] : commonVr->fingerPose[HAND_LEFT];
		fingerPose = fingerPose << 4;

		handState += fingerPose;
		//common->Printf( "Weapon hand finger pose = %d, %d\n", handState, fingerPose );
	}
	
	idThread::ReturnInt( handState );
}

/*
==================
idPlayer::Event_GetFlashHand // get flashlight hand
==================
*/
void idPlayer::Event_GetFlashHand() // get flashlight hand
{
	static int flashlightHand = 1;
	//returns 0 for right, 1 for left
		
	flashlightHand = vr_weaponHand.GetInteger() == 0 ? 1 : 0;
	
	
	idThread::ReturnInt( flashlightHand );

}

/*
==================
idPlayer::Event_GetFlashHandState // get flashlight hand state
==================
*/
void idPlayer::Event_GetFlashHandState() // get flashlight hand state
{
	
	// this is for the flashlight hand
	// this is not for weapon hand animations like firing, this is to change hand pose if no weapon is present or if using guis interactively
	// 0 = hand empty no weapon
	// 1 = fist (no flashlight)
	// 2 = flashlight
	// 3 = normal weapon idle hand anim - used for holding PDA.
			
	
	int flashlightHand = 1;
	int hand = 1 - vr_weaponHand.GetInteger();
	
	if ( hands[hand].weapon.GetEntity() && hands[hand].weapon->IdentifyWeapon() == WEAPON_PDA )
	{
		flashlightHand = 3;
	}
	else if ( spectating || !weaponEnabled || hiddenWeapon || gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) )
	{
		flashlightHand = 0;
	}
	else if( hands[hand].currentWeapon == weapon_fists && !hands[hand].holdingFlashlight() )
	{
		flashlightHand = 1;
	}
		 
	//else if ( commonVr->currentFlashlightPosition == FLASHLIGHT_HAND && !spectating && weaponEnabled &&!hiddenWeapon && !gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) )
	else if ( commonVr->currentFlashlightPosition == FLASHLIGHT_HAND ) //&& !spectating && weaponEnabled &&!hiddenWeapon && !gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) )
	{
		flashlightHand = 2;
	}
	else if( hands[hand].floatingWeapon() )
	{
		flashlightHand = 0;
	}
	else if( hands[ hand ].holdingWeapon() || hands[ hand ].floatingWeapon() )
	{
		flashlightHand = 3;
	}
	
	if ( flashlightHand <= 1 && vr_useHandPoses.GetBool() )
	{
		int fingerPose;
		fingerPose = vr_weaponHand.GetInteger() != 0 ? commonVr->fingerPose[HAND_RIGHT] : commonVr->fingerPose[HAND_LEFT];
		fingerPose = fingerPose << 4;
		//common->Printf( "Flashlight hand finger pose = %d , %d\n ", flashlightHand, fingerPose );
		flashlightHand += fingerPose;
	}
	idThread::ReturnInt( flashlightHand );

}

/*
==================
idPlayer::Event_GetFlashState // get flashlight state
==================
*/
void idPlayer::Event_GetFlashState() // get flashlight state
{
	static int flashlighton;
	flashlighton = flashlight.GetEntity()->lightOn  ? 1 : 0 ;
	// Koz debug common->Printf( "Returning flashlight state = %d\n",flashlighton );
	idThread::ReturnInt( flashlighton );
}


// Koz end

/*
==================
idPlayer::Event_OpenPDA
==================
*/
void idPlayer::Event_OpenPDA()
{
	if( vr_debugHands.GetBool() )
		common->Printf( "Event_OpenPDA():\n" );
	if( !common->IsMultiplayer() )
	{
		// Koz debug common->Printf( "idPlayer::Event_OpenPDA() calling TogglePDA\n" );
		TogglePDA( 1 - vr_weaponHand.GetInteger() );
	}
}

/*
==================
idPlayer::Event_InPDA
==================
*/
void idPlayer::Event_InPDA()
{
	idThread::ReturnInt( objectiveSystemOpen );
}

/*
==================
idPlayer::TeleportDeath
==================
*/
void idPlayer::TeleportDeath( int killer )
{
	teleportKiller = killer;
}

/*
==================
idPlayer::Event_ExitTeleporter
==================
*/
void idPlayer::Event_ForceOrigin( idVec3& origin, idAngles& angles )
{
	SetOrigin( origin + idVec3( 0, 0, CM_CLIP_EPSILON ) );
	//SetViewAngles( angles );
	
	UpdateVisuals();
}

/*
==================
idPlayer::Event_ExitTeleporter
==================
*/
void idPlayer::Event_ExitTeleporter()
{
	idEntity*	exitEnt;
	float		pushVel;
	
	// verify and setup
	exitEnt = teleportEntity.GetEntity();
	if( !exitEnt )
	{
		common->DPrintf( "Event_ExitTeleporter player %d while not being teleported\n", entityNumber );
		return;
	}
	
	pushVel = exitEnt->spawnArgs.GetFloat( "push", "300" );
	
	if( common->IsServer() )
	{
		ServerSendEvent( EVENT_EXIT_TELEPORTER, NULL, false );
	}
	
	SetPrivateCameraView( NULL );
	// setup origin and push according to the exit target
	SetOrigin( exitEnt->GetPhysics()->GetOrigin() + idVec3( 0, 0, CM_CLIP_EPSILON ) );
	SetViewAngles( exitEnt->GetPhysics()->GetAxis().ToAngles() );
	physicsObj.SetLinearVelocity( exitEnt->GetPhysics()->GetAxis()[ 0 ] * pushVel );
	physicsObj.ClearPushedVelocity();
	// teleport fx
	playerView.Flash( colorWhite, 120 );
	
	// clear the ik heights so model doesn't appear in the wrong place
	walkIK.EnableAll();
	
	UpdateVisuals();
	
	StartSound( "snd_teleport_exit", SND_CHANNEL_ANY, 0, false, NULL );
	
	if( teleportKiller != -1 )
	{
		// we got killed while being teleported
		Damage( gameLocal.entities[ teleportKiller ], gameLocal.entities[ teleportKiller ], vec3_origin, "damage_telefrag", 1.0f, INVALID_JOINT );
		teleportKiller = -1;
	}
	else
	{
		// kill anything that would have waited at teleport exit
		gameLocal.KillBox( this );
	}
	teleportEntity = NULL;
}

/*
================
idPlayer::ClientThink
================
*/
void idPlayer::ClientThink( const int curTime, const float fraction, const bool predict )
{
	if( IsLocallyControlled() )
	{
		aimAssist.Update();
	}
	
	UpdateSkinSetup();
		
	if( !IsLocallyControlled() )
	{
		// ignore attack button of other clients. that's no good for predictions
		usercmd.buttons &= ~BUTTON_ATTACK;
	}
	
	buttonMask &= usercmd.buttons;
	usercmd.buttons &= ~buttonMask;
	
	buttonMask &= usercmd.buttons;
	usercmd.buttons &= ~buttonMask;
	
	if( mountedObject )
	{
		usercmd.forwardmove = 0;
		usercmd.rightmove = 0;
		usercmd.buttons &= ~( BUTTON_JUMP | BUTTON_CROUCH );
	}
	
	if( objectiveSystemOpen )
	{
		usercmd.forwardmove = 0;
		usercmd.rightmove = 0;
		usercmd.buttons &= ~( BUTTON_JUMP | BUTTON_CROUCH );
	}
	
	
	// Koz, no zoom in VR. 
	/*
	if( IsLocallyControlled() )
	{
		// zooming
		if( ( usercmd.buttons ^ oldCmd.buttons ) & BUTTON_ZOOM )
		{
			if( ( usercmd.buttons & BUTTON_ZOOM ) && weapon.GetEntity() )
			{
				zoomFov.Init( gameLocal.time, 200.0f, CalcFov( false ), weapon.GetEntity()->GetZoomFov() );
			}
			else
			{
				zoomFov.Init( gameLocal.time, 200.0f, zoomFov.GetCurrentValue( gameLocal.time ), DefaultFov() );
			}
		}
	}
	*/

	// clear the ik before we do anything else so the skeleton doesn't get updated twice
	walkIK.ClearJointMods();
	
	// Koz
	armIK.ClearJointMods();
	
	if( gameLocal.isNewFrame )
	{
		if( usercmd.impulseSequence != oldImpulseSequence )
		{
			PerformImpulse( usercmd.impulse );
		}
	}
	
	if( forceScoreBoard )
	{
		gameLocal.mpGame.SetScoreboardActive( true );
	}
	
	AdjustSpeed();
	
	if( IsLocallyControlled() )
	{
		UpdateViewAngles();
	}
	else
	{
		idQuat interpolatedAngles = Slerp( previousViewQuat, nextViewQuat, fraction );
		viewAngles = interpolatedAngles.ToAngles();
	}
	
	smoothedOriginUpdated = false;
	
	if( !af.IsActive() )
	{
		AdjustBodyAngles();
	}
	
	if( !isLagged )
	{
		// don't allow client to move when lagged
		if( IsLocallyControlled() )
		{
			// Locally-controlled clients are authoritative on their positions, so they can move normally.
			Move();
			usercmd.pos = physicsObj.GetOrigin();
		}
		else
		{
			// Non-locally controlled players are interpolated.
			Move_Interpolated( fraction );
		}
	}
	
	if ( !g_stopTime.GetBool() && !commonVr->VR_GAME_PAUSED ) // Koz 

	{
		if( !noclip && !spectating && ( health > 0 ) && !IsHidden() )
		{
			TouchTriggers();
		}
	}
	
	// update GUIs, Items, and character interactions
	UpdateFocus();
	
	// service animations
	if( !spectating && !af.IsActive() )
	{
		UpdateConditions();
		UpdateAnimState();
		CheckBlink();
	}
	
	// clear out our pain flag so we can tell if we recieve any damage between now and the next time we think
	AI_PAIN = false;
	
	UpdateLocation();
	
	// calculate the exact bobbed view position, which is used to
	// position the view weapon, among other things
	CalculateFirstPersonView();
	
	// this may use firstPersonView, or a thirdPerson / camera view
	CalculateRenderView();
	
	if( !gameLocal.inCinematic && !Flicksync_InCutscene && hands[0].weapon.GetEntity() && hands[1].weapon.GetEntity() && ( health > 0 ) && !( common->IsMultiplayer() && spectating ) )
	{
		UpdateWeapon();
	}
	
	if ( IsLocallyControlled() )
	{
		UpdateNeckPose();
	}

	UpdateFlashlight();
	
	UpdateHud();
	
	if( gameLocal.isNewFrame )
	{
		UpdatePowerUps();
	}
	
	UpdateDeathSkin( false );
	
	renderEntity_t* headRenderEnt = NULL;
	if( head.GetEntity() )
	{
		headRenderEnt = head.GetEntity()->GetRenderEntity();
	}
	
	if( headRenderEnt )
	{
		if( influenceSkin )
		{
			headRenderEnt->customSkin = influenceSkin;
		}
		else
		{
			headRenderEnt->customSkin = NULL;
		}
	}
	
	if( common->IsMultiplayer() || g_showPlayerShadow.GetBool() )
	{
		renderEntity.suppressShadowInViewID	= 0;
		if( headRenderEnt )
		{
			headRenderEnt->suppressShadowInViewID = 0;
		}
	}
	else
	{
		renderEntity.suppressShadowInViewID	= entityNumber + 1;
		if( headRenderEnt )
		{
			// Koz begin
			if ( game->isVR )
			{
				headRenderEnt->suppressShadowInViewID = 0; //Carl:Draw the head's shadow when showing the body
			}
			else
			{
				headRenderEnt->suppressShadowInViewID = entityNumber + 1;
			}
			// Koz end
		}
	}
	// never cast shadows from our first-person muzzle flashes
	renderEntity.suppressShadowInLightID = LIGHTID_VIEW_MUZZLE_FLASH + entityNumber;
	if( headRenderEnt )
	{
		headRenderEnt->suppressShadowInLightID = LIGHTID_VIEW_MUZZLE_FLASH + entityNumber;
	}
	
	if( !gameLocal.inCinematic )
	{
		if ( armIK.IsInitialized() ) armIK.Evaluate();
		UpdateAnimation();
	}
	
	if( enviroSuitLight.IsValid() )
	{
		idAngles lightAng = firstPersonViewAxis.ToAngles();
		idVec3 lightOrg = firstPersonViewOrigin;
		const idDict* lightDef = gameLocal.FindEntityDefDict( "envirosuit_light", false );
		
		idVec3 enviroOffset = lightDef->GetVector( "enviro_offset" );
		idVec3 enviroAngleOffset = lightDef->GetVector( "enviro_angle_offset" );
		
		lightOrg += ( enviroOffset.x * firstPersonViewAxis[0] );
		lightOrg += ( enviroOffset.y * firstPersonViewAxis[1] );
		lightOrg += ( enviroOffset.z * firstPersonViewAxis[2] );
		lightAng.pitch += enviroAngleOffset.x;
		lightAng.yaw += enviroAngleOffset.y;
		lightAng.roll += enviroAngleOffset.z;
		
		enviroSuitLight.GetEntity()->GetPhysics()->SetOrigin( lightOrg );
		enviroSuitLight.GetEntity()->GetPhysics()->SetAxis( lightAng.ToMat3() );
		enviroSuitLight.GetEntity()->UpdateVisuals();
		enviroSuitLight.GetEntity()->Present();
	}
	
	if( common->IsMultiplayer() )
	{
		DrawPlayerIcons();
	}
	
	Present();
	
	UpdateDamageEffects();
	
	LinkCombat();
	
	// stereo rendering laser sight that replaces the crosshair
	for( int h = 0; h < 2; h++ )
	{
		UpdateLaserSight( h );
	}
	UpdateTeleportAim();

	if ( game->isVR ) UpdateHeadingBeam(); // Koz
		
	if( gameLocal.isNewFrame && IsLocallyControlled() )
	{
		playerView.CalculateShake();
	}
	
	// determine if portal sky is in pvs
	pvsHandle_t	clientPVS = gameLocal.pvs.SetupCurrentPVS( GetPVSAreas(), GetNumPVSAreas() );
	gameLocal.portalSkyActive = gameLocal.pvs.CheckAreasForPortalSky( clientPVS, GetPhysics()->GetOrigin() );
	gameLocal.pvs.FreeCurrentPVS( clientPVS );
	
	//InterpolatePhysics( fraction );
	
	// Make sure voice groups are set to the right team
	if( common->IsMultiplayer() && session->GetState() >= idSession::INGAME && entityNumber < MAX_CLIENTS )  		// The entityNumber < MAX_CLIENTS seems to quiet the static analyzer
	{
		// Make sure we're on the right team (at the lobby level)
		const int voiceTeam = spectating ? LOBBY_SPECTATE_TEAM_FOR_VOICE_CHAT : team;
		
		//idLib::Printf( "CLIENT: Sending voice %i / %i\n", entityNumber, voiceTeam );
		
		// Update lobby team
		session->GetActingGameStateLobbyBase().SetLobbyUserTeam( gameLocal.lobbyUserIDs[ entityNumber ], voiceTeam );
		
		// Update voice groups to match in case something changed
		session->SetVoiceGroupsToTeams();
	}
}

/*
================
idPlayer::GetPhysicsToVisualTransform
================
*/
bool idPlayer::GetPhysicsToVisualTransform( idVec3& origin, idMat3& axis )
{
	if( af.IsActive() )
	{
		af.GetPhysicsToVisualTransform( origin, axis );
		return true;
	}
	
	// smoothen the rendered origin and angles of other clients
	// smooth self origin if snapshots are telling us prediction is off
	if( common->IsClient() && gameLocal.framenum >= smoothedFrame && ( !IsLocallyControlled() || selfSmooth ) )
	{
		// render origin and axis
		idMat3 renderAxis = viewAxis * GetPhysics()->GetAxis();
		idVec3 renderOrigin = GetPhysics()->GetOrigin() + modelOffset * renderAxis;
		
		// update the smoothed origin
		if( !smoothedOriginUpdated )
		{
			idVec2 originDiff = renderOrigin.ToVec2() - smoothedOrigin.ToVec2();
			if( originDiff.LengthSqr() < Square( 100.0f ) )
			{
				// smoothen by pushing back to the previous position
				if( selfSmooth )
				{
					assert( IsLocallyControlled() );
					renderOrigin.ToVec2() -= net_clientSelfSmoothing.GetFloat() * originDiff;
				}
				else
				{
					renderOrigin.ToVec2() -= gameLocal.clientSmoothing * originDiff;
				}
			}
			smoothedOrigin = renderOrigin;
			
			smoothedFrame = gameLocal.framenum;
			smoothedOriginUpdated = true;
		}
		
		axis = idAngles( 0.0f, viewAngles.yaw, 0.0f ).ToMat3();
		origin = ( smoothedOrigin - GetPhysics()->GetOrigin() ) * axis.Transpose();
		
	}
	else
	{
	
		axis = viewAxis;
		origin = modelOffset;
	}
	return true;
}

/*
================
idPlayer::GetPhysicsToSoundTransform
================
*/
bool idPlayer::GetPhysicsToSoundTransform( idVec3& origin, idMat3& axis )
{
	idCamera* camera;
	
	if( privateCameraView )
	{
		camera = privateCameraView;
	}
	else
	{
		camera = gameLocal.GetCamera();
	}
	
	if( camera )
	{
		renderView_t view;
		
		memset( &view, 0, sizeof( view ) );
		camera->GetViewParms( &view );
		origin = view.vieworg;
		axis = view.viewaxis;
		return true;
	}
	else
	{
		return idActor::GetPhysicsToSoundTransform( origin, axis );
	}
}

/*
================
idPlayer::HandleUserCmds
================
*/
void idPlayer::HandleUserCmds( const usercmd_t& newcmd )
{
	// latch button actions
	oldButtons = usercmd.buttons;
	for( int h = 0; h < 2; h++ )
	{
		hands[ h ].oldGrabbingWorld = hands[ h ].grabbingWorld;
	}
	
	// grab out usercmd
	oldCmd = usercmd;
	oldImpulseSequence = usercmd.impulseSequence;
	usercmd = newcmd;
}

/*
================
idPlayer::WriteToSnapshot
================
*/
void idPlayer::WriteToSnapshot( idBitMsg& msg ) const
{
	physicsObj.WriteToSnapshot( msg );
	WriteBindToSnapshot( msg );
	// Only remote players will use these actual viewangles.
	idCQuat snapViewCQuat( viewAngles.ToQuat().ToCQuat() );
	msg.WriteFloat( snapViewCQuat.x );
	msg.WriteFloat( snapViewCQuat.y );
	msg.WriteFloat( snapViewCQuat.z );
	msg.WriteDeltaFloat( 0.0f, deltaViewAngles[0] );
	msg.WriteDeltaFloat( 0.0f, deltaViewAngles[1] );
	msg.WriteDeltaFloat( 0.0f, deltaViewAngles[2] );
	msg.WriteShort( health );
	msg.WriteBits( gameLocal.ServerRemapDecl( -1, DECL_ENTITYDEF, lastDamageDef ), gameLocal.entityDefBits );
	msg.WriteDir( lastDamageDir, 9 );
	msg.WriteShort( lastDamageLocation );
	msg.WriteBits( hands[vr_weaponHand.GetInteger()].idealWeapon.Get(), idMath::BitsForInteger( MAX_WEAPONS ) );
	msg.WriteBits( inventory.weapons, MAX_WEAPONS );
	msg.WriteBits( hands[vr_weaponHand.GetInteger()].weapon.GetSpawnId(), 32 ); // Carl dont change msg format
	msg.WriteBits( flashlight.GetSpawnId(), 32 );
	msg.WriteBits( spectator, idMath::BitsForInteger( MAX_CLIENTS ) );
	msg.WriteBits( lastHitToggle, 1 );
	msg.WriteBits( hands[ vr_weaponHand.GetInteger() ].weaponGone, 1 );
	msg.WriteBits( isLagged, 1 );
	msg.WriteShort( team );
	WriteToBitMsg( respawn_netEvent, msg );
	
	/* Needed for the scoreboard */
	msg.WriteBits( carryingFlag, 1 );
	msg.WriteBits( enviroSuitLight.GetSpawnId(), 32 );
	
	msg.WriteBits( AI_CROUCH, 1 );
	msg.WriteBits( AI_ONGROUND, 1 );
	msg.WriteBits( AI_ONLADDER, 1 );
	msg.WriteBits( AI_JUMP, 1 );
	msg.WriteBits( AI_WEAPON_FIRED, 1 );
	msg.WriteBits( AI_ATTACK_HELD, 1 );
	
	msg.WriteByte( usercmd.buttons );
	msg.WriteBits( usercmd.forwardmove, -8 );
	msg.WriteBits( usercmd.rightmove, -8 );
	
	msg.WriteBool( spectating );
}

/*
================
idPlayer::ReadFromSnapshot
================
*/
void idPlayer::ReadFromSnapshot( const idBitMsg& msg )
{
	int		oldHealth, newIdealWeapon, weaponSpawnId;
	int		flashlightSpawnId;
	bool	newHitToggle;
	
	oldHealth = health;
	
	physicsObj.ReadFromSnapshot( msg );
	ReadBindFromSnapshot( msg );
	
	// The remote players get updated view angles from the snapshot.
	idCQuat snapViewCQuat;
	snapViewCQuat.x = msg.ReadFloat();
	snapViewCQuat.y = msg.ReadFloat();
	snapViewCQuat.z = msg.ReadFloat();
	
	idAngles tempDeltaViewAngles;
	tempDeltaViewAngles[0] = msg.ReadDeltaFloat( 0.0f );
	tempDeltaViewAngles[1] = msg.ReadDeltaFloat( 0.0f );
	tempDeltaViewAngles[2] = msg.ReadDeltaFloat( 0.0f );
	
	deltaViewAngles = tempDeltaViewAngles;
	
	health = msg.ReadShort();
	lastDamageDef = gameLocal.ClientRemapDecl( DECL_ENTITYDEF, msg.ReadBits( gameLocal.entityDefBits ) );
	lastDamageDir = msg.ReadDir( 9 );
	lastDamageLocation = msg.ReadShort();
	newIdealWeapon = msg.ReadBits( idMath::BitsForInteger( MAX_WEAPONS ) );
	inventory.weapons = msg.ReadBits( MAX_WEAPONS );
	weaponSpawnId = msg.ReadBits( 32 );
	flashlightSpawnId = msg.ReadBits( 32 );
	spectator = msg.ReadBits( idMath::BitsForInteger( MAX_CLIENTS ) );
	newHitToggle = msg.ReadBits( 1 ) != 0;
	weaponGone = msg.ReadBits( 1 ) != 0;
	hands[ 0 ].weaponGone = weaponGone;
	hands[ 1 ].weaponGone = weaponGone;
	isLagged = msg.ReadBits( 1 ) != 0;
	team = msg.ReadShort();
	ReadFromBitMsg( respawn_netEvent, msg );
	
	carryingFlag = msg.ReadBits( 1 ) != 0;
	int enviroSpawnId;
	enviroSpawnId = msg.ReadBits( 32 );
	enviroSuitLight.SetSpawnId( enviroSpawnId );
	
	bool snapshotCrouch = msg.ReadBool();
	bool snapshotOnGround = msg.ReadBool();
	bool snapshotOnLadder = msg.ReadBool();
	bool snapshotJump = msg.ReadBool();
	bool snapShotFired = msg.ReadBool();
	bool snapShotAttackHeld = msg.ReadBool();
	
	byte snapshotButtons = msg.ReadByte();
	signed char snapshotForward = msg.ReadBits( -8 );
	signed char snapshotRight = msg.ReadBits( -8 );
	
	const bool snapshotSpectating = msg.ReadBool();
	
	// no msg reading below this
	
	// Update remote remote player state.
	if( !IsLocallyControlled() )
	{
		previousViewQuat = nextViewQuat;
		nextViewQuat = snapViewCQuat.ToQuat();
		
		AI_CROUCH = snapshotCrouch;
		AI_ONGROUND = snapshotOnGround;
		AI_ONLADDER = snapshotOnLadder;
		AI_JUMP = snapshotJump;
		AI_WEAPON_FIRED = snapShotFired;
		AI_ATTACK_HELD = snapShotAttackHeld;
		
		oldCmd = usercmd;
		
		usercmd.buttons = snapshotButtons;
		usercmd.forwardmove = snapshotForward;
		usercmd.rightmove = snapshotRight;
	}
	
	if( hands[vr_weaponHand.GetInteger()].weapon.SetSpawnId( weaponSpawnId ) )
	{
		if( hands[ vr_weaponHand.GetInteger() ].weapon.GetEntity() )
		{
			// maintain ownership locally
			hands[ vr_weaponHand.GetInteger() ].weapon.GetEntity()->SetOwner( this, vr_weaponHand.GetInteger() );
		}
		hands[ vr_weaponHand.GetInteger() ].currentWeapon = -1;
	}
	
	if( flashlight.SetSpawnId( flashlightSpawnId ) )
	{
		if( flashlight.GetEntity() )
		{
			flashlight.GetEntity()->SetFlashlightOwner( this );
		}
	}
	
	/*
	// if not a local client
	if ( !IsLocallyControlled() ) {
		// assume the client has all ammo types
		inventory.SetRemoteClientAmmo( GetEntityNumber() );
	}
	*/
	
	// Update spectating state
	const bool wasSpectating = spectating;
	spectating = snapshotSpectating;
	
	if( spectating != wasSpectating )
	{
		Spectate( spectating, false );
	}
	
	if( oldHealth > 0 && health <= 0 )
	{
		if( snapshotStale )
		{
			// so we just hide and don't show a death skin
			UpdateDeathSkin( true );
		}
		// die
		AI_DEAD = true;
		ClearPowerUps();
		SetAnimState( ANIMCHANNEL_LEGS, "Legs_Death", 4 );
		SetAnimState( ANIMCHANNEL_TORSO, "Torso_Death", 4 );
		SetWaitState( "" );
		animator.ClearAllJoints();
		StartRagdoll();
		physicsObj.SetMovementType( PM_DEAD );
		if( !snapshotStale )
		{
			StartSound( "snd_death", SND_CHANNEL_VOICE, 0, false, NULL );
		}
		for( int h = 0; h < 2; h++ )
		{
			if( hands[ h ].weapon.GetEntity() )
				hands[ h ].weapon.GetEntity()->OwnerDied();
		}
		if( flashlight.GetEntity() )
		{
			FlashlightOff();
			flashlight.GetEntity()->OwnerDied();
		}
		
		if( IsLocallyControlled() )
		{
			ControllerShakeFromDamage( oldHealth - health );
		}
		
	}
	else if( health < oldHealth && health > 0 )
	{
		if( snapshotStale )
		{
			lastDmgTime = gameLocal.time;
		}
		else
		{
			// damage feedback
			const idDeclEntityDef* def = static_cast<const idDeclEntityDef*>( declManager->DeclByIndex( DECL_ENTITYDEF, lastDamageDef, false ) );
			if( def )
			{
				if( IsLocallyControlled() )
				{
					playerView.DamageImpulse( lastDamageDir * viewAxis.Transpose(), &def->dict );
					AI_PAIN = Pain( NULL, NULL, oldHealth - health, lastDamageDir, lastDamageLocation );
				}
				lastDmgTime = gameLocal.time;
			}
			else
			{
				common->Warning( "NET: no damage def for damage feedback '%d'\n", lastDamageDef );
			}
			
			if( IsLocallyControlled() )
			{
				ControllerShakeFromDamage( oldHealth - health );
			}
			
		}
	}
	else if( health > oldHealth && PowerUpActive( MEGAHEALTH ) && !snapshotStale )
	{
		// just pulse, for any health raise
		healthPulse = true;
	}
	
	// handle respawns
	if( respawn_netEvent.Get() )
	{
		Init();
		StopRagdoll();
		SetPhysics( &physicsObj );
		// Explicitly set the current origin, since locally-controlled clients
		// don't interpolate. Reading the physics object from the snapshot only
		// updates the next state, not the current state.
		physicsObj.SnapToNextState();
		physicsObj.EnableClip();
		SetCombatContents( true );
		if( flashlight.GetEntity() )
		{
			flashlight.GetEntity()->Show();
		}
		Respawn_Shared();
	}
	
	// If the player is alive, restore proper physics object
	if( health > 0 && IsActiveAF() )
	{
		StopRagdoll();
		SetPhysics( &physicsObj );
		physicsObj.EnableClip();
		SetCombatContents( true );
	}
	
	const int oldIdealWeapon = hands[ vr_weaponHand.GetInteger() ].idealWeapon.Get();
	hands[ vr_weaponHand.GetInteger() ].idealWeapon.UpdateFromSnapshot( newIdealWeapon, GetEntityNumber() );
	
	if( oldIdealWeapon != hands[ vr_weaponHand.GetInteger() ].idealWeapon.Get() )
	{
		if( snapshotStale )
		{
			weaponCatchup = true;
		}
		UpdateHudWeapon( vr_weaponHand.GetInteger() );
	}
	
	if( lastHitToggle != newHitToggle )
	{
		SetLastHitTime( gameLocal.realClientTime );
	}
	
	if( msg.HasChanged() )
	{
		UpdateVisuals();
	}
}

/*
================
idPlayer::WritePlayerStateToSnapshot
================
*/
void idPlayer::WritePlayerStateToSnapshot( idBitMsg& msg ) const
{
	msg.WriteByte( bobCycle );
	msg.WriteLong( stepUpTime );
	msg.WriteFloat( stepUpDelta );
	msg.WriteLong( inventory.weapons );
	msg.WriteByte( inventory.armor );
	
	inventory.WriteAmmoToSnapshot( msg );
}

/*
================
idPlayer::ReadPlayerStateFromSnapshot
================
*/
void idPlayer::ReadPlayerStateFromSnapshot( const idBitMsg& msg )
{
	int newBobCycle = 0;
	int newStepUpTime = 0;
	int newStepUpDelta = 0;
	
	newBobCycle = msg.ReadByte();
	newStepUpTime = msg.ReadLong();
	newStepUpDelta = msg.ReadFloat();
	
	inventory.weapons = msg.ReadLong();
	inventory.armor = msg.ReadByte();
	
	inventory.ReadAmmoFromSnapshot( msg, GetEntityNumber() );
}

/*
================
idPlayer::ServerReceiveEvent
================
*/
bool idPlayer::ServerReceiveEvent( int event, int time, const idBitMsg& msg )
{

	if( idEntity::ServerReceiveEvent( event, time, msg ) )
	{
		return true;
	}
	
	return false;
}

/*
================
idPlayer::ClientReceiveEvent
================
*/
bool idPlayer::ClientReceiveEvent( int event, int time, const idBitMsg& msg )
{
	switch( event )
	{
		case EVENT_EXIT_TELEPORTER:
			Event_ExitTeleporter();
			return true;
		case EVENT_ABORT_TELEPORTER:
			SetPrivateCameraView( NULL );
			return true;
		case EVENT_POWERUP:
		{
			int powerup = msg.ReadShort();
			int powertime = msg.ReadShort();
			if( powertime > 0 )
			{
				GivePowerUp( powerup, powertime, ITEM_GIVE_UPDATE_STATE );
			}
			else
			{
				ClearPowerup( powerup );
			}
			return true;
		}
		case EVENT_PICKUPNAME:
		{
			char buf[MAX_EVENT_PARAM_SIZE];
			msg.ReadString( buf, MAX_EVENT_PARAM_SIZE );
			inventory.AddPickupName( buf, this ); //_D3XP
			return true;
		}
		case EVENT_SPECTATE:
		{
			bool spectate = ( msg.ReadBits( 1 ) != 0 );
			Spectate( spectate, true );
			return true;
		}
		case EVENT_ADD_DAMAGE_EFFECT:
		{
			if( spectating )
			{
				// if we're spectating, ignore
				// happens if the event and the spectate change are written on the server during the same frame (fraglimit)
				return true;
			}
			return idActor::ClientReceiveEvent( event, time, msg );
		}
		case EVENT_FORCE_ORIGIN:
		{
		
			idVec3 forceOrigin =  ReadFloatArray< idVec3 >( msg );
			idAngles forceAngles;
			forceAngles[0] = msg.ReadFloat();
			forceAngles[1] = msg.ReadFloat();
			forceAngles[2] = msg.ReadFloat();
			
			Event_ForceOrigin( forceOrigin, forceAngles );
			return true;
		}
		case EVENT_KNOCKBACK:
		{
			idVec3 linearVelocity =  ReadFloatArray< idVec3 >( msg );
			int knockbacktime = msg.ReadByte();
			physicsObj.SetLinearVelocity( linearVelocity );
			physicsObj.SetKnockBack( knockbacktime );
			return true;
		}
		default:
		{
			return idActor::ClientReceiveEvent( event, time, msg );
		}
	}
//	return false;
}

/*
================
idPlayer::Hide
================
*/
void idPlayer::Hide()
{
	idWeapon* weap;
	
	idActor::Hide();
	for( int h = 0; h < 2; h++ )
	{
		weap = hands[ h ].weapon.GetEntity();
		if( weap )
			weap->HideWorldModel();
	}
	idWeapon* flashlight_weapon = flashlight.GetEntity();
	if( flashlight_weapon )
	{
		flashlight_weapon->HideWorldModel();
	}
}

/*
================
idPlayer::Show
================
*/
void idPlayer::Show()
{
	idWeapon* weap;
	
	idActor::Show();
	for( int h = 0; h < 2; h++ )
	{
		weap = hands[ h ].weapon.GetEntity();
		if( weap )
			weap->ShowWorldModel();
	}
	idWeapon* flashlight_weapon = flashlight.GetEntity();
	if( flashlight_weapon )
	{
		flashlight_weapon->ShowWorldModel();
	}
}

/*
===============
idPlayer::IsSoundChannelPlaying
===============
*/
bool idPlayer::IsSoundChannelPlaying( const s_channelType channel )
{
	if( GetSoundEmitter() != NULL )
	{
		return GetSoundEmitter()->CurrentlyPlaying( channel );
	}
	
	return false;
}

/*
===============
idPlayer::ShowTip
===============
*/
void idPlayer::ShowTip( const char* title, const char* tip, bool autoHide )
{
	if( tipUp )
	{
		return;
	}
	
	if( hudManager )
	{
		hudManager->ShowTip( title, tip, autoHide );
	}
	tipUp = true;
}

/*
===============
idPlayer::HideTip
===============
*/
void idPlayer::HideTip()
{
	if( hudManager )
	{
		hudManager->HideTip();
	}
	tipUp = false;
}

/*
===============
idPlayer::Event_HideTip
===============
*/
void idPlayer::Event_HideTip()
{
	HideTip();
}

/*
===============
idPlayer::HideObjective
===============
*/
void idPlayer::HideObjective()
{

	StartSound( "snd_objectivedown", SND_CHANNEL_ANY, 0, false, NULL );
	
	if( hud )
	{
		if( objectiveUp )
		{
			hud->HideObjective( false );
			objectiveUp = false;
		}
		else
		{
			hud->HideObjective( true );
		}
	}
}

/*
===============
idPlayer::Event_StopAudioLog
===============
*/
void idPlayer::Event_StopAudioLog()
{
	//EndAudioLog();
}

/*
===============
idPlayer::SetSpectateOrigin
===============
*/
void idPlayer::SetSpectateOrigin()
{
	idVec3 neworig;
	
	neworig = GetPhysics()->GetOrigin();
	neworig[ 2 ] += pm_normalviewheight.GetFloat();
	neworig[ 2 ] += SPECTATE_RAISE;
	SetOrigin( neworig );
}

/*
===============
idPlayer::RemoveWeapon
===============
*/
void idPlayer::RemoveWeapon( const char* weap )
{
	if( weap && *weap )
	{
		inventory.Drop( spawnArgs, spawnArgs.GetString( weap ), -1 );
	}
}

/*
===============
idPlayer::RemoveAllButEssentialWeapons
===============
*/
void idPlayer::RemoveAllButEssentialWeapons()
{
	const idKeyValue* kv = spawnArgs.MatchPrefix( "def_weapon", NULL );
	for( ; kv != NULL; kv = spawnArgs.MatchPrefix( "def_weapon", kv ) )
	{
		// This list probably ought to be placed int the player's def
		if( kv->GetValue() == "weapon_fists" || kv->GetValue() == "weapon_soulcube" || kv->GetValue() == "weapon_pda"
				|| kv->GetValue() == "weapon_flashlight" || kv->GetValue() == "weapon_flashlight_new" )
		{
			continue;
		}
		inventory.Drop( spawnArgs, kv->GetValue(), -1 );
	}
}

/*
===============
idPlayer::CanShowWeaponViewmodel
===============
*/
bool idPlayer::CanShowWeaponViewmodel() const
{
	return ui_showGun.GetBool();
}

bool idPlayer::CanDualWield( int num ) const
{
	vr_dualwield_t d = (vr_dualwield_t)vr_dualWield.GetInteger();
	if( d == VR_DUALWIELD_YES || num == -1 )
		return true;
	if( d == VR_DUALWIELD_NOT_EVEN_FISTS )
		return false;
	if( num == weapon_fists )
		return d != VR_DUALWIELD_NOT_EVEN_FISTS;
	if( num == weapon_flashlight || num == weapon_flashlight_new )
		return d == VR_DUALWIELD_ONLY_FLASHLIGHT || d == VR_DUALWIELD_ONLY_GRENADES_FLASHLIGHT || d == VR_DUALWIELD_ONLY_PISTOLS_FLASHLIGHT || d == VR_DUALWIELD_ONLY_PISTOLS_GRENADES_FLASHLIGHT;
	if( num == weapon_pistol )
		return d == VR_DUALWIELD_ONLY_PISTOLS || d == VR_DUALWIELD_ONLY_PISTOLS_FLASHLIGHT || d == VR_DUALWIELD_ONLY_PISTOLS_GRENADES_FLASHLIGHT;
	if( num == weapon_handgrenade )
		return d == VR_DUALWIELD_ONLY_GRENADES || d == VR_DUALWIELD_ONLY_GRENADES_FLASHLIGHT || d == VR_DUALWIELD_ONLY_PISTOLS_GRENADES_FLASHLIGHT;
	return false;
}

/*
===============
idPlayer::GetWeaponInHand
Carl: Dual wielding. Returns NULL if no weapon in the hand or if hand < 0 (no hand).
===============
*/
idWeapon* idPlayer::GetWeaponInHand( int hand ) const
{
	if( hand < 0 )
		return NULL;
	else
		return hands[ hand ].weapon.GetEntity();
}

/*
===============
idPlayer::GetGrabberWeapon
Carl: Dual wielding
===============
*/
idWeapon* idPlayer::GetGrabberWeapon() const
{
	if( hands[ 1 - vr_weaponHand.GetInteger() ].weapon.GetEntity() && hands[ 1 - vr_weaponHand.GetInteger() ].weapon->IdentifyWeapon() == WEAPON_GRABBER )
		return hands[ 1 - vr_weaponHand.GetInteger() ].weapon.GetEntity();
	else
		return hands[ vr_weaponHand.GetInteger() ].weapon.GetEntity();
}

idWeapon * idPlayer::GetPDAWeapon() const
{
	if( hands[ 1 - vr_weaponHand.GetInteger() ].weapon.GetEntity() && hands[ 1 - vr_weaponHand.GetInteger() ].weapon->IdentifyWeapon() == WEAPON_PDA )
		return hands[ 1 - vr_weaponHand.GetInteger() ].weapon.GetEntity();
	else
		return hands[ vr_weaponHand.GetInteger() ].weapon.GetEntity();
}

// Carl: for now, only one of the weapons we are holding can have a flashlight mounted to it
// because we haven't implemented dual wielding flashlights yet.
idWeapon * idPlayer::GetWeaponWithMountedFlashlight()
{
	// The pistol we start with in RoE for XBox has a flashlight mounted, so prefer that weapon first (for now)
	if( commonVr->currentFlashlightMode == FLASHLIGHT_PISTOL )
	{
		if( hands[0].currentWeapon == weapon_pistol && !hands[0].isTheDuplicate )
			return hands[0].weapon.GetEntity();
		if( hands[1].currentWeapon == weapon_pistol && !hands[1].isTheDuplicate )
			return hands[1].weapon.GetEntity();
	}
	// Carl: todo
	return GetMainWeapon();
}

int idPlayer::GetBestWeaponHand()
{
	if( !weaponEnabled || hands[1 - vr_weaponHand.GetInteger()].idealWeapon == weapon_fists )
	{
		return vr_weaponHand.GetInteger();
	}
	if( hands[ vr_weaponHand.GetInteger() ].idealWeapon.Get() == weapon_fists )
	{
		return 1 - vr_weaponHand.GetInteger();
	}

	int w = MAX_WEAPONS;
	while( w > 0 )
	{
		w--;
		if( w == weapon_flashlight )
		{
			continue;
		}
		const char* weap = spawnArgs.GetString( va( "def_weapon%d", w ) );
		if( !weap[ 0 ] || ( ( inventory.weapons & ( 1 << w ) ) == 0 ) || ( !inventory.HasAmmo( weap, true, this ) ) )
		{
			continue;
		}
		if( !spawnArgs.GetBool( va( "weapon%d_best", w ) ) )
		{
			continue;
		}

		//Some weapons will report having ammo but the clip is empty and
		//will not have enough to fill the clip (i.e. Double Barrel Shotgun with 1 round left)
		//We need to skip these weapons because they cannot be used
		if( inventory.HasEmptyClipCannotRefill( weap, this, false ) && inventory.HasEmptyClipCannotRefill( weap, this, true ) )
		{
			continue;
		}

		if( hands[ vr_weaponHand.GetInteger() ].idealWeapon.Get() == w )
			return vr_weaponHand.GetInteger();
		if( hands[ 1- vr_weaponHand.GetInteger() ].idealWeapon.Get() == w )
			return 1 - vr_weaponHand.GetInteger();
	}
	w = MAX_WEAPONS;
	while( w > 0 )
	{
		w--;
		if( w == weapon_flashlight )
		{
			continue;
		}
		const char* weap = spawnArgs.GetString( va( "def_weapon%d", w ) );
		if( !weap[ 0 ] || ( ( inventory.weapons & ( 1 << w ) ) == 0 ) || ( !inventory.HasAmmo( weap, true, this ) ) )
		{
			continue;
		}

		//Some weapons will report having ammo but the clip is empty and
		//will not have enough to fill the clip (i.e. Double Barrel Shotgun with 1 round left)
		//We need to skip these weapons because they cannot be used
		if( inventory.HasEmptyClipCannotRefill( weap, this, false ) && inventory.HasEmptyClipCannotRefill( weap, this, true ) )
		{
			continue;
		}

		if( hands[ vr_weaponHand.GetInteger() ].idealWeapon.Get() == w )
			return vr_weaponHand.GetInteger();
		if( hands[ 1 - vr_weaponHand.GetInteger() ].idealWeapon.Get() == w )
			return 1 - vr_weaponHand.GetInteger();
	}
	w = MAX_WEAPONS;
	while( w > 0 )
	{
		w--;
		if( w == weapon_flashlight )
		{
			continue;
		}
		const char* weap = spawnArgs.GetString( va( "def_weapon%d", w ) );
		if( !weap[ 0 ] || ( ( inventory.weapons & ( 1 << w ) ) == 0 ) )
		{
			continue;
		}

		if( hands[ vr_weaponHand.GetInteger() ].idealWeapon.Get() == w )
			return vr_weaponHand.GetInteger();
		if( hands[ 1 - vr_weaponHand.GetInteger() ].idealWeapon.Get() == w )
			return 1 - vr_weaponHand.GetInteger();
	}

	return vr_weaponHand.GetInteger();
}

/*
===============
idPlayer::GetMainWeapon
Carl: Dual wielding, when the code needs just one weapon, guess which one is the "main" one
===============
*/
idWeapon* idPlayer::GetMainWeapon()
{
	// Carl: TODO dual wielding
	return hands[ GetBestWeaponHand() ].weapon.GetEntity();
}

/*
===============
idPlayer::GetBestWeaponHandToSteal
Carl: Dual wielding, when the code needs just one weapon, guess which one is the "main" one
===============
*/
int idPlayer::GetBestWeaponHandToSteal( idPlayer* thief )
{
	// Carl: TODO dual wielding
	return GetBestWeaponHand();
}

/*
===============
idPlayer::GetHarvestWeapon
Carl: Dual wielding, get the specific weapon used to harvest souls (Soul Cube or Artifact)
Returns the required one if you're holding it, or the other one, or the main weapon
===============
*/
idWeapon* idPlayer::GetHarvestWeapon( idStr requiredWeapons )
{
	// Carl: TODO dual wielding
	if( hands[ 0 ].weapon.GetEntity() && ( hands[ 0 ].weapon->IdentifyWeapon() == WEAPON_SOULCUBE || hands[ 0 ].weapon->IdentifyWeapon() == WEAPON_ARTIFACT ) )
		return hands[ 0 ].weapon.GetEntity();
	if( hands[ 1 ].weapon.GetEntity() && ( hands[ 1 ].weapon->IdentifyWeapon() == WEAPON_SOULCUBE || hands[ 1 ].weapon->IdentifyWeapon() == WEAPON_ARTIFACT ) )
		return hands[ 1 ].weapon.GetEntity();
	return GetMainWeapon();
}

/*
=================
idPlayer::GetCurrentHarvestWeapon
Carl: Dual wielding, get the specific weapon used to harvest souls (Soul Cube or Artifact)
Returns the required one if you're holding it, or the other one, or the main weapon
=================
*/
idStr idPlayer::GetCurrentHarvestWeapon( idStr requiredWeapons )
{
	// Carl: TODO dual wielding
	if( hands[ 0 ].weapon.GetEntity() && ( hands[ 0 ].weapon->IdentifyWeapon() == WEAPON_SOULCUBE || hands[ 0 ].weapon->IdentifyWeapon() == WEAPON_ARTIFACT ) )
		return hands[ 0 ].GetCurrentWeaponString();
	if( hands[ 1 ].weapon.GetEntity() && ( hands[ 1 ].weapon->IdentifyWeapon() == WEAPON_SOULCUBE || hands[ 1 ].weapon->IdentifyWeapon() == WEAPON_ARTIFACT ) )
		return hands[ 1 ].GetCurrentWeaponString();
	return GetCurrentWeapon();
}

/*
=====================
idPlayer::GetAnim
Carl: Virtual method idActor::GetAnim() needs to work differently on a dual wielding player
=====================
*/
int idPlayer::GetAnim( int channel, const char* animname )
{
	int			anim;
	const char* temp;
	idAnimator* animatorPtr;

	if( channel == ANIMCHANNEL_HEAD )
	{
		if( !head.GetEntity() )
		{
			return 0;
		}
		animatorPtr = head.GetEntity()->GetAnimator();
	}
	else
	{
		animatorPtr = &animator;
	}

	if( channel == ANIMCHANNEL_LEFTHAND && hands[HAND_LEFT].animPrefix.Length() )
	{
		temp = va( "%s_%s", hands[HAND_LEFT].animPrefix.c_str(), animname );
		anim = animatorPtr->GetAnim( temp );
		if( anim )
			return anim;
	}
	else if( channel == ANIMCHANNEL_RIGHTHAND && hands[HAND_RIGHT].animPrefix.Length() )
	{
		temp = va( "%s_%s", hands[HAND_RIGHT].animPrefix.c_str(), animname );
		anim = animatorPtr->GetAnim( temp );
		if( anim )
			return anim;
	}
	if( animPrefix.Length() )
	{
		temp = va( "%s_%s", animPrefix.c_str(), animname );
		anim = animatorPtr->GetAnim( temp );
		if( anim )
			return anim;
	}

	anim = animatorPtr->GetAnim( animname );

	return anim;
}



/*
===============
idPlayer::SetLevelTrigger
===============
*/
void idPlayer::SetLevelTrigger( const char* levelName, const char* triggerName )
{
	if( levelName && *levelName && triggerName && *triggerName )
	{
		idLevelTriggerInfo lti;
		lti.levelName = levelName;
		lti.triggerName = triggerName;
		inventory.levelTriggers.Append( lti );
	}
}

/*
===============
idPlayer::Event_LevelTrigger
===============
*/
void idPlayer::Event_LevelTrigger()
{
	idStr mapName = gameLocal.GetMapName();
	mapName.StripPath();
	mapName.StripFileExtension();
	for( int i = inventory.levelTriggers.Num() - 1; i >= 0; i-- )
	{
		if( idStr::Icmp( mapName, inventory.levelTriggers[i].levelName ) == 0 )
		{
			idEntity* ent = gameLocal.FindEntity( inventory.levelTriggers[i].triggerName );
			if( ent )
			{
				ent->PostEventMS( &EV_Activate, 1, this );
			}
		}
	}
}

/*
===============
idPlayer::Event_Gibbed
===============
*/
void idPlayer::Event_Gibbed()
{
	// do nothing
}

extern idCVar net_clientMaxPrediction;

/*
===============
idPlayer::UpdatePlayerIcons
===============
*/
void idPlayer::UpdatePlayerIcons()
{
	idLobbyBase& lobby = session->GetActingGameStateLobbyBase();
	int lastPacketTime = lobby.GetPeerTimeSinceLastPacket( lobby.PeerIndexFromLobbyUser( gameLocal.lobbyUserIDs[entityNumber] ) );
	isLagged = ( lastPacketTime > net_clientMaxPrediction.GetInteger() );
	//isChatting = ( ( usercmd.buttons & BUTTON_CHATTING ) && ( health > 0 ) );
}

/*
===============
idPlayer::DrawPlayerIcons
===============
*/
void idPlayer::DrawPlayerIcons()
{
	if( !NeedsIcon() )
	{
		playerIcon.FreeIcon();
		return;
	}
	
	// Never draw icons for hidden players.
	if( this->IsHidden() )
		return;
		
	playerIcon.Draw( this, headJoint );
}

/*
===============
idPlayer::HidePlayerIcons
===============
*/
void idPlayer::HidePlayerIcons()
{
	playerIcon.FreeIcon();
}

/*
===============
idPlayer::NeedsIcon
==============
*/
bool idPlayer::NeedsIcon()
{
	// local clients don't render their own icons... they're only info for other clients
	// always draw icons in CTF games
	return !IsLocallyControlled() && ( ( g_CTFArrows.GetBool() && gameLocal.mpGame.IsGametypeFlagBased() && !IsHidden() && !AI_DEAD ) || ( isLagged ) );
}

/*
===============
idPlayer::DropFlag()
==============
*/
void idPlayer::DropFlag()
{
	if( !carryingFlag || !common->IsMultiplayer() || !gameLocal.mpGame.IsGametypeFlagBased() )  /* CTF */
		return;
		
	idEntity* entity = gameLocal.mpGame.GetTeamFlag( 1 - team );
	if( entity )
	{
		idItemTeam* item = static_cast<idItemTeam*>( entity );
		
		if( item->carried && !item->dropped )
		{
			item->Drop( health <= 0 );
			carryingFlag = false;
		}
	}
	
}

void idPlayer::ReturnFlag()
{

	if( !carryingFlag || !common->IsMultiplayer() || !gameLocal.mpGame.IsGametypeFlagBased() )  /* CTF */
		return;
		
	idEntity* entity = gameLocal.mpGame.GetTeamFlag( 1 - team );
	if( entity )
	{
		idItemTeam* item = static_cast<idItemTeam*>( entity );
		
		if( item->carried && !item->dropped )
		{
			item->Return();
			carryingFlag = false;
		}
	}
}

void idPlayer::FreeModelDef()
{
	idAFEntity_Base::FreeModelDef();
	if( common->IsMultiplayer() && gameLocal.mpGame.IsGametypeFlagBased() )
		playerIcon.FreeIcon();
}

void idPlayer::SetControllerShake( float magnitude, int duration, const idVec3 &direction )
{
	idVec3 dir = direction;
	dir.Normalize();
	idVec3 left = hands[HAND_LEFT].handOrigin - hands[HAND_RIGHT].handOrigin;
	float side = left * dir * 0.5 + 0.5;

	// push magnitude up so the middle doesn't feel as weak
	float invSide = 1.0 - side;
	float rightSide = 1.0 - side*side;
	float leftSide = 1.0 - invSide*invSide;

	float leftMag = magnitude * leftSide;
	float rightMag = magnitude * rightSide;

	hands[HAND_LEFT].SetControllerShake( leftMag, duration, leftMag, duration );
	hands[HAND_RIGHT].SetControllerShake( rightMag, duration, rightMag, duration );
}

/*
========================
idPlayer::ResetControllerShake
========================
*/
void idPlayer::ResetControllerShake()
{
	for( int h = 0; h < 2; h++ )
		hands[h].ResetControllerShake();
}

/*
========================
idPlayer::GetExpansionType
========================
*/
gameExpansionType_t idPlayer::GetExpansionType() const
{
	const char* expansion = spawnArgs.GetString( "player_expansion", "d3" );
	if( idStr::Icmp( expansion, "d3" ) == 0 )
	{
		return GAME_BASE;
	}
	if( idStr::Icmp( expansion, "d3xp" ) == 0 )
	{
		return GAME_D3XP;
	}
	if( idStr::Icmp( expansion, "d3le" ) == 0 )
	{
		return GAME_D3LE;
	}
	return GAME_UNKNOWN;
}
