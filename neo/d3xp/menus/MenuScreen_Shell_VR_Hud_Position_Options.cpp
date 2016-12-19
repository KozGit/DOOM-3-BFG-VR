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

enum settingMenuCmds_t
{
	SETTING_CMD_BINDINGS,
};

const static int NUM_SYSTEM_VR_HUD_POSITION_OPTIONS = 8;

float LinearAdjust( const float input, const float currentMin, const float currentMax, const float desiredMin, float desiredMax );
int	AdjustOption( const int currentValue, const int values[], const int numValues, const int adjustment );

/*
========================
idMenuScreen_Shell_VR_HUD_Position_Options::Initialize
========================
*/
void idMenuScreen_Shell_VR_HUD_Position_Options::Initialize( idMenuHandler * data ) {
	idMenuScreen::Initialize( data );

	if ( data != NULL ) {
		menuGUI = data->GetGUI();
	}

	SetSpritePath( "menuSystemOptions" );
	
	options = new (TAG_SWF)idMenuWidget_DynamicList();
	options->SetNumVisibleOptions( NUM_SYSTEM_VR_HUD_POSITION_OPTIONS );
	options->SetSpritePath( GetSpritePath(), "info", "options" );
	options->SetWrappingAllowed( true );
	options->SetControlList( true );
	options->Initialize( data );

	btnBack = new (TAG_SWF)idMenuWidget_Button();
	btnBack->Initialize( data );
	btnBack->SetLabel( "VR HUD Options" );
	btnBack->SetSpritePath( GetSpritePath(), "info", "btnBack" );
	btnBack->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_GO_BACK );

	AddChild( options );
	AddChild( btnBack );

	idMenuWidget_ControlButton * control;
	

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "HUD Position Dist" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_HUD_Position_Options::HUD_POSITION_OPTIONS_FIELD_DISTANCE );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_HUD_Position_Options::HUD_POSITION_OPTIONS_FIELD_DISTANCE );
	options->AddChild( control );

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "HUD Position Vert" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_HUD_Position_Options::HUD_POSITION_OPTIONS_FIELD_VERTICAL );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_HUD_Position_Options::HUD_POSITION_OPTIONS_FIELD_VERTICAL );
	options->AddChild( control );

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "HUD Position Horiz" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_HUD_Position_Options::HUD_POSITION_OPTIONS_FIELD_HORIZONTAL );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_HUD_Position_Options::HUD_POSITION_OPTIONS_FIELD_HORIZONTAL );
	options->AddChild( control );

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "HUD Pitch" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_HUD_Position_Options::HUD_POSITION_OPTIONS_FIELD_PITCH );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_HUD_Position_Options::HUD_POSITION_OPTIONS_FIELD_PITCH );
	options->AddChild( control );

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "HUD Scale" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_HUD_Position_Options::HUD_POSITION_OPTIONS_FIELD_SCALE );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_HUD_Position_Options::HUD_POSITION_OPTIONS_FIELD_SCALE );
	options->AddChild( control );

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "Talk Cursor Adj" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_HUD_Position_Options::HUD_POSITION_OPTIONS_FIELD_TALK_CURSOR );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_HUD_Position_Options::HUD_POSITION_OPTIONS_FIELD_TALK_CURSOR );
	options->AddChild( control );

	options->AddEventAction( WIDGET_EVENT_SCROLL_DOWN ).Set( new (TAG_SWF)idWidgetActionHandler( options, WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER, WIDGET_EVENT_SCROLL_DOWN ) );
	options->AddEventAction( WIDGET_EVENT_SCROLL_UP ).Set( new (TAG_SWF)idWidgetActionHandler( options, WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER, WIDGET_EVENT_SCROLL_UP ) );
	options->AddEventAction( WIDGET_EVENT_SCROLL_DOWN_RELEASE ).Set( new (TAG_SWF)idWidgetActionHandler( options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_DOWN_RELEASE ) );
	options->AddEventAction( WIDGET_EVENT_SCROLL_UP_RELEASE ).Set( new (TAG_SWF)idWidgetActionHandler( options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_UP_RELEASE ) );
	options->AddEventAction( WIDGET_EVENT_SCROLL_DOWN_LSTICK ).Set( new (TAG_SWF)idWidgetActionHandler( options, WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER, WIDGET_EVENT_SCROLL_DOWN_LSTICK ) );
	options->AddEventAction( WIDGET_EVENT_SCROLL_UP_LSTICK ).Set( new (TAG_SWF)idWidgetActionHandler( options, WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER, WIDGET_EVENT_SCROLL_UP_LSTICK ) );
	options->AddEventAction( WIDGET_EVENT_SCROLL_DOWN_LSTICK_RELEASE ).Set( new (TAG_SWF)idWidgetActionHandler( options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_DOWN_LSTICK_RELEASE ) );
	options->AddEventAction( WIDGET_EVENT_SCROLL_UP_LSTICK_RELEASE ).Set( new (TAG_SWF)idWidgetActionHandler( options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_UP_LSTICK_RELEASE ) );
}

