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
	FLICK_KELLY = 7,     // Sergeant Thomas Kelly
	FLICK_BROOKS = 8,		 // Mars Sec underground who gives you equipment
	FLICK_MARK_RYAN = 9, // Guarding airlock to decommissioned comm facility
	FLICK_ISHII = 10,     // Jonathon Ishii in decomissioned com facility
	FLICK_ROLAND = 11,   // Roland, hiding in the ceiling in Mars City
	FLICK_MCNEIL = 12,   // Dr. Elizabeth McNeil from Resurrection of Evil
	FLICK_MARINE_PDA = 13, // Marine with PDA at start of RoE
	FLICK_MARINE_TORCH = 14, // Marine with Torch (and possibly others) at start of RoE
	FLICK_POINT = 15,    // Marine with explosives at start of RoE
};

bool Flicksync_Voice( const char* entity, const char* animation, const char* lineName, uint32 length );
void Flicksync_AddVoiceLines();
void Flicksync_HearLine( const char* line, int confidence, uint64 startTime, uint32 length );
void Flicksync_NewGame();
bool Flicksync_UseCueCard();
void Flicksync_Cheat();

extern idCVar vr_flicksyncCharacter;
extern idCVar vr_flicksyncCueCards;

extern int Flicksync_Score;
extern int Flicksync_CueCards;
extern int Flicksync_CorrectInARow;
extern int Flicksync_FailsInARow;
extern idStr Flicksync_CueCardText;	// What our cue card would say if we used it
extern bool Flicksync_CueCardActive;	// Are we currently using one of our Cue Card Power-Ups?
extern int Flicksync_CheatCount;	// Cheat once = warning, cheat twice it's GAME OVER!
extern bool Flicksync_GameOver;

#endif
