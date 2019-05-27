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

#pragma hdrstop
#include "precompiled.h"
#include "ConsoleHistory.h"
#include "../renderer/ResolutionScale.h"
#include "Common_local.h"
#include "d3xp/Game_local.h"

// Koz begin

#include "vr/Vr.h"

idCVar vr_hmdPerfHud( "vr_hmdPerfHud", "0", CVAR_INTEGER, "Oculus Performance HUD.\n 0 Off\n 1 Latency Timing\n 2 RenderTiming\n 3 Perf Headroom\n 4 Version\n 6 Async Timewarp\n", 0, 7 );
// Koz end
idCVar vr_asw( "vr_asw", "0", CVAR_INTEGER | CVAR_ARCHIVE, "Oculus Asynchronous SpaceWarp. -1 = force off (hack), 0 = default (don't interfere), 1 = enabled (hack), 2 = 45 FPS ATW (hack), 3 = 45 FPS ASW (hack)", -1, 3 );

#define	CON_TEXTSIZE			0x30000
#define	NUM_CON_TIMES			4
#define CONSOLE_FIRSTREPEAT		200
#define CONSOLE_REPEAT			100

#define	COMMAND_HISTORY			64

struct overlayText_t
{
	idStr			text;
	justify_t		justify;
	int				time;
};

// the console will query the cvar and command systems for
// command completion information

class idConsoleLocal : public idConsole
{
public:
	virtual	void		Init();
	virtual void		Shutdown();
	virtual	bool		ProcessEvent( const sysEvent_t* event, bool forceAccept );
	virtual	bool		Active();
	virtual	void		ClearNotifyLines();
	virtual void		Open();
	virtual	void		Close();
	virtual	void		Print( const char* text );
	virtual	void		Draw( bool forceFullScreen );
	
	virtual void		PrintOverlay( idOverlayHandle& handle, justify_t justify, const char* text, ... );
	
	virtual idDebugGraph* 	CreateGraph( int numItems );
	virtual void			DestroyGraph( idDebugGraph* graph );
	
	void				Dump( const char* toFile );
	void				Clear();
	
private:
	void				Resize();
	
	void				KeyDownEvent( int key );
	
	void				Linefeed();
	
	void				PageUp();
	void				PageDown();
	void				Top();
	void				Bottom();
	
	void				DrawInput();
	void				DrawNotify();
	void				DrawSolidConsole( float frac );
	
	void				Scroll();
	void				SetDisplayFraction( float frac );
	void				UpdateDisplayFraction();
	
	void				DrawTextLeftAlign( float x, float& y, const char* text, ... );
	void				DrawTextRightAlign( float x, float& y, const char* text, ... );
	
	float				DrawFPS( float y );
	float				DrawMemoryUsage( float y );
	
	// Koz 
	float				DrawVRWip( float y );
	void				DrawVRBinding();
	// Koz end
	void				DrawFlicksync( float& leftY, float& centerY );

	void				DrawOverlayText( float& leftY, float& rightY, float& centerY );
	void				DrawDebugGraphs();
	
	//============================
	
	// allow these constants to be adjusted for HMD
	int					LOCALSAFE_LEFT;
	int					LOCALSAFE_RIGHT;
	int					LOCALSAFE_TOP;
	int					LOCALSAFE_BOTTOM;
	int					LOCALSAFE_WIDTH;
	int					LOCALSAFE_HEIGHT;
	int					LINE_WIDTH;
	int					TOTAL_LINES;
	
	bool				keyCatching;
	
	short				text[CON_TEXTSIZE];
	int					current;		// line where next message will be printed
	int					x;				// offset in current line for next print
	int					display;		// bottom of console displays this line
	int					lastKeyEvent;	// time of last key event for scroll delay
	int					nextKeyEvent;	// keyboard repeat rate
	
	float				displayFrac;	// approaches finalFrac at con_speed
	float				finalFrac;		// 0.0 to 1.0 lines of console to display
	int					fracTime;		// time of last displayFrac update
	
	int					vislines;		// in scanlines
	
	int					times[NUM_CON_TIMES];	// cls.realtime time the line was generated
	// for transparent notify lines
	idVec4				color;
	
	idEditField			historyEditLines[COMMAND_HISTORY];
	
	int					nextHistoryLine;// the last line in the history buffer, not masked
	int					historyLine;	// the line being displayed from history buffer
	// will be <= nextHistoryLine
	
	idEditField			consoleField;
	
	idList< overlayText_t >	overlayText;
	idList< idDebugGraph*> debugGraphs;
	
	int					lastVirtualScreenWidth;
	int					lastVirtualScreenHeight;
	
	static idCVar		con_speed;
	static idCVar		con_notifyTime;
	static idCVar		con_noPrint;
};

static idConsoleLocal localConsole;
idConsole* console = &localConsole;

idCVar idConsoleLocal::con_speed( "con_speed", "3", CVAR_SYSTEM, "speed at which the console moves up and down" );
idCVar idConsoleLocal::con_notifyTime( "con_notifyTime", "3", CVAR_SYSTEM, "time messages are displayed onscreen when console is pulled up" );
#ifdef DEBUG
idCVar idConsoleLocal::con_noPrint( "con_noPrint", "0", CVAR_BOOL | CVAR_SYSTEM | CVAR_NOCHEAT, "print on the console but not onscreen when console is pulled up" );
#else
idCVar idConsoleLocal::con_noPrint( "con_noPrint", "1", CVAR_BOOL | CVAR_SYSTEM | CVAR_NOCHEAT, "print on the console but not onscreen when console is pulled up" );
#endif

/*
=============================================================================

	Misc stats

=============================================================================
*/

/*
==================
idConsoleLocal::DrawTextLeftAlign
==================
*/
void idConsoleLocal::DrawTextLeftAlign( float x, float& y, const char* text, ... )
{
	char string[MAX_STRING_CHARS];
	va_list argptr;
	va_start( argptr, text );
	idStr::vsnPrintf( string, sizeof( string ), text, argptr );
	va_end( argptr );
	renderSystem->DrawSmallStringExt( x, y + 2, string, colorWhite, true );
	y += SMALLCHAR_HEIGHT + 4;
}

/*
==================
idConsoleLocal::DrawTextRightAlign
==================
*/
void idConsoleLocal::DrawTextRightAlign( float x, float& y, const char* text, ... )
{
	char string[MAX_STRING_CHARS];
	va_list argptr;
	va_start( argptr, text );
	int i = idStr::vsnPrintf( string, sizeof( string ), text, argptr );
	va_end( argptr );
	renderSystem->DrawSmallStringExt( x - i * SMALLCHAR_WIDTH, y + 2, string, colorWhite, true );
	y += SMALLCHAR_HEIGHT + 4;
}




