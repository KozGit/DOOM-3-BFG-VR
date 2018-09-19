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

const static int NUM_NEW_GAME_OPTIONS = 8;

float LinearAdjust( const float input, const float currentMin, const float currentMax, const float desiredMin, float desiredMax );
int	AdjustOption( const int currentValue, const int values[], const int numValues, const int adjustment );

/*
========================
idMenuScreen_Shell_NewGame::Initialize
========================
*/
void idMenuScreen_Shell_NewGame::Initialize( idMenuHandler* data )
{
	idMenuScreen::Initialize( data );
	
	if( data != NULL )
	{
		menuGUI = data->GetGUI();
	}
	
	SetSpritePath( "menuSystemOptions" ); // Carl: Necessary for options to show up

	options = new( TAG_SWF ) idMenuWidget_DynamicList();
	options->SetNumVisibleOptions( NUM_NEW_GAME_OPTIONS );
	options->SetSpritePath( GetSpritePath(), "info", "options" );
	options->SetWrappingAllowed( true );
	options->SetControlList( true );
	AddChild( options );

	helpWidget = new( TAG_SWF ) idMenuWidget_Help();
	helpWidget->SetSpritePath( GetSpritePath(), "info", "helpTooltip" );
	AddChild( helpWidget );

	btnBack = new( TAG_SWF ) idMenuWidget_Button();
	btnBack->Initialize( data );
	btnBack->SetLabel( "#str_swf_campaign" );
	btnBack->SetSpritePath( GetSpritePath(), "info", "btnBack" );
	btnBack->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_GO_BACK );
	
	AddChild( btnBack );


	idMenuWidget_ControlButton * control;

	control = new( TAG_SWF ) idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_BUTTON_TEXT );
	control->SetLabel( "#str_swf_doom3" );	// doom 3
	control->SetDescription( "Start the main Doom 3 campaign." );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_NewGame::NEWGAME_FIELD_DOOM3 );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_PRESS_FOCUSED, idMenuDataSource_Shell_NewGame::NEWGAME_FIELD_DOOM3 );
	control->RegisterEventObserver( helpWidget );
	options->AddChild( control );

	control = new( TAG_SWF ) idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_BUTTON_TEXT );
	control->SetLabel( "#str_swf_resurrection" );	// resurrection of evil
	control->SetDescription( "Start the Resurrection of Evil expansion/sequel campaign." );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_NewGame::NEWGAME_FIELD_ROE );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_PRESS_FOCUSED, idMenuDataSource_Shell_NewGame::NEWGAME_FIELD_ROE );
	control->RegisterEventObserver( helpWidget );
	options->AddChild( control );

	control = new( TAG_SWF ) idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_BUTTON_TEXT );
	control->SetLabel( "#str_swf_lost_episodes" );	// lost episodes
	control->SetDescription( "BFG Edition's Lost Episodes campaign, the events of Doom 3 from Bravo Team's POV." );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_NewGame::NEWGAME_FIELD_LE );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_PRESS_FOCUSED, idMenuDataSource_Shell_NewGame::NEWGAME_FIELD_LE );
	control->RegisterEventObserver( helpWidget );
	options->AddChild( control );

	control = new( TAG_SWF ) idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "Bonus Character" );
	control->SetDescription( "Select an unlockable bonus character to play as." );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_NewGame::NEWGAME_FIELD_CHARACTER );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_PRESS_FOCUSED, idMenuDataSource_Shell_NewGame::NEWGAME_FIELD_CHARACTER );
	control->RegisterEventObserver( helpWidget );
	options->AddChild( control );

	options->AddEventAction( WIDGET_EVENT_SCROLL_DOWN ).Set( new( TAG_SWF ) idWidgetActionHandler( options, WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER, WIDGET_EVENT_SCROLL_DOWN ) );
	options->AddEventAction( WIDGET_EVENT_SCROLL_UP ).Set( new( TAG_SWF ) idWidgetActionHandler( options, WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER, WIDGET_EVENT_SCROLL_UP ) );
	options->AddEventAction( WIDGET_EVENT_SCROLL_DOWN_RELEASE ).Set( new( TAG_SWF ) idWidgetActionHandler( options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_DOWN_RELEASE ) );
	options->AddEventAction( WIDGET_EVENT_SCROLL_UP_RELEASE ).Set( new( TAG_SWF ) idWidgetActionHandler( options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_UP_RELEASE ) );
	options->AddEventAction( WIDGET_EVENT_SCROLL_DOWN_LSTICK ).Set( new( TAG_SWF ) idWidgetActionHandler( options, WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER, WIDGET_EVENT_SCROLL_DOWN_LSTICK ) );
	options->AddEventAction( WIDGET_EVENT_SCROLL_UP_LSTICK ).Set( new( TAG_SWF ) idWidgetActionHandler( options, WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER, WIDGET_EVENT_SCROLL_UP_LSTICK ) );
	options->AddEventAction( WIDGET_EVENT_SCROLL_DOWN_LSTICK_RELEASE ).Set( new( TAG_SWF ) idWidgetActionHandler( options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_DOWN_LSTICK_RELEASE ) );
	options->AddEventAction( WIDGET_EVENT_SCROLL_UP_LSTICK_RELEASE ).Set( new( TAG_SWF ) idWidgetActionHandler( options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_UP_LSTICK_RELEASE ) );
}

