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

const static int NUM_SYSTEM_VR_PROFILE_OPTIONS = 8;

float LinearAdjust( const float input, const float currentMin, const float currentMax, const float desiredMin,  float desiredMax );
int	AdjustOption( const int currentValue, const int values[], const int numValues, const int adjustment );

#undef strncmp

/*
========================
idMenuScreen_Shell_VR_Profile_Options::Initialize
========================
*/
void idMenuScreen_Shell_VR_Profile_Options::Initialize( idMenuHandler * data ) {
	idMenuScreen::Initialize( data );

	if ( data != NULL ) {
		menuGUI = data->GetGUI();
	}

	SetSpritePath( "menuSystemOptions" );

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	options->SetNumVisibleOptions( NUM_SYSTEM_VR_PROFILE_OPTIONS );
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
	control->SetLabel( "Active Profile" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Profile_Options::VR_PROFILE_USE_OCULUS_PROFILE );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Profile_Options::VR_PROFILE_USE_OCULUS_PROFILE );
	options->AddChild( control );
	
	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "Oculus Profile IPD" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Profile_Options::VR_OCULUS_IPD );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Profile_Options::VR_OCULUS_IPD );
	options->AddChild( control );

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "Oculus Profile Height" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Profile_Options::VR_OCULUS_HEIGHT );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Profile_Options::VR_OCULUS_HEIGHT );
	options->AddChild( control );

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "Manual Profile IPD" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Profile_Options::VR_PROFILE_IPD );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Profile_Options::VR_PROFILE_IPD );
	options->AddChild( control );

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "Manual Profile Height" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Profile_Options::VR_PROFILE_HEIGHT );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Profile_Options::VR_PROFILE_HEIGHT );
	options->AddChild( control );
	
	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "World Scale Adjust" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Profile_Options::VR_PROFILE_SCALE );
	control->SetupEvents( 2, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Profile_Options::VR_PROFILE_SCALE );
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
idMenuScreen_Shell_VR_Profile_Options::Update
========================
*/
void idMenuScreen_Shell_VR_Profile_Options::Update() {

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
			heading->SetText( "VR Profile Options" );	
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
idMenuScreen_Shell_VR_Profile_Options::ShowScreen
========================
*/
void idMenuScreen_Shell_VR_Profile_Options::ShowScreen( const mainMenuTransition_t transitionType ) {
	
	systemData.LoadData();
	
	idMenuScreen::ShowScreen( transitionType );
}

/*
========================
idMenuScreen_Shell_VR_Profile_Options::HideScreen
========================
*/
void idMenuScreen_Shell_VR_Profile_Options::HideScreen( const mainMenuTransition_t transitionType ) {

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
idMenuScreen_Shell_VR_Profile_Options::HandleAction h
========================
*/
bool idMenuScreen_Shell_VR_Profile_Options::HandleAction( idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled ) {

	if ( menuData == NULL ) {
		return true;
	}
	
	if ( menuData->ActiveScreen() != SHELL_AREA_VR_PROFILE_OPTIONS ) {
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
idMenuScreen_Shell_VR_Profile_Options::idMenuDataSource_Shell_VR_Profile_Options::idMenuDataSource_Shell_VR_Profile_Options
========================
*/
idMenuScreen_Shell_VR_Profile_Options::idMenuDataSource_Shell_VR_Profile_Options::idMenuDataSource_Shell_VR_Profile_Options() {
}

/*
========================
idMenuScreen_Shell_VR_Profile_Options::idMenuDataSource_Shell_VR_Profile_Options::LoadData
========================
*/
void idMenuScreen_Shell_VR_Profile_Options::idMenuDataSource_Shell_VR_Profile_Options::LoadData() {
	
	originalUseOculusProfile = vr_useOculusProfile.GetInteger();
	originalIPD = vr_manualIPD.GetFloat();
	originalHeight = vr_manualHeight.GetFloat();
	originalScale = vr_scale.GetFloat();
}

/*
========================
idMenuScreen_Shell_VR_Profile_Options::idMenuDataSource_Shell_VR_Profile_Options::IsRestartRequired
========================
*/
bool idMenuScreen_Shell_VR_Profile_Options::idMenuDataSource_Shell_VR_Profile_Options::IsRestartRequired() const {

	return false;
}

/*
========================
idMenuScreen_Shell_VR_Profile_Options::idMenuDataSource_Shell_VR_Profile_Options::CommitData
========================
*/
void idMenuScreen_Shell_VR_Profile_Options::idMenuDataSource_Shell_VR_Profile_Options::CommitData() {
	cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
}

/*
========================
idMenuScreen_Shell_VR_Profile_Options::idMenuDataSource_Shell_VR_Profile_Options::AdjustField
========================
*/
void idMenuScreen_Shell_VR_Profile_Options::idMenuDataSource_Shell_VR_Profile_Options::AdjustField( const int fieldIndex, const int adjustAmount ) {
	switch ( fieldIndex ) {
		
		case VR_PROFILE_SCALE: {
			const float percent = vr_scale.GetFloat();
			const float adjusted = percent + (float) adjustAmount * .01;
			const float clamped = idMath::ClampFloat( 0.2f, 2.0f, adjusted );
			vr_scale.SetFloat( clamped );
			break;
		}
		case VR_PROFILE_USE_OCULUS_PROFILE : {
			static const int numValues = 2;
			static const int values[numValues] = { 0, 1 };
			vr_useOculusProfile.SetInteger( AdjustOption( vr_useOculusProfile.GetInteger(), values, numValues, adjustAmount ) );
			break;
		}
		case VR_PROFILE_IPD: {
			const float percent = LinearAdjust( vr_manualIPD.GetFloat(), 40.0, 90.0f, 0.0f, 100.0f );
			const float adjusted = percent + (float)adjustAmount ;
			const float clamped = idMath::ClampFloat( 0.0f, 100.0f, adjusted );
			vr_manualIPD.SetFloat( LinearAdjust( clamped, 0.0f, 100.0f, 40.0f,90.0f ) );
			break;
		}
		case VR_PROFILE_HEIGHT: {
			const float percent = LinearAdjust( vr_manualHeight.GetFloat(), 40.0, 90.0f, 0.0f, 100.0f );
			const float adjusted = percent + (float)adjustAmount ;
			const float clamped = idMath::ClampFloat( 0.0f, 100.0f, adjusted );
			vr_manualHeight.SetFloat( LinearAdjust( clamped, 0.0f, 100.0f, 40.0f,90.0f ) );
			break;
		}


	}
	cvarSystem->ClearModifiedFlags( CVAR_ARCHIVE );
}

/*
========================
idMenuScreen_Shell_VR_Profile_Options::idMenuDataSource_Shell_VR_Profile_Options::GetField	
========================
*/
idSWFScriptVar idMenuScreen_Shell_VR_Profile_Options::idMenuDataSource_Shell_VR_Profile_Options::GetField( const int fieldIndex ) const {
	switch ( fieldIndex ) {
		
		case VR_PROFILE_USE_OCULUS_PROFILE:
			if ( vr_useOculusProfile.GetInteger() == 0 ) {
				return "Manual Profile";
			} else {
				return "Oculus Profile";
			}
		
		case VR_OCULUS_IPD:
			return va( "%.2f MM", vr->oculusIPD );

		case VR_OCULUS_HEIGHT:
			return va( "%.1f In, ( %.2f M )", vr->oculusHeight, ( vr->oculusHeight * .0254) );
		
		case VR_PROFILE_IPD:
			return va( "%.1f MM", vr_manualIPD.GetFloat() );
			
		case VR_PROFILE_HEIGHT:
			return va( "%.1f In, ( %.2f M )", vr_manualHeight.GetFloat(), ( vr_manualHeight.GetFloat() * .0254 ) );
		
		case VR_PROFILE_SCALE:
			return va( "%.2f", vr_scale.GetFloat() );
	}
	return false;
}

/*
========================
idMenuScreen_Shell_VR_Profile_Options::idMenuDataSource_Shell_VR_Profile_Options::IsDataChanged	
========================
*/
bool idMenuScreen_Shell_VR_Profile_Options::idMenuDataSource_Shell_VR_Profile_Options::IsDataChanged() const {
	
	if ( originalUseOculusProfile != vr_useOculusProfile.GetInteger() ) {
		return true;
	}
	if ( originalIPD != vr_manualIPD.GetFloat() ) {
		return true;
	}
	if ( originalHeight != vr_manualHeight.GetFloat() ) {
		return true;
	}
	if ( originalScale != vr_scale.GetFloat()) {
		return true;
	}
		
	return false;
}
