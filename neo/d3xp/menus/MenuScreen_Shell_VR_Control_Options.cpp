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
#include "../Game_local.h"

const static int NUM_SYSTEM_VR_CONTROL_OPTIONS = 3;

float LinearAdjust( const float input, const float currentMin, const float currentMax, const float desiredMin,  float desiredMax );
int	AdjustOption( const int currentValue, const int values[], const int numValues, const int adjustment );

/*
========================
idMenuScreen_Shell_VR_Control_Options::Initialize
========================
*/
void idMenuScreen_Shell_VR_Control_Options::Initialize( idMenuHandler * data ) {
	idMenuScreen::Initialize( data );

	if ( data != NULL ) {
		menuGUI = data->GetGUI();
	}

	SetSpritePath( "menuSystemOptions" );

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	options->SetNumVisibleOptions( NUM_SYSTEM_VR_CONTROL_OPTIONS );
	options->SetSpritePath( GetSpritePath(), "info", "options" );
	options->SetWrappingAllowed( true );
	options->SetControlList( true );
	options->Initialize( data );

	btnBack = new (TAG_SWF) idMenuWidget_Button();
	btnBack->Initialize( data );
	btnBack->SetLabel( "VR Control Options" );
	btnBack->SetSpritePath( GetSpritePath(), "info", "btnBack" );
	btnBack->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_GO_BACK );

	AddChild( options );
	AddChild( btnBack );

	idMenuWidget_ControlButton * control;
	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "VR Controls" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Control_Options::VR_CONTROLS_FIELD_ENABLE );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Control_Options::VR_CONTROLS_FIELD_ENABLE );
	options->AddChild( control );
		

	options->AddEventAction( WIDGET_EVENT_SCROLL_DOWN ).Set( new (TAG_SWF) idWidgetActionHandler( options, WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER, WIDGET_EVENT_SCROLL_DOWN ) );
	options->AddEventAction( WIDGET_EVENT_SCROLL_UP ).Set( new (TAG_SWF) idWidgetActionHandler( options, WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER, WIDGET_EVENT_SCROLL_UP ) );
	options->AddEventAction( WIDGET_EVENT_SCROLL_DOWN_RELEASE ).Set( new (TAG_SWF) idWidgetActionHandler( options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_DOWN_RELEASE ) );
	options->AddEventAction( WIDGET_EVENT_SCROLL_UP_RELEASE ).Set( new (TAG_SWF) idWidgetActionHandler( options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_UP_RELEASE ) );
	options->AddEventAction( WIDGET_EVENT_SCROLL_DOWN_LSTICK ).Set( new (TAG_SWF) idWidgetActionHandler( options, WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER, WIDGET_EVENT_SCROLL_DOWN_LSTICK ) );
	options->AddEventAction( WIDGET_EVENT_SCROLL_UP_LSTICK ).Set( new (TAG_SWF) idWidgetActionHandler( options, WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER, WIDGET_EVENT_SCROLL_UP_LSTICK ) );
	options->AddEventAction( WIDGET_EVENT_SCROLL_DOWN_LSTICK_RELEASE ).Set( new (TAG_SWF) idWidgetActionHandler( options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_DOWN_LSTICK_RELEASE ) );
	options->AddEventAction( WIDGET_EVENT_SCROLL_UP_LSTICK_RELEASE ).Set( new (TAG_SWF) idWidgetActionHandler( options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_UP_LSTICK_RELEASE ) );
}

