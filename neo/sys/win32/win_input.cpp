/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 Robert Beckebans

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
#include "../sys_session_local.h"

#include "win_local.h"

// Koz begin
#include "vr\vr.h"

// Koz end

#define DINPUT_BUFFERSIZE           256

idCVar vr_rumbleDiv( "vr_rumbleDiv", "1", CVAR_FLOAT | CVAR_ARCHIVE, "rumble divisor\n" );
idCVar vr_rumbleSkip( "vr_rumbleSkip", "1", CVAR_FLOAT | CVAR_ARCHIVE, "frames to skip\n" );
idCVar vr_rumbleEnable( "vr_rumbleEnable", "1", CVAR_BOOL | CVAR_ARCHIVE, "Enable VR controller rumble\n" );

idCVar vr_openVrStuckPadAxisFix( "vr_openVrStuckPadAxisFix", "1", CVAR_BOOL | CVAR_ARCHIVE, "Check for openVR controller stuck pad axis." );
idCVar vr_openVrStuckPadAxisFixThresh( "vr_openVrStuckPadAxisFixThresh", "12", CVAR_INTEGER | CVAR_ARCHIVE, "# of identical non zero input polls before axis flagged as stuck." );


/*
============================================================

DIRECT INPUT KEYBOARD CONTROL

============================================================
*/

bool IN_StartupKeyboard()
{
	HRESULT hr;
	bool    bExclusive;
	bool    bForeground;
	bool    bImmediate;
	bool    bDisableWindowsKey;
	DWORD   dwCoopFlags;
	
	if( !win32.g_pdi )
	{
		common->Printf( "keyboard: DirectInput has not been started\n" );
		return false;
	}
	
	if( win32.g_pKeyboard )
	{
		win32.g_pKeyboard->Release();
		win32.g_pKeyboard = NULL;
	}
	
	// Detrimine where the buffer would like to be allocated
	bExclusive         = false;
	bForeground        = true;
	bImmediate         = false;
	bDisableWindowsKey = true;
	
	if( bExclusive )
		dwCoopFlags = DISCL_EXCLUSIVE;
	else
		dwCoopFlags = DISCL_NONEXCLUSIVE;
		
	if( bForeground )
		dwCoopFlags |= DISCL_FOREGROUND;
	else
		dwCoopFlags |= DISCL_BACKGROUND;
		
	// Disabling the windows key is only allowed only if we are in foreground nonexclusive
	if( bDisableWindowsKey && !bExclusive && bForeground )
		dwCoopFlags |= DISCL_NOWINKEY;
		
	// Obtain an interface to the system keyboard device.
	if( FAILED( hr = win32.g_pdi->CreateDevice( GUID_SysKeyboard, &win32.g_pKeyboard, NULL ) ) )
	{
		common->Printf( "keyboard: couldn't find a keyboard device\n" );
		return false;
	}
	
	// Set the data format to "keyboard format" - a predefined data format
	//
	// A data format specifies which controls on a device we
	// are interested in, and how they should be reported.
	//
	// This tells DirectInput that we will be passing an array
	// of 256 bytes to IDirectInputDevice::GetDeviceState.
	if( FAILED( hr = win32.g_pKeyboard->SetDataFormat( &c_dfDIKeyboard ) ) )
		return false;
		
	// Set the cooperativity level to let DirectInput know how
	// this device should interact with the system and with other
	// DirectInput applications.
	hr = win32.g_pKeyboard->SetCooperativeLevel( win32.hWnd, dwCoopFlags );
	if( hr == DIERR_UNSUPPORTED && !bForeground && bExclusive )
	{
		common->Printf( "keyboard: SetCooperativeLevel() returned DIERR_UNSUPPORTED.\nFor security reasons, background exclusive keyboard access is not allowed.\n" );
		return false;
	}
	
	if( FAILED( hr ) )
	{
		return false;
	}
	
	if( !bImmediate )
	{
		// IMPORTANT STEP TO USE BUFFERED DEVICE DATA!
		//
		// DirectInput uses unbuffered I/O (buffer size = 0) by default.
		// If you want to read buffered data, you need to set a nonzero
		// buffer size.
		//
		// Set the buffer size to DINPUT_BUFFERSIZE (defined above) elements.
		//
		// The buffer size is a DWORD property associated with the device.
		DIPROPDWORD dipdw;
		
		dipdw.diph.dwSize       = sizeof( DIPROPDWORD );
		dipdw.diph.dwHeaderSize = sizeof( DIPROPHEADER );
		dipdw.diph.dwObj        = 0;
		dipdw.diph.dwHow        = DIPH_DEVICE;
		dipdw.dwData            = DINPUT_BUFFERSIZE; // Arbitary buffer size
		
		if( FAILED( hr = win32.g_pKeyboard->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) )
			return false;
	}
	
	// Acquire the newly created device
	win32.g_pKeyboard->Acquire();
	
	common->Printf( "keyboard: DirectInput initialized.\n" );
	return true;
}

/*
==========================
IN_DeactivateKeyboard
==========================
*/
void IN_DeactivateKeyboard()
{
	if( !win32.g_pKeyboard )
	{
		return;
	}
	win32.g_pKeyboard->Unacquire( );
}

/*
============================================================

DIRECT INPUT MOUSE CONTROL

============================================================
*/

/*
========================
IN_InitDirectInput
========================
*/

void IN_InitDirectInput()
{
	HRESULT		hr;
	
	common->Printf( "Initializing DirectInput...\n" );
	
	if( win32.g_pdi != NULL )
	{
		win32.g_pdi->Release();			// if the previous window was destroyed we need to do this
		win32.g_pdi = NULL;
	}
	
	// Register with the DirectInput subsystem and get a pointer
	// to a IDirectInput interface we can use.
	// Create the base DirectInput object
	if( FAILED( hr = DirectInput8Create( GetModuleHandle( NULL ), DIRECTINPUT_VERSION, IID_IDirectInput8, ( void** )&win32.g_pdi, NULL ) ) )
	{
		common->Printf( "DirectInputCreate failed\n" );
	}
}

/*
========================
IN_InitDIMouse
========================
*/
bool IN_InitDIMouse()
{
	HRESULT		hr;
	
	if( win32.g_pdi == NULL )
	{
		return false;
	}
	
	// obtain an interface to the system mouse device.
	hr = win32.g_pdi->CreateDevice( GUID_SysMouse, &win32.g_pMouse, NULL );
	
	if( FAILED( hr ) )
	{
		common->Printf( "mouse: Couldn't open DI mouse device\n" );
		return false;
	}
	
	// Set the data format to "mouse format" - a predefined data format
	//
	// A data format specifies which controls on a device we
	// are interested in, and how they should be reported.
	//
	// This tells DirectInput that we will be passing a
	// DIMOUSESTATE2 structure to IDirectInputDevice::GetDeviceState.
	if( FAILED( hr = win32.g_pMouse->SetDataFormat( &c_dfDIMouse2 ) ) )
	{
		common->Printf( "mouse: Couldn't set DI mouse format\n" );
		return false;
	}
	
	// set the cooperativity level.
	hr = win32.g_pMouse->SetCooperativeLevel( win32.hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND );
	
	if( FAILED( hr ) )
	{
		common->Printf( "mouse: Couldn't set DI coop level\n" );
		return false;
	}
	
	
	// IMPORTANT STEP TO USE BUFFERED DEVICE DATA!
	//
	// DirectInput uses unbuffered I/O (buffer size = 0) by default.
	// If you want to read buffered data, you need to set a nonzero
	// buffer size.
	//
	// Set the buffer size to SAMPLE_BUFFER_SIZE (defined above) elements.
	//
	// The buffer size is a DWORD property associated with the device.
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize       = sizeof( DIPROPDWORD );
	dipdw.diph.dwHeaderSize = sizeof( DIPROPHEADER );
	dipdw.diph.dwObj        = 0;
	dipdw.diph.dwHow        = DIPH_DEVICE;
	dipdw.dwData            = DINPUT_BUFFERSIZE; // Arbitary buffer size
	
	if( FAILED( hr = win32.g_pMouse->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) )
	{
		common->Printf( "mouse: Couldn't set DI buffersize\n" );
		return false;
	}
	
	IN_ActivateMouse();
	
	// clear any pending samples
	int	mouseEvents[MAX_MOUSE_EVENTS][2];
	Sys_PollMouseInputEvents( mouseEvents );
	
	common->Printf( "mouse: DirectInput initialized.\n" );
	return true;
}


/*
==========================
IN_ActivateMouse
==========================
*/
void IN_ActivateMouse()
{
	int i;
	HRESULT hr;
	
	if( !win32.in_mouse.GetBool() || win32.mouseGrabbed || !win32.g_pMouse )
	{
		return;
	}
	
	win32.mouseGrabbed = true;
	for( i = 0; i < 10; i++ )
	{
		if( ::ShowCursor( false ) < 0 )
		{
			break;
		}
	}
	
	// we may fail to reacquire if the window has been recreated
	hr = win32.g_pMouse->Acquire();
	if( FAILED( hr ) )
	{
		return;
	}
	
	// set the cooperativity level.
	hr = win32.g_pMouse->SetCooperativeLevel( win32.hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND );
}