/*
========================
idMenuScreen_Shell_NewGame::Update
========================
*/
void idMenuScreen_Shell_NewGame::Update()
{

	if( menuData != NULL )
	{
		idMenuWidget_CommandBar* cmdBar = menuData->GetCmdBar();
		if( cmdBar != NULL )
		{
			cmdBar->ClearAllButtons();
			idMenuWidget_CommandBar::buttonInfo_t* buttonInfo;
			buttonInfo = cmdBar->GetButton( idMenuWidget_CommandBar::BUTTON_JOY2 );
			if( menuData->GetPlatform() != 2 )
			{
				buttonInfo->label = "#str_00395";
			}
			buttonInfo->action.Set( WIDGET_ACTION_GO_BACK );
			
			buttonInfo = cmdBar->GetButton( idMenuWidget_CommandBar::BUTTON_JOY1 );
			if( menuData->GetPlatform() != 2 )
			{
				buttonInfo->label = "#str_SWF_SELECT";
			}
			buttonInfo->action.Set( WIDGET_ACTION_PRESS_FOCUSED );
		}
	}
	
	idSWFScriptObject& root = GetSWFObject()->GetRootObject();
	if( BindSprite( root ) )
	{
		idSWFTextInstance* heading = GetSprite()->GetScriptObject()->GetNestedText( "info", "txtHeading" );
		if( heading != NULL )
		{
			heading->SetText( "#str_02207" );	// NEW GAME
			heading->SetStrokeInfo( true, 0.75f, 1.75f );
		}
		
		idSWFSpriteInstance* gradient = GetSprite()->GetScriptObject()->GetNestedSprite( "info", "gradient" );
		if( gradient != NULL && heading != NULL )
		{
			gradient->SetXPos( heading->GetTextLength() );
		}
	}
	
	if( btnBack != NULL )
	{
		btnBack->BindSprite( root );
	}
	
	idMenuScreen::Update();
}

/*
========================
idMenuScreen_Shell_NewGame::ShowScreen
========================
*/
void idMenuScreen_Shell_NewGame::ShowScreen( const mainMenuTransition_t transitionType )
{
	systemData.LoadData();
	BonusCheckOtherGames();

	idMenuScreen::ShowScreen( transitionType );
}

