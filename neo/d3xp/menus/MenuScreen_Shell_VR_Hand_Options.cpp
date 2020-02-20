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

const static int NUM_MENU_OPTIONS = 6; // reload mode isn't implemented yet
const static int NUM_DISPLAY_LINES = 8;

static int currentOffset = 0;
static int lastIndex = 0;

static idStr cmdName[NUM_MENU_OPTIONS+1] = { "Dual Wielding", "Grip Mode", "Pick-Up", "Must Empty Hands", "Context Sensitive", "Weapon Cycle", "Reload Mode" };
static idStr cmdDescription[NUM_MENU_OPTIONS+1] = { "Can you use two weapons at once? (slight unfair advantage)", "How the grip button works",
	"How to pick up/collect/use items and power-ups", "Must manually empty your hands before interacting", "Buttons behave according to context (recommended) instead of always as bound.", "How to cycle through weapons", "Reload mode" };
static menuOption_t cmdType[NUM_MENU_OPTIONS+1] = { OPTION_SLIDER_TEXT, OPTION_SLIDER_TEXT, OPTION_SLIDER_TEXT, OPTION_SLIDER_TEXT, OPTION_SLIDER_TEXT, OPTION_SLIDER_TEXT, OPTION_SLIDER_TEXT };

float LinearAdjust( const float input, const float currentMin, const float currentMax, const float desiredMin,  float desiredMax );
int	AdjustOption( const int currentValue, const int values[], const int numValues, const int adjustment );

/*
========================
idMenuScreen_Shell_VR_Hand_Options::Initialize
========================
*/
void idMenuScreen_Shell_VR_Hand_Options::Initialize( idMenuHandler * data ) {
	idMenuScreen::Initialize( data );

	currentOffset = 0;
	lastIndex = 0;

	if ( data != NULL ) {
		menuGUI = data->GetGUI();
	}

	SetSpritePath( "menuSystemOptions" );
		
	options = new (TAG_SWF) idMenuWidget_DynamicList();
	options->SetNumVisibleOptions( Max( NUM_MENU_OPTIONS, NUM_DISPLAY_LINES ) );
	options->SetSpritePath( GetSpritePath(), "info", "options" );
	options->SetWrappingAllowed( false );
	options->SetControlList( true );
	options->Initialize( data );
	AddChild( options );

	idMenuWidget_Help* const helpWidget = new( TAG_SWF ) idMenuWidget_Help();
	helpWidget->SetSpritePath( GetSpritePath(), "info", "helpTooltip" );
	AddChild( helpWidget );

	btnBack = new ( TAG_SWF ) idMenuWidget_Button();
	btnBack->Initialize( data );
	btnBack->SetLabel( "VR Options" );
	btnBack->SetSpritePath( GetSpritePath(), "info", "btnBack" );
	btnBack->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_GO_BACK );
	AddChild( btnBack );

	idMenuWidget_ControlButton * control;
		
	for ( int ctrl = 0; ctrl < NUM_MENU_OPTIONS; ctrl++ )
	{
		control = new (TAG_SWF)idMenuWidget_ControlButton();
		control->SetOptionType( cmdType[ctrl] );
		control->SetLabel( cmdName[ctrl] );
		control->SetDescription( cmdDescription[ctrl] );
		control->RegisterEventObserver( helpWidget );
		control->SetDataSource( &systemData, idDataSource::FIELD_FIRST + ctrl );
		control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
		control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idDataSource::FIELD_FIRST + ctrl );
		control->AddEventAction( WIDGET_EVENT_SCROLL_UP ).Set( WIDGET_ACTION_COMMAND, idDataSource::FIELD_FIRST + ctrl );
		control->AddEventAction( WIDGET_EVENT_SCROLL_DOWN ).Set( WIDGET_ACTION_COMMAND, idDataSource::FIELD_FIRST + ctrl );

		options->AddChild( control );
	}

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
idMenuScreen_Shell_VR_Hand_Options::Update
========================
*/
void idMenuScreen_Shell_VR_Hand_Options::Update() {
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
			heading->SetText( "VR Hand Interactions" );	
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
	


	if ( NUM_MENU_OPTIONS > NUM_DISPLAY_LINES )
	{
		int activeIndex = options->GetFocusIndex();
	
		if ( lastIndex == 0 && activeIndex == NUM_MENU_OPTIONS - 1 ) // we wrapped off the top of the screen.
		{
			currentOffset--;
			activeIndex = 0;
		}
		else if ( lastIndex == NUM_DISPLAY_LINES - 1 && activeIndex == NUM_DISPLAY_LINES )// wrapped off bottom of screen.
		{
			currentOffset++;
			activeIndex = NUM_DISPLAY_LINES - 1;
		}
		
		if ( currentOffset < 0 ){
			currentOffset = 0;
			activeIndex = 0;
		}

		if ( currentOffset > NUM_MENU_OPTIONS - NUM_DISPLAY_LINES )
		{
			currentOffset = NUM_MENU_OPTIONS - NUM_DISPLAY_LINES;
			activeIndex = NUM_DISPLAY_LINES-1;
		}

		if ( currentOffset > 0 && lastIndex == 0 && activeIndex == 0 ) currentOffset --;// if this is the case, the user tried
		
				
		for ( int ctrl = 0; ctrl < NUM_DISPLAY_LINES ; ctrl++ )
		{
			if ( options != NULL && options->GetTotalNumberOfOptions() > 0 )
			{
				idMenuWidget_ControlButton* button = dynamic_cast<idMenuWidget_ControlButton*>(&options->GetChildByIndex( ctrl ));
				if ( button != NULL )
				{
					button->SetDataSource( &systemData, idDataSource::FIELD_FIRST + ctrl + currentOffset );
					button->SetOptionType( cmdType[ctrl + currentOffset] );
					button->SetLabel( cmdName[ctrl + currentOffset] );
					button->SetDescription( cmdDescription[ctrl + currentOffset] );
				}
			}
		}
		common->Printf( "lastIndex%d CurrentOffset %d activeIndex %d\n", lastIndex,currentOffset, activeIndex );
		options->SetFocusIndex( activeIndex );
		lastIndex = activeIndex;
	}


	common->Printf( "Update %d\n", Sys_Milliseconds() );
	idMenuScreen::Update();
}