/*
==================
idConsoleLocal::DrawFPS
==================
*/
#define	FPS_FRAMES	6
float idConsoleLocal::DrawFPS( float y )
{
	static int previousTimes[FPS_FRAMES];
	static int index;
	static int previous;
	
	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	int t = Sys_Milliseconds();
	int frameTime = t - previous;
	previous = t;
	
	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;
	if( index > FPS_FRAMES )
	{
		// average multiple frames together to smooth changes out a bit
		int total = 0;
		for( int i = 0 ; i < FPS_FRAMES ; i++ )
		{
			total += previousTimes[i];
		}
		if( !total )
		{
			total = 1;
		}
		int fps = 1000000 * FPS_FRAMES / total;
		fps = ( fps + 500 ) / 1000;
		
		const char* s = va( "%ifps", fps );
		int w = (int)strlen( s ) * BIGCHAR_WIDTH;
		
		renderSystem->DrawBigStringExt( LOCALSAFE_RIGHT - w, idMath::Ftoi( y ) + 2, s, colorWhite, true );
	}
	
	y += BIGCHAR_HEIGHT + 4;
	
	// DG: "com_showFPS 2" means: show FPS only, like in classic doom3
	if( com_showFPS.GetInteger() == 2 )
	{
#if 0
		if( gameLocal.GetLocalPlayer() && gameLocal.GetLocalPlayer()->GetPhysics() )
		{
			idStr s;
			int w;
			idPlayer* player = gameLocal.GetLocalPlayer();
			y += SMALLCHAR_HEIGHT + 4;
			s.Format( "%d %d %d", (int)player->GetPhysics()->GetOrigin().x, (int)player->GetPhysics()->GetOrigin().y, (int)player->GetPhysics()->GetOrigin().z );
			w = s.LengthWithoutColors() * SMALLCHAR_WIDTH;
			renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - w, idMath::Ftoi( y ) + 2, s.c_str(), colorYellow, false );
			y += SMALLCHAR_HEIGHT + 4;
			s.Format( "%d %d %d", (int)player->firstPersonViewOrigin.x, (int)player->firstPersonViewOrigin.y, (int)player->firstPersonViewOrigin.z );
			w = s.LengthWithoutColors() * SMALLCHAR_WIDTH;
			renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - w, idMath::Ftoi( y ) + 2, s.c_str(), colorGreen, false );
			y += SMALLCHAR_HEIGHT + 4;
			s.Format( "%d", (int)(player->firstPersonViewOrigin.z - player->GetPhysics()->GetOrigin().z ) );
			w = s.LengthWithoutColors() * SMALLCHAR_WIDTH;
			renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - w, idMath::Ftoi( y ) + 2, s.c_str(), colorRed, false );
			y += SMALLCHAR_HEIGHT + 4;
			s.Format( "%d %d %d", (int)player->eyeOffset.x, (int)player->eyeOffset.y, (int)player->eyeOffset.z );
			w = s.LengthWithoutColors() * SMALLCHAR_WIDTH;
			renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - w, idMath::Ftoi( y ) + 2, s.c_str(), colorBlue, false );
			y += SMALLCHAR_HEIGHT + 4;
			s.Format( "%d", (int)commonVr->headHeightDiff );
			w = s.LengthWithoutColors() * SMALLCHAR_WIDTH;
			renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - w, idMath::Ftoi( y ) + 2, s.c_str(), colorCyan, false );
			y += SMALLCHAR_HEIGHT + 4;
		}
#endif
#if 0
		if( gameLocal.GetLocalPlayer() && gameLocal.GetLocalPlayer()->GetPhysics() && gameLocal.GetLocalPlayer()->hands[0].weapon )
		{
			idStr s;
			int w;
			idPlayer* player = gameLocal.GetLocalPlayer();
			idPlayerHand* hand = &player->hands[0];
			idWeapon* weapon = hand->weapon;
			y += SMALLCHAR_HEIGHT + 4;
			// Red: Head View
			s.Format( "eye: %d %d %d", (int)player->firstPersonViewOrigin.x, (int)player->firstPersonViewOrigin.y, (int)player->firstPersonViewOrigin.z );
			w = s.LengthWithoutColors() * SMALLCHAR_WIDTH;
			renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - w, idMath::Ftoi( y ) + 2, s.c_str(), colorGreen, false );
			y += SMALLCHAR_HEIGHT + 4;
			s.Format( "eye: %d %d %d", (int)player->GetEyePosition().x, (int)player->GetEyePosition().y, (int)player->GetEyePosition().z );
			w = s.LengthWithoutColors() * SMALLCHAR_WIDTH;
			renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - w, idMath::Ftoi( y ) + 2, s.c_str(), colorBlue, false );
			y += SMALLCHAR_HEIGHT + 4;
			s.Format( "weap: %d %d %d", (int)weapon->GetPhysics()->GetOrigin().x, (int)weapon->GetPhysics()->GetOrigin().y, (int)weapon->GetPhysics()->GetOrigin().z );
			w = s.LengthWithoutColors() * SMALLCHAR_WIDTH;
			renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - w, idMath::Ftoi( y ) + 2, s.c_str(), colorYellow, false );
			y += SMALLCHAR_HEIGHT + 4;
			s.Format( "hand: %d %d %d", (int)hand->handOrigin.x, (int)hand->handOrigin.y, (int)hand->handOrigin.z );
			w = s.LengthWithoutColors() * SMALLCHAR_WIDTH;
			renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - w, idMath::Ftoi( y ) + 2, s.c_str(), colorRed, false );
			y += SMALLCHAR_HEIGHT + 4;
			/*
			s.Format( "%d %d %d", (int)player->eyeOffset.x, (int)player->eyeOffset.y, (int)player->eyeOffset.z );
			w = s.LengthWithoutColors() * SMALLCHAR_WIDTH;
			renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - w, idMath::Ftoi( y ) + 2, s.c_str(), colorBlue, false );
			y += SMALLCHAR_HEIGHT + 4;
			s.Format( "%d", (int)commonVr->headHeightDiff );
			w = s.LengthWithoutColors() * SMALLCHAR_WIDTH;
			renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - w, idMath::Ftoi( y ) + 2, s.c_str(), colorCyan, false );
			y += SMALLCHAR_HEIGHT + 4;
			*/
		}
#endif
		return y;
	}
	// DG end
	
	// print the resolution scale so we can tell when we are at reduced resolution
	idStr resolutionText;
	resolutionScale.GetConsoleText( resolutionText );
	int w = resolutionText.Length() * BIGCHAR_WIDTH;
	renderSystem->DrawBigStringExt( LOCALSAFE_RIGHT - w, idMath::Ftoi( y ) + 2, resolutionText.c_str(), colorWhite, true );
	
	const int gameThreadTotalTime = commonLocal.GetGameThreadTotalTime();
	const int gameThreadGameTime = commonLocal.GetGameThreadGameTime();
	const int gameThreadRenderTime = commonLocal.GetGameThreadRenderTime();
	const int rendererBackEndTime = commonLocal.GetRendererBackEndMicroseconds();
	const int rendererShadowsTime = commonLocal.GetRendererShadowsMicroseconds();
	const int rendererGPUIdleTime = commonLocal.GetRendererIdleMicroseconds();
	const int rendererGPUTime = commonLocal.GetRendererGPUMicroseconds();
	const int maxTime = 16;
	
	y += SMALLCHAR_HEIGHT + 4;
	idStr timeStr;
	timeStr.Format( "%sG+RF: %4d", gameThreadTotalTime > maxTime ? S_COLOR_RED : "", gameThreadTotalTime );
	w = timeStr.LengthWithoutColors() * SMALLCHAR_WIDTH;
	renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - w, idMath::Ftoi( y ) + 2, timeStr.c_str(), colorWhite, false );
	y += SMALLCHAR_HEIGHT + 4;
	
	timeStr.Format( "%sG: %4d", gameThreadGameTime > maxTime ? S_COLOR_RED : "", gameThreadGameTime );
	w = timeStr.LengthWithoutColors() * SMALLCHAR_WIDTH;
	renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - w, idMath::Ftoi( y ) + 2, timeStr.c_str(), colorWhite, false );
	y += SMALLCHAR_HEIGHT + 4;
	
	timeStr.Format( "%sRF: %4d", gameThreadRenderTime > maxTime ? S_COLOR_RED : "", gameThreadRenderTime );
	w = timeStr.LengthWithoutColors() * SMALLCHAR_WIDTH;
	renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - w, idMath::Ftoi( y ) + 2, timeStr.c_str(), colorWhite, false );
	y += SMALLCHAR_HEIGHT + 4;
	
	timeStr.Format( "%sRB: %4.1f", rendererBackEndTime > maxTime * 1000 ? S_COLOR_RED : "", rendererBackEndTime / 1000.0f );
	w = timeStr.LengthWithoutColors() * SMALLCHAR_WIDTH;
	renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - w, idMath::Ftoi( y ) + 2, timeStr.c_str(), colorWhite, false );
	y += SMALLCHAR_HEIGHT + 4;
	
	timeStr.Format( "%sSV: %4.1f", rendererShadowsTime > maxTime * 1000 ? S_COLOR_RED : "", rendererShadowsTime / 1000.0f );
	w = timeStr.LengthWithoutColors() * SMALLCHAR_WIDTH;
	renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - w, idMath::Ftoi( y ) + 2, timeStr.c_str(), colorWhite, false );
	y += SMALLCHAR_HEIGHT + 4;
	
	timeStr.Format( "%sIDLE: %4.1f", rendererGPUIdleTime > maxTime * 1000 ? S_COLOR_RED : "", rendererGPUIdleTime / 1000.0f );
	w = timeStr.LengthWithoutColors() * SMALLCHAR_WIDTH;
	renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - w, idMath::Ftoi( y ) + 2, timeStr.c_str(), colorWhite, false );
	y += SMALLCHAR_HEIGHT + 4;
	
	timeStr.Format( "%sGPU: %4.1f", rendererGPUTime > maxTime * 1000 ? S_COLOR_RED : "", rendererGPUTime / 1000.0f );
	w = timeStr.LengthWithoutColors() * SMALLCHAR_WIDTH;
	renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - w, idMath::Ftoi( y ) + 2, timeStr.c_str(), colorWhite, false );
	
	return y + BIGCHAR_HEIGHT + 4;
}

