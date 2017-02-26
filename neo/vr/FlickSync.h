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
	FMV_UAC, // (Announcer)
	CUTSCENE_DARKSTAR, // Tower, Betruger, Swann, Campbell, Player, (Announcer, Darkstar, Dr. Caseon)
	ACTING_BIOSCAN, // Player, (Bioscanner)
	CUTSCENE_RECEPTION, // Reception, Player
	CUTSCENE_MEETING, // Betruger, Swann, Campbell, Player, Reception
	ACTING_SUITS, // Player
	ACTING_KITCHEN, // Player
	ACTING_BEFORE_SARGE, // Scientists, Player
	CUTSCENE_SARGE, // Player

	// MC Underground
	ACTING_GEARUP,
	ACTING_CRANE,
	ACTING_HEY_YOURE_LOOKING,
	ACTING_SCOTTY,
	ACTING_SNEAKING,
	ACTING_MAINTENANCE,
	ACTING_AIRLOCK,
	CUTSCENE_ISHII, // Scientists, Player
	CUTSCENE_IMP, // Player

	// Mars City 2 always plays in the background?
	ACTING_CEILING,
	ACTING_SARGE_VIDEO, // Sarge, Player

	// Admin
	CUTSCENE_ADMIN, // Swann, Campbell, Betruger, Player
	CUTSCENE_PINKY, // Player
	ACTING_OVERHEAR, // Swann, Campbell, Player

	// Alpha Labs 1
	CUTSCENE_ALPHALABS1, // Player
	// Alpha Labs 4
	CUTSCENE_VAGARY, // Player

	// Enpro
	CUTSCENE_ENPRO, // Bravo Lead, Marine PDA, Marine Torch, Point
	CUTSCENE_ENPRO_ESCAPE,

	// Recycling 1
	CUTSCENE_REVINTRO, // Player
	// Recycling 2
	CUTSCENE_MANCINTRO, // Player

	// Monorail
	CUTSCENE_MONORAIL_RAISE_COMMANDO, // Betruger
	CUTSCENE_MONORAIL_CRASH, // Player

	// Delta 2a
	CUTSCENE_DELTA_SCIENTIST, // Scientists, Player
	CUTSCENE_DELTA_TELEPORTER, // Player

	// Delta 4
	CUTSCENE_DELTA_HKINTRO, // Betruger, Player

	// Hell 1
	CUTSCENE_GUARDIAN_INTRO, // Player
	CUTSCENE_GUARDIAN_DEATH, // Player

	// CPU 1
	CUTSCENE_CAMPHUNT, // Campbell
	// CPU Boss
	CUTSCENE_CPU_BOSS, // Player

	// Hellhole
	CUTSCENE_CYBERDEMON, // Player
	CUTSCENE_ENDING, // Player, Rescue Team, Betruger, Swann

	// RoE: Erebus 1
	FMV_ROE, // (Announcer)
	CUTSCENE_ARTIFACT, // Tower, McNeil, Marine PDA, Marine Torch, Point, Player
	CUTSCENE_BLOOD, // Player
	CUTSCENE_GRABBER, // Point, Player
	// Erebus 2
	CUTSCENE_VULGARINTRO, // Player
	CUTSCENE_HUNTERINTRO, // Player
	// Erebus 5
	CUTSCENE_CLOUD, // Dr. Cloud, McNeil, Player
	CUTSCENE_ENVIROSUIT_ON, // ?
	CUTSCENE_ENVIROSUIT_OFF, // ?
	// Erebus 6
	CUTSCENE_EREBUS6_MONSTERS,
	CUTSCENE_EREBUS6_BER, // Player, (Hunters)
	CUTSCENE_EREBUS6_BER_DEATH, // Player, (Hunters)
	// Phobos 2
	CUTSCENE_PHOBOS2, // McNeil, Player
	// Hell
	CUTSCENE_HELL_MALEDICT, // Player, Betruger
	CUTSCENE_HELL_MALEDICT_DEATH, // Player, Betruger, McNeil

	// Lost Missions: Enpro
	FMV_LOST_MISSIONS, // (Announcer)
	CUTSCENE_BRAVO_TEAM, // Bravo Lead, Marine PDA, Marine Torch, Point, Player

	CUTSCENE_FLICKSYNC_COMPLETE,
	CUTSCENE_FLICKSYNC_GAMEOVER
} t_cutscene;

typedef enum {
	SCENES_ALL = 0,
	SCENES_CHAPTER = 1,
	SCENES_MYSTART = 2,
	SCENES_STORYLINE = 3,
	SCENES_MINEONLY = 4
};

bool Flicksync_Voice( const char* entity, const char* animation, const char* lineName, uint32 length );
void Flicksync_AddVoiceLines();
void Flicksync_HearLine( const char* line, int confidence, uint64 startTime, uint32 length );
void Flicksync_StoppedTalking();
void Flicksync_NewGame(bool notFlicksync);
bool Flicksync_UseCueCard();
void Flicksync_Cheat();
void Flicksync_GiveUp();
void Flicksync_StartCutscene();
bool Flicksync_EndCutscene();
bool Flicksync_NextCutscene();
void Flicksync_GoToCutscene(t_cutscene scene);
t_cutscene Flicksync_GetNextCutscene();
void Flicksync_ResumeCutscene();

extern idCVar vr_flicksyncCharacter;
extern idCVar vr_flicksyncCueCards;
extern idCVar vr_cutscenesOnly;
extern idCVar vr_flicksyncScenes;
extern idCVar vr_flicksyncSpoiler;

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
extern bool Flicksync_complete;
extern bool Flicksync_InCutscene;

extern t_cutscene Flicksync_skipToCutscene;
extern t_cutscene Flicksync_currentCutscene;

#endif