/*
========================
idMenuScreen_Shell_VR_Control_Options::Update
========================
*/
void idMenuScreen_Shell_VR_Control_Options::Update() {

	if ( menuData != NULL ) {
		idMenuWidget_CommandBar * cmdBar = menuData->GetCmdBar();
		if ( cmdBar != NULL ) {
			cmdBar->ClearAllButtons();
			idMenuWidget_CommandBar::buttonInfo_t * buttonInfo;			
			buttonInfo = cmdBar->GetButton( idMenuWidget_CommandBar::BUTTON_JOY2 );
			if ( menuData->GetPlatform() != 2 ) {
				buttonInfo->label = "#str_00395";
			}
			buttonInfo->action.Set( WIDGET_ACTION_GO_BACK );

			buttonInfo = cmdBar->GetButton( idMenuWidget_CommandBar::BUTTON_JOY1 );
			buttonInfo->action.Set( WIDGET_ACTION_PRESS_FOCUSED );
		}		
	}

	idSWFScriptObject & root = GetSWFObject()->GetRootObject();
	if ( BindSprite( root ) ) {
		idSWFTextInstance * heading = GetSprite()->GetScriptObject()->GetNestedText( "info", "txtHeading" );
		if ( heading != NULL ) {
			heading->SetText( "VR Settings" );	// FULLSCREEN
			heading->SetStrokeInfo( true, 0.75f, 1.75f );
		}

		idSWFSpriteInstance * gradient = GetSprite()->GetScriptObject()->GetNestedSprite( "info", "gradient" );
		if ( gradient != NULL && heading != NULL ) {
			gradient->SetXPos( heading->GetTextLength() );
		}
	}

	if ( btnBack != NULL ) {
		btnBack->BindSprite( root );
	}

	idMenuScreen::Update();
}

/*
========================
idMenuScreen_Shell_VR_Control_Options::ShowScreen
========================
*/
void idMenuScreen_Shell_VR_Control_Options::ShowScreen( const mainMenuTransition_t transitionType ) {
	
	systemData.LoadData();
	
	idMenuScreen::ShowScreen( transitionType );
}

/*
========================
idMenuScreen_Shell_VR_Control_Options::HideScreen
========================
*/
void idMenuScreen_Shell_VR_Control_Options::HideScreen( const mainMenuTransition_t transitionType ) {

	if ( systemData.IsRestartRequired() ) {
		class idSWFScriptFunction_Restart : public idSWFScriptFunction_RefCounted {
		public:
			idSWFScriptFunction_Restart( gameDialogMessages_t _msg, bool _restart ) {
				msg = _msg;
				restart = _restart;
			}
			idSWFScriptVar Call( idSWFScriptObject * thisObject, const idSWFParmList & parms ) {
				common->Dialog().ClearDialog( msg );
				if ( restart ) {
					idStr cmdLine = Sys_GetCmdLine();
					if ( cmdLine.Find( "com_skipIntroVideos" ) < 0 ) {
						cmdLine.Append( " +set com_skipIntroVideos 1" );
					}
					Sys_ReLaunch(); // Sys_ReLaunch no longer needs params
				}
				return idSWFScriptVar();
			}
		private:
			gameDialogMessages_t msg;
			bool restart;
		};
		idStaticList<idSWFScriptFunction *, 4> callbacks;
		idStaticList<idStrId, 4> optionText;
		callbacks.Append( new idSWFScriptFunction_Restart( GDM_GAME_RESTART_REQUIRED, false ) );
		callbacks.Append( new idSWFScriptFunction_Restart( GDM_GAME_RESTART_REQUIRED, true ) );
		optionText.Append( idStrId( "#str_00100113" ) ); // Continue
		optionText.Append( idStrId( "#str_02487" ) ); // Restart Now
		common->Dialog().AddDynamicDialog( GDM_GAME_RESTART_REQUIRED, callbacks, optionText, true, idStr() );
	}

	if ( systemData.IsDataChanged() ) {
		systemData.CommitData();
	}

	idMenuScreen::HideScreen( transitionType );
}