/*
==================
idConsoleLocal::DrawMemoryUsage
==================
*/
float idConsoleLocal::DrawMemoryUsage( float y )
{
	return y;
}


/*
==================
idConsoleLocal::DrawFlicksync
==================
*/
void idConsoleLocal::DrawFlicksync( float& leftY, float& centerY )
{
	if ( ( gameLocal.inCinematic || Flicksync_InCutscene ) && !vr_hudPosLock.GetInteger() )
		centerY += BIGCHAR_HEIGHT * 3;
	if (leftY < centerY)
		leftY = centerY;

	bool inFlicksync = gameLocal.inCinematic || Flicksync_InCutscene || Flicksync_complete || Flicksync_GameOver;
	if ( inFlicksync )
	{
		const char* s = va("SCORE: %i", Flicksync_Score);
		int w = (int)strlen(s) * BIGCHAR_WIDTH;
		renderSystem->DrawBigStringExt(LOCALSAFE_LEFT + (LOCALSAFE_WIDTH - w + 4) * 0.5f, idMath::Ftoi(centerY) + 2, s, colorWhite, true);
		centerY += BIGCHAR_HEIGHT + 4;
	}
	else
	{
		// Make score less intrusive during normal play between cutscenes
		idVec4 color = idVec4( 0.5f, 0.5f, 0.5f, 0.5f );
		const char* s = va("%i", Flicksync_Score);
		int w = (int)strlen(s) * SMALLCHAR_WIDTH;
		renderSystem->DrawSmallStringExt(LOCALSAFE_LEFT + (LOCALSAFE_WIDTH - w + 4) * 0.5f, idMath::Ftoi(centerY) + 2, s, colorWhite, true);
		centerY += SMALLCHAR_HEIGHT + 4;
	}

	if ( Flicksync_CueCards > 0 && inFlicksync )
	{
		renderSystem->DrawSmallStringExt(LOCALSAFE_LEFT, idMath::Ftoi(leftY) + 2, va("Cue cards: %i", Flicksync_CueCards), colorWhite, true);
		leftY += SMALLCHAR_HEIGHT + 4;
	}

	if (Flicksync_complete)
	{
		const char* s = "FLICKSYNC COMPLETE";
		int w = (int)strlen(s) * BIGCHAR_WIDTH;
		renderSystem->DrawBigStringExt(LOCALSAFE_LEFT + (LOCALSAFE_WIDTH - w + 4) * 0.5f, idMath::Ftoi(centerY) + 2, s, colorGreen, true);
		centerY += BIGCHAR_HEIGHT + 4;
	}
	else if (Flicksync_GameOver)
	{
		const char* s = "GAME OVER";
		int w = (int)strlen(s) * BIGCHAR_WIDTH;
		renderSystem->DrawBigStringExt(LOCALSAFE_LEFT + (LOCALSAFE_WIDTH - w + 4) * 0.5f, idMath::Ftoi(centerY) + 2, s, colorRed, true);
		centerY += BIGCHAR_HEIGHT + 4;
	}

	if (Flicksync_complete || Flicksync_GameOver || (!gameLocal.inCinematic && !Flicksync_InCutscene))
		return;

	if (Flicksync_CorrectInARow > 0)
	{
		renderSystem->DrawSmallStringExt(LOCALSAFE_LEFT, idMath::Ftoi(leftY) + 2, va("Correct in a row: %i", Flicksync_CorrectInARow), colorWhite, true);
		leftY += SMALLCHAR_HEIGHT + 4;
	}
	else
	{
		idVec4 color;
		if (Flicksync_FailsInARow > 0 || Flicksync_GameOver)
			color = colorRed;
		else
			color = colorWhite;
		if (Flicksync_FailsInARow == 2)
		{
			const char* s = "FINAL WARNING";
			int w = (int)strlen(s) * BIGCHAR_WIDTH;
			renderSystem->DrawBigStringExt(LOCALSAFE_LEFT + (LOCALSAFE_WIDTH - w + 4) * 0.5f, idMath::Ftoi(centerY) + 2, s, color, true);
			centerY += BIGCHAR_HEIGHT + 4;
		}
		else
		{
			renderSystem->DrawSmallStringExt(LOCALSAFE_LEFT, idMath::Ftoi(leftY) + 2, va("Lives: %i", 3 - Flicksync_FailsInARow), color, true);
			leftY += SMALLCHAR_HEIGHT + 4;
		}
	}

	if ( Flicksync_CueCardActive )
	{
		if ( leftY < centerY )
			leftY = centerY;
		//renderSystem->DrawSmallStringExt(LOCALSAFE_LEFT, idMath::Ftoi(leftY) + 2, Flicksync_CueCardText.c_str(), colorWhite, true);
		renderSystem->SetColor( colorWhite );

		const char *text_p = Flicksync_CueCardText.c_str();
		if (*text_p == '\0')
			text_p = "( Wait )";
		const char *end_p = text_p + idStr::Length( text_p );

		while (text_p != end_p)
		{
			for (int x = 0; x < LINE_WIDTH && text_p != end_p; x++, text_p++)
			{
				if (*text_p == ' ')
					continue;
				if (*text_p == '\n')
				{
					text_p++;
					break;
				}
				renderSystem->DrawSmallChar( LOCALSAFE_LEFT + (x + 1)*SMALLCHAR_WIDTH, idMath::Ftoi(leftY), *text_p );
			}
			leftY += SMALLCHAR_HEIGHT + 4;
		}
	}
	// Subtitles for the cue spoken by the other character that we must respond to
	if (Flicksync_CueActive)
	{
		float y = LOCALSAFE_BOTTOM - 10 * (BIGCHAR_HEIGHT + 4);
		if (y < leftY)
			y = leftY;
		if (y < centerY)
			y = centerY;
		const idVec4 colorSubtitle = idVec4( 0.96f, 0.96f, 0.25f, 1.0f );
		renderSystem->SetColor( colorSubtitle );

		const char *text_p = Flicksync_CueText.c_str();
		const char *end_p = text_p + idStr::Length(text_p);

		while (text_p != end_p)
		{
			for (int x = 0; x < LINE_WIDTH && text_p != end_p; x++, text_p++)
			{
				if (*text_p == ' ')
					continue;
				if (*text_p == '\n')
				{
					text_p++;
					break;
				}
				renderSystem->DrawSmallChar(LOCALSAFE_LEFT + (x + 1)*SMALLCHAR_WIDTH, idMath::Ftoi(y), *text_p);
			}
			y += SMALLCHAR_HEIGHT + 4;
		}
		{
			const char* s = "FINAL DIALOGUE WARNING!";
			int w = (int)strlen(s) * SMALLCHAR_WIDTH;
			static int flash = 0;
			flash++;
			if (flash > 20)
				flash = 0;
			if (flash < 10)
			{
				renderSystem->DrawSmallStringExt(LOCALSAFE_LEFT + (LOCALSAFE_WIDTH - w + 4) * 0.5f, idMath::Ftoi(y) + 2, s, colorRed, true);
			}
			y += SMALLCHAR_HEIGHT + 4;
		}
	}
}

