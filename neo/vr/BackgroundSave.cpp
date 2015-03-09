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

#include "vr\vr.h" 
#include "sys\win32\win_local.h" 
#include "framework\Common_local.h" 
#include "vr\BackgroundSave.h" 

BackgroundSave vrBackgroundSave;

const int BACKGROUND_SAVE_STACK_SIZE = 256 * 1024;

/*
============================
BackgroundSave::BackgroundSave
============================
*/
BackgroundSave::BackgroundSave() {
}

/*
============================
BackgroundSave::Run
============================
*/
int BackgroundSave::Run() {
	
	if ( saveType == BACKGROUND_SAVE )
	{
		//common->SaveGame( "autosave" );
		common->SaveGame( fileName.c_str() );
	}
	else
	{
		// load file here
	}

	vr->vrIsBackgroundSaving = false; // file will be done loading or saving when this is reached.
	return 0; // this ends the thread.
}

/*
============================
BackgroundSave::StartBackgroundSave
============================
*/

void BackgroundSave::StartBackgroundSave( backgroundSaveType_t loadSave, idStr saveName ) 
{	
	
	if ( vr->vrIsBackgroundSaving )
	{
		common->Warning( "Warning: StartBackgroundSave called when already running.\n" );
		return;
	}
	
	common->Printf("Starting Background ");
	if ( loadSave == BACKGROUND_SAVE ) 
	{ 
		common->Printf( " Save.\n" );
	}
	else
	{
		common->Printf( " Load.\n" );
	}

	saveType = loadSave;
	fileName = saveName;

	//StartThread("BackgroundAutoSwaps", CORE_0B, THREAD_NORMAL, AUTO_RENDER_STACK_SIZE );
	StartThread( "BackgroundSave", CORE_ANY, THREAD_LOWEST, BACKGROUND_SAVE_STACK_SIZE );
	
	vr->vrIsBackgroundSaving = true;

}

/*
============================
BackgroundSave::BackgoundLoadGame
============================
*/
void BackgroundSave::BackgoundLoadGame()
{
}