/*
==========================
IN_DeactivateMouse
==========================
*/
void IN_DeactivateMouse()
{
	int i;
	
	if( !win32.g_pMouse || !win32.mouseGrabbed )
	{
		return;
	}
	
	win32.g_pMouse->Unacquire();
	
	for( i = 0; i < 10; i++ )
	{
		if( ::ShowCursor( true ) >= 0 )
		{
			break;
		}
	}
	win32.mouseGrabbed = false;
}

/*
==========================
IN_DeactivateMouseIfWindowed
==========================
*/
void IN_DeactivateMouseIfWindowed()
{
	if( !win32.cdsFullscreen )
	{
		IN_DeactivateMouse();
	}
}

/*
============================================================

  MOUSE CONTROL

============================================================
*/


/*
===========
Sys_ShutdownInput
===========
*/
void Sys_ShutdownInput()
{
	IN_DeactivateMouse();
	IN_DeactivateKeyboard();
	if( win32.g_pKeyboard )
	{
		win32.g_pKeyboard->Release();
		win32.g_pKeyboard = NULL;
	}
	
	if( win32.g_pMouse )
	{
		win32.g_pMouse->Release();
		win32.g_pMouse = NULL;
	}
	
	if( win32.g_pdi )
	{
		win32.g_pdi->Release();
		win32.g_pdi = NULL;
	}
}

/*
===========
Sys_InitInput
===========
*/
void Sys_InitInput()
{
	common->Printf( "\n------- Input Initialization -------\n" );
	IN_InitDirectInput();
	if( win32.in_mouse.GetBool() )
	{
		IN_InitDIMouse();
		// don't grab the mouse on initialization
		Sys_GrabMouseCursor( false );
	}
	else
	{
		common->Printf( "Mouse control not active.\n" );
	}
	IN_StartupKeyboard();
	
	common->Printf( "------------------------------------\n" );
	win32.in_mouse.ClearModified();
}

/*
==================
IN_Frame

Called every frame, even if not generating commands
==================
*/
void IN_Frame()
{
	bool	shouldGrab = true;
	
	if( !win32.in_mouse.GetBool() )
	{
		shouldGrab = false;
	}
	// if fullscreen, we always want the mouse
	if( !win32.cdsFullscreen )
	{
		if( win32.mouseReleased )
		{
			shouldGrab = false;
		}
		if( win32.movingWindow )
		{
			shouldGrab = false;
		}
		if( !win32.activeApp )
		{
			shouldGrab = false;
		}
	}
	
	if( shouldGrab != win32.mouseGrabbed )
	{
		if( usercmdGen != NULL )
		{
			usercmdGen->Clear();
		}
		
		if( win32.mouseGrabbed )
		{
			IN_DeactivateMouse();
		}
		else
		{
			IN_ActivateMouse();
			
#if 0	// if we can't reacquire, try reinitializing
			if( !IN_InitDIMouse() )
			{
				win32.in_mouse.SetBool( false );
				return;
			}
#endif
		}
	}
}


void	Sys_GrabMouseCursor( bool grabIt )
{
	win32.mouseReleased = !grabIt;
	if( !grabIt )
	{
		// release it right now
		IN_Frame();
	}
}

//=====================================================================================

static DIDEVICEOBJECTDATA polled_didod[ DINPUT_BUFFERSIZE ];  // Receives buffered data

static int diFetch;
static byte toggleFetch[2][ 256 ];


#if 1
// I tried doing the full-state get to address a keyboard problem on one system,
// but it didn't make any difference

/*
====================
Sys_PollKeyboardInputEvents
====================
*/
int Sys_PollKeyboardInputEvents()
{
	DWORD              dwElements;
	HRESULT            hr;
	
	if( win32.g_pKeyboard == NULL )
	{
		return 0;
	}
	
	dwElements = DINPUT_BUFFERSIZE;
	hr = win32.g_pKeyboard->GetDeviceData( sizeof( DIDEVICEOBJECTDATA ),
										   polled_didod, &dwElements, 0 );
	if( hr != DI_OK )
	{
		// We got an error or we got DI_BUFFEROVERFLOW.
		//
		// Either way, it means that continuous contact with the
		// device has been lost, either due to an external
		// interruption, or because the buffer overflowed
		// and some events were lost.
		hr = win32.g_pKeyboard->Acquire();
		
		
		
		// nuke the garbage
		if( !FAILED( hr ) )
		{
			//Bug 951: The following command really clears the garbage input.
			//The original will still process keys in the buffer and was causing
			//some problems.
			win32.g_pKeyboard->GetDeviceData( sizeof( DIDEVICEOBJECTDATA ), NULL, &dwElements, 0 );
			dwElements = 0;
		}
		// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
		// may occur when the app is minimized or in the process of
		// switching, so just try again later
	}
	
	if( FAILED( hr ) )
	{
		return 0;
	}
	
	return dwElements;
}

#else

/*
====================
Sys_PollKeyboardInputEvents

Fake events by getting the entire device state
and checking transitions
====================
*/
int Sys_PollKeyboardInputEvents()
{
	HRESULT            hr;
	
	if( win32.g_pKeyboard == NULL )
	{
		return 0;
	}
	
	hr = win32.g_pKeyboard->GetDeviceState( sizeof( toggleFetch[ diFetch ] ), toggleFetch[ diFetch ] );
	if( hr != DI_OK )
	{
		// We got an error or we got DI_BUFFEROVERFLOW.
		//
		// Either way, it means that continuous contact with the
		// device has been lost, either due to an external
		// interruption, or because the buffer overflowed
		// and some events were lost.
		hr = win32.g_pKeyboard->Acquire();
		
		// nuke the garbage
		if( !FAILED( hr ) )
		{
			hr = win32.g_pKeyboard->GetDeviceState( sizeof( toggleFetch[ diFetch ] ), toggleFetch[ diFetch ] );
		}
		// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
		// may occur when the app is minimized or in the process of
		// switching, so just try again later
	}
	
	if( FAILED( hr ) )
	{
		return 0;
	}
	
	// build faked events
	int		numChanges = 0;
	
	for( int i = 0 ; i < 256 ; i++ )
	{
		if( toggleFetch[0][i] != toggleFetch[1][i] )
		{
			polled_didod[ numChanges ].dwOfs = i;
			polled_didod[ numChanges ].dwData = toggleFetch[ diFetch ][i] ? 0x80 : 0;
			numChanges++;
		}
	}
	
	diFetch ^= 1;
	
	return numChanges;
}

#endif

/*
====================
Sys_PollKeyboardInputEvents
====================
*/
int Sys_ReturnKeyboardInputEvent( const int n, int& ch, bool& state )
{
	ch = polled_didod[ n ].dwOfs;
	state = ( polled_didod[ n ].dwData & 0x80 ) == 0x80;
	if( ch == K_PRINTSCREEN || ch == K_LCTRL || ch == K_LALT || ch == K_RCTRL || ch == K_RALT )
	{
		// for windows, add a keydown event for print screen here, since
		// windows doesn't send keydown events to the WndProc for this key.
		// ctrl and alt are handled here to get around windows sending ctrl and
		// alt messages when the right-alt is pressed on non-US 102 keyboards.
		Sys_QueEvent( SE_KEY, ch, state, 0, NULL, 0 );
	}
		
	return ch;
}


void Sys_EndKeyboardInputEvents()
{
}

//=====================================================================================