/*
==================
Koz begin
idConsoleLocal::DrawVRBinding
==================
*/
void idConsoleLocal::DrawVRBinding()
{
	
	int y = LOCALSAFE_BOTTOM - (BIGCHAR_HEIGHT + 6) * 2 ;
	int left = LOCALSAFE_LEFT + BIGCHAR_WIDTH * 6;
	int maxChar = (LOCALSAFE_RIGHT - left) / SMALLCHAR_WIDTH;
	idStr temp;
	if ( commonVr->currentBindingDisplay.Icmp( "" ) != 0 )
	{
		renderSystem->DrawSmallStringExt( LOCALSAFE_LEFT + SMALLCHAR_WIDTH * 28 , y - 4 , "Current Bindings:", colorWhite, true );
		y += SMALLCHAR_HEIGHT + 4;
		temp = commonVr->currentBindingDisplay.Left( maxChar );
		renderSystem->DrawSmallStringExt(left, y, temp.c_str(), colorWhite, true );
		if ( commonVr->currentBindingDisplay.Length() > maxChar )
		{
			y += SMALLCHAR_HEIGHT + 4;
			temp = commonVr->currentBindingDisplay.Mid( maxChar, maxChar );
			renderSystem->DrawSmallStringExt( left, y, temp.c_str(), colorWhite, true );
		}
	}
	return;

}

/*
==================
Koz begin
idConsoleLocal::DrawVRWip
==================
*/
float idConsoleLocal::DrawVRWip( float y )
{
	

	y += BIGCHAR_HEIGHT * 10;
	

	idStr stepStr;
	stepStr.Format( "Num Steps: %d", commonVr->wipNumSteps );
	int w = stepStr.Length() * SMALLCHAR_WIDTH;
	renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - 150 - w , idMath::Ftoi( y ) + 2, stepStr.c_str(), colorWhite, true );
	y += SMALLCHAR_HEIGHT + 4;

	switch ( commonVr->wipStepState )
	{
		case 3:
			stepStr = S_COLOR_RED;
			stepStr += "Waiting";
			break;
		case 2:
			stepStr = S_COLOR_RED;
			stepStr += "HalfStepWaiting";
			break;
		
		case 1:
			stepStr = S_COLOR_BLUE;
			stepStr += "HalfStepStarted";
			break;
		case 0:
			stepStr = S_COLOR_GREEN;
			stepStr += "Stepping";
			break;
		default:
			stepStr = S_COLOR_WHITE;
			stepStr += "????";
	}
		
	w = stepStr.LengthWithoutColors() * SMALLCHAR_WIDTH;
	renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - 150 - w, idMath::Ftoi( y ) + 2, stepStr.c_str(), colorWhite, false );
	y += SMALLCHAR_HEIGHT + 4;

	stepStr.Format( "LPeriod: %d", commonVr->wipLastPeriod );
	w = stepStr.LengthWithoutColors() * SMALLCHAR_WIDTH;
	renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - 150 - w, idMath::Ftoi( y ) + 2, stepStr.c_str(), colorWhite, false );
	y += SMALLCHAR_HEIGHT + 4;
	
	stepStr.Format( "LPeriod Vel: %f", commonVr->wipPeriodVel );
	w = stepStr.LengthWithoutColors() * SMALLCHAR_WIDTH;
	renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - 150 - w, idMath::Ftoi( y ) + 2, stepStr.c_str(), colorWhite, false );
	y += SMALLCHAR_HEIGHT + 4;
	
	stepStr.Format( "Avg Period: %f", commonVr->wipAvgPeriod );
	w = stepStr.LengthWithoutColors() * SMALLCHAR_WIDTH;
	renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - 150 - w, idMath::Ftoi( y ) + 2, stepStr.c_str(), colorWhite, false );
	y += SMALLCHAR_HEIGHT + 4;

	stepStr.Format( "Cur Delta %f", commonVr->wipCurrentDelta );
	w = stepStr.LengthWithoutColors() * SMALLCHAR_WIDTH;
	renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - 150 - w, idMath::Ftoi( y ) + 2, stepStr.c_str(), colorWhite, false );
	y += SMALLCHAR_HEIGHT + 4;

	stepStr.Format( "Cur Vel %f", commonVr->wipCurrentVelocity );
	w = stepStr.LengthWithoutColors() * SMALLCHAR_WIDTH;
	renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - 150 - w, idMath::Ftoi( y ) + 2, stepStr.c_str(), colorWhite, false );
	y += SMALLCHAR_HEIGHT + 4;

	stepStr.Format( "Delta Tot %f", commonVr->wipTotalDelta );
	w = stepStr.LengthWithoutColors() * SMALLCHAR_WIDTH;
	renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - 150 - w, idMath::Ftoi( y ) + 2, stepStr.c_str(), colorWhite, false );
	y += SMALLCHAR_HEIGHT + 4;

	stepStr.Format( "Delta Tot Avg %f", commonVr->wipTotalDeltaAvg );
	w = stepStr.LengthWithoutColors() * SMALLCHAR_WIDTH;
	renderSystem->DrawSmallStringExt( LOCALSAFE_RIGHT - 150 - w, idMath::Ftoi( y ) + 2, stepStr.c_str(), colorWhite, false );
	y += SMALLCHAR_HEIGHT + 4;







	
	//wipLastAcces = 0.0f;


	/*stepStr.Format( "%sSV: %4.1f", rendererShadowsTime > maxTime * 1000 ? S_COLOR_RED : "", rendererShadowsTime / 1000.0f );
	w = stepStr.LengthWithoutColors() * BIGCHAR_WIDTH;
	renderSystem->DrawBigStringExt( LOCALSAFE_RIGHT - 100 - w, idMath::Ftoi( y ) + 2, stepStr.c_str(), colorWhite, false );
	y += BIGCHAR_HEIGHT + 4;

	stepStr.Format( "%sIDLE: %4.1f", rendererGPUIdleTime > maxTime * 1000 ? S_COLOR_RED : "", rendererGPUIdleTime / 1000.0f );
	w = stepStr.LengthWithoutColors() * BIGCHAR_WIDTH;
	renderSystem->DrawBigStringExt( LOCALSAFE_RIGHT - 100 - w, idMath::Ftoi( y ) + 2, stepStr.c_str(), colorWhite, false );
	y += BIGCHAR_HEIGHT + 4;

	stepStr.Format( "%sGPU: %4.1f", rendererGPUTime > maxTime * 1000 ? S_COLOR_RED : "", rendererGPUTime / 1000.0f );
	w = stepStr.LengthWithoutColors() * BIGCHAR_WIDTH;
	renderSystem->DrawBigStringExt( LOCALSAFE_RIGHT - 100 - w, idMath::Ftoi( y ) + 2, stepStr.c_str(), colorWhite, false );
	*/

	return y + BIGCHAR_HEIGHT + 4;
}
// Koz end



//=========================================================================

/*
==============
Con_Clear_f
==============
*/
static void Con_Clear_f( const idCmdArgs& args )
{
	localConsole.Clear();
}

/*
==============
Con_Dump_f
==============
*/
static void Con_Dump_f( const idCmdArgs& args )
{
	if( args.Argc() != 2 )
	{
		common->Printf( "usage: conDump <filename>\n" );
		return;
	}
	
	idStr fileName = args.Argv( 1 );
	fileName.DefaultFileExtension( ".txt" );
	
	common->Printf( "Dumped console text to %s.\n", fileName.c_str() );
	
	localConsole.Dump( fileName.c_str() );
}