/*
========================
idMenuScreen_Shell_NewGame::HideScreen
========================
*/
void idMenuScreen_Shell_NewGame::HideScreen( const mainMenuTransition_t transitionType )
{

	if( systemData.IsRestartRequired() ) {
		class idSWFScriptFunction_Restart : public idSWFScriptFunction_RefCounted {
		public:
			idSWFScriptFunction_Restart( gameDialogMessages_t _msg, bool _restart ) {
				msg = _msg;
				restart = _restart;
			}
			idSWFScriptVar Call( idSWFScriptObject * thisObject, const idSWFParmList & parms ) {
				common->Dialog().ClearDialog( msg );
				if( restart ) {
					idStr cmdLine = Sys_GetCmdLine();
					if( cmdLine.Find( "com_skipIntroVideos" ) < 0 ) {
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

	if( systemData.IsDataChanged() ) {
		systemData.CommitData();
	}

	idMenuScreen::HideScreen( transitionType );
}

/*
========================
idMenuScreen_Shell_NewGame::HandleAction h
========================
*/
bool idMenuScreen_Shell_NewGame::HandleAction( idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled )
{

	if( menuData != NULL )
	{
		if( menuData->ActiveScreen() != SHELL_AREA_NEW_GAME )
		{
			return false;
		}
	}
	
	widgetAction_t actionType = action.GetType();
	const idSWFParmList& parms = action.GetParms();
	
	switch( actionType )
	{
		case WIDGET_ACTION_GO_BACK:
		{
			if( menuData != NULL )
			{
				menuData->SetNextScreen( SHELL_AREA_CAMPAIGN, MENU_TRANSITION_SIMPLE );
			}
			return true;
		}
		case WIDGET_ACTION_PRESS_FOCUSED:
		{
			if( options == NULL )
			{
				return true;
			}
			
			int selectionIndex = options->GetViewIndex();
			if( parms.Num() == 1 )
			{
				selectionIndex = parms[0].ToInteger();
			}
			
			if( selectionIndex != options->GetFocusIndex() )
			{
				options->SetViewIndex( selectionIndex );
				options->SetFocusIndex( selectionIndex );
			}
			
			if( selectionIndex >= idMenuDataSource_Shell_NewGame::NEWGAME_FIELD_CHARACTER ) // Carl: Bonus character
			{
				systemData.AdjustField( selectionIndex, 1 );
				options->Update();
				return true;
			}

			idMenuHandler_Shell* shell = dynamic_cast< idMenuHandler_Shell* >( menuData );
			if( shell != NULL )
			{
				if( !BonusCharUnlocked( ( bonus_char_t )bonus_char.GetInteger() ) )
					bonus_char.SetInteger( BONUS_CHAR_NONE );
				shell->SetNewGameType( selectionIndex - idMenuDataSource_Shell_NewGame::NEWGAME_FIELD_DOOM3 );
				menuData->SetNextScreen( SHELL_AREA_DIFFICULTY, MENU_TRANSITION_SIMPLE );
			}
			
			return true;
		}
		case WIDGET_ACTION_ADJUST_FIELD:
		{
			if( widget != NULL && widget->GetDataSource() != NULL )
			{
				widget->GetDataSource()->AdjustField( widget->GetDataSourceFieldIndex(), parms[0].ToInteger() );
				if( widget->GetDataSourceFieldIndex() == idMenuDataSource_Shell_NewGame::NEWGAME_FIELD_CHARACTER )
				{
					dynamic_cast< idMenuWidget_Button* >( widget )->SetDescription( BonusCharDescription( ( bonus_char_t )bonus_char.GetInteger() ) );
					idWidgetEvent e;
					e.type = WIDGET_EVENT_FOCUS_ON;
					helpWidget->ObserveEvent(*widget, e);
				}
				widget->Update();
			}
			return true;
		}
		case WIDGET_ACTION_START_REPEATER: {

			if( options == NULL ) {
				return true;
			}

			if( parms.Num() == 4 ) {
				int selectionIndex = parms[3].ToInteger();
				if( selectionIndex != options->GetFocusIndex() ) {
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
idMenuScreen_Shell_NewGame::idMenuDataSource_Shell_NewGame::idMenuDataSource_Shell_NewGame
========================
*/
idMenuScreen_Shell_NewGame::idMenuDataSource_Shell_NewGame::idMenuDataSource_Shell_NewGame() {
}

/*
========================
idMenuScreen_Shell_NewGame::idMenuDataSource_Shell_VR_Gameplay_Options::LoadData
========================
*/
void idMenuScreen_Shell_NewGame::idMenuDataSource_Shell_NewGame::LoadData() {

	originalBonusCharacter = bonus_char.GetInteger();
}

/*
========================
idMenuScreen_Shell_NewGame::idMenuDataSource_Shell_VR_Gameplay_Options::IsRestartRequired
========================
*/
bool idMenuScreen_Shell_NewGame::idMenuDataSource_Shell_NewGame::IsRestartRequired() const {

	return false;
}

/*
========================
idMenuScreen_Shell_NewGame::idMenuDataSource_Shell_VR_Gameplay_Options::CommitData
========================
*/
void idMenuScreen_Shell_NewGame::idMenuDataSource_Shell_NewGame::CommitData() {
	cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
}

/*
========================
idMenuScreen_Shell_NewGame::idMenuDataSource_Shell_VR_Gameplay_Options::AdjustField
========================
*/
void idMenuScreen_Shell_NewGame::idMenuDataSource_Shell_NewGame::AdjustField( const int fieldIndex, const int adjustAmount ) {
	switch( fieldIndex ) {
	case NEWGAME_FIELD_CHARACTER:
	{
		static const int numValues = 16;
		static const int values[numValues] = { BONUS_CHAR_NONE, BONUS_CHAR_PHOBOS, BONUS_CHAR_MARINE, BONUS_CHAR_ROE, BONUS_CHAR_LE,
			BONUS_CHAR_CAMPBELL, BONUS_CHAR_SARGE, BONUS_CHAR_BETRUGER, BONUS_CHAR_SWANN,
			BONUS_CHAR_DOOMGUY, BONUS_CHAR_SLAYER, BONUS_CHAR_VFR, BONUS_CHAR_ETERNAL,
			BONUS_CHAR_ASH, BONUS_CHAR_SAMUS, BONUS_CHAR_WITCH };
		bonus_char.SetInteger( AdjustOption( bonus_char.GetInteger(), values, numValues, adjustAmount ) );
		break;

	}

	}
	cvarSystem->ClearModifiedFlags( CVAR_ARCHIVE );
}

/*
========================
idMenuScreen_Shell_NewGame::idMenuDataSource_Shell_VR_Gameplay_Options::GetField
========================
*/
idSWFScriptVar idMenuScreen_Shell_NewGame::idMenuDataSource_Shell_NewGame::GetField( const int fieldIndex ) const {
	switch( fieldIndex )
	{

	case NEWGAME_FIELD_CHARACTER:
		return BonusCharName( (bonus_char_t)bonus_char.GetInteger(), BonusCharUnlocked( (bonus_char_t)bonus_char.GetInteger() ) );
	}
	return false;
}

/*
========================
idMenuScreen_Shell_NewGame::idMenuDataSource_Shell_VR_Gameplay_Options::IsDataChanged
========================
*/
bool idMenuScreen_Shell_NewGame::idMenuDataSource_Shell_NewGame::IsDataChanged() const {

	if( originalBonusCharacter != bonus_char.GetInteger() )
	{
		return true;
	}

	return false;
}