int Sys_PollMouseInputEvents( int mouseEvents[MAX_MOUSE_EVENTS][2] )
{
	DWORD				dwElements;
	HRESULT				hr;
	
	if( !win32.g_pMouse || !win32.mouseGrabbed )
	{
		return 0;
	}
	
	dwElements = DINPUT_BUFFERSIZE;
	hr = win32.g_pMouse->GetDeviceData( sizeof( DIDEVICEOBJECTDATA ), polled_didod, &dwElements, 0 );
	
	if( hr != DI_OK )
	{
		hr = win32.g_pMouse->Acquire();
		// clear the garbage
		if( !FAILED( hr ) )
		{
			win32.g_pMouse->GetDeviceData( sizeof( DIDEVICEOBJECTDATA ), polled_didod, &dwElements, 0 );
		}
	}
	
	if( FAILED( hr ) )
	{
		return 0;
	}
	
	if( dwElements > MAX_MOUSE_EVENTS )
	{
		dwElements = MAX_MOUSE_EVENTS;
	}
	
	for( DWORD i = 0; i < dwElements; i++ )
	{
		mouseEvents[i][0] = M_INVALID;
		mouseEvents[i][1] = 0;
		
		if( polled_didod[i].dwOfs >= DIMOFS_BUTTON0 && polled_didod[i].dwOfs <= DIMOFS_BUTTON7 )
		{
			const int mouseButton = ( polled_didod[i].dwOfs - DIMOFS_BUTTON0 );
			const bool mouseDown = ( polled_didod[i].dwData & 0x80 ) == 0x80;
			mouseEvents[i][0] = M_ACTION1 + mouseButton;
			mouseEvents[i][1] = mouseDown;
			Sys_QueEvent( SE_KEY, K_MOUSE1 + mouseButton, mouseDown, 0, NULL, 0 );
		}
		else
		{
			// RB: replaced switch enum for MinGW
			int diaction = polled_didod[i].dwOfs;
			
			if( diaction == DIMOFS_X )
			{
				mouseEvents[i][0] = M_DELTAX;
				mouseEvents[i][1] = polled_didod[i].dwData;
				Sys_QueEvent( SE_MOUSE, polled_didod[i].dwData, 0, 0, NULL, 0 );
			}
			else if( diaction == DIMOFS_Y )
			{
				mouseEvents[i][0] = M_DELTAY;
				mouseEvents[i][1] = polled_didod[i].dwData;
				Sys_QueEvent( SE_MOUSE, 0, polled_didod[i].dwData, 0, NULL, 0 );
			}
			else if( diaction == DIMOFS_Z )
			{
				mouseEvents[i][0] = M_DELTAZ;
				mouseEvents[i][1] = ( int )polled_didod[i].dwData / WHEEL_DELTA;
				{
					const int value = ( int )polled_didod[i].dwData / WHEEL_DELTA;
					const int key = value < 0 ? K_MWHEELDOWN : K_MWHEELUP;
					const int iterations = abs( value );
					for( int i = 0; i < iterations; i++ )
					{
						Sys_QueEvent( SE_KEY, key, true, 0, NULL, 0 );
						Sys_QueEvent( SE_KEY, key, false, 0, NULL, 0 );
					}
				}
			}
			// RB end
		}
	}
	
	return dwElements;
}

//=====================================================================================
//	Joystick Input Handling
//=====================================================================================

void Sys_SetRumble( int device, int low, int hi )
{
	if ( commonVr->hasOculusRift ) {
		// Koz begin
		if ( commonVr->VR_USE_MOTION_CONTROLS && vr_rumbleEnable.GetBool() )
		{
			commonVr->MotionControllerSetHapticOculus( low, hi );
			return;
		}
		// Koz end
	} 
	
	else if( commonVr->motionControlType == MOTION_STEAMVR && vr_rumbleEnable.GetBool() )
	{
	
		static int currentFrame = 0;
		static int val = 0;
				
		// Steam controller has one linear actuator for haptic feedback instead of
		// two rumble motors. Haptic feedback is based on pulse length in microseconds
		// not freq like a standard controller.
		// this is a stupidly crude hack to provide some basic feedback to the controller
		// if enabled. pulse strength is hacked from the low channel
		// pulse only sent every frameskip frames to keep the feedback from feeling
		// like a vibration instead of a pulse.
		int skipFrames = vr_rumbleSkip.GetInteger();

		//if ( low + hi > 0 ) common->Printf( "Rumble low %d hi %d\n", low, hi );

		if ( currentFrame == 0 || hi > 16384 )
		{
			if ( hi > 65535 ) hi = 16384;

			val = currentFrame == 0 ? low : ( (hi *2 ) / skipFrames );

			if ( val > 65535 ) val = 65535;

			val = (( 3500 / vr_rumbleDiv.GetFloat()) * val ) / 65535;
				
			// dont send the controller zero values - no need to turn off pulse as already time based.
			if ( val >= 10 ) commonVr->MotionControllerSetHapticOpenVR( vr_weaponHand.GetInteger(), val );
		}

		currentFrame++;
		if ( currentFrame >= skipFrames ) currentFrame = 0;
		return;
	}
	else
	{
		return win32.g_Joystick.SetRumble( device, low, hi );
	}
}

int Sys_PollJoystickInputEvents( int deviceNum )
{
	return win32.g_Joystick.PollInputEvents( deviceNum );
}


int Sys_ReturnJoystickInputEvent( const int n, int& action, int& value )
{
	return win32.g_Joystick.ReturnInputEvent( n, action, value );
}


void Sys_EndJoystickInputEvents()
{
}


/*
========================
JoystickSamplingThread
========================
*/
static int	threadTimeDeltas[256];
static int	threadPacket[256];
static int	threadCount;
void JoystickSamplingThread( void* data )
{
	static int prevTime = 0;
	static uint64 nextCheck[MAX_JOYSTICKS] = { 0 };
	const uint64 waitTime = 5000000; // poll every 5 seconds to see if a controller was connected
	while( 1 )
	{
		// hopefully we see close to 4000 usec each loop
		int	now = Sys_Microseconds();
		int	delta;
		if( prevTime == 0 )
		{
			delta = 4000;
		}
		else
		{
			delta = now - prevTime;
		}
		prevTime = now;
		threadTimeDeltas[threadCount & 255] = delta;
		threadCount++;
		
		{
			XINPUT_STATE	joyData[MAX_JOYSTICKS];
			bool			validData[MAX_JOYSTICKS];
			for( int i = 0 ; i < MAX_JOYSTICKS ; i++ )
			{
				if( now >= nextCheck[i] )
				{
					// XInputGetState might block... for a _really_ long time..
					validData[i] = XInputGetState( i, &joyData[i] ) == ERROR_SUCCESS;
					
					// allow an immediate data poll if the input device is connected else
					// wait for some time to see if another device was reconnected.
					// Checking input state infrequently for newly connected devices prevents
					// severe slowdowns on PC, especially on WinXP64.
					if( validData[i] )
					{
						nextCheck[i] = 0;
					}
					else
					{
						nextCheck[i] = now + waitTime;
					}
				}
			}
			
			// do this short amount of processing inside a critical section
			idScopedCriticalSection cs( win32.g_Joystick.mutexXis );
			
			for( int i = 0 ; i < MAX_JOYSTICKS ; i++ )
			{
				controllerState_t* cs = &win32.g_Joystick.controllers[i];
				
				if( !validData[i] )
				{
					cs->valid = false;
					continue;
				}
				cs->valid = true;
				
				XINPUT_STATE& current = joyData[i];
				
				cs->current = current;
				
				// Switch from using cs->current to current to reduce chance of Load-Hit-Store on consoles
				
				threadPacket[threadCount & 255] = current.dwPacketNumber;
#if 0
				if( xis.dwPacketNumber == oldXis[ inputDeviceNum ].dwPacketNumber )
				{
					return numEvents;
				}
#endif
				cs->buttonBits |= current.Gamepad.wButtons;
			}
		}
		
		// we want this to be processed at least 250 times a second
		WaitForSingleObject( win32.g_Joystick.timer, INFINITE );
	}
}


/*
========================
idJoystickWin32::idJoystickWin32
========================
*/
idJoystickWin32::idJoystickWin32()
{
	numEvents = 0;
	memset( &events, 0, sizeof( events ) );
	memset( &controllers, 0, sizeof( controllers ) );
	memset( buttonStates, 0, sizeof( buttonStates ) );
	memset( joyAxis, 0, sizeof( joyAxis ) );
}

/*
========================
idJoystickWin32::Init
========================
*/
bool idJoystickWin32::Init()
{
	idJoystick::Init();
	
	// setup the timer that the high frequency thread will wait on
	// to fire every 4 msec
	timer = CreateWaitableTimer( NULL, FALSE, "JoypadTimer" );
	LARGE_INTEGER dueTime;
	dueTime.QuadPart = -1;
	if( !SetWaitableTimer( timer, &dueTime, 4, NULL, NULL, FALSE ) )
	{
		idLib::FatalError( "SetWaitableTimer for joystick failed" );
	}
	
	// spawn the high frequency joystick reading thread
	Sys_CreateThread( ( xthread_t )JoystickSamplingThread, NULL, THREAD_HIGHEST, "Joystick", CORE_1A );
	
	return false;
}

/*
========================
idJoystickWin32::SetRumble
========================
*/
void idJoystickWin32::SetRumble( int inputDeviceNum, int rumbleLow, int rumbleHigh )
{
	if( inputDeviceNum < 0 || inputDeviceNum >= MAX_JOYSTICKS )
	{
		return;
	}
	if( !controllers[inputDeviceNum].valid )
	{
		return;
	}
	XINPUT_VIBRATION vibration;
	vibration.wLeftMotorSpeed = idMath::ClampInt( 0, 65535, rumbleLow );
	vibration.wRightMotorSpeed = idMath::ClampInt( 0, 65535, rumbleHigh );
	DWORD err = XInputSetState( inputDeviceNum, &vibration );
	if( err != ERROR_SUCCESS )
	{
		idLib::Warning( "XInputSetState error: 0x%x", err );
	}
}

