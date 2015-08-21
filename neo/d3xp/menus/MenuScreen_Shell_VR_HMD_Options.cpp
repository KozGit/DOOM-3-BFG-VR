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

const static int NUM_VR_HMD_OPTIONS_OPTIONS = 8;

extern idCVar vr_pixelDensity;
extern idCVar vr_vignette;
extern idCVar vr_scale;

float LinearAdjust( const float input, const float currentMin, const float currentMax, const float desiredMin,  float desiredMax );
int	AdjustOption( const int currentValue, const int values[], const int numValues, const int adjustment );

/*
========================
idMenuScreen_Shell_VR_HMD_Options::Initialize
========================
*/
void idMenuScreen_Shell_VR_HMD_Options::Initialize( idMenuHandler * data ) {
	idMenuScreen::Initialize( data );

	if ( data != NULL ) {
		menuGUI = data->GetGUI();
	}

	SetSpritePath( "menuSystemOptions" );

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	options->SetNumVisibleOptions( NUM_VR_HMD_OPTIONS_OPTIONS );
	options->SetSpritePath( GetSpritePath(), "info", "options" );
	options->SetWrappingAllowed( true );
	options->SetControlList( true );
	options->Initialize( data );

	btnBack = new (TAG_SWF) idMenuWidget_Button();
	btnBack->Initialize( data );
	btnBack->SetLabel( "VR Options" ); // #str_swf_settings
	btnBack->SetSpritePath( GetSpritePath(), "info", "btnBack" );
	btnBack->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_GO_BACK );

	AddChild( options );
	AddChild( btnBack );

	idMenuWidget_ControlButton * control;
		
	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "Vignette" );
	control->SetDataSource( &systemData, idMenuDataSource_VR_HMD_Options::HMD_OPTIONS_FIELD_VIGNETTE );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_VR_HMD_Options::HMD_OPTIONS_FIELD_VIGNETTE );
	options->AddChild( control );
		
	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "Pixel Density" );
	control->SetDataSource( &systemData, idMenuDataSource_VR_HMD_Options::HMD_OPTIONS_FIELD_PIXEL_DENSITY );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_VR_HMD_Options::HMD_OPTIONS_FIELD_PIXEL_DENSITY );
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
idMenuScreen_Shell_VR_HMD_Options::Update
========================
*/
void idMenuScreen_Shell_VR_HMD_Options::Update() {

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
			heading->SetText( "HMD Settings" );	
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
idMenuScreen_Shell_VR_HMD_Options::ShowScreen
========================
*/
void idMenuScreen_Shell_VR_HMD_Options::ShowScreen( const mainMenuTransition_t transitionType ) {
	
	systemData.LoadData();
	
	idMenuScreen::ShowScreen( transitionType );
}

/*
========================
idMenuScreen_Shell_VR_HMD_Options::HideScreen
========================
*/
void idMenuScreen_Shell_VR_HMD_Options::HideScreen( const mainMenuTransition_t transitionType ) {

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
idMenuScreen_Shell_VR_HMD_Options::HandleAction h
========================
*/
bool idMenuScreen_Shell_VR_HMD_Options::HandleAction( idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled ) {

	if ( menuData == NULL ) {
		return true;
	}
	
	if ( menuData->ActiveScreen() != SHELL_AREA_VR_HMD_OPTIONS ) {
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
			if ( widget->GetDataSourceFieldIndex() == idMenuDataSource_VR_HMD_Options::HMD_OPTIONS_FIELD_MANUAL_IPD ) {
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
				/*case idMenuDataSource_VR_HMD_Options::SYSTEM_FIELD_FULLSCREEN: {
					menuData->SetNextScreen( SHELL_AREA_RESOLUTION, MENU_TRANSITION_SIMPLE );
					return true;
				} */
				case 1:

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
idMenuScreen_Shell_VR_HMD_Options::idMenuDataSource_VR_HMD_Options::idMenuDataSource_VR_HMD_Options
========================
*/
idMenuScreen_Shell_VR_HMD_Options::idMenuDataSource_VR_HMD_Options::idMenuDataSource_VR_HMD_Options() {
}

/*
========================
idMenuScreen_Shell_VR_HMD_Options::idMenuDataSource_VR_HMD_Options::LoadData
========================
*/
void idMenuScreen_Shell_VR_HMD_Options::idMenuDataSource_VR_HMD_Options::LoadData() {
		
	originalVignette = vr_vignette.GetInteger();
	originalPixelDensity = vr_pixelDensity.GetFloat();
}

/*
========================
idMenuScreen_Shell_VR_HMD_Options::idMenuDataSource_VR_HMD_Options::IsRestartRequired
========================
*/
bool idMenuScreen_Shell_VR_HMD_Options::idMenuDataSource_VR_HMD_Options::IsRestartRequired() const {
	
	if ( originalVignette != vr_vignette.GetInteger() ) {
		return true;
	}
	if ( originalPixelDensity != vr_pixelDensity.GetFloat() ) {
		return true;
	}

	return false;
}

/*
========================
idMenuScreen_Shell_VR_HMD_Options::idMenuDataSource_VR_HMD_Options::CommitData
========================
*/
void idMenuScreen_Shell_VR_HMD_Options::idMenuDataSource_VR_HMD_Options::CommitData() {
	cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
}

/*
========================
AdjustOption
Given a current value in an array of possible values, returns the next n value
========================

int idMenuScreen_Shell_VR_HMD_Options::idMenuDataSource_VR_HMD_Options::AdjustOption( int currentValue, const int values[], int numValues, int adjustment ) {
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

float idMenuScreen_Shell_VR_HMD_Options::idMenuDataSource_VR_HMD_Options::LinearAdjust( float input, float currentMin, float currentMax, float desiredMin, float desiredMax ) {
	return ( ( input - currentMin ) / ( currentMax - currentMin ) ) * ( desiredMax - desiredMin ) + desiredMin;
}
*/
/*
========================
idMenuScreen_Shell_VR_HMD_Options::idMenuDataSource_VR_HMD_Options::AdjustField
========================
*/
void idMenuScreen_Shell_VR_HMD_Options::idMenuDataSource_VR_HMD_Options::AdjustField( const int fieldIndex, const int adjustAmount ) {
	switch ( fieldIndex ) {
		
		
		
		case HMD_OPTIONS_FIELD_VIGNETTE: {
			static const int numValues = 2;
			static const int values[numValues] = { 0, 1 };
			vr_vignette.SetInteger( AdjustOption( vr_vignette.GetInteger(), values, numValues, adjustAmount ) );
			break;
		}
		case HMD_OPTIONS_FIELD_PIXEL_DENSITY: {
			const float percent = LinearAdjust(vr_pixelDensity.GetFloat(), .5f, 2.0f, 0.0f, 100.0f );
			const float adjusted = percent + (float)adjustAmount * 5.0f;
			const float clamped = idMath::ClampFloat( 0.0f, 100.0f, adjusted );
			vr_pixelDensity.SetFloat( LinearAdjust( clamped, 0.0f, 100.0f, .5f, 2.0f ) );
			break;
		}
		
	}
	cvarSystem->ClearModifiedFlags( CVAR_ARCHIVE );
}

/*
========================
idMenuScreen_Shell_VR_HMD_Options::idMenuDataSource_VR_HMD_Options::GetField	
========================
*/
idSWFScriptVar idMenuScreen_Shell_VR_HMD_Options::idMenuDataSource_VR_HMD_Options::GetField( const int fieldIndex ) const {
	switch ( fieldIndex ) {
		
		case HMD_OPTIONS_FIELD_VIGNETTE:
			if ( vr_vignette.GetInteger() == 0 ) {
				return "#str_swf_disabled";
			} else {
				return "#str_swf_enabled";
			}
		
		case HMD_OPTIONS_FIELD_PIXEL_DENSITY:
			return va( "%1.2f", vr_pixelDensity.GetFloat() );
				
	}
	return false;
}

/*
========================
idMenuScreen_Shell_VR_HMD_Options::idMenuDataSource_VR_HMD_Options::IsDataChanged	
========================
*/
bool idMenuScreen_Shell_VR_HMD_Options::idMenuDataSource_VR_HMD_Options::IsDataChanged() const {
		
	if ( originalVignette != vr_vignette.GetInteger() ) {
		return true;
	}
	if ( originalPixelDensity != vr_pixelDensity.GetFloat() ) {
		return true;
	}
	
	return false;
}
