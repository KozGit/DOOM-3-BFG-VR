
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
idCVar bonus_char_sarge( "bonus_char_sarge", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character Sergeant Kelly from Doom 3" );
idCVar bonus_char_betruger( "bonus_char_betruger", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character Dr. Malcom Betruger from Doom 3" );
idCVar bonus_char_swann( "bonus_char_swann", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character Swann from Doom 3" );
idCVar bonus_char_roland( "bonus_char_roland", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character R. Roland from Doom 3" );
idCVar bonus_char_doomguy( "bonus_char_doomguy", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character Doomguy from Doom 1 and 2" );
idCVar bonus_char_phobos( "bonus_char_phobos", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character Phobos from Quake 3" );
idCVar bonus_char_slayer( "bonus_char_slayer", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character Doom Slayer from Doom 2016" );
idCVar bonus_char_eternal( "bonus_char_eternal", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character Doom Slayer from Doom Eternal" );
idCVar bonus_char_vfr( "bonus_char_vfr", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character Dr. M. Peters (via Combat Chasis) from Doom VFR" );
idCVar bonus_char_ash( "bonus_char_ash", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character Ash from Evil Dead" );
idCVar bonus_char_samus( "bonus_char_samus", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character Samus Aran from Metroid" );
idCVar bonus_char_witch( "bonus_char_witch", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Unlocked bonus character Witch from Hicky" );
idCVar bonus_chainsaw( "bonus_chainsaw", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Has collected the chainsaw" );
idCVar bonus_boomstick( "bonus_boomstick", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Has collected the double-barrel shotgun" );
idCVar bonus_cataract( "bonus_cataract", "0.7", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Opacity of Betruger's cataract. 0 = no cataract, 1 = vision completely blocked" );
idCVar bonus_cataract_eye( "bonus_cataract_eye", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Which of Betruger's eyes should have the cataract. 0 = right, 1 = left. In RoE they changed which eye it was (from Right to Left). Let player match their own good eye." );

bool BonusCharUnlocked( bonus_char_t ch )
{
	switch( ch )
	{
	case BONUS_CHAR_NONE: return true;
	case BONUS_CHAR_MARINE: return bonus_char_marine.GetBool();
	case BONUS_CHAR_ROE: return bonus_char_roe.GetBool();
	case BONUS_CHAR_LE: return bonus_char_le.GetBool();
	case BONUS_CHAR_CAMPBELL: return bonus_char_campbell.GetBool();
	case BONUS_CHAR_SARGE: return bonus_char_sarge.GetBool();
	case BONUS_CHAR_BETRUGER: return bonus_char_betruger.GetBool();
	case BONUS_CHAR_SWANN: return bonus_char_swann.GetBool();
	case BONUS_CHAR_ROLAND: return bonus_char_roland.GetBool();
	case BONUS_CHAR_DOOMGUY: return bonus_char_doomguy.GetBool();
	case BONUS_CHAR_PHOBOS: return bonus_char_phobos.GetBool();
	case BONUS_CHAR_SLAYER: return bonus_char_slayer.GetBool();
	case BONUS_CHAR_ETERNAL: return bonus_char_eternal.GetBool();
	case BONUS_CHAR_VFR: return bonus_char_vfr.GetBool();
	case BONUS_CHAR_ASH: return bonus_char_ash.GetBool();
	case BONUS_CHAR_SAMUS: return bonus_char_samus.GetBool();
	case BONUS_CHAR_WITCH: return bonus_char_witch.GetBool();
	default: return false;
	}
}

bool BonusCharHasWeapons( bonus_char_t ch )
{
	return BonusCharUnlocked( ch ) && ch != BONUS_CHAR_NONE && ch != BONUS_CHAR_SWANN && ch != BONUS_CHAR_ROLAND && ch != BONUS_CHAR_WITCH;
}

const char* BonusCharDefPlayer( bonus_char_t ch, const char* default, const char* exp, bool hell )
{
	switch( ch )
	{
	case BONUS_CHAR_MARINE: return hell ? "bonus_player_marine_hell" : "bonus_player_marine";
	case BONUS_CHAR_ROE: return hell ? "bonus_player_roe_hell": "bonus_player_roe"; //"d3xp_player_doommarine";
	case BONUS_CHAR_LE: return hell ? "bonus_player_le_hell" : "bonus_player_le"; //"d3le_player_doommarine";
	case BONUS_CHAR_CAMPBELL: return hell ? "bonus_player_campbell_hell" : "bonus_player_campbell";
	case BONUS_CHAR_SARGE: return hell ? "bonus_player_sarge_hell" : "bonus_player_sarge";
	case BONUS_CHAR_BETRUGER:
		if( idStr::Cmp( exp, "d3" ) == 0 )
			return "bonus_player_betruger";
		else
			return hell ? "bonus_player_betruger_hell" : "bonus_player_betruger_d3xp";
	case BONUS_CHAR_SWANN: 
		if( idStr::Cmp( exp, "d3" ) == 0 )
			return "bonus_player_swann";
		else
			return hell ? "bonus_player_swann_hell" : "bonus_player_swann_d3xp";
	case BONUS_CHAR_ROLAND:
		if( idStr::Cmp( exp, "d3" ) == 0 )
			return "bonus_player_roland";
		else
			return hell ? "bonus_player_roland_hell" : "bonus_player_roland_d3xp";
	case BONUS_CHAR_DOOMGUY: return hell ? "bonus_player_doomguy_hell" : "bonus_player_doomguy";
	case BONUS_CHAR_PHOBOS: return hell ? "bonus_player_phobos_hell" : "bonus_player_phobos";
	case BONUS_CHAR_SLAYER: return hell ? "bonus_player_slayer_hell" : "bonus_player_slayer";
	case BONUS_CHAR_ETERNAL: return hell ? "bonus_player_eternal_hell" : "bonus_player_eternal";
	case BONUS_CHAR_VFR: return hell ? "bonus_player_vfr_hell" : "bonus_player_vfr";
	case BONUS_CHAR_ASH: return hell ? "bonus_player_ash_hell" : "bonus_player_ash";
	case BONUS_CHAR_SAMUS: return hell ? "bonus_player_samus_hell" : "bonus_player_samus";
	case BONUS_CHAR_WITCH: return hell ? "bonus_player_witch_hell" : "bonus_player_witch";
	default:
		return default;
	}
}


void BonusGiveSignatureWeapons( idPlayer *player, bonus_char_t ch )
{
	switch( ch )
	{
	case BONUS_CHAR_MARINE:
		//player->GiveInventoryItem( "weapon_chainsaw" );
		//player->GiveInventoryItem( "weapon_soulcube" );
		break;
	case BONUS_CHAR_ROE:
		//player->GiveInventoryItem( "weapon_grabber" );
		//player->GiveInventoryItem( "weapon_shotgun_double" );
		//player->GiveInventoryItem( "weapon_bloodstone_passive" );
		break;
	case BONUS_CHAR_LE:
		//player->GiveInventoryItem( "weapon_grabber" );
		//player->GiveInventoryItem( "weapon_shotgun_double" );
		break;
	case BONUS_CHAR_CAMPBELL:
		//player->GiveInventoryItem( "weapon_bfg" );
		break;
	case BONUS_CHAR_DOOMGUY:
		//player->GiveInventoryItem( "weapon_chainsaw" );
		//player->GiveInventoryItem( "weapon_shotgun_double" );
		break;
	case BONUS_CHAR_SLAYER:
		// only give double shotgun in main campaign
		//player->GiveInventoryItem( "weapon_shotgun_double" );
		break;
	case BONUS_CHAR_ETERNAL:
		//player->GiveInventoryItem( "weapon_shotgun_double" );
		break;
	case BONUS_CHAR_VFR:
		//player->GiveInventoryItem( "weapon_handgrenade" );
		break;
	case BONUS_CHAR_ASH:
		//player->GiveInventoryItem( "weapon_chainsaw" );
		//player->GiveInventoryItem( "weapon_shotgun_double" );
		break;
	case BONUS_CHAR_SAMUS:
		//player->GiveInventoryItem( "weapon_pistol" );
		break;
	case BONUS_CHAR_WITCH:
		break;
	}

}


const char * BonusCharName( bonus_char_t ch, bool unlocked )
{
	switch( ch )
	{
	case BONUS_CHAR_NONE: return "Default";
	case BONUS_CHAR_MARINE: return unlocked ? "Marine (D3)" : "Locked (D3)";
	case BONUS_CHAR_ROE: return unlocked ? "Red Team (RoE)" : "Locked (RoE)";
	case BONUS_CHAR_LE: return unlocked ? "Bravo Team (LM)" : "Locked (LM)";
	case BONUS_CHAR_CAMPBELL: return unlocked ? "Campbell (D3)" : "Locked 2 (D3)";
	case BONUS_CHAR_SARGE: return unlocked ? "Sarge (D3)" : "Locked 3 (D3)";
	case BONUS_CHAR_BETRUGER: return unlocked ? "Betruger (D3)" : "Locked 4 (D3)";
	case BONUS_CHAR_SWANN: return unlocked ? "Swann (D3)" : "Locked 5 (D3)";
	case BONUS_CHAR_ROLAND: return unlocked ? "R Roland (D3)" : "Locked 6 (D3)";
	case BONUS_CHAR_DOOMGUY: return unlocked ? "Doomguy (D2)" : "Locked (D2)";
	case BONUS_CHAR_PHOBOS: return unlocked ? "Phobos (Q3)" : "Locked (Q3)";
	case BONUS_CHAR_SLAYER: return unlocked ? "Slayer (2016)" : "Locked (2016)";
	case BONUS_CHAR_ETERNAL: return unlocked ? "Slayer (Eternal)" : "Locked (Eternal)";
	case BONUS_CHAR_VFR: return unlocked ? "M. Peters (VFR)" : "Locked (VFR)";
	case BONUS_CHAR_ASH: return unlocked ? "Ash (Evil Dead)" : "Locked (Evil Dead)";
	case BONUS_CHAR_SAMUS: return unlocked ? "Samus (Metroid)" : "Locked (Metroid)";
	case BONUS_CHAR_WITCH: return unlocked ? "Witch" : "Locked";
	default: return "ERROR!";
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
		case BONUS_CHAR_SARGE: return "Sergeant Kelly with double-barrel shotgun and access codes";
		case BONUS_CHAR_BETRUGER: return "Dr. Malcolm Betruger with Soul Cube and access codes";
		case BONUS_CHAR_SWANN: return "Weak character with nothing but access codes";
		case BONUS_CHAR_ROLAND: return "Scared weak survivor hiding in the Mars City vents";
		case BONUS_CHAR_DOOMGUY: return "Doomguy from Doom 2 with chainsaw and double-barrel shotgun";
		case BONUS_CHAR_PHOBOS: return "Phobos from Quake 3 with no special weapons";
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
		case BONUS_CHAR_SARGE: return "Unlock by reuniting with Sarge or finding his office";
		case BONUS_CHAR_BETRUGER: return "Unlock by ";
		case BONUS_CHAR_SWANN: return "Unlock by ";
		case BONUS_CHAR_ROLAND: return "Unlock by talking to R. Roland in the vents";
		case BONUS_CHAR_DOOMGUY: return "Unlock by beating, or getting the chainsaw in, Doom 1 or 2";
		case BONUS_CHAR_PHOBOS: return "Unlock by console command bonus_char_phobos"; // starts unlocked
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
	case BONUS_CHAR_BETRUGER: return "models/md5/chars/suit.md5mesh";
	case BONUS_CHAR_SARGE: return "models/md5/chars/marine.md5mesh";
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
	if( ( skinname.Icmp( "skins/characters/npcs/marine_player.skin" ) == 0 ) )
	{
		switch( bonus_char.GetInteger() )
		{
		case BONUS_CHAR_PHOBOS:
			skinname = "skins/characters/player/phobos_tshirt/vrBody";
			break;
		case BONUS_CHAR_ASH:
			// same skin
			break;
		default:
			skinname = "";
		}
	}
	else if( ( skinname.Icmp( "skins/characters/npcs/marine_player_bag.skin" ) == 0 ) )
	{
		switch( bonus_char.GetInteger() )
		{
		case BONUS_CHAR_PHOBOS:
			skinname = "skins/characters/player/phobos_tshirt";
			break;
		case BONUS_CHAR_ASH:
			// same skin
			break;
		default:
			skinname = "";
		}
	}
	return skinname;
}

idStr BonusCharReplaceTShirtModel( bonus_char_t ch )
{
	// Returns a model def. The returned model def must inherit from the MODEL npc_base (not the entity) in order to be compatible with this method.
	// npc_tshirt, npc_labcoat, npc_suit, npc_suit2, npc_jumpsuit, npc_marine, npc_security, npc_skeleton, char_betruger, char_swann, char_campbell,
	// char_hazmat (npc_hazmat is the same), 
	switch( bonus_char.GetInteger() )
	{
	case BONUS_CHAR_MARINE:
		return "npc_player_armour";
	case BONUS_CHAR_BETRUGER:
		return "char_betruger";
	case BONUS_CHAR_SWANN:
		return "char_swann";
	case BONUS_CHAR_CAMPBELL:
		return "char_campbell"; // Carl: TODO is this the best way? This uses the marine model, but he also has his own model with different anims.
	case BONUS_CHAR_ROLAND:
		return "npc_jumpsuit";
	case BONUS_CHAR_SARGE:
	case BONUS_CHAR_LE:
		return "npc_marine";
	case BONUS_CHAR_PHOBOS:
		return "black_soldier_tshirt";
	case BONUS_CHAR_SLAYER:
	case BONUS_CHAR_ETERNAL:
		return "npc_doom_slayer"; // Carl: Added this 

	case BONUS_CHAR_ROE:
		return "npc_marine"; // Carl: Close, but not perfect
	case BONUS_CHAR_DOOMGUY:
		return "npc_player_armour"; // Carl: Added this compatible player model wearing Doom 3 player's armor.
	case BONUS_CHAR_SAMUS:
	case BONUS_CHAR_VFR:
		return "npc_marine"; // We don't have proper models for these
	case BONUS_CHAR_WITCH:
		return "npc_suit";
	case BONUS_CHAR_ASH:
	default:
		return "npc_tshirt";
	}
}

idStr BonusCharReplaceCompatibleHead( bonus_char_t ch )
{
	// Returns a model def. The returned model def must have the right number of joints, and should ideally implement idle, stand, and blink.
	switch( bonus_char.GetInteger() )
	{
	default:
		return "head_player";
	}
}

// Carl: only used for replacing models that are INCOMPATIBLE with the default animations (mostly heads)
// Returns a model def. The returned model def should implement the anims: idle, stand, and blink
// It works by changing the def_head to a different model. The downside is, we lose any animations in the original head.
const char* BonusCharReplaceIncompatibleHead( const char* m, bonus_char_t ch )
{
	// Don't replace the player with Betruger in any Mars City cutscenes, because Betruge is already in those cutscenes.
	if( bonus_char.GetInteger() == BONUS_CHAR_BETRUGER && idStr::Icmp( m, "marscity_head_player" ) == 0 )
		return m;
	if( bonus_char.GetInteger() != BONUS_CHAR_MARINE && ( idStr::Icmp(m, "head_player" ) == 0 || idStr::Icmp( m, "hellhole_cin_npcplayerhead" ) == 0 || idStr::Icmp( m, "marscity_head_player" ) == 0 ) )
	{
		switch( bonus_char.GetInteger() )
		{
		case BONUS_CHAR_ROE:
			m = "model_d3xp_sp_head";
			break;
		case BONUS_CHAR_LE:
			m = "model_d3le_sp_helmet";
			break;
		case BONUS_CHAR_CAMPBELL:
			return "head_campbell";
		case BONUS_CHAR_SARGE:
			return "model_sarge_head";
		case BONUS_CHAR_BETRUGER:
			return "head_betruger";
		case BONUS_CHAR_SWANN:
			return "head_swann";
		case BONUS_CHAR_ROLAND:
			return "model_roland_head"; // marscity2_ceiling_head
		case BONUS_CHAR_PHOBOS:
			return "head_black";
		case BONUS_CHAR_DOOMGUY:
		case BONUS_CHAR_SLAYER:
		case BONUS_CHAR_ETERNAL:
		case BONUS_CHAR_SAMUS:
		case BONUS_CHAR_VFR:
			m = "head_slayer";
			break;
		// Carl: Other heads are handled elsewhere because they're compatible with head_player
		}
	}
	return m;
}

#define eq(s1, s2) ( idStr::Icmp(s1, s2) == 0 )

const char* BonusCharReplaceIncompatibleEntityClass( const char* e, bonus_char_t ch )
{
	if( !e || !ch )
		return NULL;
	bool replace = false;
	if( ch != BONUS_CHAR_MARINE && ( idStr::Icmp( e, "admin_overhear_player" ) == 0 ) )
		replace = true;
	if( replace )
	{
		switch( bonus_char.GetInteger() )
		{
		case BONUS_CHAR_MARINE:
			return "bonus_player_marine_cutscene";
		case BONUS_CHAR_ROE:
			return "bonus_player_roe_cutscene";
		case BONUS_CHAR_LE:
			return "bonus_player_le_cutscene";
		case BONUS_CHAR_CAMPBELL:
			return "bonus_player_campbell_cutscene";
		case BONUS_CHAR_SARGE:
			return "bonus_player_sarge_cutscene";
		case BONUS_CHAR_BETRUGER:
			return "bonus_player_betruger_cutscene";
		case BONUS_CHAR_SWANN:
			return "bonus_player_swann_cutscene";
		case BONUS_CHAR_ROLAND:
			return "bonus_player_roland_cutscene";
		case BONUS_CHAR_DOOMGUY:
			return "bonus_player_doomguy_cutscene";
		case BONUS_CHAR_SLAYER:
			return "bonus_player_slayer_cutscene";
		case BONUS_CHAR_ETERNAL:
			return "bonus_player_eternal_cutscene";
		case BONUS_CHAR_VFR:
			return "bonus_player_vfr_cutscene";
		case BONUS_CHAR_ASH:
			return "bonus_player_ash_cutscene";
		case BONUS_CHAR_SAMUS:
			return "bonus_player_samus_cutscene";
		case BONUS_CHAR_WITCH:
			return "bonus_player_witch_cutscene";
		}
	}
	return NULL;
}

const char* ItemToMoveableEntityClass( const char* e, bonus_char_t ch )
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
			{ "weapon_grabber", "moveable_item_grabber" }, // Carl: I added this item
			{ "weapon_flashlight", "moveable_item_flashlight" },
			{ "weapon_plasmagun", "moveable_item_plasmagun" },
			// moveable_item_flashlight
			// mp versions moveable_item_chainsaw_mp, pistol, shotgun, machinegun, chaingun, grenades, plasmagun, rocketlauncher, bfg
		};
		for( int i = 0; i < 14; i++ )
		{
			if( idStr::Icmp( e, weapons[i][0] ) == 0 )
			{
				common->Printf( "Converting %s to %s\n", e, weapons[i][1] );
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
				common->Printf( "Converting %s to %s\n", e, ammo[i][1] );
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

// Carl: model can be NULL, "", or the "model" field of a func_static
// Returns the corresponding moveable entity class (instead of "func_static"), or NULL if there isn't one.
const char* ModelToMoveableEntityClass( const char* model, bonus_char_t ch )
{
	if( !model || !model[0] )
		return NULL;
	const char* lookup[78][2] = {
		// moveable.def
		{ "models/mapobjects/lab/filecabinet1/filecabinet1.lwo", "moveable_filecabinet1" }, // 19 in Mars City
		{ "models/mapobjects/lab/diamondbox/diamondbox.lwo", "moveable_diamondbox" }, // 11 in Mars City (1)
		{ "models/mapobjects/lab/plasticjar2/plasticjar2.lwo", "moveable_plasticjar2" }, // 10 in Mars City
		{ "models/mapobjects/mcity/deskcomp/deskcomp.lwo", "moveable_computer" }, // 7 in Mars City
		{ "models/mapobjects/filler/phone.ase", "moveable_phone" }, // 6 in Mars City 1
		{ "models/mapobjects/lab/compcart/compcart.lwo", "moveable_compcart" }, // 5 in Mars City
		{ "models/mapobjects/lab/plasticjar1/plasticjar1.lwo", "moveable_plasticjar1" }, // 5 in Mars City
		{ "models/mapobjects/filler/laptop.lwo", "moveable_laptop" }, // 3 in Mars City (1)
		{ "models/mapobjects/filler/ktable.lwo", "moveable_ktable" }, // 2 in Mars City
		{ "models/mapobjects/lab/plasticbin/plasticbin.lwo", "moveable_plasticbin" }, // 2 in Mars City (2)
		{ "models/mapobjects/filler/monkeywrench.lwo", "moveable_wrench" }, // 1 in Mars City
		{ "models/mapobjects/lab/infusion/infusion.lwo", "moveable_infusion" }, // 1 in Mars City
		{ "models/mapobjects/monitors/hangingmonitor.lwo", "moveable_hangingmonitor" }, // 1 in Mars City
		{ "models/mapobjects/base/misc/fireext.ase", "moveable_fireext" }, // 1 in Mars City (1)
		{ "models/mapobjects/lamps/desklamp.lwo", "moveable_desklamp" }, // 1 in Mars City (1)
		{ "models/mapobjects/filler/monitor.ase", "moveable_monitor" }, // 1 in Mars City (1)
		{ "models/mapobjects/filler/monitorflip.lwo", "moveable_monitorflip" }, // 1 in Mars City (1)

		{ "models/mapobjects/filler/mkeyboard.lwo", "moveable_keyboard1" }, // 2 in Mars City, 3 in Erebus 1 (for cutscene?), 1 in Erebus 2 (for cutscene?)


		{ "models/mapobjects/base/tech/chair1.ase", "moveable_tech_chair1" }, //
		{ "models/mapobjects/base/chairs/chair1.lwo", "moveable_chair1" }, //
		// { "models/mapobjects/chairs/d3xp_chair2.lwo", "moveable_chair2" }, // All the chairs in Doom 3 were already moveable
		// { "models/mapobjects/chairs/d3xp_chair5.lwo", "moveable_chair5" },
		{ "models/mapobjects/base/chairs/normchair.lwo", "moveable_normchair" }, //
		{ "models/mapobjects/chairs/kitchenchair/kitchenchair.lwo", "moveable_kitchenchair" }, // moveable in Mars City
		{ "models/mapobjects/cannister/cannister.ase", "moveable_cannister" }, //
		{ "models/mapobjects/washroom/trashcan01.ase", "moveable_trashcan01" }, //
		{ "models/mapobjects/washroom/mop.lwo", "moveable_mop" }, //
		{ "models/mapobjects/washroom/mopbucket.lwo", "moveable_mopbucket" }, //
		{ "models/mapobjects/lab/beaker/beaker.lwo", "moveable_beaker" }, //
		{ "models/mapobjects/lab/Blaptop/blaptop.lwo", "moveable_blaptop" }, //
		{ "models/mapobjects/lab/plasticbinmini/plasticbinmini.lwo", "moveable_plasticbinmini" }, //
		{ "models/mapobjects/filler/tbox1.ase", "moveable_cartonbox1" }, //
		{ "models/mapobjects/filler/tbox2.ase", "moveable_cartonbox2" },
		{ "models/mapobjects/filler/tbox3.ase", "moveable_cartonbox3" },
		{ "models/mapobjects/filler/tbox4.ase", "moveable_cartonbox4" },
		{ "models/mapobjects/filler/tbox5.ase", "moveable_cartonbox5" },
		{ "models/mapobjects/filler/tbox6.ase", "moveable_cartonbox6" },
		{ "models/mapobjects/filler/tbox7.ase", "moveable_cartonbox7" },
		{ "models/mapobjects/filler/tbox8.ase", "moveable_cartonbox8" },
		{ "models/mapobjects/boxes/metalbox1.lwo", "moveable_metalbox1" },
		{ "models/mapobjects/lab/diamondbox/diamondbox_sm.lwo", "moveable_diamondbox_sm" }, //

		{ "models/mapobjects/filler/cone.ASE", "moveable_cone" }, //
		{ "models/mapobjects/filler/cola1.lwo", "moveable_cokecan" }, //
		{ "models/mapobjects/filler/cola2.lwo", "moveable_cokecan" }, // this will convert it to an unbroken coke can!
		{ "models/items/lantern/lantern_world.lwo", "moveable_item_lantern_world" }, // light on version that doesn't bounce
		// { "models/items/lantern/lantern_world.lwo", "moveable_item_lantern" }, // moveable_items.def, light off version that bounces, moveable in MarsCity 2
		// { "models/items/lantern/lantern_world_broken.lwo", "moveable_item_lantern_world" }, // commented out

		{ "models/mapobjects/filler/foamcup.lwo", "moveable_foamcup" }, //
		{ "models/mapobjects/lab/bottle1/bottle1.lwo", "moveable_bottle1" }, //
		{ "models/mapobjects/lab/microscope/microscope.lwo", "moveable_microscope" }, //
		{ "models/mapobjects/filler/pc1.ase", "moveable_pc1" }, //
		{ "models/mapobjects/lab/gizmo1/gizmo1.lwo", "moveable_gizmo1" }, //
		{ "models/mapobjects/lab/gizmo2/gizmo2.lwo", "moveable_gizmo2" }, //
		{ "models/mapobjects/lab/gizmo3/gizmo3.lwo", "moveable_gizmo3" }, //
		{ "models/mapobjects/lab/spigotcan/spigotcan.lwo", "moveable_spigotcan" }, //
		{ "models/mapobjects/lab/tablecart1/tablecart1.lwo", "moveable_tablecart1" }, //
		{ "models/mapobjects/lab/tablecart2/tablecart2.lwo", "moveable_tablecart2" }, //
		{ "models/mapobjects/lab/tray/tray.lwo", "moveable_tray" }, //
		{ "models/mal/y_utilitylampcm.ase", "moveable_utilitylamp" }, //

		{ "models/mapobjects/fuel_barrel/gbarrel.ASE", "moveable_barrel1" }, //
		{ "models/mapobjects/fuel_barrel/p_barrel.ase", "moveable_barrel2" }, //
		// { "models/mapobjects/fuel_barrel/exp_barrel.lwo", "moveable_base_barrel" },
		{ "models/mapobjects/fuel_barrel/exp_barrel.lwo", "moveable_explodingbarrel" },
		// { "models/mapobjects/fuel_barrel/exp_barrel.lwo", "moveable_burningbarrel" },
		// { "models/mapobjects/fuel_barrel/exp_barrel.lwo", "moveable_explodingbarrel_slime" },

		{ "models/mapobjects/fuel_barrel/exp_barrel2b.lwo", "debris_barrelpiece" },
		// { "models/mapobjects/fuel_barrel/exp_barrel2b.lwo", "debris_barrelpiece_slime" },
		{ "models/mapobjects/fuel_barrel/exp_barrel2c.lwo", "debris_barreltop" },
		// { "models/mapobjects/fuel_barrel/exp_barrel2c.lwo", "debris_barreltop_slime" },
		{ "models/mapobjects/fuel_barrel/exp_barrel2b.lwo", "debris_barrelpiece2" },
		{ "models/mapobjects/fuel_barrel/exp_barrel2c.lwo", "debris_barreltop2" },

		{ "models/mapobjects/washroom/airtank.lwo", "moveable_explodingtank" },
		// { "models/mapobjects/washroom/airtank.lwo", "moveable_burningtank" },

		// Carl: I added these entities and collision meshes because they were broken
		{ "models/mapobjects/filler/binder2.ase", "moveable_binder2_vr" }, // Open binder falling off edge of desk in Admin, delta2b, delta3, deltax, le_exis2
		{ "models/mapobjects/filler/binder3.ase", "moveable_binder3_vr" }, // Binder on the desk in Reception

		// moveable_items.def
		{ "models/monsters/zsecurity/zsheild.lwo", "moveable_item_shield" },
		{ "models/items/tablet/tablet_world.lwo", "moveable_item_tabletpc" },
		{ "models/characters/sarge2/w_helmet.lwo", "moveable_item_helmet" },

		{ "models/gibs/skull_pork.lwo", "moveable_item_skull_pork" },
		{ "models/gibs/head_pork.lwo", "moveable_item_head_pork" },
		{ "models/gibs/torso_pork.lwo", "moveable_item_torso_pork" },
		{ "models/gibs/rup_arm_pork.lwo", "moveable_item_rup_arm_pork"},
		{ "models/gibs/left_waist_pork.lwo", "moveable_item_left_waist_pork" },
		{ "models/gibs/lup_leg_pork.lwo", "moveable_item_lup_leg_pork" },
		{ "models/gibs/rup_leg_pork.lwo", "moveable_item_rup_leg_pork" },
		{ "models/gibs/rup2_leg_pork.lwo", "moveable_item_rup2_leg_pork" },
		{ "models/gibs/pelvis_pork.lwo", "moveable_item_pelvis_pork" },

		// vr_moveable.def
		{ "models/mapobjects/filler/lunchbag.lwo", "moveable_lunchbag" },
		

	};
	for( int i = 0; i < 77; i++ )
	{
		if( idStr::Icmp( model, lookup[i][0] ) == 0 )
		{
			common->Printf( "Converting %s to %s\n", model, lookup[i][1] );
			return lookup[i][1];
		}
	}
	return NULL;
}

// Carl: Examples of glitchy entities - falling through the world, or having magazines on top that stay when the entity is moved from underneath
bool WouldMoveableEntityBeGlitchy( const char* name, const char* mapname )
{
	// Note that we only need to check Doom 3 maps, not the expansions which were designed for the grabber
	if( idStr::Cmp( mapname, "game/mars_city1" )==0 )
	{
		if( idStr::Cmp( name, "tim_func_static_2246" ) == 0 ) // Outside world bounds warning
			return true;
		if( idStr::Cmp( name, "tim_func_static_2247" ) == 0 ) // Outside world bounds warning
			return true;
		if( idStr::Cmp( name, "tim_func_static_2263" ) == 0 ) // Outside world bounds warning
			return true;
		if( idStr::Cmp( name, "func_static_5217" ) == 0 ) // Outside world bounds warning
			return true;
		if( idStr::Cmp( name, "func_static_5219" ) == 0 ) // Outside world bounds warning
			return true;
		if( idStr::Cmp( name, "func_static_5205" ) == 0 ) // Outside world bounds warning
			return true;
		if( idStr::Cmp( name, "tim_func_static_2675" ) == 0 ) // Reception computer monitor (because it's being used)
			return true;
		if( idStr::Cmp( name, "func_static_5267" ) == 0 ) // Soul Cube computer monitor (because it falls half into the table)
			return true;
		if( idStr::Cmp( name, "mal_func_static_1908" ) == 0 ) // Lamp in meeting room (because it falls half into the table)
			return true;
		if( idStr::Cmp( name, "tim_moveable_trashcan01_5" ) == 0 ) // Rubish bin in toilets that falls through the floor
			return true;
		if( idStr::Cmp( name, "tim_func_mover_1" ) == 0 ) // TV in kitchen that falls off the wall
			return true;
		if( idStr::Cmp( name, "func_static_866" ) == 0 ) // Kitchen table with magazines on top and people sitting at it
			return true;
		if( idStr::Cmp( name, "func_static_868" ) == 0 ) // Kitchen table with magazines on top
			return true;
		if( idStr::Cmp( name, "weapon_shotgun_5" ) == 0 || idStr::Cmp( name, "weapon_shotgun_7" ) == 0 ) // Shotguns that fall on floor in Combat Prep weapon room
			return true;
		if( idStr::Cmp( name, "func_static_5202" ) == 0 ) // Keyboard that's being used in Comms before Command HQ
			return true;
		if( idStr::Cmp( name, "func_static_5227" ) == 0 ) // Keyboard that's being used in Command HQ by NPC who only appears when it's moved
			return true;
		// player_pda, item_videocd_18, tim_func_static_1711 (binder1), func_static_5283 SHOULD be moveable (dufflebag), func_static_449 (keyboard in medlab)
		// func_static_53015, func_static_53016 - tiles in corridor heading towards sarge
		// marscity_cinematic_victim has bad skin due to bonus characters (originally was tshirt guy)
	}
	else if( idStr::Cmp( mapname, "game/mc_underground" ) == 0 )
	{
		if( idStr::Cmp( name, "item_armor_security_2" ) == 0 ) // armor stops gear storage locker from opening?
			return true;
		if( idStr::Cmp( name, "weapon_pistol_1" ) == 0 ) // our pistol falls in storage locker
			return true;
		if( idStr::Cmp( name, "item_medkit_small_24" ) == 0 ) // medkit bouncing around in storage locker
			return true;
		if( idStr::Cmp( name, "ammo_clip_small_12" ) == 0 ) // ammo clip falls through left storage locker
			return true;
		if( idStr::Cmp( name, "secpanel1" ) == 0 ) // important GUI falls through table
			return true;

		if( idStr::Cmp( name, "func_static_52812" ) == 0 ) // box with magazines and lunch on top
			return true;
	}
	else if( idStr::Cmp( mapname, "game/mars_city2" ) == 0 )
	{
		//if( idStr::Cmp( name, "" ) == 0 )
		//	return true;
	}
	else if( idStr::Cmp( mapname, "game/admin" ) == 0 )
	{
		//if( idStr::Cmp( name, "" ) == 0 )
		//	return true;
	}
	return false;
}

bool BonusCharNeedsMoveables( bonus_char_t ch )
{
	return ch == BONUS_CHAR_ROE || ch == BONUS_CHAR_LE;
}

bool BonusCharNeedsFullAccess( bonus_char_t ch )
{
	return ch == BONUS_CHAR_BETRUGER || ch == BONUS_CHAR_SARGE || ch == BONUS_CHAR_SWANN || ch == BONUS_CHAR_CAMPBELL || ch == BONUS_CHAR_ROLAND;
}

void BonusCharPreUnlockDoor( idDict& spawnArgs )
{
	int locked = spawnArgs.GetInt( "locked" );
	if( locked == 2 )
	{
		const char* name = spawnArgs.GetString( "name" );
		if( idStr::Cmp( name, "mal_func_door_89" ) == 0
			|| idStr::Cmp( name, "tim_func_door_186" ) == 0 || idStr::Cmp( name, "tim_func_door_187" ) == 0
			|| idStr::Cmp( name, "tim_func_door_72" ) == 0 || idStr::Cmp( name, "tim_func_door_73" ) == 0 // listen buddy, you don't have clearance for this area
			|| idStr::Cmp( name, "tim_func_door_98" ) == 0
			|| idStr::Cmp( name, "func_door_12" ) == 0 ) // Combat Prep, which is fun to unlock, but the doorway is glitchy and it makes the game too easy
			return;
		// Roland only has access to the basement door and nothing else.
		if( bonus_char.GetInteger() == BONUS_CHAR_ROLAND && idStr::Cmp( name, "tim_func_door_219" ) != 0 && idStr::Cmp( name, "tim_func_door_220" ) != 0 )
			return;
		common->Printf( "Bonus Character: Unlocking door %s\n", name );
		spawnArgs.SetInt( "locked", 0 );
		spawnArgs.SetInt( "shaderparm7", 1 );
		// Carl: todo check what kind of gui it had before
		spawnArgs.Set( "gui", "guis/doors/aco_door_open.gui" );
		spawnArgs.Set( "gui2", "guis/doors/aco_door_open.gui" );
	}
	else if( locked == 0 )
	{
		const char* name = spawnArgs.GetString( "name" );
		// Note caverns1, commoutside, and le_enpro1 also have "func_door_15" but their movedir is different
		if( idStr::Cmp( name, "func_door_15" ) == 0 && spawnArgs.GetInt( "movedir", 5678 ) == 0 ) // door behind other locked door that was unlocked but opens into blackness
		{
			common->Printf( "Bonus Character: locking door %s\n", name );
			spawnArgs.SetInt( "locked", 2 );
			spawnArgs.SetInt( "shaderparm7", 0 );
			return;
		}
	}
}

void BonusCharPreUnlockPanel( idDict& spawnArgs )
{
	if( !spawnArgs.GetInt( "shaderparm7" ) && idStr::Cmp( spawnArgs.GetString( "gui" ), "guis/doors/elevatorcall.gui" ) != 0
		&& idStr::Cmp( spawnArgs.GetString( "gui" ), "guis/marscity/elevatorcall.gui" ) != 0
		&& idStr::Cmp( spawnArgs.GetString( "gui" ), "guis/screens/malfunction2a.gui" ) != 0
		&& idStr::Cmp( spawnArgs.GetString( "gui2" ), "guis/doors/areakeypad.gui" ) != 0 ) // Combat Prep room is fun, but glitchy and makes it too easy
	{
		const char* name = spawnArgs.GetString( "name" );
		// Roland only has access to the basement door and nothing else.
		if( bonus_char.GetInteger() == BONUS_CHAR_ROLAND && idStr::Cmp( name, "tim_func_static_2703" ) != 0 )
			return;
		common->Printf( "Bonus Character: Unlocking door panel %s\n", name );
		spawnArgs.SetInt( "shaderparm7", 1 );
		// Carl: todo check what kind of gui it had before
		spawnArgs.Set( "gui", "guis/doors/aco_door_open.gui" );
		spawnArgs.Set( "gui2", "guis/doors/aco_door_open.gui" );
	}
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