/*
==============
idConsoleLocal::Init
==============
*/
void idConsoleLocal::Init()
{
	int		i;
	
	keyCatching = false;
	
	// Koz begin - in vr move the console in. 
	if ( game->isVR )
	{
		LOCALSAFE_LEFT = 180;
		LOCALSAFE_TOP = 150;
	}
	else
	{
		LOCALSAFE_LEFT = 32;
		LOCALSAFE_TOP = 24;
	}

	LOCALSAFE_RIGHT = SCREEN_WIDTH - LOCALSAFE_LEFT;
	LOCALSAFE_BOTTOM = SCREEN_HEIGHT - LOCALSAFE_TOP;
	LOCALSAFE_WIDTH = LOCALSAFE_RIGHT - LOCALSAFE_LEFT;
	LOCALSAFE_HEIGHT = LOCALSAFE_BOTTOM - LOCALSAFE_TOP;
	// Koz end
	
	LINE_WIDTH = ( ( LOCALSAFE_WIDTH / SMALLCHAR_WIDTH ) - 2 );
	TOTAL_LINES = ( CON_TEXTSIZE / LINE_WIDTH );
	
	lastKeyEvent = -1;
	nextKeyEvent = CONSOLE_FIRSTREPEAT;
	
	consoleField.Clear();
	consoleField.SetWidthInChars( LINE_WIDTH );
	
	for( i = 0 ; i < COMMAND_HISTORY ; i++ )
	{
		historyEditLines[i].Clear();
		historyEditLines[i].SetWidthInChars( LINE_WIDTH );
	}
	
	cmdSystem->AddCommand( "clear", Con_Clear_f, CMD_FL_SYSTEM, "clears the console" );
	cmdSystem->AddCommand( "conDump", Con_Dump_f, CMD_FL_SYSTEM, "dumps the console text to a file" );
}

/*
==============
idConsoleLocal::Shutdown
==============
*/
void idConsoleLocal::Shutdown()
{
	cmdSystem->RemoveCommand( "clear" );
	cmdSystem->RemoveCommand( "conDump" );
	
	debugGraphs.DeleteContents( true );
}

/*
================
idConsoleLocal::Active
================
*/
bool	idConsoleLocal::Active()
{
	return keyCatching;
}

/*
================
idConsoleLocal::ClearNotifyLines
================
*/
void	idConsoleLocal::ClearNotifyLines()
{
	int		i;
	
	for( i = 0 ; i < NUM_CON_TIMES ; i++ )
	{
		times[i] = 0;
	}
}

/*
================
idConsoleLocal::Open
================
*/
void	idConsoleLocal::Open()
{
	if( keyCatching )
		return; // already open
		
	consoleField.ClearAutoComplete();
	consoleField.Clear();
	keyCatching = true;
	SetDisplayFraction( 0.5f );
}

/*
================
idConsoleLocal::Close
================
*/
void	idConsoleLocal::Close()
{
	keyCatching = false;
	SetDisplayFraction( 0 );
	displayFrac = 0;	// don't scroll to that point, go immediately
	ClearNotifyLines();
}

/*
================
idConsoleLocal::Clear
================
*/
void idConsoleLocal::Clear()
{
	int		i;
	
	for( i = 0 ; i < CON_TEXTSIZE ; i++ )
	{
		text[i] = ( idStr::ColorIndex( C_COLOR_CYAN ) << 8 ) | ' ';
	}
	
	Bottom();		// go to end
}

/*
================
idConsoleLocal::Dump

Save the console contents out to a file
================
*/
void idConsoleLocal::Dump( const char* fileName )
{
	int		l, x, i;
	short* 	line;
	idFile* f;
	char*	 buffer = ( char* )alloca( LINE_WIDTH + 3 );
	
	f = fileSystem->OpenFileWrite( fileName );
	if( !f )
	{
		common->Warning( "couldn't open %s", fileName );
		return;
	}
	
	// skip empty lines
	l = current - TOTAL_LINES + 1;
	if( l < 0 )
	{
		l = 0;
	}
	for( ; l <= current ; l++ )
	{
		line = text + ( l % TOTAL_LINES ) * LINE_WIDTH;
		for( x = 0; x < LINE_WIDTH; x++ )
			if( ( line[x] & 0xff ) > ' ' )
				break;
		if( x != LINE_WIDTH )
			break;
	}
	
	// write the remaining lines
	for( ; l <= current; l++ )
	{
		line = text + ( l % TOTAL_LINES ) * LINE_WIDTH;
		for( i = 0; i < LINE_WIDTH; i++ )
		{
			buffer[i] = line[i] & 0xff;
		}
		for( x = LINE_WIDTH - 1; x >= 0; x-- )
		{
			if( buffer[x] <= ' ' )
			{
				buffer[x] = 0;
			}
			else
			{
				break;
			}
		}
		buffer[x + 1] = '\r';
		buffer[x + 2] = '\n';
		buffer[x + 3] = 0;
		f->Write( buffer, (int)strlen( buffer ) );
	}
	
	fileSystem->CloseFile( f );
}

/*
==============
idConsoleLocal::Resize
==============
*/
void idConsoleLocal::Resize()
{
	if( renderSystem->GetVirtualWidth() == lastVirtualScreenWidth && renderSystem->GetVirtualHeight() == lastVirtualScreenHeight )
		return;
		
	lastVirtualScreenWidth = renderSystem->GetVirtualWidth();
	lastVirtualScreenHeight = renderSystem->GetVirtualHeight();
	LOCALSAFE_RIGHT		= renderSystem->GetVirtualWidth() - LOCALSAFE_LEFT;
	LOCALSAFE_BOTTOM	= renderSystem->GetVirtualHeight() - LOCALSAFE_TOP;
	LOCALSAFE_WIDTH		= LOCALSAFE_RIGHT - LOCALSAFE_LEFT;
	LOCALSAFE_HEIGHT	= LOCALSAFE_BOTTOM - LOCALSAFE_TOP;
}

/*
================
idConsoleLocal::PageUp
================
*/
void idConsoleLocal::PageUp()
{
	display -= 2;
	if( current - display >= TOTAL_LINES )
	{
		display = current - TOTAL_LINES + 1;
	}
}

/*
================
idConsoleLocal::PageDown
================
*/
void idConsoleLocal::PageDown()
{
	display += 2;
	if( display > current )
	{
		display = current;
	}
}

/*
================
idConsoleLocal::Top
================
*/
void idConsoleLocal::Top()
{
	display = 0;
}

/*
================
idConsoleLocal::Bottom
================
*/
void idConsoleLocal::Bottom()
{
	display = current;
}


/*
=============================================================================

CONSOLE LINE EDITING

==============================================================================
*/