/*
========================
idMenuScreen_Shell_VR_Control_Options::HandleAction h
========================
*/
bool idMenuScreen_Shell_VR_Control_Options::HandleAction( idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled ) {

	if ( menuData == NULL ) {
		return true;
	}
	
	if ( menuData->ActiveScreen() != SHELL_AREA_VR_CONTROL_OPTIONS ) {
		return false;
	}

	widgetAction_t actionType = action.GetType();
	const idSWFParmList & parms = action.GetParms();

	switch ( actionType ) {
		case WIDGET_ACTION_GO_BACK: {
			if ( menuData != NULL ) {
				menuData->SetNextScreen( SHELL_AREA_VR_SETTINGS, MENU_TRANSITION_SIMPLE );
			}
			return true;
		}
		/*case WIDGET_ACTION_ADJUST_FIELD:
			if ( widget->GetDataSourceFieldIndex() == idMenuDataSource_Shell_VR_Control_Options::SYSTEM_FIELD_FULLSCREEN ) {
				menuData->SetNextScreen( SHELL_AREA_RESOLUTION, MENU_TRANSITION_SIMPLE );
				return true;
			}
			break; */
		case WIDGET_ACTION_COMMAND: {

			if ( options == NULL ) {
				return true;
			}

			int selectionIndex = options->GetFocusIndex();
			if ( parms.Num() > 0 ) {
				selectionIndex = parms[0].ToInteger();
			}

			if ( options && selectionIndex != options->GetFocusIndex() ) {
				options->SetViewIndex( options->GetViewOffset() + selectionIndex );
				options->SetFocusIndex( selectionIndex );
			}

			switch ( parms[0].ToInteger() ) {
			case 1: /*idMenuDataSource_Shell_VR_Control_Options::SYSTEM_FIELD_FULLSCREEN: {
					menuData->SetNextScreen( SHELL_AREA_RESOLUTION, MENU_TRANSITION_SIMPLE );
					return true;
				} */
				default: {
					systemData.AdjustField( parms[0].ToInteger(), 1 );
					options->Update();
				}
			}

			return true;
		}
		case WIDGET_ACTION_START_REPEATER: {

			if ( options == NULL ) {
				return true;
			}

			if ( parms.Num() == 4 ) {
				int selectionIndex = parms[3].ToInteger();
				if ( selectionIndex != options->GetFocusIndex() ) {
					options->SetViewIndex( options->GetViewOffset() + selectionIndex );
					options->SetFocusIndex( selectionIndex );
				}
			}
			break;
		}
	}

	return idMenuWidget::HandleAction( action, event, widget, forceHandled );
}

/////////////////////////////////
// SCREEN SETTINGS
/////////////////////////////////

/*
========================
idMenuScreen_Shell_VR_Control_Options::idMenuDataSource_Shell_VR_Control_Options::idMenuDataSource_Shell_VR_Control_Options
========================
*/
idMenuScreen_Shell_VR_Control_Options::idMenuDataSource_Shell_VR_Control_Options::idMenuDataSource_Shell_VR_Control_Options() {
}

/*
========================
idMenuScreen_Shell_VR_Control_Options::idMenuDataSource_Shell_VR_Control_Options::LoadData
========================
*/
void idMenuScreen_Shell_VR_Control_Options::idMenuDataSource_Shell_VR_Control_Options::LoadData() {
	//originalFramerate = com_engineHz.GetInteger();
	//originalAntialias = r_multiSamples.GetInteger();
	//originalMotionBlur = r_motionBlur.GetInteger();
	//originalVsync = r_swapInterval.GetInteger();
	//originalBrightness = r_lightScale.GetFloat();
	//originalVolume = s_volume_dB.GetFloat();

	//const int fullscreen = r_fullscreen.GetInteger();
	//if ( fullscreen > 0 ) {
	//	R_GetModeListForDisplay( fullscreen - 1, modeList );
	//} else {
	///	modeList.Clear();
	//}
}

/*
========================
idMenuScreen_Shell_VR_Control_Options::idMenuDataSource_Shell_VR_Control_Options::IsRestartRequired
========================
*/
bool idMenuScreen_Shell_VR_Control_Options::idMenuDataSource_Shell_VR_Control_Options::IsRestartRequired() const {
//	if ( originalAntialias != r_multiSamples.GetInteger() ) {
	//	return true;
//	}
//	if ( originalFramerate != com_engineHz.GetInteger() ) {
//		return true;
//	}
	return false;
}

/*
========================
idMenuScreen_Shell_VR_Control_Options::idMenuDataSource_Shell_VR_Control_Options::CommitData
========================
*/
void idMenuScreen_Shell_VR_Control_Options::idMenuDataSource_Shell_VR_Control_Options::CommitData() {
	cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
}