/*
========================
idMenuScreen_Shell_VR_Hand_Options::ShowScreen
========================
*/
void idMenuScreen_Shell_VR_Hand_Options::ShowScreen( const mainMenuTransition_t transitionType ) {
	systemData.LoadData();
	idMenuScreen::ShowScreen( transitionType );
}

/*
========================
idMenuScreen_Shell_VR_Hand_Options::HideScreen
========================
*/
void idMenuScreen_Shell_VR_Hand_Options::HideScreen( const mainMenuTransition_t transitionType ) {
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
idMenuScreen_Shell_VR_Hand_Options::HandleAction h
========================
*/
bool idMenuScreen_Shell_VR_Hand_Options::HandleAction( idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled ) {
	if ( menuData == NULL ) {
		return true;
	}
	
	if ( menuData->ActiveScreen() != SHELL_AREA_VR_HAND_OPTIONS ) {
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
idMenuScreen_Shell_VR_Hand_Options::idDataSource::idDataSource
========================
*/
idMenuScreen_Shell_VR_Hand_Options::idDataSource::idDataSource() {
}

/*
========================
idMenuScreen_Shell_VR_Hand_Options::idDataSource::LoadData
========================
*/
void idMenuScreen_Shell_VR_Hand_Options::idDataSource::LoadData() {

	originalWeaponHand = vr_weaponHand.GetInteger();
	originalSlotDisable = vr_slotDisable.GetInteger();
	originalDualWield = vr_dualWield.GetInteger();
	originalGripMode = vr_gripMode.GetInteger();
	originalPickupMode = vr_pickUpMode.GetInteger();
	originalReloadMode = vr_reloadMode.GetInteger();
	originalWeaponCycleMode = vr_weaponCycleMode.GetInteger();
	originalMustEmptyHands = vr_mustEmptyHands.GetBool();
	originalContextSensitive = vr_contextSensitive.GetBool();
}

/*
========================
idMenuScreen_Shell_VR_Hand_Options::idDataSource::IsRestartRequired
========================
*/
bool idMenuScreen_Shell_VR_Hand_Options::idDataSource::IsRestartRequired() const {
	return false;
}

/*
========================
idMenuScreen_Shell_VR_Hand_Options::idDataSource::CommitData
========================
*/
void idMenuScreen_Shell_VR_Hand_Options::idDataSource::CommitData() {
	cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
}


/*
========================
idMenuScreen_Shell_VR_Hand_Options::idDataSource::AdjustField
========================
*/
void idMenuScreen_Shell_VR_Hand_Options::idDataSource::AdjustField( const int fieldIndex, const int adjustAmount ) {
	switch ( fieldIndex )
	{
		case FIELD_WEAPON_HAND:
		{
			static const int numValues = 2;
			static const int values[numValues] = { 0, 1 };
			int new_hand = AdjustOption( vr_weaponHand.GetInteger(), values, numValues, adjustAmount );
			if( new_hand != vr_weaponHand.GetInteger() )
				SwapWeaponHand();
			break;
		}

		case FIELD_HOLSTER_SLOTS:
		{
			static const int numValues = 2;
			static const int values[numValues] = { 1, 0 };
			vr_slotDisable.SetInteger( AdjustOption( vr_slotDisable.GetInteger(), values, numValues, adjustAmount ) );
			break;
		}

		case FIELD_DUAL_WIELD:
		{
			static const int numValues = 9;
			static const int values[numValues] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
			vr_dualWield.SetInteger( AdjustOption( vr_dualWield.GetInteger(), values, numValues, adjustAmount ) );
			break;
		}

		case FIELD_PICK_UP_MODE:
		{
			static const int numValues = 7; // How to pick up/collect/use items and powerups 0 = walk/touch , 1 = walk over, 2 = touch, 3 = manual grip, 4 = put in body, 5 = put in properly, 6 = hold and press trigger
			static const int values[numValues] = { 0, 1, 2, 3, 4, 5, 6 };
			vr_pickUpMode.SetInteger( AdjustOption( vr_pickUpMode.GetInteger(), values, numValues, adjustAmount ) );
			break;
		}

		case FIELD_RELOAD_MODE:
		{
			//vr_reloadMode.GetInteger();
			break;
		}

		case FIELD_GRIP_MODE:
		{
			static const int numValues = 8; // How the grip button works\n0 = context sensitive toggle, 1 = context sensitive toggle no surface, 2 = toggle for weapons/items hold for physics objects, 3 = toggle for weapons hold for physics/items, 4 = always toggle (can drop), 5 = Dead and Burried, 6 = hold to hold, 7 = hold to hold squeeze for action
			static const int values[numValues] = { 0, 1, 2, 3, 4, 5, 6, 7 };
			vr_gripMode.SetInteger( AdjustOption( vr_gripMode.GetInteger(), values, numValues, adjustAmount ) );
			break;
		}

		case FIELD_WEAPON_CYCLE_MODE:
		{
			static const int numValues = 5; // When cycling through weapons\n0 = skip holstered weapons, 1 = include holstered weapons, 2 = flashlight but not holstered, 3 = holstered+flashlight, 4 = holstered+flashlight+pda
			static const int values[numValues] = { 0, 1, 2, 3, 4 };
			vr_weaponCycleMode.SetInteger( AdjustOption( vr_weaponCycleMode.GetInteger(), values, numValues, adjustAmount ) );
			break;
		}

		case FIELD_MUST_EMPTY_HANDS:
		{
			vr_mustEmptyHands.SetBool( !vr_mustEmptyHands.GetBool() );
			break;
		}
	}
	cvarSystem->ClearModifiedFlags( CVAR_ARCHIVE );
}

/*
========================
idMenuScreen_Shell_VR_Hand_Options::idDataSource::GetField	
========================
*/
idSWFScriptVar idMenuScreen_Shell_VR_Hand_Options::idDataSource::GetField( const int fieldIndex ) const {
	switch ( fieldIndex )
	{
		case FIELD_WEAPON_HAND:
		{
			if( vr_weaponHand.GetInteger() == 0 )
				return "Right Hand";
			else
				return "Left Hand";
		}

		case FIELD_HOLSTER_SLOTS:
		{
			if( vr_slotDisable.GetInteger() == 0 )
				return "#str_swf_enabled";
			else
				return "#str_swf_disabled";
		}

		case FIELD_DUAL_WIELD:
		{
			const char* names[] = { "not even fists", "nothing", "only flashlight", "only grenades (VFR)", "only grenades/flashlight", "only pistols", "only pistols/flashlight", "pistol/grenade/flashlight", "yes" };
			return names[vr_dualWield.GetInteger()];
		}

		case FIELD_GRIP_MODE:
		{
			const char* names[] = { "context toggle", "toggle no surface", "hold physics", "hold non-weapons", "toggle with drop", "Dead and Burried", "hold to hold", "hold/squeeze" };
			return names[vr_gripMode.GetInteger()];
		}

		case FIELD_PICK_UP_MODE:
		{
			const char* names[] = { "walk over", "walk over/touch", "touch", "grab", "put in body", "put in properly", "hold and trigger" };
			return names[vr_pickUpMode.GetInteger()];
		}

		case FIELD_MUST_EMPTY_HANDS:
		{
			if( vr_mustEmptyHands.GetBool() )
				return "Yes";
			else
				return "No";
		}

		case FIELD_CONTEXT_SENSITIVE:
		{
			if( vr_slotDisable.GetInteger() == 0 )
				return "On (recommended)";
			else
				return "#str_swf_disabled";
		}

		case FIELD_WEAPON_CYCLE_MODE:
		{
			const char* names[] = { "skip holstered", "include holstered", "include flashlight", "holstered + flashlight", "even include PDA" };
			return names[vr_weaponCycleMode.GetInteger()];
		}
	}
	return false;
}

/*
========================
idMenuScreen_Shell_VR_Hand_Options::idDataSource::IsDataChanged	
========================
*/
bool idMenuScreen_Shell_VR_Hand_Options::idDataSource::IsDataChanged() const {

	if( originalWeaponHand != vr_weaponHand.GetInteger() )
		return true;
	if( originalSlotDisable != vr_slotDisable.GetFloat() )
		return true;
	if( originalDualWield != vr_dualWield.GetInteger() )
		return true;
	if( originalGripMode != vr_gripMode.GetInteger() )
		return true;
	if( originalMustEmptyHands != vr_mustEmptyHands.GetBool() )
		return true;
	if( originalPickupMode != vr_pickUpMode.GetInteger() )
		return true;
	if( originalReloadMode != vr_reloadMode.GetInteger() )
		return true;
	if( originalWeaponCycleMode != vr_weaponCycleMode.GetInteger() )
		return true;
	if( originalContextSensitive != vr_contextSensitive.GetBool() )
		return true;

	return false;
}