/*
====================
KeyDownEvent

Handles history and console scrollback
====================
*/
void idConsoleLocal::KeyDownEvent( int key )
{

	// Execute F key bindings
	if( key >= K_F1 && key <= K_F12 )
	{
		idKeyInput::ExecKeyBinding( key );
		return;
	}
	
	// ctrl-L clears screen
	if( key == K_L && ( idKeyInput::IsDown( K_LCTRL ) || idKeyInput::IsDown( K_RCTRL ) ) )
	{
		Clear();
		return;
	}
	
	// enter finishes the line
	if( key == K_ENTER || key == K_KP_ENTER )
	{
	
		common->Printf( "]%s\n", consoleField.GetBuffer() );
		
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, consoleField.GetBuffer() );	// valid command
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "\n" );
		
		// copy line to history buffer
		
		if( consoleField.GetBuffer()[ 0 ] != '\n' && consoleField.GetBuffer()[ 0 ] != '\0' )
		{
			consoleHistory.AddToHistory( consoleField.GetBuffer() );
		}
		
		consoleField.Clear();
		consoleField.SetWidthInChars( LINE_WIDTH );
		
		const bool captureToImage = false;
		common->UpdateScreen( captureToImage );// force an update, because the command
		// may take some time
		return;
	}
	
	// command completion
	
	if( key == K_TAB )
	{
		consoleField.AutoComplete();
		return;
	}
	
	// command history (ctrl-p ctrl-n for unix style)
	
	if( ( key == K_UPARROW ) ||
			( key == K_P && ( idKeyInput::IsDown( K_LCTRL ) || idKeyInput::IsDown( K_RCTRL ) ) ) )
	{
		idStr hist = consoleHistory.RetrieveFromHistory( true );
		if( !hist.IsEmpty() )
		{
			consoleField.SetBuffer( hist );
		}
		return;
	}
	
	if( ( key == K_DOWNARROW ) ||
			( key == K_N && ( idKeyInput::IsDown( K_LCTRL ) || idKeyInput::IsDown( K_RCTRL ) ) ) )
	{
		idStr hist = consoleHistory.RetrieveFromHistory( false );
		if( !hist.IsEmpty() )
		{
			consoleField.SetBuffer( hist );
		}
		else // DG: if no more lines are in the history, show a blank line again
		{
			consoleField.Clear();
		} // DG end
		
		return;
	}
	
	// console scrolling
	if( key == K_PGUP )
	{
		PageUp();
		lastKeyEvent = eventLoop->Milliseconds();
		nextKeyEvent = CONSOLE_FIRSTREPEAT;
		return;
	}
	
	if( key == K_PGDN )
	{
		PageDown();
		lastKeyEvent = eventLoop->Milliseconds();
		nextKeyEvent = CONSOLE_FIRSTREPEAT;
		return;
	}
	
	if( key == K_MWHEELUP )
	{
		PageUp();
		return;
	}
	
	if( key == K_MWHEELDOWN )
	{
		PageDown();
		return;
	}
	
	// ctrl-home = top of console
	if( key == K_HOME && ( idKeyInput::IsDown( K_LCTRL ) || idKeyInput::IsDown( K_RCTRL ) ) )
	{
		Top();
		return;
	}
	
	// ctrl-end = bottom of console
	if( key == K_END && ( idKeyInput::IsDown( K_LCTRL ) || idKeyInput::IsDown( K_RCTRL ) ) )
	{
		Bottom();
		return;
	}
	
	// pass to the normal editline routine
	consoleField.KeyDownEvent( key );
}

/*
==============
Scroll
deals with scrolling text because we don't have key repeat
==============
*/
void idConsoleLocal::Scroll( )
{
	if( lastKeyEvent == -1 || ( lastKeyEvent + 200 ) > eventLoop->Milliseconds() )
	{
		return;
	}
	// console scrolling
	if( idKeyInput::IsDown( K_PGUP ) )
	{
		PageUp();
		nextKeyEvent = CONSOLE_REPEAT;
		return;
	}
	
	if( idKeyInput::IsDown( K_PGDN ) )
	{
		PageDown();
		nextKeyEvent = CONSOLE_REPEAT;
		return;
	}
}

/*
==============
SetDisplayFraction

Causes the console to start opening the desired amount.
==============
*/
void idConsoleLocal::SetDisplayFraction( float frac )
{
	finalFrac = frac;
	fracTime = Sys_Milliseconds();
}

/*
==============
UpdateDisplayFraction

Scrolls the console up or down based on conspeed
==============
*/
void idConsoleLocal::UpdateDisplayFraction()
{
	if( con_speed.GetFloat() <= 0.1f )
	{
		fracTime = Sys_Milliseconds();
		displayFrac = finalFrac;
		return;
	}
	
	// scroll towards the destination height
	if( finalFrac < displayFrac )
	{
		displayFrac -= con_speed.GetFloat() * ( Sys_Milliseconds() - fracTime ) * 0.001f;
		if( finalFrac > displayFrac )
		{
			displayFrac = finalFrac;
		}
		fracTime = Sys_Milliseconds();
	}
	else if( finalFrac > displayFrac )
	{
		displayFrac += con_speed.GetFloat() * ( Sys_Milliseconds() - fracTime ) * 0.001f;
		if( finalFrac < displayFrac )
		{
			displayFrac = finalFrac;
		}
		fracTime = Sys_Milliseconds();
	}
}

/*
==============
ProcessEvent
==============
*/
bool	idConsoleLocal::ProcessEvent( const sysEvent_t* event, bool forceAccept )
{
	const bool consoleKey = event->evType == SE_KEY && event->evValue == K_GRAVE && com_allowConsole.GetBool();
	
	// we always catch the console key event
	if( !forceAccept && consoleKey )
	{
		// ignore up events
		if( event->evValue2 == 0 )
		{
			return true;
		}
		
		consoleField.ClearAutoComplete();
		
		// a down event will toggle the destination lines
		if( keyCatching )
		{
			Close();
			Sys_GrabMouseCursor( true );
		}
		else
		{
			consoleField.Clear();
			keyCatching = true;
			if( idKeyInput::IsDown( K_LSHIFT ) || idKeyInput::IsDown( K_RSHIFT ) )
			{
				// if the shift key is down, don't open the console as much
				SetDisplayFraction( 0.2f );
			}
			else
			{
				SetDisplayFraction( 0.5f );
			}
		}
		return true;
	}
	
	// if we aren't key catching, dump all the other events
	if( !forceAccept && !keyCatching )
	{
		return false;
	}
	
	// handle key and character events
	if( event->evType == SE_CHAR )
	{
		// never send the console key as a character
		if( event->evValue != '`' && event->evValue != '~' )
		{
			consoleField.CharEvent( event->evValue );
		}
		return true;
	}
	
	if( event->evType == SE_KEY )
	{
		// ignore up key events
		if( event->evValue2 == 0 )
		{
			return true;
		}
		
		KeyDownEvent( event->evValue );
		return true;
	}
	
	// we don't handle things like mouse, joystick, and network packets
	return false;
}

/*
==============================================================================

PRINTING

==============================================================================
*/

/*
===============
Linefeed
===============
*/
void idConsoleLocal::Linefeed()
{
	int		i;
	
	// mark time for transparent overlay
	if( current >= 0 )
	{
		times[current % NUM_CON_TIMES] = Sys_Milliseconds();
	}
	
	x = 0;
	if( display == current )
	{
		display++;
	}
	current++;
	for( i = 0; i < LINE_WIDTH; i++ )
	{
		int offset = ( ( unsigned int )current % TOTAL_LINES ) * LINE_WIDTH + i;
		text[offset] = ( idStr::ColorIndex( C_COLOR_CYAN ) << 8 ) | ' ';
	}
}


/*
================
Print

Handles cursor positioning, line wrapping, etc
================
*/
void idConsoleLocal::Print( const char* txt )
{
	int		y;
	int		c, l;
	int		color;
	
	if( TOTAL_LINES == 0 )
	{
		// not yet initialized
		return;
	}
	
	color = idStr::ColorIndex( C_COLOR_CYAN );
	
	while( ( c = *( const unsigned char* )txt ) != 0 )
	{
		if( idStr::IsColor( txt ) )
		{
			if( *( txt + 1 ) == C_COLOR_DEFAULT )
			{
				color = idStr::ColorIndex( C_COLOR_CYAN );
			}
			else
			{
				color = idStr::ColorIndex( *( txt + 1 ) );
			}
			txt += 2;
			continue;
		}
		
		y = current % TOTAL_LINES;
		
		// if we are about to print a new word, check to see
		// if we should wrap to the new line
		if( c > ' ' && ( x == 0 || text[y * LINE_WIDTH + x - 1] <= ' ' ) )
		{
			// count word length
			for( l = 0 ; l < LINE_WIDTH ; l++ )
			{
				if( txt[l] <= ' ' )
				{
					break;
				}
			}
			
			// word wrap
			if( l != LINE_WIDTH && ( x + l >= LINE_WIDTH ) )
			{
				Linefeed();
			}
		}
		
		txt++;
		
		switch( c )
		{
			case '\n':
				Linefeed();
				break;
			case '\t':
				do
				{
					text[y * LINE_WIDTH + x] = ( color << 8 ) | ' ';
					x++;
					if( x >= LINE_WIDTH )
					{
						Linefeed();
						x = 0;
					}
				}
				while( x & 3 );
				break;
			case '\r':
				x = 0;
				break;
			default:	// display character and advance
				text[y * LINE_WIDTH + x] = ( color << 8 ) | c;
				x++;
				if( x >= LINE_WIDTH )
				{
					Linefeed();
					x = 0;
				}
				break;
		}
	}
	
	
	// mark time for transparent overlay
	if( current >= 0 )
	{
		times[current % NUM_CON_TIMES] = Sys_Milliseconds();
	}
}


