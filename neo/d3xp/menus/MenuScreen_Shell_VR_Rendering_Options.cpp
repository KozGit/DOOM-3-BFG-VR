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

const static int NUM_VR_RENDERING_OPTIONS = 8;

extern idCVar vr_enable;
extern idCVar vr_FBOscale;
extern idCVar r_multiSamples;
extern idCVar vr_FBOEnabled;
extern idCVar vr_FBOAAmode;


float LinearAdjust( const float input, const float currentMin, const float currentMax, const float desiredMin,  float desiredMax );
int	AdjustOption( const int currentValue, const int values[], const int numValues, const int adjustment );

/*
========================
idMenuScreen_Shell_VR_Rendering_Options::Initialize
========================
*/
void idMenuScreen_Shell_VR_Rendering_Options::Initialize( idMenuHandler * data ) {
	idMenuScreen::Initialize( data );

	if ( data != NULL ) {
		menuGUI = data->GetGUI();
	}

	SetSpritePath( "menuSystemOptions" );

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	options->SetNumVisibleOptions( NUM_VR_RENDERING_OPTIONS );
	options->SetSpritePath( GetSpritePath(), "info", "options" );
	options->SetWrappingAllowed( true );
	options->SetControlList( true );
	options->Initialize( data );

	btnBack = new (TAG_SWF) idMenuWidget_Button();
	btnBack->Initialize( data );
	btnBack->SetLabel( "VR OPTIONS" );
	btnBack->SetSpritePath( GetSpritePath(), "info", "btnBack" );
	btnBack->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_GO_BACK );

	AddChild( options );
	AddChild( btnBack );

	idMenuWidget_ControlButton * control;
	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "Enable VR" ); // Enable VR mode
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Rendering_Options::VR_OPTIONS_FIELD_ENABLE_VR );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Rendering_Options::VR_OPTIONS_FIELD_ENABLE_VR );
	options->AddChild( control );

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "Enable FBO rendering." );// Enable FBO rendering path.
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Rendering_Options::VR_OPTIONS_FIELD_ENABLE_FBO );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Rendering_Options::VR_OPTIONS_FIELD_ENABLE_FBO );
	options->AddChild( control );

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_BAR );
	control->SetLabel( "FBO size %" );// % value to increase/decrease FBO size (for downscaling/antialiasing)
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Rendering_Options::VR_OPTIONS_FIELD_FBO_SIZE );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Rendering_Options::VR_OPTIONS_FIELD_FBO_SIZE );
	options->AddChild( control );

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "#str_04128" ); // Antialiasing
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Rendering_Options::VR_OPTIONS_FIELD_ANTIALIASINGTYPE );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Rendering_Options::VR_OPTIONS_FIELD_ANTIALIASINGTYPE );
	options->AddChild( control );

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "MSAA Level" );// # of MSAA samples if enabled
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Rendering_Options::VR_OPTIONS_FIELD_MSAALEVEL );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Rendering_Options::VR_OPTIONS_FIELD_MSAALEVEL );
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
idMenuScreen_Shell_VR_Rendering_Options::Update
========================
*/
void idMenuScreen_Shell_VR_Rendering_Options::Update() {

	if ( menuData != NULL ) {
		idMenuWidget_CommandBar * cmdBar = menuData->GetCmdBar();
		if ( cmdBar != NULL ) {
			cmdBar->ClearAllButtons();
			idMenuWidget_CommandBar::buttonInfo_t * buttonInfo;			
			buttonInfo = cmdBar->GetButton( idMenuWidget_CommandBar::BUTTON_JOY2 );
			if ( menuData->GetPlatform() != 2 ) {
				buttonInfo->label = "#str_00395"; // back
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
			heading->SetText( "VR Rendering Options" );	
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
idMenuScreen_Shell_VR_Rendering_Options::ShowScreen
========================
*/
void idMenuScreen_Shell_VR_Rendering_Options::ShowScreen( const mainMenuTransition_t transitionType ) {
	
	systemData.LoadData();
	
	idMenuScreen::ShowScreen( transitionType );
}

/*
========================
idMenuScreen_Shell_VR_Rendering_Options::HideScreen
========================
*/
void idMenuScreen_Shell_VR_Rendering_Options::HideScreen( const mainMenuTransition_t transitionType ) {

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
idMenuScreen_Shell_VR_Rendering_Options::HandleAction h
========================
*/
bool idMenuScreen_Shell_VR_Rendering_Options::HandleAction( idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled ) {

	if ( menuData == NULL ) {
		return true;
	}
	
	if ( menuData->ActiveScreen() != SHELL_AREA_VR_RENDERING_OPTIONS ) {
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
idMenuScreen_Shell_VR_Rendering_Options::idMenuDataSource_Shell_VR_Rendering_Options::idMenuDataSource_Shell_VR_Rendering_Options
========================
*/
idMenuScreen_Shell_VR_Rendering_Options::idMenuDataSource_Shell_VR_Rendering_Options::idMenuDataSource_Shell_VR_Rendering_Options() {
}

/*
========================
idMenuScreen_Shell_VR_Rendering_Options::idMenuDataSource_Shell_VR_Rendering_Options::LoadData
========================
*/
void idMenuScreen_Shell_VR_Rendering_Options::idMenuDataSource_Shell_VR_Rendering_Options::LoadData() {
	
	
	originalEnableVR = vr_enable.GetInteger();
	originalEnableFBO = vr_FBOEnabled.GetInteger();
	originalFBOsize = vr_FBOscale.GetFloat();
	originalAntialiasingType = vr_FBOAAmode.GetInteger();
	originalMSAAlevel = r_multiSamples.GetInteger();
			
}

/*
========================
idMenuScreen_Shell_VR_Rendering_Options::idMenuDataSource_Shell_VR_Rendering_Options::IsRestartRequired
========================
*/
bool idMenuScreen_Shell_VR_Rendering_Options::idMenuDataSource_Shell_VR_Rendering_Options::IsRestartRequired() const {
	
	if ( originalAntialiasingType != vr_FBOAAmode.GetInteger() ) {
		return true;
	}
	if ( originalMSAAlevel != r_multiSamples.GetInteger() && vr_FBOAAmode.GetInteger() == VR_AA_MSAA ) {
		return true;
	}
	if ( originalEnableFBO != vr_FBOEnabled.GetInteger() ) {
		return true;
	}
		
	return false;
}

/*
========================
idMenuScreen_Shell_VR_Rendering_Options::idMenuDataSource_Shell_VR_Rendering_Options::CommitData
========================
*/
void idMenuScreen_Shell_VR_Rendering_Options::idMenuDataSource_Shell_VR_Rendering_Options::CommitData() {
	cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
}

/*
========================
idMenuScreen_Shell_VR_Rendering_Options::idMenuDataSource_Shell_VR_Rendering_Options::AdjustField
========================
*/
void idMenuScreen_Shell_VR_Rendering_Options::idMenuDataSource_Shell_VR_Rendering_Options::AdjustField( const int fieldIndex, const int adjustAmount ) {
	switch ( fieldIndex ) {
		
		case VR_OPTIONS_FIELD_ENABLE_VR: {
			static const int numValues = 2;
			static const int values[numValues] = { 0, 1 };
			vr_enable.SetInteger( AdjustOption(vr_enable.GetInteger(), values, numValues, adjustAmount ) );
			break;
		}
		
		case VR_OPTIONS_FIELD_ENABLE_FBO: {
			static const int numValues = 2;
			static const int values[numValues] = { 0, 1 };
			vr_FBOEnabled.SetInteger( AdjustOption(vr_FBOEnabled.GetInteger(), values, numValues, adjustAmount ) );
			break;
		}
			
		case VR_OPTIONS_FIELD_ANTIALIASINGTYPE: {
			static const int numValues = 3;
			static const int values[numValues] = { 0, 1, 2 };
			vr_FBOAAmode.SetInteger( AdjustOption(vr_FBOAAmode.GetInteger(), values, numValues, adjustAmount ) );
			break;
		}
										 
		case VR_OPTIONS_FIELD_MSAALEVEL: {
			static const int numValues = 5;
			static const int values[numValues] = { 0, 2, 4, 8 };
			r_multiSamples.SetInteger( AdjustOption( r_multiSamples.GetInteger(), values, numValues, adjustAmount ) );
			break;
		}
		
		case VR_OPTIONS_FIELD_FBO_SIZE: {
			const float percent = LinearAdjust( vr_FBOscale.GetFloat(), 0.8f, 4.0f, 0.0f, 100.0f );
			const float adjusted = percent + (float)adjustAmount * 2.0f;// koz added multiplier
			const float clamped = idMath::ClampFloat( 0.0f, 100.0f, adjusted );
			vr_FBOscale.SetFloat( LinearAdjust( clamped, 0.0f, 100.0f, 0.8f, 4.0f ) );
			break;
		}
	}
	cvarSystem->ClearModifiedFlags( CVAR_ARCHIVE );
}

/*
========================
idMenuScreen_Shell_VR_Rendering_Options::idMenuDataSource_Shell_VR_Rendering_Options::GetField	
========================
*/
idSWFScriptVar idMenuScreen_Shell_VR_Rendering_Options::idMenuDataSource_Shell_VR_Rendering_Options::GetField( const int fieldIndex ) const {
	switch ( fieldIndex ) {
		case VR_OPTIONS_FIELD_ENABLE_VR: {
			const int VRenabled = vr_enable.GetInteger();
			
			if ( VRenabled == 0 ) {
				return "#str_swf_disabled";
			} else {
				return "#str_swf_enabled";
			}
			
		}
		
		case VR_OPTIONS_FIELD_ENABLE_FBO: {
			const int FBOenabled = vr_FBOEnabled.GetInteger();
			
			if ( FBOenabled == 0 ) {
				return "#str_swf_disabled";
			} else {
				return "#str_swf_enabled";
			}
		}
		
		case VR_OPTIONS_FIELD_FBO_SIZE: { 
			
			 float result =  LinearAdjust( vr_FBOscale.GetFloat(), 0.8f, 4.0f, 0.0f, 100.0f );
			return result;
		}
		case VR_OPTIONS_FIELD_ANTIALIASINGTYPE:
			if ( vr_FBOAAmode.GetInteger() == VR_AA_FXAA ) {
				return "FXAA";
			} else if ( vr_FBOAAmode.GetInteger() == VR_AA_MSAA ) {
				return "MSAA";
			} else {
				return "#str_swf_disabled";
			}
		
		case VR_OPTIONS_FIELD_MSAALEVEL:
			
			return va( "%dx", r_multiSamples.GetInteger() );
		
		
	}
	return false;
}

/*
========================
idMenuScreen_Shell_VR_Rendering_Options::idMenuDataSource_Shell_VR_Rendering_Options::IsDataChanged	
========================
*/
bool idMenuScreen_Shell_VR_Rendering_Options::idMenuDataSource_Shell_VR_Rendering_Options::IsDataChanged() const {
	if ( originalEnableVR != vr_enable.GetInteger() ) {
		return true;
	}
	if ( originalEnableFBO != vr_FBOEnabled.GetInteger() ) {
		return true;
	}
	if ( originalFBOsize != vr_FBOscale.GetFloat() ) {
		return true;
	}
	if ( originalAntialiasingType != vr_FBOAAmode.GetInteger() ) {
		return true;
	}
	if ( originalMSAAlevel != r_multiSamples.GetInteger() ) {
		return true;
	}
			
	return false;
}