/*
========================
idJoystickWin32::PostInputEvent
========================
*/
void idJoystickWin32::PostInputEvent( int inputDeviceNum, int event, int value, int range )
{
	// These events are used for GUI button presses
	if( ( event >= J_ACTION1 ) && ( event <= J_ACTION_MAX ) )
	{
		PushButton( inputDeviceNum, K_JOY1 + ( event - J_ACTION1 ), value != 0 );
	}
	else if( ( event >= J_TALK ) && ( event <= J_SAY_MAX ) )
	{
		PushButton( inputDeviceNum, K_TALK + ( event - J_TALK ), value != 0 );
	}
	else if( event == J_AXIS_LEFT_X )
	{
		PushButton( inputDeviceNum, K_JOY_STICK1_LEFT, ( value < -range ) );
		PushButton( inputDeviceNum, K_JOY_STICK1_RIGHT, ( value > range ) );
	}
	else if( event == J_AXIS_LEFT_Y )
	{
		PushButton( inputDeviceNum, K_JOY_STICK1_UP, ( value < -range ) );
		PushButton( inputDeviceNum, K_JOY_STICK1_DOWN, ( value > range ) );
	}
	else if( event == J_AXIS_RIGHT_X )
	{
		PushButton( inputDeviceNum, K_JOY_STICK2_LEFT, ( value < -range ) );
		PushButton( inputDeviceNum, K_JOY_STICK2_RIGHT, ( value > range ) );
	}
	else if( event == J_AXIS_RIGHT_Y )
	{
		PushButton( inputDeviceNum, K_JOY_STICK2_UP, ( value < -range ) );
		PushButton( inputDeviceNum, K_JOY_STICK2_DOWN, ( value > range ) );
	}
	else if( ( event >= J_DPAD_UP ) && ( event <= J_DPAD_RIGHT ) )
	{
		PushButton( inputDeviceNum, K_JOY_DPAD_UP + ( event - J_DPAD_UP ), value != 0 );
	}
	else if( event == J_AXIS_LEFT_TRIG )
	{
		PushButton( inputDeviceNum, K_JOY_TRIGGER1, ( value > range ) );
	}
	else if( event == J_AXIS_RIGHT_TRIG )
	{
		PushButton( inputDeviceNum, K_JOY_TRIGGER2, ( value > range ) );
	}

	// Koz begin add touch
	else if ( event == J_AXIS_LEFT_TOUCH_X )
	{
		PushButton( inputDeviceNum, K_TOUCH_LEFT_STICK_LEFT, (value < -range) );
		PushButton( inputDeviceNum, K_TOUCH_LEFT_STICK_RIGHT, (value > range) );
	}
	else if ( event == J_AXIS_LEFT_TOUCH_Y )
	{
		PushButton( inputDeviceNum, K_TOUCH_LEFT_STICK_UP, (value < -range) );
		PushButton( inputDeviceNum, K_TOUCH_LEFT_STICK_DOWN, (value > range) );
	}
	else if ( event == J_AXIS_RIGHT_TOUCH_X )
	{
		PushButton( inputDeviceNum, K_TOUCH_RIGHT_STICK_LEFT, (value < -range) );
		PushButton( inputDeviceNum, K_TOUCH_RIGHT_STICK_RIGHT, (value > range) );
	}
	else if ( event == J_AXIS_RIGHT_TOUCH_Y )
	{
		PushButton( inputDeviceNum, K_TOUCH_RIGHT_STICK_UP, (value < -range) );
		PushButton( inputDeviceNum, K_TOUCH_RIGHT_STICK_DOWN, (value > range) );
	}
	else if ( event == J_AXIS_LEFT_TOUCH_TRIG )
	{
		PushButton( inputDeviceNum, K_L_TOUCHTRIG, (value > range) );
	}
	else if ( event == J_AXIS_RIGHT_TOUCH_TRIG )
	{
		PushButton( inputDeviceNum, K_R_TOUCHTRIG, (value > range) );
	}


	// add SteamVR controllers
	else if ( event == J_AXIS_LEFT_STEAMVR_X )
	{
		//common->Printf( "Pushing button K_STEAMVR_LEFT_PAD x\n" );
		PushButton( inputDeviceNum, K_STEAMVR_LEFT_PAD_LEFT, (value < -range) );
		PushButton( inputDeviceNum, K_STEAMVR_LEFT_PAD_RIGHT, (value > range) );
	}
	else if ( event == J_AXIS_LEFT_STEAMVR_Y )
	{
		//common->Printf( "Pushing button K_STEAMVR_LEFT_PAD y\n" );
		PushButton( inputDeviceNum, K_STEAMVR_LEFT_PAD_UP, (value < -range) );
		PushButton( inputDeviceNum, K_STEAMVR_LEFT_PAD_DOWN, (value > range) );
	}
	else if ( event == J_AXIS_RIGHT_STEAMVR_X )
	{
		//common->Printf( "Pushing button K_STEAMVR_RIGHT_PAD x\n" );
		PushButton( inputDeviceNum, K_STEAMVR_RIGHT_PAD_LEFT, (value < -range) );
		PushButton( inputDeviceNum, K_STEAMVR_RIGHT_PAD_RIGHT, (value > range) );
	}
	else if ( event == J_AXIS_RIGHT_STEAMVR_Y )
	{
		//common->Printf( "Pushing button K_STEAMVR_RIGHT_PAD y\n" );
		PushButton( inputDeviceNum, K_STEAMVR_RIGHT_PAD_UP, (value < -range) );
		PushButton( inputDeviceNum, K_STEAMVR_RIGHT_PAD_DOWN, (value > range) );
	}
	else if ( event == J_AXIS_LEFT_STEAMVR_TRIG )
	{
		//common->Printf( "Pushing button K_L_STEAMVRTRIG\n" );
		PushButton( inputDeviceNum, K_L_STEAMVRTRIG, (value > range) );
	}
	else if ( event == J_AXIS_RIGHT_STEAMVR_TRIG )
	{
		//common->Printf( "Pushing button K_R_STEAMVRTRIG y\n" );
		PushButton( inputDeviceNum, K_R_STEAMVRTRIG, (value > range) );
	}
	else if (event == J_AXIS_LEFT_JS_STEAMVR_X)
	{
		//common->Printf( "Pushing button K_STEAMVR_LEFT_PAD x\n" );
		PushButton(inputDeviceNum, K_STEAMVR_LEFT_JS_LEFT, (value < -range));
		PushButton(inputDeviceNum, K_STEAMVR_LEFT_JS_RIGHT, (value > range));
	}
	else if (event == J_AXIS_LEFT_JS_STEAMVR_Y)
	{
		//common->Printf( "Pushing button K_STEAMVR_LEFT_PAD y\n" );
		PushButton(inputDeviceNum, K_STEAMVR_LEFT_JS_UP, (value < -range));
		PushButton(inputDeviceNum, K_STEAMVR_LEFT_JS_DOWN, (value > range));
	}
	else if (event == J_AXIS_RIGHT_JS_STEAMVR_X)
	{
		//common->Printf( "Pushing button K_STEAMVR_RIGHT_PAD x\n" );
		PushButton(inputDeviceNum, K_STEAMVR_RIGHT_JS_LEFT, (value < -range));
		PushButton(inputDeviceNum, K_STEAMVR_RIGHT_JS_RIGHT, (value > range));
	}
	else if (event == J_AXIS_RIGHT_JS_STEAMVR_Y)
	{
		//common->Printf( "Pushing button K_STEAMVR_RIGHT_PAD y\n" );
		PushButton(inputDeviceNum, K_STEAMVR_RIGHT_JS_UP, (value < -range));
		PushButton(inputDeviceNum, K_STEAMVR_RIGHT_JS_DOWN, (value > range));
	}

	// Koz end

	if( event >= J_AXIS_MIN && event <= J_AXIS_MAX )
	{
		int axis = event - J_AXIS_MIN;
		int percent = ( value * 16 ) / range;
		if( joyAxis[inputDeviceNum][axis] != percent )
		{
			joyAxis[inputDeviceNum][axis] = percent;
			Sys_QueEvent( SE_JOYSTICK, axis, percent, 0, NULL, inputDeviceNum );
		}
	}
	
	// These events are used for actual game input
	events[numEvents].event = event;
	events[numEvents].value = value;
	numEvents++;
}