/*
==============================================================================

DRAWING

==============================================================================
*/


/*
================
DrawInput

Draw the editline after a ] prompt
================
*/
void idConsoleLocal::DrawInput()
{
	int y, autoCompleteLength;
	
	y = vislines - ( SMALLCHAR_HEIGHT * 2 );
	
	if( consoleField.GetAutoCompleteLength() != 0 )
	{
		autoCompleteLength = (int)strlen( consoleField.GetBuffer() ) - consoleField.GetAutoCompleteLength();
		
		if( autoCompleteLength > 0 )
		{
			renderSystem->DrawFilled( idVec4( 0.8f, 0.2f, 0.2f, 0.45f ),
									  LOCALSAFE_LEFT + 2 * SMALLCHAR_WIDTH + consoleField.GetAutoCompleteLength() * SMALLCHAR_WIDTH,
									  y + 2, autoCompleteLength * SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT - 2 );
		}
	}
	
	renderSystem->SetColor( idStr::ColorForIndex( C_COLOR_CYAN ) );
	
	renderSystem->DrawSmallChar( LOCALSAFE_LEFT + 1 * SMALLCHAR_WIDTH, y, ']' );
	
	consoleField.Draw( LOCALSAFE_LEFT + 2 * SMALLCHAR_WIDTH, y, renderSystem->GetVirtualWidth() - 3 * SMALLCHAR_WIDTH, true );
}


/*
================
DrawNotify

Draws the last few lines of output transparently over the game top
================
*/
void idConsoleLocal::DrawNotify()
{
	int		x, v;
	short*	text_p;
	int		i;
	int		time;
	int		currentColor;
	
	if( con_noPrint.GetBool() )
	{
		return;
	}
	
	currentColor = idStr::ColorIndex( C_COLOR_WHITE );
	renderSystem->SetColor( idStr::ColorForIndex( currentColor ) );
	
	v = 0;
	for( i = current - NUM_CON_TIMES + 1; i <= current; i++ )
	{
		if( i < 0 )
		{
			continue;
		}
		time = times[i % NUM_CON_TIMES];
		if( time == 0 )
		{
			continue;
		}
		time = Sys_Milliseconds() - time;
		if( time > con_notifyTime.GetFloat() * 1000 )
		{
			continue;
		}
		text_p = text + ( i % TOTAL_LINES ) * LINE_WIDTH;
		
		for( x = 0; x < LINE_WIDTH; x++ )
		{
			if( ( text_p[x] & 0xff ) == ' ' )
			{
				continue;
			}
			if( idStr::ColorIndex( text_p[x] >> 8 ) != currentColor )
			{
				currentColor = idStr::ColorIndex( text_p[x] >> 8 );
				renderSystem->SetColor( idStr::ColorForIndex( currentColor ) );
			}
			renderSystem->DrawSmallChar( LOCALSAFE_LEFT + ( x + 1 )*SMALLCHAR_WIDTH, v, text_p[x] & 0xff );
		}
		
		v += SMALLCHAR_HEIGHT;
	}
	
	renderSystem->SetColor( colorCyan );
}

/*
================
DrawSolidConsole

Draws the console with the solid background
================
*/
void idConsoleLocal::DrawSolidConsole( float frac )
{
	int				i, x;
	float			y;
	int				rows;
	short*			text_p;
	int				row;
	int				lines;
	int				currentColor;
	
	lines = idMath::Ftoi( renderSystem->GetVirtualHeight() * frac );
	if( lines <= 0 )
	{
		return;
	}
	
	if( lines > renderSystem->GetVirtualHeight() )
	{
		lines = renderSystem->GetVirtualHeight();
	}
	
	// draw the background
	y = frac * renderSystem->GetVirtualHeight() - 2;
	if( y < 1.0f )
	{
		y = 0.0f;
	}
	else
	{
		renderSystem->DrawFilled( idVec4( 0.0f, 0.0f, 0.0f, 0.75f ), 0, 0, renderSystem->GetVirtualWidth(), y );
	}
	
	renderSystem->DrawFilled( colorCyan, 0, y, renderSystem->GetVirtualWidth(), 2 );
	
	// draw the version number
	
	renderSystem->SetColor( idStr::ColorForIndex( C_COLOR_CYAN ) );
	
	idStr version = va( "%s.%i.%i", ENGINE_VERSION, BUILD_NUMBER, BUILD_NUMBER_MINOR );
	i = version.Length();
	
	for( x = 0; x < i; x++ )
	{
		renderSystem->DrawSmallChar( LOCALSAFE_WIDTH - ( i - x ) * SMALLCHAR_WIDTH,
									 ( lines - ( SMALLCHAR_HEIGHT + SMALLCHAR_HEIGHT / 4 ) ), version[x] );
									 
	}
	
	
	// draw the text
	vislines = lines;
	rows = ( lines - SMALLCHAR_WIDTH ) / SMALLCHAR_WIDTH;		// rows of text to draw
	
	y = lines - ( SMALLCHAR_HEIGHT * 3 );
	
	// draw from the bottom up
	if( display != current )
	{
		// draw arrows to show the buffer is backscrolled
		renderSystem->SetColor( idStr::ColorForIndex( C_COLOR_CYAN ) );
		for( x = 0; x < LINE_WIDTH; x += 4 )
		{
			renderSystem->DrawSmallChar( LOCALSAFE_LEFT + ( x + 1 )*SMALLCHAR_WIDTH, idMath::Ftoi( y ), '^' );
		}
		y -= SMALLCHAR_HEIGHT;
		rows--;
	}
	
	row = display;
	
	if( x == 0 )
	{
		row--;
	}
	
	currentColor = idStr::ColorIndex( C_COLOR_WHITE );
	renderSystem->SetColor( idStr::ColorForIndex( currentColor ) );
	
	for( i = 0; i < rows; i++, y -= SMALLCHAR_HEIGHT, row-- )
	{
		if( row < 0 )
		{
			break;
		}
		if( current - row >= TOTAL_LINES )
		{
			// past scrollback wrap point
			continue;
		}
		
		text_p = text + ( row % TOTAL_LINES ) * LINE_WIDTH;
		
		for( x = 0; x < LINE_WIDTH; x++ )
		{
			if( ( text_p[x] & 0xff ) == ' ' )
			{
				continue;
			}
			
			if( idStr::ColorIndex( text_p[x] >> 8 ) != currentColor )
			{
				currentColor = idStr::ColorIndex( text_p[x] >> 8 );
				renderSystem->SetColor( idStr::ColorForIndex( currentColor ) );
			}
			renderSystem->DrawSmallChar( LOCALSAFE_LEFT + ( x + 1 )*SMALLCHAR_WIDTH, idMath::Ftoi( y ), text_p[x] & 0xff );
		}
	}
	
	// draw the input prompt, user text, and cursor if desired
	DrawInput();
	
	renderSystem->SetColor( colorCyan );
}


