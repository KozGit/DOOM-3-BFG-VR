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

#ifndef __BONUS_H__
#define __BONUS_H__

typedef enum
{
	BONUS_CHAR_NONE, // Default character for campaign
	BONUS_CHAR_MARINE, // Doom 3 main campaign's Doom Marine (with Chainsaw and Soul Cube)
	BONUS_CHAR_ROE, // Doom 3 RoE campaign's character (with Grabber and Artifact)
	BONUS_CHAR_LE, // Doom 3 LE's Bravo team character (with Grabber)
	BONUS_CHAR_CAMPBELL, // Doom 3 main campaign's Campbell (with BFG)
	BONUS_CHAR_SARGE, // Sergeant Kelly (with double barrel shotgun)
	BONUS_CHAR_BETRUGER, // Malcom Betruger (with Soul Cube)
	BONUS_CHAR_SWANN, // 
	BONUS_CHAR_ROLAND, // Guy you meet in the vent shaft
	BONUS_CHAR_DOOMGUY, // Doom 1 and Doom 2's Doomguy (with Chainsaw, double barrel?)
	BONUS_CHAR_PHOBOS, // Quake 3's Phobos (a black version of Doomguy in orange armour with no special weapons)
	BONUS_CHAR_SLAYER, // Doom 2016's Doom Slayer (with praetorian armor, double jump boots)
	BONUS_CHAR_ETERNAL, // Doom Eternal's Doom Slayer (with meathook, bayonet left arm, and shoulder mounted weapon?)
	BONUS_CHAR_VFR, // Doom VFR's Combat Chasis
	BONUS_CHAR_ASH, // Ash from Evil Dead (with Chainsaw arm and boomstick)
	BONUS_CHAR_SAMUS, // Samus Aran from Metroid
	BONUS_CHAR_WITCH, // Witch who can cure zombies
} bonus_char_t;

extern idCVar bonus_char;
extern idCVar bonus_char_marine, bonus_char_roe, bonus_char_le;
extern idCVar bonus_char_campbell, bonus_char_sarge, bonus_char_betruger, bonus_char_swann, bonus_char_roland;
extern idCVar bonus_char_doomguy, bonus_char_phobos, bonus_char_slayer, bonus_char_eternal, bonus_char_vfr;
extern idCVar bonus_char_ash, bonus_char_samus, bonus_char_witch;
extern idCVar bonus_chainsaw, bonus_boomstick;
extern idCVar bonus_cataract, bonus_cataract_eye;

bool BonusCharUnlocked( bonus_char_t ch );

// Carl: For the New Game menu. See MenuScreen_Shell_NewGame.
const char *BonusCharName( bonus_char_t ch, bool unlocked );
const char *BonusCharDescription( bonus_char_t ch );
// Carl: For checking if we've unlocked things in this game by playing other games.
void BonusCheckOtherGames();

bool BonusCharHasWeapons( bonus_char_t ch );

// Carl: Give the player their signature weapons. This is probably obsolete now.
void BonusGiveSignatureWeapons( idPlayer *player, bonus_char_t ch );

// Carl: For replacing the player
const char* BonusCharDefPlayer( bonus_char_t ch, const char* default, const char* exp, bool hell );

// Carl: For replacing the NPC version of the character in cutscenes. See Anim_Blend.cpp
idStr BonusCharReplaceTShirtModel( bonus_char_t ch );
idStr BonusCharReplaceCompatibleHead( bonus_char_t ch );
idStr BonusCharSkin( idStr skinname, bonus_char_t ch );
const char* BonusCharReplaceIncompatibleHead( const char* m, bonus_char_t ch );
const char* BonusCharReplaceIncompatibleEntityClass( const char* e, bonus_char_t ch );

const char* BonusCharMesh( bonus_char_t ch );
const char* BonusCharMeshHead( bonus_char_t ch );

// Carl: For replacing props/items with moveable entities
bool BonusCharNeedsMoveables( bonus_char_t ch );
const char* ItemToMoveableEntityClass( const char* e, bonus_char_t ch );
const char* ModelToMoveableEntityClass( const char* model, bonus_char_t ch );
bool WouldMoveableEntityBeGlitchy( const char* name, const char* mapname );

bool BonusCharNeedsFullAccess( bonus_char_t ch );
void BonusCharPreUnlockDoor( idDict& spawnArgs );
void BonusCharPreUnlockPanel( idDict& spawnArgs );

#endif
