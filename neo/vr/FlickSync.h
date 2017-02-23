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

#ifndef __Flicksync_H__
#define __Flicksync_H__

typedef enum
{
	FLICK_NONE = 0,
	FLICK_BETRUGER = 1,	 // Dr. Malcom Betruger
	FLICK_SWANN = 2,     // Councillor Elliot Swann
	FLICK_CAMPBELL = 3,  // Swann's bodyguard, Jack Campbell
	FLICK_DARKSTAR = 4,  // Darkstar pilot
	FLICK_TOWER = 5,     // Air traffic controller
	FLICK_RECEPTION = 6, // Receptionist who gives you the PDA
	FLICK_SARGE = 7,     // Sergeant Thomas Kelly
	FLICK_BROOKS = 8,		 // Mars Sec underground who gives you equipment
	FLICK_MARK_RYAN = 9, // Guarding airlock to decommissioned comm facility
	FLICK_SCIENTIST = 10,     // Jonathon Ishii in decomissioned com facility, and Delta2a scientist
	FLICK_ROLAND = 11,   // Roland, hiding in the ceiling in Mars City
	FLICK_MCNEIL = 12,   // Dr. Elizabeth McNeil from Resurrection of Evil
	FLICK_MARINE_PDA = 13, // Marine with PDA at start of RoE
	FLICK_MARINE_TORCH = 14, // Marine with Torch (and possibly others) at start of RoE
	FLICK_POINT = 15,    // Marine with explosives at start of RoE
	FLICK_BRAVO_LEAD = 16,
	FLICK_PLAYER = 17,   // POV or our silent protagonist, even though we have no lines
};

typedef enum
{
	// Mars City 1
	CUTSCENE_NONE = 0,
	FMV_UAC,
	CUTSCENE_DARKSTAR,
	ACTING_BIOSCAN,
	CUTSCENE_RECEPTION,
	CUTSCENE_MEETING,
	ACTING_SUITS,
	ACTING_KITCHEN,
	ACTING_BEFORE_SARGE,
	CUTSCENE_SARGE,

	// MC Underground
	ACTING_GEARUP,
	ACTING_CRANE,
	ACTING_HEY_YOURE_LOOKING,
	ACTING_SCOTTY,
	ACTING_SNEAKING,
	ACTING_MAINTENANCE,
	ACTING_AIRLOCK,
	CUTSCENE_ISHII,
	CUTSCENE_IMP,

	// Mars City 2 always plays in the background?
	ACTING_CEILING,
	ACTING_SARGE_VIDEO,

	// Admin
	CUTSCENE_ADMIN,
	CUTSCENE_PINKY,
	ACTING_OVERHEAR,

	// Alpha Labs 1
	CUTSCENE_ALPHALABS1,
	// Alpha Labs 4
	CUTSCENE_VAGARY,

	// Monorail
	CUTSCENE_MONORAIL_RAISE_COMMANDO,
	CUTSCENE_MONORAIL_CRASH,

	// Delta 2a
	CUTSCENE_DELTA_SCIENTIST,
	CUTSCENE_DELTA_TELEPORTER,

	// Delta 4
	CUTSCENE_DELTA_HKINTRO,

	// Hell 1
	CUTSCENE_GUARDIAN_INTRO,
	CUTSCENE_GUARDIAN_DEATH,

	// CPU 1
	CUTSCENE_CAMPHUNT,
	// CPU Boss
	CUTSCENE_CPU_BOSS,

	// RoE: Erebus 1
	FMV_ROE,
	CUTSCENE_ARTIFACT,
	CUTSCENE_BLOOD,
	CUTSCENE_GRABBER,
	// Erebus 2
	CUTSCENE_VULGARINTRO,
	// Erebus 5
	CUTSCENE_ENVIROSUIT_ON,
	CUTSCENE_ENVIROSUIT_OFF,
	CUTSCENE_CLOUD,
	// Erebus 6
	// Hell
	// Phobos 2
	CUTSCENE_PHOBOS2,


	// Lost Missions: Enpro
	FMV_LOST_MISSIONS,
	CUTSCENE_BRAVO_TEAM,
} t_cutscene;

bool Flicksync_Voice( const char* entity, const char* animation, const char* lineName, uint32 length );
void Flicksync_AddVoiceLines();
void Flicksync_HearLine( const char* line, int confidence, uint64 startTime, uint32 length );
void Flicksync_StoppedTalking();
void Flicksync_NewGame();
bool Flicksync_UseCueCard();
void Flicksync_Cheat();
void Flicksync_GiveUp();
void Flicksync_StartCutscene();
bool Flicksync_EndCutscene();
void Flicksync_GoToCutscene( t_cutscene scene );
t_cutscene Flicksync_GetNextCutscene();

extern idCVar vr_flicksyncCharacter;
extern idCVar vr_flicksyncCueCards;
extern idCVar vr_cutscenesOnly;

extern int Flicksync_Score;
extern int Flicksync_CueCards;
extern int Flicksync_CorrectInARow;
extern int Flicksync_FailsInARow;
extern idStr Flicksync_CueCardText;	// What our cue card would say if we used it
extern bool Flicksync_CueCardActive;	// Are we currently using one of our Cue Card Power-Ups?
extern idStr Flicksync_CueText;
extern bool Flicksync_CueActive;
extern int Flicksync_CheatCount;	// Cheat once = warning, cheat twice it's GAME OVER!
extern bool Flicksync_GameOver;
extern bool Flicksync_InCutscene;

extern t_cutscene Flicksync_skipToCutscene;
extern t_cutscene Flicksync_currentCutscene;

#endif