/*
========================
AdjustOption
Given a current value in an array of possible values, returns the next n value
========================

int idMenuScreen_Shell_VR_Control_Options::idMenuDataSource_Shell_VR_Control_Options::AdjustOption( int currentValue, const int values[], int numValues, int adjustment ) {
	int index = 0;
	for ( int i = 0; i < numValues; i++ ) {
		if ( currentValue == values[i] ) {
			index = i;
			break;
		}
	}
	index += adjustment;
	while ( index < 0 ) {
		index += numValues;
	}
	index %= numValues;
	return values[index];
}


========================
LinearAdjust
Linearly converts a float from one scale to another
========================

float idMenuScreen_Shell_VR_Control_Options::idMenuDataSource_Shell_VR_Control_Options::LinearAdjust( float input, float currentMin, float currentMax, float desiredMin, float desiredMax ) {
	return ( ( input - currentMin ) / ( currentMax - currentMin ) ) * ( desiredMax - desiredMin ) + desiredMin;
}
*/
/*
========================
idMenuScreen_Shell_VR_Control_Options::idMenuDataSource_Shell_VR_Control_Options::AdjustField
========================
*/
void idMenuScreen_Shell_VR_Control_Options::idMenuDataSource_Shell_VR_Control_Options::AdjustField( const int fieldIndex, const int adjustAmount ) {
	switch ( fieldIndex ) {
	
	case 1: 
	default :	;
		/*	case SYSTEM_FIELD_FRAMERATE: {
			//Carl: Oculus Rift DK1 can be hacked to use refresh rates from 60Hz to 83Hz (71Hz max undistorted). CV1 will probably support 90Hz.
			//Carl: Doom 3 BFG also originally supported 120Hz. So list everything from 60 to 83, with 90 and 120 last.
			static const int numValues = 26;
			static const int values[numValues] = { 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 90, 120 };
			com_engineHz.SetInteger( AdjustOption( com_engineHz.GetInteger(), values, numValues, adjustAmount ) );
			break;
		}
		case SYSTEM_FIELD_VSYNC: {
			static const int numValues = 3;
			static const int values[numValues] = { 0, 1, 2 };
			r_swapInterval.SetInteger( AdjustOption( r_swapInterval.GetInteger(), values, numValues, adjustAmount ) );
			break;
		}
		case SYSTEM_FIELD_ANTIALIASING: {
			static const int numValues = 5;
			static const int values[numValues] = { 0, 2, 4, 8, 16 };
			r_multiSamples.SetInteger( AdjustOption( r_multiSamples.GetInteger(), values, numValues, adjustAmount ) );
			break;
		}
		case SYSTEM_FIELD_MOTIONBLUR: {
			static const int numValues = 5;
			static const int values[numValues] = { 0, 2, 3, 4, 5 };
			r_motionBlur.SetInteger( AdjustOption( r_motionBlur.GetInteger(), values, numValues, adjustAmount ) );
			break;
		}
		case SYSTEM_FIELD_LODBIAS: {
			const float percent = LinearAdjust( r_lodBias.GetFloat(), -1.0f, 1.0f, 0.0f, 100.0f );
			const float adjusted = percent + (float)adjustAmount * 5.0f;
			const float clamped = idMath::ClampFloat( 0.0f, 100.0f, adjusted );
			r_lodBias.SetFloat( LinearAdjust( clamped, 0.0f, 100.0f, -1.0f, 1.0f ) );
			break;
		}
		case SYSTEM_FIELD_BRIGHTNESS: {
			const float percent = LinearAdjust( r_lightScale.GetFloat(), 2.0f, 4.0f, 0.0f, 100.0f );
			const float adjusted = percent + (float)adjustAmount;
			const float clamped = idMath::ClampFloat( 0.0f, 100.0f, adjusted );
			r_lightScale.SetFloat( LinearAdjust( clamped, 0.0f, 100.0f, 2.0f, 4.0f ) );
			break;
		}
		case SYSTEM_FIELD_VOLUME: {
			const float percent = 100.0f * Square( 1.0f - ( s_volume_dB.GetFloat() / DB_SILENCE ) );
			const float adjusted = percent + (float)adjustAmount;
			const float clamped = idMath::ClampFloat( 0.0f, 100.0f, adjusted );
			s_volume_dB.SetFloat( DB_SILENCE - ( idMath::Sqrt( clamped / 100.0f ) * DB_SILENCE ) );
			break;
		} */
	}
	cvarSystem->ClearModifiedFlags( CVAR_ARCHIVE );
}

