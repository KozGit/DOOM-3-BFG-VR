
#include"precompiled.h"
#pragma hdrstop

#include "framework/Common_local.h"

#include "d3xp/Game_local.h"

#include "Bonus.h"
#include "Voice.h"
#include "d3xp/Player.h"

idCVar bonus_char( "bonus_char", "0", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "Current bonus character", 0, BONUS_CHAR_WITCH );
idCVar bonus_char_marine( "bonus_char_marine", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character Doom Marine from Doom 3" );
idCVar bonus_char_roe( "bonus_char_roe", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character Red Team from RoE expansion" );
idCVar bonus_char_le( "bonus_char_le", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character Bravo Team from LE expansion" );
idCVar bonus_char_campbell( "bonus_char_campbell", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character Jack Campbell (Swann's bodyguard) from Doom 3" );
idCVar bonus_char_doomguy( "bonus_char_doomguy", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character Doomguy from Doom 1 and 2" );
idCVar bonus_char_slayer( "bonus_char_slayer", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character Doom Slayer from Doom 2016" );
idCVar bonus_char_eternal( "bonus_char_eternal", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character Doom Slayer from Doom Eternal" );
idCVar bonus_char_vfr( "bonus_char_vfr", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character Dr. M. Peters (via Combat Chasis) from Doom VFR" );
idCVar bonus_char_ash( "bonus_char_ash", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character Ash from Evil Dead" );
idCVar bonus_char_samus( "bonus_char_samus", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character Samus Aran from Metroid" );
idCVar bonus_char_witch( "bonus_char_witch", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character Witch from Hicky" );
idCVar bonus_chainsaw( "bonus_chainsaw", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Has collected the chainsaw" );
idCVar bonus_boomstick( "bonus_boomstick", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Has collected the double-barrel shotgun" );

bool BonusCharUnlocked( bonus_char_t ch )
{
	switch( ch )
	{
	case BONUS_CHAR_NONE: return true;
	case BONUS_CHAR_MARINE: return bonus_char_marine.GetBool();
	case BONUS_CHAR_ROE: return bonus_char_roe.GetBool();
	case BONUS_CHAR_LE: return bonus_char_le.GetBool();
	case BONUS_CHAR_CAMPBELL: return bonus_char_campbell.GetBool();
	case BONUS_CHAR_DOOMGUY: return bonus_char_doomguy.GetBool();
	case BONUS_CHAR_SLAYER: return bonus_char_slayer.GetBool();
	case BONUS_CHAR_ETERNAL: return bonus_char_eternal.GetBool();
	case BONUS_CHAR_VFR: return bonus_char_vfr.GetBool();
	case BONUS_CHAR_ASH: return bonus_char_ash.GetBool();
	case BONUS_CHAR_SAMUS: return bonus_char_samus.GetBool();
	case BONUS_CHAR_WITCH: return bonus_char_witch.GetBool();
	default: return false;
	}
}

void BonusGiveSignatureWeapons( idPlayer *player, bonus_char_t ch )
{
	switch( ch )
	{
	case BONUS_CHAR_MARINE:
		player->GiveInventoryItem( "weapon_chainsaw" );
		player->GiveInventoryItem( "weapon_soulcube" );
		break;
	case BONUS_CHAR_ROE:
		player->GiveInventoryItem( "weapon_grabber" );
		player->GiveInventoryItem( "weapon_shotgun_double" );
		player->GiveInventoryItem( "weapon_bloodstone_passive" );
		break;
	case BONUS_CHAR_LE:
		player->GiveInventoryItem( "weapon_grabber" );
		player->GiveInventoryItem( "weapon_shotgun_double" );
		break;
	case BONUS_CHAR_CAMPBELL:
		player->GiveInventoryItem( "weapon_bfg" );
		break;
	case BONUS_CHAR_DOOMGUY:
		player->GiveInventoryItem( "weapon_chainsaw" );
		player->GiveInventoryItem( "weapon_shotgun_double" );
		break;
	case BONUS_CHAR_SLAYER:
		// only give double shotgun in main campaign
		player->GiveInventoryItem( "weapon_shotgun_double" );
		break;
	case BONUS_CHAR_ETERNAL:
		player->GiveInventoryItem( "weapon_shotgun_double" );
		break;
	case BONUS_CHAR_VFR:
		player->GiveInventoryItem( "weapon_handgrenade" );
		break;
	case BONUS_CHAR_ASH:
		player->GiveInventoryItem( "weapon_chainsaw" );
		player->GiveInventoryItem( "weapon_shotgun_double" );
		break;
	case BONUS_CHAR_SAMUS:
		player->GiveInventoryItem( "weapon_pistol" );
		break;
	case BONUS_CHAR_WITCH:
		break;
	}

}

const char * BonusCharDescription( bonus_char_t ch )
{
	if( BonusCharUnlocked( ch ) )
	{
		switch( ch )
		{
		case BONUS_CHAR_NONE: return "Use the default character for that campaign";
		case BONUS_CHAR_MARINE: return "Doom Marine from Doom 3 with chainsaw and Soul Cube";
		case BONUS_CHAR_ROE: return "RoE Marine with Artifact, Grabber, and double-barrel shotgun";
		case BONUS_CHAR_LE: return "Bravo Team Marine with Grabber and double-barrel shotgun";
		case BONUS_CHAR_CAMPBELL: return "Jack Campbell with BFG 9000 and access codes";
		case BONUS_CHAR_DOOMGUY: return "Doomguy from Doom 2 with chainsaw and double-barrel shotgun";
		case BONUS_CHAR_SLAYER: return "Doom Slayer with double jump boots";
		case BONUS_CHAR_ETERNAL: return "Doom Slayer with double-barrel shotgun";
		case BONUS_CHAR_VFR: return "Combat chasis with grenades, shield burst, and teleporter";
		case BONUS_CHAR_ASH: return "Ash with chainsaw instead of right hand, and double-barrel shotgun";
		case BONUS_CHAR_SAMUS: return "Bounty hunter Samus Aran";
		case BONUS_CHAR_WITCH: return "A witch with the power to cure zombies";
		default: return "";
		}
	}
	else
	{
		switch( ch )
		{
		case BONUS_CHAR_MARINE: return "Unlock by getting the chainsaw in (or beating) Doom 3";
		case BONUS_CHAR_ROE: return "Unlock by getting the Grabber in (or beating) RoE";
		case BONUS_CHAR_LE: return "Unlock by getting the Grabber in (or beating) Lost Mission";
		case BONUS_CHAR_CAMPBELL: return "Unlock by you or Campbell getting the BFG in Doom 3";
		case BONUS_CHAR_DOOMGUY: return "Unlock by beating, or getting the chainsaw in, Doom 1 or 2";
		case BONUS_CHAR_SLAYER: return "Unlock by playing Doom 2016";
		case BONUS_CHAR_ETERNAL: return "Unlock by console command bonus_char_eternal";
		case BONUS_CHAR_VFR: return "Unlock by playing Doom VFR";
		case BONUS_CHAR_ASH: return "Unlock by separately getting both the chainsaw and double barrel shotgun";
		case BONUS_CHAR_SAMUS: return "Unlock by playing any Metroid game in Dolphin VR";
		case BONUS_CHAR_WITCH: return "Unlock by console command bonus_char_witch";
		default: return "";
		}

	}
	return "";
}

const char *BonusCharMesh( bonus_char_t ch )
{
	switch( ch )
	{
	case BONUS_CHAR_MARINE: return "models/md5/characters/player/mocap/player.md5mesh"; // "models/md5/characters/player/mocap/wepplayer.md5mesh";
	case BONUS_CHAR_ROE: return "models/md5/chars/marine.md5mesh"; // "models/md5/cinematics/erebus5/e5_player.md5mesh";
	case BONUS_CHAR_LE: return "models/md5/chars/marine.md5mesh"; // "models/md5/characters/player/mocap/wepplayer.md5mesh";
	case BONUS_CHAR_CAMPBELL: return "models/md5/chars/campbell/campbell.md5mesh";
	case BONUS_CHAR_DOOMGUY: return "models/md5/characters/player/mocap/player.md5mesh";
	case BONUS_CHAR_SLAYER: return "models/md5/characters/player/mocap/player.md5mesh";
	case BONUS_CHAR_ETERNAL: return "models/md5/characters/player/mocap/player.md5mesh";
	case BONUS_CHAR_VFR: return "models/md5/characters/player/mocap/player.md5mesh";
	case BONUS_CHAR_ASH: return "models/md5/chars/tshirt.md5mesh";
	case BONUS_CHAR_SAMUS: return "models/md5/characters/player/mocap/player.md5mesh";
	case BONUS_CHAR_WITCH: return "models/md5/characters/poppy/poppy.md5mesh"; // Theresa Chasar?
	default: return "";
	}
}

const char *BonusCharMeshFull( bonus_char_t ch )
{
	switch( ch )
	{
	case BONUS_CHAR_MARINE: return "models/md5/characters/player/mocap/player.md5mesh"; // "models/md5/characters/player/mocap/wepplayer.md5mesh";
	case BONUS_CHAR_ROE: return "models/md5/cinematics/erebus5/e5_player.md5mesh";
	case BONUS_CHAR_LE: return "models/md5/characters/player/mocap/wepplayer.md5mesh";
	case BONUS_CHAR_CAMPBELL: return "models/md5/chars/campbell/campbell.md5mesh"; // probably missing head, but oh well.
	case BONUS_CHAR_DOOMGUY: return "models/md5/characters/player/mocap/player.md5mesh";
	case BONUS_CHAR_SLAYER: return "models/md5/characters/player/mocap/player.md5mesh";
	case BONUS_CHAR_ETERNAL: return "models/md5/characters/player/mocap/player.md5mesh";
	case BONUS_CHAR_VFR: return "models/md5/characters/player/mocap/player.md5mesh";
	case BONUS_CHAR_ASH: return "models/md5/chars/tshirt.md5mesh";
	case BONUS_CHAR_SAMUS: return "models/md5/characters/player/mocap/player.md5mesh";
	case BONUS_CHAR_WITCH: return "models/md5/characters/poppy/poppy.md5mesh"; // models/md5/chars/mcneil/erebus5_mcneil.md5mesh
	default: return "";
	}
}

const char *BonusCharMeshHead( bonus_char_t ch )
{
	switch( ch )
	{
	case BONUS_CHAR_MARINE: return "models/md5/heads/player/player.md5mesh";
	case BONUS_CHAR_ROE: return "models/md5/heads/player/player.md5mesh"; // "models/md5/characters/player/head/d3xp_head.md5mesh";
	case BONUS_CHAR_LE: return "models/md5/heads/player/player.md5mesh"; // "models/md5/characters/player/head/d3xp_head.md5mesh"; // weird, why is it the same?
	case BONUS_CHAR_CAMPBELL: return "models/md5/heads/campbell/campbell.md5mesh";
	case BONUS_CHAR_DOOMGUY: return "models/md5/characters/npcs/heads/mhelmet.md5mesh";
	case BONUS_CHAR_SLAYER: return "models/md5/characters/npcs/heads/mhelmet.md5mesh";
	case BONUS_CHAR_ETERNAL: return "models/md5/characters/npcs/heads/mhelmet.md5mesh";
	case BONUS_CHAR_VFR: return "models/md5/characters/npcs/heads/mhelmet.md5mesh";
	case BONUS_CHAR_ASH: return "models/md5/heads/player/player.md5mesh";
	case BONUS_CHAR_SAMUS: return "models/md5/characters/npcs/heads/mhelmet.md5mesh";
	case BONUS_CHAR_WITCH: return "models/md5/characters/npcs/heads/mhelmet.md5mesh";
	default: return "";
	}
}

idStr BonusCharSkin( idStr skinname, bonus_char_t ch )
{
	if( ( skinname.Icmp( "skins/characters/npcs/marine_player.skin" ) == 0 && bonus_char.GetInteger() != BONUS_CHAR_MARINE ) )
	{
		switch( bonus_char.GetInteger() )
		{
		case BONUS_CHAR_ASH:
			// same skin
			break;
		default:
			skinname = "";
		}
	}
	else if( ( skinname.Icmp( "skins/characters/npcs/marine_player_bag.skin" ) == 0 && bonus_char.GetInteger() != BONUS_CHAR_MARINE ) )
	{
		switch( bonus_char.GetInteger() )
		{
		case BONUS_CHAR_ASH:
			// same skin
			break;
		default:
			skinname = "";
		}
	}
	return skinname;
}

const char* BonusCharModel( const char* m, bonus_char_t ch )
{
	if( idStr::Icmp(m, "head_player" ) == 0 && bonus_char.GetInteger() != BONUS_CHAR_MARINE )
	{
		switch( bonus_char.GetInteger() )
		{
		case BONUS_CHAR_ROE:
			m = "model_d3xp_sp_head";
			break;
		case BONUS_CHAR_LE:
		case BONUS_CHAR_VFR:
			m = "model_d3le_sp_helmet";
			break;
		case BONUS_CHAR_DOOMGUY:
		case BONUS_CHAR_SLAYER:
		case BONUS_CHAR_ETERNAL:
		case BONUS_CHAR_SAMUS:
			m = "model_d3_sp_helmet";
			break;
		}
	}
	else if( idStr::Icmp( m, "hellhole_cin_npcplayerhead" ) == 0 && bonus_char.GetInteger() != BONUS_CHAR_MARINE )
	{
		switch( bonus_char.GetInteger() )
		{
		case BONUS_CHAR_ROE:
			m = "model_d3xp_sp_head";
			break;
		case BONUS_CHAR_LE:
		case BONUS_CHAR_VFR:
			m = "model_d3le_sp_helmet";
			break;
		case BONUS_CHAR_DOOMGUY:
		case BONUS_CHAR_SLAYER:
		case BONUS_CHAR_ETERNAL:
		case BONUS_CHAR_SAMUS:
			m = "model_d3_sp_helmet";
			break;
		}
	}
	else if( idStr::Icmp( m, "marscity_head_player" ) == 0 && bonus_char.GetInteger() != BONUS_CHAR_MARINE )
	{
		switch( bonus_char.GetInteger() )
		{
		case BONUS_CHAR_ROE:
			m = "model_d3xp_sp_head";
			break;
		case BONUS_CHAR_LE:
		case BONUS_CHAR_VFR:
			m = "model_d3le_sp_helmet";
			break;
		case BONUS_CHAR_DOOMGUY:
		case BONUS_CHAR_SLAYER:
		case BONUS_CHAR_ETERNAL:
		case BONUS_CHAR_SAMUS:
			m = "model_d3_sp_helmet";
			break;
		}
	}
	return m;
}

const char* BonusCharEntityClass( const char* e, bonus_char_t ch )
{
	if( !e || ( bonus_char.GetInteger() != BONUS_CHAR_ROE && bonus_char.GetInteger() != BONUS_CHAR_LE ) )
		return e;
	// Make all weapons moveable
	if( idStr::Icmpn( e, "weapon_", strlen( "weapon_" ) ) == 0 )
	{
		const char* weapons[14][2] = {
			{ "weapon_pistol", "moveable_item_pistol" },
			{ "weapon_shotgun", "moveable_item_shotgun" },
			{ "weapon_shotgun_double", "moveable_item_shotgun_double" },
			{ "weapon_machinegun", "moveable_item_machinegun" },
			{ "weapon_chaingun", "moveable_item_chaingun" },
			{ "weapon_plasmagun", "moveable_item_plasmagun" },
			{ "weapon_rocketlauncher", "moveable_item_rocketlauncher" },
			{ "weapon_handgrenade", "moveable_item_handgrenade" },
			{ "weapon_bfg", "moveable_item_bfg" },
			{ "weapon_soulcube", "moveable_item_soulcube" },
			{ "weapon_chainsaw", "moveable_item_chainsaw" },
			{ "weapon_grabber", "moveable_item_grabber" },
			{ "weapon_flashlight", "moveable_item_flashlight" },
			{ "weapon_plasmagun", "moveable_item_plasmagun" },
		};
		for( int i = 0; i < 14; i++ )
		{
			if( idStr::Icmp( e, weapons[i][0] ) == 0 )
			{
				common->Printf( "Converting %s to %s", e, weapons[i][1] );
				return weapons[i][1];
			}
		}
	}
	// Make all ammo moveable
	else if( idStr::Icmpn( e, "ammo_", strlen( "ammo_" ) ) == 0 )
	{
		const char* ammo[13][2] = {
			{ "ammo_bullets_small", "moveable_ammo_bullets_small" },
			{ "ammo_bullets_large", "moveable_ammo_bullets_large" },
			{ "ammo_shells_small", "moveable_ammo_shells_small" },
			{ "ammo_shells_large", "moveable_ammo_shells_large" },
			{ "ammo_clip_small", "moveable_ammo_clip_small" },
			{ "ammo_clip_large", "moveable_ammo_clip_large" },
			{ "ammo_belt_small", "moveable_ammo_belt_small" },
			{ "ammo_rockets_small", "moveable_ammo_rockets_small" },
			{ "ammo_rockets_large", "moveable_ammo_rockets_large" },
			{ "ammo_grenade_small", "moveable_ammo_grenade_small" },
			{ "ammo_cells_small", "moveable_ammo_cells_small" },
			{ "ammo_cells_large", "moveable_ammo_cells_large" },
			{ "ammo_bfg_small", "moveable_ammo_bfg_small" },
		};
		for( int i = 0; i < 13; i++ )
		{
			if( idStr::Icmp( e, ammo[i][0] ) == 0 )
			{
				common->Printf( "Converting %s to %s", e, ammo[i][1] );
				return ammo[i][1];
			}
		}
	}
	// Make all items moveable
	else if( idStr::Icmpn( e, "item_", strlen( "item_" ) ) == 0 )
	{
		const char* items[5][2] = {
			{ "item_medkit", "moveable_item_medkit" },
			{ "item_medkit_small", "moveable_item_medkit_small" },
			{ "item_armor_security", "moveable_item_armor_security" },
			{ "item_armor_shard", "moveable_item_armor_shard" },
			{ "item_backpack", "moveable_item_backpack" },
		};
		for( int i = 0; i < 5; i++ )
		{
			if( idStr::Icmp( e, items[i][0] ) == 0 )
			{
				common->Printf( "Converting %s to %s\n", e, items[i][1] );
				return items[i][1];
			}
		}
	}
	else if( idStr::Icmp( e, "powerup_adrenaline" ) == 0 )
	{
		common->Printf( "Converting %s to %s\n", e, "moveable_powerup_adrenaline" );
		return "moveable_powerup_adrenaline";
	}
	return e;
}

bool HasPlayedDoom2016()
{
	return false;
}

bool HasPlayedDoomVFR()
{
	return false;
}

bool HasPlayedMetroid()
{
	return false;
}

void BonusCheckOtherGames()
{
	static bool checkedAlready = false; // Carl: This can't change while we're playing, so only check once
	if (checkedAlready)
		return;
	if( !BonusCharUnlocked( BONUS_CHAR_SLAYER ) )
		bonus_char_slayer.SetBool( HasPlayedDoom2016() );
	if( !BonusCharUnlocked( BONUS_CHAR_VFR ) )
		bonus_char_vfr.SetBool( HasPlayedDoomVFR() );
	if( !BonusCharUnlocked( BONUS_CHAR_SAMUS ) )
		bonus_char_samus.SetBool( HasPlayedMetroid() );
	checkedAlready = true;
}