/*
========================
idMenuScreen_Shell_VR_HUD_Position_Options::Update
========================
*/
void idMenuScreen_Shell_VR_HUD_Position_Options::Update() {

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
			heading->SetText( "VR HUD Pos Options" );
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
idMenuScreen_Shell_VR_HUD_Position_Options::ShowScreen
========================
*/
void idMenuScreen_Shell_VR_HUD_Position_Options::ShowScreen( const mainMenuTransition_t transitionType ) {

	systemData.LoadData();

	idMenuScreen::ShowScreen( transitionType );
}

/*
========================
idMenuScreen_Shell_VR_HUD_Position_Options::HideScreen
========================
*/
void idMenuScreen_Shell_VR_HUD_Position_Options::HideScreen( const mainMenuTransition_t transitionType ) {

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
idMenuScreen_Shell_VR_HUD_Position_Options::HandleAction h
========================
*/
bool idMenuScreen_Shell_VR_HUD_Position_Options::HandleAction( idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled ) {

	if ( menuData == NULL ) {
		return true;
	}

	if ( menuData->ActiveScreen() != SHELL_AREA_VR_HUD_POSITION_OPTIONS ) {
		return false;
	}

	widgetAction_t actionType = action.GetType();
	const idSWFParmList & parms = action.GetParms();
	switch ( actionType ) {
	case WIDGET_ACTION_GO_BACK: {
		if ( menuData != NULL ) {
			menuData->SetNextScreen( SHELL_AREA_VR_HUD_OPTIONS, MENU_TRANSITION_SIMPLE );
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
idMenuScreen_Shell_VR_HUD_Position_Options::idMenuDataSource_Shell_VR_HUD_Position_Options::idMenuDataSource_Shell_VR_HUD_Position_Options
========================
*/
idMenuScreen_Shell_VR_HUD_Position_Options::idMenuDataSource_Shell_VR_HUD_Position_Options::idMenuDataSource_Shell_VR_HUD_Position_Options() {
}

/*
========================
idMenuScreen_Shell_VR_HUD_Position_Options::idMenuDataSource_Shell_VR_HUD_Position_Options::LoadData
========================
*/
void idMenuScreen_Shell_VR_HUD_Position_Options::idMenuDataSource_Shell_VR_HUD_Position_Options::LoadData() {

	originalHudDistance = vr_hudPosDis.GetFloat();
	originalHudVertical = vr_hudPosVer.GetFloat();
	originalHudHorizontal = vr_hudPosHor.GetFloat();
	originalHudPitch = vr_hudPosAngle.GetFloat();
	originalHudScale = vr_hudScale.GetFloat();
	originalTalkCursor = vr_tweakTalkCursor.GetFloat();

}

/*
========================
idMenuScreen_Shell_VR_HUD_Position_Options::idMenuDataSource_Shell_VR_HUD_Position_Options::IsRestartRequired
========================
*/
bool idMenuScreen_Shell_VR_HUD_Position_Options::idMenuDataSource_Shell_VR_HUD_Position_Options::IsRestartRequired() const {
	return false;
}

/*
========================
idMenuScreen_Shell_VR_HUD_Position_Options::idMenuDataSource_Shell_VR_HUD_Position_Options::CommitData
========================
*/
void idMenuScreen_Shell_VR_HUD_Position_Options::idMenuDataSource_Shell_VR_HUD_Position_Options::CommitData() {
	cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
}


/*
========================
idMenuScreen_Shell_VR_HUD_Position_Options::idMenuDataSource_Shell_VR_HUD_Position_Options::AdjustField
========================
*/
void idMenuScreen_Shell_VR_HUD_Position_Options::idMenuDataSource_Shell_VR_HUD_Position_Options::AdjustField( const int fieldIndex, const int adjustAmount ) {
	switch ( fieldIndex )
	{
	
		case HUD_POSITION_OPTIONS_FIELD_TALK_CURSOR:
		{
			float p = vr_tweakTalkCursor.GetFloat();
			p += adjustAmount;
			if ( p < 0.0f ) p = 0.0f;
			if ( p > 100.0f ) p = 100.0f;
			vr_tweakTalkCursor.SetFloat( p );
			break;
		}

		case HUD_POSITION_OPTIONS_FIELD_SCALE:
		{
			float p = vr_hudScale.GetFloat();
			p += adjustAmount * .1;
			if ( p < 0.5f ) p = 0.5f;
			if ( p > 5.0f ) p = 5.0f;
			vr_hudScale.SetFloat( p );
			break;
		}

		case HUD_POSITION_OPTIONS_FIELD_PITCH:
		{
			float p = vr_hudPosAngle.GetFloat();
			p += adjustAmount * .5;
			if ( p < -100.0f ) p = -100.0f;
			if ( p > 100 ) p = 100;
			vr_hudPosAngle.SetFloat( p );
			break;
		}

		case HUD_POSITION_OPTIONS_FIELD_DISTANCE:
		{
			float p = vr_hudPosDis.GetFloat();
			p += adjustAmount * .5;
			if ( p < 0.0f ) p = 0.0f;
			if ( p > 100.0f ) p = 100.0f;
			vr_hudPosDis.SetFloat( p );
			break;
		}

		case HUD_POSITION_OPTIONS_FIELD_HORIZONTAL:
		{
			float p = vr_hudPosHor.GetFloat();
			p += adjustAmount * .5;
			if ( p < -50.0f ) p = -50.0f;
			if ( p > 50.0f ) p = 50.0f;
			vr_hudPosDis.SetFloat( p );
			break;
		}

		case HUD_POSITION_OPTIONS_FIELD_VERTICAL:
		{
			float p = vr_hudPosVer.GetFloat();
			p += adjustAmount * .5;
			if ( p < -70.0f ) p = -70.0f;
			if ( p > 70.0f ) p = 70.0f;
			vr_hudPosVer.SetFloat( p );
			break;
		}
	}

	cvarSystem->ClearModifiedFlags( CVAR_ARCHIVE );
}

/*
========================
idMenuScreen_Shell_VR_HUD_Position_Options::idMenuDataSource_Shell_VR_HUD_Position_Options::GetField
========================
*/
idSWFScriptVar idMenuScreen_Shell_VR_HUD_Position_Options::idMenuDataSource_Shell_VR_HUD_Position_Options::GetField( const int fieldIndex ) const {
	switch ( fieldIndex )
	{
	
	case HUD_POSITION_OPTIONS_FIELD_TALK_CURSOR:
		return va( "%.0f", vr_tweakTalkCursor.GetFloat() );
		
	case HUD_POSITION_OPTIONS_FIELD_SCALE:
		return va( "%.0f", vr_hudScale.GetFloat() );

	case HUD_POSITION_OPTIONS_FIELD_PITCH:
		return va( "%.0f", vr_hudPosAngle.GetFloat() );

	case HUD_POSITION_OPTIONS_FIELD_DISTANCE:
		return va( "%.1f", vr_hudPosDis.GetFloat() );

	case HUD_POSITION_OPTIONS_FIELD_HORIZONTAL:
		return va( "%.1f", vr_hudPosHor.GetFloat() );

	case HUD_POSITION_OPTIONS_FIELD_VERTICAL:
		return va( "%.1f", vr_hudPosVer.GetFloat() );
	}
	return false;
}

/*
========================
idMenuScreen_Shell_VR_HUD_Position_Options::idMenuDataSource_Shell_VR_HUD_Position_Options::IsDataChanged
========================
*/
bool idMenuScreen_Shell_VR_HUD_Position_Options::idMenuDataSource_Shell_VR_HUD_Position_Options::IsDataChanged() const {


	if ( originalHudDistance != vr_hudPosDis.GetFloat() )
	{
		return true;
	}

	if ( originalHudVertical != vr_hudPosVer.GetFloat() )
	{
		return true;
	}

	if ( originalHudHorizontal != vr_hudPosHor.GetFloat() )
	{
		return true;
	}

	if ( originalHudPitch != vr_hudPosAngle.GetFloat() )
	{
		return true;
	}

	if ( originalHudScale != vr_hudScale.GetFloat() )
	{
		return true;
	}

	if ( originalTalkCursor != vr_tweakTalkCursor.GetFloat() )
	{
		return true;
	}

	return false;
}