/*
========================
idMenuScreen_Shell_VR_Control_Options::idMenuDataSource_Shell_VR_Control_Options::GetField	
========================
*/
idSWFScriptVar idMenuScreen_Shell_VR_Control_Options::idMenuDataSource_Shell_VR_Control_Options::GetField( const int fieldIndex ) const {
	switch ( fieldIndex ) {
		
	case 1:
	default :; /*
	case SYSTEM_FIELD_FULLSCREEN: {
			const int fullscreen = r_fullscreen.GetInteger();
			const int vidmode = r_vidMode.GetInteger();
			if ( fullscreen == 0 ) {
				return "#str_swf_disabled";
			}
			if ( fullscreen < 0 || vidmode < 0 || vidmode >= modeList.Num() ) {
				return "???";
			}
			if ( modeList[vidmode].displayHz == 60 ) {
				return va( "%4i x %4i", modeList[vidmode].width, modeList[vidmode].height );
			} else {
				return va( "%4i x %4i @ %dhz", modeList[vidmode].width, modeList[vidmode].height, modeList[vidmode].displayHz );
			}
		}
		case SYSTEM_FIELD_FRAMERATE:
			return va( "%d FPS", com_engineHz.GetInteger() );
		case SYSTEM_FIELD_VSYNC:
			if ( r_swapInterval.GetInteger() == 1 ) {
				return "#str_swf_smart";
			} else if ( r_swapInterval.GetInteger() == 2 ) {
				return "#str_swf_enabled";
			} else {
				return "#str_swf_disabled";
			}
		case SYSTEM_FIELD_ANTIALIASING:
			if ( r_multiSamples.GetInteger() == 0 ) {
				return "#str_swf_disabled";
			}
			return va( "%dx", r_multiSamples.GetInteger() );
		case SYSTEM_FIELD_MOTIONBLUR:
			if ( r_motionBlur.GetInteger() == 0 ) {
				return "#str_swf_disabled";
			}
			return va( "%dx", idMath::IPow( 2, r_motionBlur.GetInteger() ) );
		case SYSTEM_FIELD_LODBIAS:
			return LinearAdjust( r_lodBias.GetFloat(), -1.0f, 1.0f, 0.0f, 100.0f );
		case SYSTEM_FIELD_BRIGHTNESS:
			return LinearAdjust( r_lightScale.GetFloat(), 2.0f, 4.0f, 0.0f, 100.0f );
		case SYSTEM_FIELD_VOLUME: {
			return 100.0f * Square( 1.0f - ( s_volume_dB.GetFloat() / DB_SILENCE ) );
		}
	} */
	return false;
}
}

/*
========================
idMenuScreen_Shell_VR_Control_Options::idMenuDataSource_Shell_VR_Control_Options::IsDataChanged	
========================
*/
bool idMenuScreen_Shell_VR_Control_Options::idMenuDataSource_Shell_VR_Control_Options::IsDataChanged() const {
	/*if ( originalFramerate != com_engineHz.GetInteger() ) {
		return true;
	}
	if ( originalAntialias != r_multiSamples.GetInteger() ) {
		return true;
	}
	if ( originalMotionBlur != r_motionBlur.GetInteger() ) {
		return true;
	}
	if ( originalVsync != r_swapInterval.GetInteger() ) {
		return true;
	}
	if ( originalBrightness != r_lightScale.GetFloat() ) {
		return true;
	}
	if ( originalVolume != s_volume_dB.GetFloat() ) {
		return true;
	}*/
	return false;
}