/*
==============
Draw

ForceFullScreen is used by the editor
==============
*/
void idConsoleLocal::Draw( bool forceFullScreen )
{

	Resize();
	
	if( forceFullScreen )
	{
		// if we are forced full screen because of a disconnect,
		// we want the console closed when we go back to a session state
		Close();
		// we are however catching keyboard input
		keyCatching = true;
	}
	
	Scroll();
	
	UpdateDisplayFraction();
	
	if( forceFullScreen )
	{
		DrawSolidConsole( 1.0f );
	}
	else if( displayFrac )
	{
		DrawSolidConsole( displayFrac );
	}
	else
	{
		// only draw the notify lines if the developer cvar is set,
		// or we are a debug build
		if( !con_noPrint.GetBool() )
		{
			DrawNotify();
		}
	}
	
	float lefty = LOCALSAFE_TOP;
	float righty = LOCALSAFE_TOP;
	float centery = LOCALSAFE_TOP;
	if( com_showFPS.GetBool() )
	{
		righty = DrawFPS( righty );
	}
	if( com_showMemoryUsage.GetBool() )
	{
		righty = DrawMemoryUsage( righty );
	}
	// Koz
	DrawVRBinding();
	if ( vr_showWIP.GetBool() )
	{
		righty = DrawVRWip( righty );
	}
	// Koz end
	if ( vr_flicksyncCharacter.GetInteger() )
	{
		DrawFlicksync( lefty, centery );
	}

	DrawOverlayText( lefty, righty, centery );
	DrawDebugGraphs();

	// Koz begin
	// Enable the oculus performance hud
	if ( vr_hmdPerfHud.IsModified() )
	{
#ifdef USE_OVR
		ovr_SetInt( commonVr->hmdSession, "PerfHudMode", vr_hmdPerfHud.GetInteger() );
#endif
		vr_hmdPerfHud.ClearModified();
	}
	// Koz end
	if ( vr_asw.IsModified() )
	{
		vr_asw.ClearModified();
#ifdef _WIN32
		INPUT i[4];
		i[0].type = INPUT_KEYBOARD;
		i[0].ki.dwFlags = KEYEVENTF_SCANCODE;
		i[0].ki.wVk = VK_LCONTROL;
		i[0].ki.wScan = 0x1D; // Left Ctrl
		i[0].ki.dwExtraInfo = 0;
		i[0].ki.time = 0;
		i[1] = i[0];
		switch (vr_asw.GetInteger())
		{
		case -1:
			i[1].ki.wVk = VK_NUMPAD1;
			i[1].ki.wScan = 0x4F;
			break;
		case 1:
		default:
			i[1].ki.wVk = VK_NUMPAD4;
			i[1].ki.wScan = 0x4B;
			break;
		case 2:
			i[1].ki.wVk = VK_NUMPAD2;
			i[1].ki.wScan = 0x50;
			break;
		case 3:
			i[1].ki.wVk = VK_NUMPAD3;
			i[1].ki.wScan = 0x51;
			break;
		}
		i[2] = i[1];
		i[2].ki.dwFlags |= KEYEVENTF_KEYUP;
		i[3] = i[0];
		i[3].ki.dwFlags |= KEYEVENTF_KEYUP;
		SendInput(4, i, sizeof(INPUT));
#else
		printf("TODO: handle input on non windows platforms \n");
#endif
	}

}

/*
========================
idConsoleLocal::PrintOverlay
========================
*/
void idConsoleLocal::PrintOverlay( idOverlayHandle& handle, justify_t justify, const char* text, ... )
{
	if( handle.index >= 0 && handle.index < overlayText.Num() )
	{
		if( overlayText[handle.index].time == handle.time )
		{
			return;
		}
	}
	
	char string[MAX_PRINT_MSG];
	va_list argptr;
	va_start( argptr, text );
	idStr::vsnPrintf( string, sizeof( string ), text, argptr );
	va_end( argptr );
	
	overlayText_t& overlay = overlayText.Alloc();
	overlay.text = string;
	overlay.justify = justify;
	overlay.time = Sys_Milliseconds();
	
	handle.index = overlayText.Num() - 1;
	handle.time = overlay.time;
}

/*
========================
idConsoleLocal::DrawOverlayText
========================
*/
void idConsoleLocal::DrawOverlayText( float& leftY, float& rightY, float& centerY )
{
	for( int i = 0; i < overlayText.Num(); i++ )
	{
		const idStr& text = overlayText[i].text;
		
		int maxWidth = 0;
		int numLines = 0;
		for( int j = 0; j < text.Length(); j++ )
		{
			int width = 1;
			for( ; j < text.Length() && text[j] != '\n'; j++ )
			{
				width++;
			}
			numLines++;
			if( width > maxWidth )
			{
				maxWidth = width;
			}
		}
		
		idVec4 bgColor( 0.0f, 0.0f, 0.0f, 0.75f );
		
		const float width = maxWidth * SMALLCHAR_WIDTH;
		const float height = numLines * ( SMALLCHAR_HEIGHT + 4 );
		const float bgAdjust = - 0.5f * SMALLCHAR_WIDTH;
		if( overlayText[i].justify == JUSTIFY_LEFT )
		{
			renderSystem->DrawFilled( bgColor, LOCALSAFE_LEFT + bgAdjust, leftY, width, height );
		}
		else if( overlayText[i].justify == JUSTIFY_RIGHT )
		{
			renderSystem->DrawFilled( bgColor, LOCALSAFE_RIGHT - width + bgAdjust, rightY, width, height );
		}
		else if( overlayText[i].justify == JUSTIFY_CENTER_LEFT || overlayText[i].justify == JUSTIFY_CENTER_RIGHT )
		{
			renderSystem->DrawFilled( bgColor, LOCALSAFE_LEFT + ( LOCALSAFE_WIDTH - width + bgAdjust ) * 0.5f, centerY, width, height );
		}
		else
		{
			assert( false );
		}
		
		idStr singleLine;
		for( int j = 0; j < text.Length(); j += singleLine.Length() + 1 )
		{
			singleLine = "";
			for( int k = j; k < text.Length() && text[k] != '\n'; k++ )
			{
				singleLine.Append( text[k] );
			}
			if( overlayText[i].justify == JUSTIFY_LEFT )
			{
				DrawTextLeftAlign( LOCALSAFE_LEFT, leftY, "%s", singleLine.c_str() );
			}
			else if( overlayText[i].justify == JUSTIFY_RIGHT )
			{
				DrawTextRightAlign( LOCALSAFE_RIGHT, rightY, "%s", singleLine.c_str() );
			}
			else if( overlayText[i].justify == JUSTIFY_CENTER_LEFT )
			{
				DrawTextLeftAlign( LOCALSAFE_LEFT + ( LOCALSAFE_WIDTH - width ) * 0.5f, centerY, "%s", singleLine.c_str() );
			}
			else if( overlayText[i].justify == JUSTIFY_CENTER_RIGHT )
			{
				DrawTextRightAlign( LOCALSAFE_LEFT + ( LOCALSAFE_WIDTH + width ) * 0.5f, centerY, "%s", singleLine.c_str() );
			}
			else
			{
				assert( false );
			}
		}
	}
	overlayText.SetNum( 0 );
}

/*
========================
idConsoleLocal::CreateGraph
========================
*/
idDebugGraph* idConsoleLocal::CreateGraph( int numItems )
{
	idDebugGraph* graph = new( TAG_SYSTEM ) idDebugGraph( numItems );
	debugGraphs.Append( graph );
	return graph;
}

/*
========================
idConsoleLocal::DestroyGraph
========================
*/
void idConsoleLocal::DestroyGraph( idDebugGraph* graph )
{
	debugGraphs.Remove( graph );
	delete graph;
}

/*
========================
idConsoleLocal::DrawDebugGraphs
========================
*/
void idConsoleLocal::DrawDebugGraphs()
{
	for( int i = 0; i < debugGraphs.Num(); i++ )
	{
		debugGraphs[i]->Render( renderSystem );
	}
}