/*
========================
idJoystickWin32::PollInputEvents
========================
*/
int idJoystickWin32::PollInputEvents( int inputDeviceNum )
{
	numEvents = 0;
	
	if( !win32.activeApp )
	{
		return numEvents;
	}
	//common->Printf( "Pollinput polling device # %d\n", inputDeviceNum );
	assert( inputDeviceNum < 4 );
		if ( inputDeviceNum < 4 )
		{
			//	if ( inputDeviceNum > in_joystick.GetInteger() ) {
			//		return numEvents;
			//	}

			controllerState_t* cs = &controllers[inputDeviceNum];

			// grab the current packet under a critical section
			XINPUT_STATE xis;
			XINPUT_STATE old;
			int		orBits;
			{
				idScopedCriticalSection crit( mutexXis );
				xis = cs->current;
				old = cs->previous;
				cs->previous = xis;
				// fetch or'd button bits
				orBits = cs->buttonBits;
				cs->buttonBits = 0;
			}
#if 0
			if( XInputGetState( inputDeviceNum, &xis ) != ERROR_SUCCESS )
			{
				return numEvents;
			}
#endif
			// Carl: Update VR_USE_MOTION_CONTROLS if we've just started using the XBox controller
			if (commonVr->VR_USE_MOTION_CONTROLS && vr_autoSwitchControllers.GetBool() && (xis.Gamepad.wButtons || xis.Gamepad.bLeftTrigger > 64 || xis.Gamepad.bRightTrigger > 64
				|| abs(xis.Gamepad.sThumbLX) + abs(xis.Gamepad.sThumbLY) > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || abs(xis.Gamepad.sThumbRX) + abs(xis.Gamepad.sThumbRY) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ) )
			{
				commonVr->VR_USE_MOTION_CONTROLS = false;
			}

			for ( int i = 0; i < 32; i++ )
			{
				int	bit = 1 << i;

				if ( ((xis.Gamepad.wButtons | old.Gamepad.wButtons) & bit) == 0
					&& (orBits & bit) )
				{
					idLib::Printf( "Dropped button press on bit %i\n", i );
				}
			}

			if ( session->IsSystemUIShowing() )
			{
				// memset xis so the current input does not get latched if the UI is showing
				memset( &xis, 0, sizeof( XINPUT_STATE ) );
			}

			int joyRemap[16] =
			{
				J_DPAD_UP, J_DPAD_DOWN,	// Up, Down
				J_DPAD_LEFT, J_DPAD_RIGHT,	// Left, Right
				J_XBOX_START, J_XBOX_BACK,		// Start, Back
				J_XBOX_LSTICK, J_XBOX_RSTICK,		// Left Stick Down, Right Stick Down
				J_XBOX_LSHOULDER, J_XBOX_RSHOULDER,		// Black, White (Left Shoulder, Right Shoulder)
				J_ACTION11, J_ACTION12,				// Unused
				J_XBOX_A, J_XBOX_B,		// A, B
				J_XBOX_X, J_XBOX_Y,		// X, Y
			};

			// Check the digital buttons
			for ( int i = 0; i < 16; i++ )
			{
				int mask = (1 << i);
				if ( (xis.Gamepad.wButtons & mask) != (old.Gamepad.wButtons & mask) )
				{
					PostInputEvent( inputDeviceNum, joyRemap[i], (xis.Gamepad.wButtons & mask) > 0 );
				}
			}

			// Check the triggers
			if ( xis.Gamepad.bLeftTrigger != old.Gamepad.bLeftTrigger )
			{
				PostInputEvent( inputDeviceNum, J_AXIS_LEFT_TRIG, xis.Gamepad.bLeftTrigger * 128 );
			}
			if ( xis.Gamepad.bRightTrigger != old.Gamepad.bRightTrigger )
			{
				PostInputEvent( inputDeviceNum, J_AXIS_RIGHT_TRIG, xis.Gamepad.bRightTrigger * 128 );
			}

			if ( xis.Gamepad.sThumbLX != old.Gamepad.sThumbLX )
			{
				PostInputEvent( inputDeviceNum, J_AXIS_LEFT_X, xis.Gamepad.sThumbLX );
			}
			if ( xis.Gamepad.sThumbLY != old.Gamepad.sThumbLY )
			{
				PostInputEvent( inputDeviceNum, J_AXIS_LEFT_Y, -xis.Gamepad.sThumbLY );
			}
			if ( xis.Gamepad.sThumbRX != old.Gamepad.sThumbRX )
			{
				PostInputEvent( inputDeviceNum, J_AXIS_RIGHT_X, xis.Gamepad.sThumbRX );
			}
			if ( xis.Gamepad.sThumbRY != old.Gamepad.sThumbRY )
			{
				PostInputEvent( inputDeviceNum, J_AXIS_RIGHT_Y, -xis.Gamepad.sThumbRY );
			}


			//===================
			// add speech recognition + oculus & openvr controllers
			// device handling code is odd building usercmds, so 
			// add all controllers as device 0.

			if ( inputDeviceNum == 0 )
			{

				//=============================
				// Carl add talk

				if ( vr_talkMode.GetInteger() > 0 )
				{
					static bool oldTalk = false;
					bool talk = commonVoice->GetTalkButton();
					if ( talk != oldTalk )
					{
						//common->Printf( "Posting talk input event\n" );
						PostInputEvent( inputDeviceNum, J_TALK, talk );
						oldTalk = talk;
					}
				}
        
				if ( vr_voiceCommands.GetInteger() > 0 )   
				{
					static bool oldSay[ J_SAY_MAX - J_SAY_MIN + 1 ] = {};
					int max = vr_voiceCommands.GetInteger() > 1 ? J_SAY_MAX : J_SAY_RELOAD;
					
					for ( int i = J_SAY_MIN; i <= max; ++i )
					{
						bool say = commonVoice->GetSayButton( i );
						if ( say != oldSay[ i - J_SAY_MIN ] )
						{
							//common->Printf( "Posting say input event %d %d\n", i, say );
							PostInputEvent( inputDeviceNum, i, say );
							oldSay[ i - J_SAY_MIN ] = say;
						}
					}
				}
				else
				{
					commonVr->forceRun = false;
				}


				//=============================
				// Koz begin add SteamVR controllers

				if ( commonVr->hasHMD && !commonVr->hasOculusRift ) // was  commonVr->VR_USE_MOTION_CONTROLS && commonVr->motionControlType == MOTION_STEAMVR
				{
					int dupeThreshold = vr_openVrStuckPadAxisFixThresh.GetInteger();
					bool defaultX = false;
					bool defaultY = false;
					static int lXcount = 0;
					static int lYcount = 0;
					static int rXcount = 0;
					static int rYcount = 0;

					static uint32 triggerAxis[2] = {};
					static uint32 padAxis[2] = {};
					static uint32 jsAxis[2] = {};
					static uint32 axisType;
					static float triggerVal[2] = {};
					static float padAxisX[2] = {};
					static float padAxisY[2] = {};
					static float jsAxisX[2] = {};
					static float jsAxisY[2] = {};

					static float padX = 0.0f;
					static float padY = 0.0f;
					static float trig = 0.0f;

					static float jsX = 0.0f;
					static float jsY = 0.0f;

					static uint64_t	 oldButton[2] = {};
					static uint32 lastPacketL = -1;
					static uint32 lastPacketR = -1;
					uint64_t button;

					bool lGood = false;
					bool rGood = false;
					
					vr::VRControllerState_t& currentStateL = commonVr->pControllerStateL;
					lGood = commonVr->m_pHMD->GetControllerState( commonVr->leftControllerDeviceNo, &currentStateL, sizeof( currentStateL ) );

					vr::VRControllerState_t& currentStateR = commonVr->pControllerStateR;
					rGood = commonVr->m_pHMD->GetControllerState( commonVr->rightControllerDeviceNo, &currentStateR, sizeof( currentStateR ) );


					// left steam controller
					if ( lGood )  
					{
						
						for ( uint32 axis = 0; axis < vr::k_unControllerStateAxisCount; axis++ )
						{
							uint32 axisNum = vr::Prop_Axis0Type_Int32 + axis;

							axisType = vr::VRSystem()->GetInt32TrackedDeviceProperty( commonVr->leftControllerDeviceNo, (vr::ETrackedDeviceProperty) (vr::Prop_Axis0Type_Int32 + axis) );
							if ( axisType == vr::k_eControllerAxis_Trigger )
							{
								triggerAxis[0] = axis;
							}
							if ( axisType == vr::k_eControllerAxis_TrackPad)
							{
								padAxis[0] = axis;
							}
							if (axisType == vr::k_eControllerAxis_Joystick)
							{
								jsAxis[0] = axis;
							}

						}

						trig = currentStateL.rAxis[triggerAxis[0]].x;
						padX = currentStateL.rAxis[padAxis[0]].x;
						padY = currentStateL.rAxis[padAxis[0]].y;

						if ( fabs( padX ) < vr_padDeadzone.GetFloat() ) padX = 0.0f;
						if ( fabs( padY ) < vr_padDeadzone.GetFloat() ) padY = 0.0f;

						// using (testing) the trigger button instead of reading the analog axis, so comment this out for now
						/*
						if ( trig != triggerVal[0] )
						{
							PostInputEvent( inputDeviceNum, J_AXIS_LEFT_STEAMVR_TRIG, triggerVal[0] * 32767.0f );
						}
						triggerVal[0] = trig;
						*/

						// the vive controllers ( at least mine, in multiple games, even after calibration) sometimes miss when a finger has been removed from the touchpad, 
						// and keep sending the exact same values until the pad is touched again.  This causes movement or rotation to get stuck on 
						// until the controller is touched again, which is really bad, especially if its rotation.
						// it's pretty much physically impossible to keep you finger so still on the pad it reports the exact same values 
						// more than a few times in a row, so count non zero repeats, and default the value to 0 if it's been repeated too many times.
											

						if ( vr_openVrStuckPadAxisFix.GetBool() )
						{
							if ( padX != 0.0f && padAxisX[0] == padX )
							{
								if ( lXcount > dupeThreshold )
								{
									padX = 0.0f;
									defaultX = true;
								}
								else
								{
									lXcount++;
									if ( lXcount > dupeThreshold ) common->Printf( "Defaulting left x axis val %f time %d\n", padX, Sys_Milliseconds() );
								}
							}
							else
							{
								lXcount = 0;
							}

							if ( padY != 0.0f && padAxisY[0] == padY )
							{
								if ( lYcount > dupeThreshold )
								{
									padY = 0.0f;
									defaultY = true;
								}
								else
								{
									lYcount++;
									if ( lYcount > dupeThreshold ) common->Printf( "Defaulting left y axis val %f time %d \n", padY, Sys_Milliseconds() );
								}
							}
							else
							{
								lYcount = 0;
							}
						}
						
						//post the axes 
						if ( padX != padAxisX[0] || defaultX ) 
						{
							//common->Printf( "Posting input event left steamvr pad x value %f time %d\n", padX, Sys_Milliseconds() );
							PostInputEvent( inputDeviceNum, J_AXIS_LEFT_STEAMVR_X, padX * 32767.0f );
						}
						
						if ( padY != padAxisY[0] || defaultY )
						{
							//common->Printf( "Posting input event left steamvr pad y value %f time %d\n", padY, Sys_Milliseconds() );
							PostInputEvent( inputDeviceNum, J_AXIS_LEFT_STEAMVR_Y, -padY * 32767.0f );
						}
						
						jsX = currentStateL.rAxis[jsAxis[0]].x;
						jsY = currentStateL.rAxis[jsAxis[0]].y;

						if (fabs(jsX) < vr_jsDeadzone.GetFloat()) jsX = 0.0f;
						if (fabs(jsY) < vr_jsDeadzone.GetFloat()) jsY = 0.0f;

						if (jsX != jsAxisX[0])
						{
							//common->Printf( "Posting input event left steamvr pad x value %f time %d\n", padX, Sys_Milliseconds() );
							PostInputEvent(inputDeviceNum, J_AXIS_LEFT_JS_STEAMVR_X, jsX * 32767.0f);
							jsAxisX[0] = jsX;
						}

						if (jsY != jsAxisY[0])
						{
							//common->Printf( "Posting input event left steamvr pad y value %f time %d\n", padY, Sys_Milliseconds() );
							PostInputEvent(inputDeviceNum, J_AXIS_LEFT_JS_STEAMVR_Y, -jsY * 32767.0f);
							jsAxisY[0] = jsY;
						}
						
						if ( !defaultX ) padAxisX[0] = padX;
						if ( !defaultY ) padAxisY[0] = padY;
						
						// process buttons ( appmenu, grip, trigger, touchpad pressed )
						button = currentStateL.ulButtonPressed;

						if( !commonVr->VR_USE_MOTION_CONTROLS && vr_autoSwitchControllers.GetBool() && ( button > oldButton[0] || trig > 0.25f || (fabs(padX) + fabs(padY) > 0.5f) ) )
						{
							commonVr->VR_USE_MOTION_CONTROLS = true;
							commonVr->motionControlType = MOTION_STEAMVR;
						}

						if ( button != oldButton[0] )
						{
							if ( (button & ButtonMaskFromId( vr::k_EButton_ApplicationMenu )) != (oldButton[0] & ButtonMaskFromId( vr::k_EButton_ApplicationMenu )) )
							{
								//common->Printf( "inputDeviceNum %d L AppMenu\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_LV_MENU, (button & ButtonMaskFromId( vr::k_EButton_ApplicationMenu )) > 0 );
							}

							if ( (button & ButtonMaskFromId( vr::k_EButton_Grip )) != (oldButton[0] & ButtonMaskFromId( vr::k_EButton_Grip )) )
							{
								//common->Printf( "inputDeviceNum %d L Grip\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_LV_GRIP, (button & ButtonMaskFromId( vr::k_EButton_Grip )) > 0 );
							}

							if ( (button & ButtonMaskFromId( vr::k_EButton_SteamVR_Trigger )) != (oldButton[0] & ButtonMaskFromId( vr::k_EButton_SteamVR_Trigger )) )
							{
								//common->Printf( "inputDeviceNum %d LTrig\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_LV_TRIGGER, (button & ButtonMaskFromId( vr::k_EButton_SteamVR_Trigger )) > 0 );
							}

							if ( (button & ButtonMaskFromId( vr::k_EButton_SteamVR_Touchpad )) != (oldButton[0] & ButtonMaskFromId( vr::k_EButton_SteamVR_Touchpad )) )
							{
								//common->Printf( "inputDeviceNum %d LPad\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_LV_PAD, (button & ButtonMaskFromId( vr::k_EButton_SteamVR_Touchpad )) > 0 );
							}

							if ( (button & ButtonMaskFromId( vr::k_EButton_A )) != (oldButton[0] & ButtonMaskFromId( vr::k_EButton_A )) )
							{
								//common->Printf( "inputDeviceNum %d L k_EButton_A\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_LSTEAMVR_A, (button & ButtonMaskFromId( vr::k_EButton_A )) > 0 );
							}

							if ( (button & (1ull << 8)) != (oldButton[0] & (1ull << 8)) )
							{
								//common->Printf( "inputDeviceNum %d L Button 8\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_LSTEAMVR_8, (button & (1ull << 8)) > 0 );
							}

							if ( (button & (1ull << 9)) != (oldButton[0] & (1ull << 9)) )
							{
								//common->Printf( "inputDeviceNum %d L Button 9\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_LSTEAMVR_9, (button & (1ull << 9)) > 0 );
							}

							if ( (button & (1ull << 10)) != (oldButton[0] & (1ull << 10)) )
							{
								//common->Printf( "inputDeviceNum %d L Button 10\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_LSTEAMVR_10, (button & (1ull << 10)) > 0 );
							}

							if ( (button & (1ull << 11)) != (oldButton[0] & (1ull << 11)) )
							{
								//common->Printf( "inputDeviceNum %d L Button 11\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_LSTEAMVR_11, (button & (1ull << 11)) > 0 );
							}

							if ( (button & (1ull << 12)) != (oldButton[0] & (1ull << 12)) )
							{
								//common->Printf( "inputDeviceNum %d L Button 12\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_LSTEAMVR_12, (button & (1ull << 12)) > 0 );
							}

							if ( (button & (1ull << 13)) != (oldButton[0] & (1ull << 13)) )
							{
								//common->Printf( "inputDeviceNum %d L Button 13\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_LSTEAMVR_13, (button & (1ull << 13)) > 0 );
							}

							if ( (button & (1ull << 14)) != (oldButton[0] & (1ull << 14)) )
							{
								//common->Printf( "inputDeviceNum %d L Button 14\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_LSTEAMVR_14, (button & (1ull << 14)) > 0 );
							}

							if ( (button & (1ull << 15)) != (oldButton[0] & (1ull << 15)) )
							{
								//common->Printf( "inputDeviceNum %d L Button 15\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_LSTEAMVR_15, (button & (1ull << 15)) > 0 );
							}


							oldButton[0] = button;
						}
					}

					//====================================================================
					//right steamvr controller

					
					if ( rGood )
					{
						for ( int axis = 0; axis < vr::k_unControllerStateAxisCount; axis++ )
						{
							axisType = vr::VRSystem()->GetInt32TrackedDeviceProperty( commonVr->rightControllerDeviceNo, (vr::ETrackedDeviceProperty) ((int)vr::Prop_Axis0Type_Int32 + axis) );
							if ( axisType == vr::k_eControllerAxis_Trigger )
							{
								triggerAxis[1] = axis;
							}
							if ( axisType == vr::k_eControllerAxis_TrackPad )
							{
								padAxis[1] = axis;
							}
							if (axisType == vr::k_eControllerAxis_Joystick)
							{
								jsAxis[1] = axis;
							}
						}

						// using (testing) the trigger button instead of reading the analog axis, so comment this out for now
						/*
						trig = currentStateR.rAxis[triggerAxis[1]].x;
						if ( trig != triggerVal[1] )
						{
						PostInputEvent( inputDeviceNum, J_AXIS_RIGHT_STEAMVR_TRIG, triggerVal[1] * 32767.0f );
						}
						triggerVal[1] = trig;
						*/
						
						padX = currentStateR.rAxis[padAxis[1]].x;
						padY = currentStateR.rAxis[padAxis[1]].y;
								
						//pad deadzone
						if ( fabs( padX ) < vr_padDeadzone.GetFloat() ) padX = 0.0f;
						if ( fabs( padY ) < vr_padDeadzone.GetFloat() ) padY = 0.0f;

						// the vive controllers ( at least mine, in multiple games, even after calibration) sometimes miss when a finger has been removed from the touchpad, 
						// and keep sending the exact same values until the pad is touched again.  This causes movement or rotation to get stuck on 
						// until the controller is touched again, which is really bad, especially if its rotation.
						// it's pretty much physically impossible to keep you finger so still on the pad it reports the exact same values 
						// more than a few times in a row, so count non zero repeats, and default the value to 0 if it's been repeated too many times.
						
						defaultX = false;
						defaultY = false;

						if ( vr_openVrStuckPadAxisFix.GetBool() )
						{
							if ( padX != 0.0f && padAxisX[1] == padX )
							{
								rXcount++;
								if ( rXcount > dupeThreshold )
								{
									padX = 0;
									defaultX = true;
									common->Printf( "Defaulting right X axis val %f time %d count %d\n", padX, Sys_Milliseconds(),rXcount );
								}
							}
							else
							{
								if ( rXcount > dupeThreshold ) common->Printf( "rXcount reset to 0 from %d\n", rXcount );
								rXcount = 0;
							}

							if ( padY != 0.0f && padAxisY[1] == padY )
							{
								rYcount++;
								if ( rYcount > dupeThreshold )
								{
									padY = 0;
									defaultY = true;
									common->Printf( "Defaulting right Y axis val %f time %d count %d\n", padY, Sys_Milliseconds(),rYcount );
								}
							}
							else
							{
								if ( rYcount > dupeThreshold ) common->Printf( "rYcount reset to 0 from %d\n", rYcount );
								rYcount = 0;
							}
						}


						//post the axes
						if ( padX != padAxisX[1] || defaultX )
						{
							//common->Printf( "Posting input event right steamvr pad x value %f time %d \n", padX, Sys_Milliseconds() );
							PostInputEvent( inputDeviceNum, J_AXIS_RIGHT_STEAMVR_X, padX * 32767.0f );
						}

						if ( padY != padAxisY[1] || defaultY )
						{
							//common->Printf( "Posting input event right steamvr pad y value %f time %d\n", padY, Sys_Milliseconds() );
							PostInputEvent( inputDeviceNum, J_AXIS_RIGHT_STEAMVR_Y, -padY * 32767.0f );
						}
												
						padAxisX[1] = padX;
						padAxisY[1] = padY;
						
						jsX = currentStateR.rAxis[jsAxis[1]].x;
						jsY = currentStateR.rAxis[jsAxis[1]].y;

						if (fabs(jsX) < vr_jsDeadzone.GetFloat()) jsX = 0.0f;
						if (fabs(jsY) < vr_jsDeadzone.GetFloat()) jsY = 0.0f;

						if (jsX != jsAxisX[1])
						{
							//common->Printf( "Posting input event left steamvr pad x value %f time %d\n", padX, Sys_Milliseconds() );
							PostInputEvent(inputDeviceNum, J_AXIS_RIGHT_JS_STEAMVR_X, jsX * 32767.0f);
							jsAxisX[1] = jsX;
						}

						if (jsY != jsAxisY[1])
						{
							//common->Printf( "Posting input event left steamvr pad y value %f time %d\n", padY, Sys_Milliseconds() );
							PostInputEvent(inputDeviceNum, J_AXIS_RIGHT_JS_STEAMVR_Y, -jsY * 32767.0f);
							jsAxisY[1] = jsY;
						}

						// process buttons ( appmenu, grip, trigger, touchpad pressed )
						button = currentStateR.ulButtonPressed;

						if( !commonVr->VR_USE_MOTION_CONTROLS && vr_autoSwitchControllers.GetBool() && ( button > oldButton[0] || trig > 0.25f || (fabs(padX) + fabs(padY) > 0.5f) ) )
						{
							commonVr->VR_USE_MOTION_CONTROLS = true;
							commonVr->motionControlType = MOTION_STEAMVR;
						}

						if ( button != oldButton[1] )
						{

							if ( (button & ButtonMaskFromId( vr::k_EButton_ApplicationMenu )) != (oldButton[1] & ButtonMaskFromId( vr::k_EButton_ApplicationMenu )) )
							{
								//common->Printf( "inputDeviceNum %d R AppMenu\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_RV_MENU, (button & ButtonMaskFromId( vr::k_EButton_ApplicationMenu )) > 0 );
							}

							if ( (button & ButtonMaskFromId( vr::k_EButton_Grip )) != (oldButton[1] & ButtonMaskFromId( vr::k_EButton_Grip )) )
							{
								//common->Printf( "inputDeviceNum %d R Grip\n", inputDeviceNum );
								PostInputEvent(inputDeviceNum, J_RV_GRIP, (button & ButtonMaskFromId(vr::k_EButton_Grip)) > 0);
							}

							if ( (button & ButtonMaskFromId( vr::k_EButton_SteamVR_Trigger )) != (oldButton[1] & ButtonMaskFromId( vr::k_EButton_SteamVR_Trigger )) )
							{
								//common->Printf( "inputDeviceNum %d R Trig\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_RV_TRIGGER, (button & ButtonMaskFromId( vr::k_EButton_SteamVR_Trigger )) > 0 );
							}

							if ( (button & ButtonMaskFromId( vr::k_EButton_SteamVR_Touchpad )) != (oldButton[1] & ButtonMaskFromId( vr::k_EButton_SteamVR_Touchpad )) )
							{
								//common->Printf( "inputDeviceNum %d R Pad\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_RV_PAD, (button & ButtonMaskFromId( vr::k_EButton_SteamVR_Touchpad )) > 0 );
							}

							if ( (button & ButtonMaskFromId( vr::k_EButton_A )) != (oldButton[1] & ButtonMaskFromId( vr::k_EButton_A )) )
							{
								//common->Printf( "inputDeviceNum %d R k_EButton_A\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_RSTEAMVR_A, (button & ButtonMaskFromId( vr::k_EButton_A )) > 0 );
							}




							if ( (button & (1ull << 8)) != (oldButton[1] & (1ull << 8)) )
							{
								//common->Printf( "inputDeviceNum %d R Button 8\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_RSTEAMVR_8, (button & (1ull << 8)) > 0 );
							}

							if ( (button & (1ull << 9)) != (oldButton[1] & (1ull << 9)) )
							{
								//common->Printf( "inputDeviceNum %d R Button 9\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_RSTEAMVR_9, (button & (1ull << 9)) > 0 );
							}

							if ( (button & (1ull << 10)) != (oldButton[1] & (1ull << 10)) )
							{
								//common->Printf( "inputDeviceNum %d R Button 10\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_RSTEAMVR_10, (button & (1ull << 10)) > 0 );
							}

							if ( (button & (1ull << 11)) != (oldButton[1] & (1ull << 11)) )
							{
								//common->Printf( "inputDeviceNum %d R Button 11\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_RSTEAMVR_11, (button & (1ull << 11)) > 0 );
							}

							if ( (button & (1ull << 12)) != (oldButton[1] & (1ull << 12)) )
							{
								//common->Printf( "inputDeviceNum %d R Button 12\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_RSTEAMVR_12, (button & (1ull << 12)) > 0 );
							}

							if ( (button & (1ull << 13)) != (oldButton[1] & (1ull << 13)) )
							{
								//common->Printf( "inputDeviceNum %d R Button 13\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_RSTEAMVR_13, (button & (1ull << 13)) > 0 );
							}

							if ( (button & (1ull << 14)) != (oldButton[1] & (1ull << 14)) )
							{
								//common->Printf( "inputDeviceNum %d R Button 14\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_RSTEAMVR_14, (button & (1ull << 14)) > 0 );
							}

							if ( (button & (1ull << 15)) != (oldButton[1] & (1ull << 15)) )
							{
								//common->Printf( "inputDeviceNum %d R Button 15\n", inputDeviceNum );
								PostInputEvent( inputDeviceNum, J_RSTEAMVR_15, (button & (1ull << 15)) > 0 );
							}

							oldButton[1] = button;
						}
					}
				}

				//=================================================
				// Koz begin add touch controls
				// Carl: There's never any reason not to recognise button presses or joysticks from Touch controllers.
				// Touch controllers will turn themselves off if not in use, and send no buttons.
				// And Touch controllers were cleverly designed so if you place them on a flat surface, no buttons are bumped.
#ifdef USE_OVR
				if ( commonVr->hasOculusRift ) // was ( commonVr->VR_USE_MOTION_CONTROLS && commonVr->motionControlType == MOTION_OCULUS )
				{

					static ovrInputState oldInputState;
					ovrInputState    inputState;

					if ( OVR_SUCCESS( ovr_GetInputState( commonVr->hmdSession, ovrControllerType_Touch, &inputState ) ) )
					{
						// Carl: Update VR_USE_MOTION_CONTROLS if we've just started using Touch
						if( !commonVr->VR_USE_MOTION_CONTROLS  && vr_autoSwitchControllers.GetBool() && !vr_controllerStandard.GetInteger() && (
							inputState.Buttons || inputState.HandTrigger[0] > 0.25f || inputState.HandTrigger[1] > 0.25f || inputState.IndexTrigger[0] > 0.25f || inputState.IndexTrigger[1] > 0.25f
							|| (fabs(inputState.Thumbstick[0].x) + fabs(inputState.Thumbstick[0].y) > 0.5f) || (fabs(inputState.Thumbstick[1].x) + fabs(inputState.Thumbstick[1].y) > 0.5f)
							|| (inputState.Touches & (ovrTouch_LButtonMask | ovrTouch_RButtonMask)) ) )
						{
							unsigned int ctrlrs = ovr_GetConnectedControllerTypes( commonVr->hmdSession );
							if( ( ctrlrs & ovrControllerType_Touch ) != 0 )
							{
								commonVr->VR_USE_MOTION_CONTROLS = true;
								commonVr->motionControlType = MOTION_OCULUS;
							}
						}

						if ( (inputState.Buttons & ovrButton_A) != (oldInputState.Buttons & ovrButton_A) )
						{
							//common->Printf( "Posting input event for r_touch_a val : %d\n", (inputState.Buttons & ovrButton_A)  );
							PostInputEvent( inputDeviceNum, J_RT_A, (inputState.Buttons & ovrButton_A) );
						}

						if ( (inputState.Buttons & ovrButton_B) != (oldInputState.Buttons & ovrButton_B) )
						{
							PostInputEvent( inputDeviceNum, J_RT_B, (inputState.Buttons & ovrButton_B) );
						}

						if ( (inputState.Buttons & ovrButton_Home) != (oldInputState.Buttons & ovrButton_Home) )
						{
							PostInputEvent( inputDeviceNum, J_RT_OCULUS, (inputState.Buttons & ovrButton_Home) );
						}

						if ( (inputState.Buttons & ovrButton_RThumb) != (oldInputState.Buttons & ovrButton_RThumb) )
						{
							PostInputEvent( inputDeviceNum, J_RT_STICK, (inputState.Buttons & ovrButton_RThumb) );
						}

						if ( (inputState.Touches & ovrTouch_RThumbRest) != (oldInputState.Touches & ovrTouch_RThumbRest) )
						{
							PostInputEvent( inputDeviceNum, J_RT_REST, (inputState.Touches & ovrTouch_RThumbRest) );
						}

						// left touch
						if ( (inputState.Buttons & ovrButton_X) != (oldInputState.Buttons & ovrButton_X) )
						{
							PostInputEvent( inputDeviceNum, J_LT_X, (inputState.Buttons & ovrButton_X) );
						}

						if ( (inputState.Buttons & ovrButton_Y) != (oldInputState.Buttons & ovrButton_Y) )
						{
							PostInputEvent( inputDeviceNum, J_LT_Y, (inputState.Buttons & ovrButton_Y) );
						}

						if ( (inputState.Buttons & ovrButton_Enter) != (oldInputState.Buttons & ovrButton_Enter) )
						{
							PostInputEvent( inputDeviceNum, J_LT_MENU, (inputState.Buttons & ovrButton_Enter) );
						}

						if ( (inputState.Buttons & ovrButton_LThumb) != (oldInputState.Buttons & ovrButton_LThumb) )
						{
							PostInputEvent( inputDeviceNum, J_LT_STICK, (inputState.Buttons & ovrButton_LThumb) );
						}

						if ( (inputState.Touches & ovrTouch_LThumbRest) != (oldInputState.Touches & ovrTouch_LThumbRest) )
						{
							PostInputEvent( inputDeviceNum, J_LT_REST, (inputState.Touches & ovrTouch_LThumbRest) );
						}

						if ( (inputState.HandTrigger[ovrHand_Left] > 0.9f) != (oldInputState.HandTrigger[ovrHand_Left] > 0.9f) )
						{
							PostInputEvent(inputDeviceNum, J_LT_GRIP, inputState.HandTrigger[ovrHand_Left] > 0.9f);
						}

						if ( (inputState.IndexTrigger[ovrHand_Left] > 0.25f) != (oldInputState.IndexTrigger[ovrHand_Left] > 0.25f) )
						{
							PostInputEvent( inputDeviceNum, J_LT_TRIGGER, inputState.IndexTrigger[ovrHand_Left] > 0.25f );
						}

						if ( (inputState.HandTrigger[ovrHand_Right] > 0.9f) != (oldInputState.HandTrigger[ovrHand_Right] > 0.9f) )
						{
							PostInputEvent(inputDeviceNum, J_RT_GRIP, inputState.HandTrigger[ovrHand_Right] > 0.9f);
						}

						if ( (inputState.IndexTrigger[ovrHand_Right] > 0.25f) != (oldInputState.IndexTrigger[ovrHand_Right] > 0.25f) )
						{
							PostInputEvent( inputDeviceNum, J_RT_TRIGGER, inputState.IndexTrigger[ovrHand_Right] > 0.25f );
						}

						PostInputEvent( inputDeviceNum, J_AXIS_LEFT_TOUCH_X, inputState.Thumbstick[ovrHand_Left].x * 32767.0f );
						PostInputEvent( inputDeviceNum, J_AXIS_LEFT_TOUCH_Y, -inputState.Thumbstick[ovrHand_Left].y * 32767.0f );

						PostInputEvent( inputDeviceNum, J_AXIS_RIGHT_TOUCH_X, inputState.Thumbstick[ovrHand_Right].x * 32767.0f );
						PostInputEvent( inputDeviceNum, J_AXIS_RIGHT_TOUCH_Y, -inputState.Thumbstick[ovrHand_Right].y * 32767.0f );

						oldInputState = inputState;

						// add finger poses
						
						int fingerPose = POSE_FINGER;
						
						// left hand
						//if ( inputState.Touches & ovrTouch_LThumb ) fingerPose |= POSE_THUMB;
						if ( inputState.Touches & ovrTouch_LThumbRest) fingerPose |= POSE_THUMB;
						if ( inputState.Touches & ovrTouch_X ) fingerPose |= POSE_THUMB;
						if ( inputState.Touches & ovrTouch_Y ) fingerPose |= POSE_THUMB;
						if ( inputState.Touches & ovrTouch_LIndexTrigger ) fingerPose |= POSE_INDEX;
						if ( inputState.HandTrigger[ovrHand_Left] > 0.015f ) fingerPose |= POSE_GRIP;
						commonVr->fingerPose[HAND_LEFT] = fingerPose;
						
						//right hand
						fingerPose = POSE_FINGER;
						//if ( inputState.Touches & ovrTouch_RThumb ) fingerPose |= POSE_THUMB;
						if ( inputState.Touches & ovrTouch_RThumbRest ) fingerPose |= POSE_THUMB;
						if ( inputState.Touches & ovrTouch_A ) fingerPose |= POSE_THUMB;
						if ( inputState.Touches & ovrTouch_B ) fingerPose |= POSE_THUMB;
						if ( inputState.Touches & ovrTouch_RIndexTrigger ) fingerPose |= POSE_INDEX;
						if ( inputState.HandTrigger[ovrHand_Right] > 0.015f ) fingerPose |= POSE_GRIP;
						commonVr->fingerPose[HAND_RIGHT] = fingerPose;
												
					}
				}
#endif
			} // end if inputdeviceno == 0
		}
	return numEvents;
}


/*
========================
idJoystickWin32::ReturnInputEvent
========================
*/
int idJoystickWin32::ReturnInputEvent( const int n, int& action, int& value )
{
	if( ( n < 0 ) || ( n >= MAX_JOY_EVENT ) )
	{
		return 0;
	}
	
	action = events[ n ].event;
	value = events[ n ].value;
	
	return 1;
}

/*
========================
idJoystickWin32::PushButton
========================
*/
void idJoystickWin32::PushButton( int inputDeviceNum, int key, bool value )
{
	// So we don't keep sending the same SE_KEY message over and over again
	if( buttonStates[inputDeviceNum][key] != value )
	{
		//common->Printf( "Button state %i set to %i\n", key, value );
		buttonStates[inputDeviceNum][key] = value;
		Sys_QueEvent( SE_KEY, key, value, 0, NULL, inputDeviceNum );
	}
}
