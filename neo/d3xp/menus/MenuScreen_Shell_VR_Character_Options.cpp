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

const static int NUM_SYSTEM_VR_GAMEPLAY_OPTIONS = 8;


float LinearAdjust( const float input, const float currentMin, const float currentMax, const float desiredMin,  float desiredMax );
int	AdjustOption( const int currentValue, const int values[], const int numValues, const int adjustment );

/*
========================
idMenuScreen_Shell_VR_Character_Options::Initialize
========================
*/
void idMenuScreen_Shell_VR_Character_Options::Initialize( idMenuHandler * data ) {
	idMenuScreen::Initialize( data );

	if ( data != NULL ) {
		menuGUI = data->GetGUI();
	}

	SetSpritePath( "menuSystemOptions" );
	
	options = new (TAG_SWF) idMenuWidget_DynamicList();
	options->SetNumVisibleOptions( NUM_SYSTEM_VR_GAMEPLAY_OPTIONS );
	options->SetSpritePath( GetSpritePath(), "info", "options" );
	options->SetWrappingAllowed( true );
	options->SetControlList( true );
	options->Initialize( data );

	btnBack = new (TAG_SWF) idMenuWidget_Button();
	btnBack->Initialize( data );
	btnBack->SetLabel( "VR Options" );
	btnBack->SetSpritePath( GetSpritePath(), "info", "btnBack" );
	btnBack->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_GO_BACK );

	AddChild( options );
	AddChild( btnBack );
	
	idMenuWidget_ControlButton * control;
	
	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "Body Mode" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Character_Options::CHARACTER_OPTIONS_FIELD_BODY_MODE );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Character_Options::CHARACTER_OPTIONS_FIELD_BODY_MODE );
	options->AddChild( control );

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "Flashlight Mount" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Character_Options::CHARACTER_OPTIONS_FIELD_FLASH_MODE );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Character_Options::CHARACTER_OPTIONS_FIELD_FLASH_MODE );
	options->AddChild( control );

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "Weapon Hand" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Character_Options::CHARACTER_OPTIONS_FIELD_WEAPON_HAND );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Character_Options::CHARACTER_OPTIONS_FIELD_WEAPON_HAND );
	options->AddChild( control );

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel("Holster Slots");
	control->SetDataSource(&systemData, idMenuDataSource_Shell_VR_Character_Options::CHARACTER_OPTIONS_FIELD_HOLSTER_SLOTS);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Character_Options::CHARACTER_OPTIONS_FIELD_HOLSTER_SLOTS);
	options->AddChild(control);

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "Use Height" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Character_Options::CHARACTER_OPTIONS_FIELD_USE_FLOOR_HEIGHT );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction(WIDGET_EVENT_PRESS).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Character_Options::CHARACTER_OPTIONS_FIELD_USE_FLOOR_HEIGHT );
	options->AddChild( control );

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel("Custom Eye Height");
	control->SetDataSource(&systemData, idMenuDataSource_Shell_VR_Character_Options::CHARACTER_OPTIONS_FIELD_VIEW_HEIGHT);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Character_Options::CHARACTER_OPTIONS_FIELD_VIEW_HEIGHT);
	options->AddChild(control);

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel("World Scale Adjust");
	control->SetDataSource(&systemData, idMenuDataSource_Shell_VR_Character_Options::CHARACTER_OPTIONS_FIELD_SCALE);
	control->SetupEvents(2, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Character_Options::CHARACTER_OPTIONS_FIELD_SCALE);
	options->AddChild(control);

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
idMenuScreen_Shell_VR_Character_Options::Update
========================
*/
void idMenuScreen_Shell_VR_Character_Options::Update() {

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
			heading->SetText( "VR Character Options" );	
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
idMenuScreen_Shell_VR_Character_Options::ShowScreen
========================
*/
void idMenuScreen_Shell_VR_Character_Options::ShowScreen( const mainMenuTransition_t transitionType ) {
	
	systemData.LoadData();
	
	idMenuScreen::ShowScreen( transitionType );
}

/*
========================
idMenuScreen_Shell_VR_Character_Options::HideScreen
========================
*/
void idMenuScreen_Shell_VR_Character_Options::HideScreen( const mainMenuTransition_t transitionType ) {

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
idMenuScreen_Shell_VR_Character_Options::HandleAction h
========================
*/
bool idMenuScreen_Shell_VR_Character_Options::HandleAction( idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled ) {

	if ( menuData == NULL ) {
		return true;
	}
	
	if ( menuData->ActiveScreen() != SHELL_AREA_VR_CHARACTER_OPTIONS ) {
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
idMenuScreen_Shell_VR_Character_Options::idMenuDataSource_Shell_VR_Gameplay_Options::idMenuDataSource_Shell_VR_Gameplay_Options
========================
*/
idMenuScreen_Shell_VR_Character_Options::idMenuDataSource_Shell_VR_Character_Options::idMenuDataSource_Shell_VR_Character_Options() {
}

/*
========================
idMenuScreen_Shell_VR_Character_Options::idMenuDataSource_Shell_VR_Gameplay_Options::LoadData
========================
*/
void idMenuScreen_Shell_VR_Character_Options::idMenuDataSource_Shell_VR_Character_Options::LoadData() {
	

	originalBodyMode = vr_playerBodyMode.GetInteger();
	originalFlashMode = vr_flashlightMode.GetInteger();
	originalWeaponHand = vr_weaponHand.GetInteger();
	originalSlotDisable = vr_slotDisable.GetInteger();
	originalViewHeight = vr_normalViewHeight.GetFloat();
	originalUseFloorHeight = vr_useFloorHeight.GetInteger();
	originalScale = vr_scale.GetFloat();
}

/*
========================
idMenuScreen_Shell_VR_Character_Options::idMenuDataSource_Shell_VR_Gameplay_Options::IsRestartRequired
========================
*/
bool idMenuScreen_Shell_VR_Character_Options::idMenuDataSource_Shell_VR_Character_Options::IsRestartRequired() const {
		
	return false;
}

/*
========================
idMenuScreen_Shell_VR_Character_Options::idMenuDataSource_Shell_VR_Gameplay_Options::CommitData
========================
*/
void idMenuScreen_Shell_VR_Character_Options::idMenuDataSource_Shell_VR_Character_Options::CommitData() {
	cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
}

/*
========================
idMenuScreen_Shell_VR_Character_Options::idMenuDataSource_Shell_VR_Gameplay_Options::AdjustField
========================
*/
void idMenuScreen_Shell_VR_Character_Options::idMenuDataSource_Shell_VR_Character_Options::AdjustField( const int fieldIndex, const int adjustAmount ) {
	switch ( fieldIndex ) {
		case CHARACTER_OPTIONS_FIELD_BODY_MODE:
		{
			static const int numValues = 3;
			static const int values[numValues] = { 0, 1, 2 };
			vr_playerBodyMode.SetInteger( AdjustOption( vr_playerBodyMode.GetInteger(), values, numValues, adjustAmount ) );
			break;
		}
		
		case CHARACTER_OPTIONS_FIELD_FLASH_MODE:
		{
			static const int numValues = 4;
			static const int values[numValues] = { 0, 1, 2, 3 };
			vr_flashlightMode.SetInteger( AdjustOption( vr_flashlightMode.GetInteger(), values, numValues, adjustAmount ) );
			break;
		}
		
		case CHARACTER_OPTIONS_FIELD_WEAPON_HAND:
		{
			static const int numValues = 2;
			static const int values[numValues] = { 0, 1 };
			int new_hand = AdjustOption( vr_weaponHand.GetInteger(), values, numValues, adjustAmount );
			if ( new_hand != vr_weaponHand.GetInteger() )
				SwapWeaponHand();
			break;
		}
		
		case CHARACTER_OPTIONS_FIELD_HOLSTER_SLOTS:
		{
			static const int numValues = 2;
			static const int values[numValues] = { 1, 0 };
			vr_slotDisable.SetInteger(AdjustOption(vr_slotDisable.GetInteger(), values, numValues, adjustAmount));
			break;
		}

		case CHARACTER_OPTIONS_FIELD_VIEW_HEIGHT: {
			const float value = vr_normalViewHeight.GetFloat();
			const float adjusted = value + (float) adjustAmount * .5f;
			const float clamped = idMath::ClampFloat( 40.0f, 80.0f, adjusted );
			vr_normalViewHeight.SetFloat( clamped );
			break;
		}
	
		case CHARACTER_OPTIONS_FIELD_USE_FLOOR_HEIGHT:
		{
			static const int numValues = 5;
			static const int values[numValues] = { 0, 1, 2, 3, 4 };
			vr_useFloorHeight.SetInteger( AdjustOption( vr_useFloorHeight.GetInteger(), values, numValues, adjustAmount ) );
			break;
		}

		case CHARACTER_OPTIONS_FIELD_SCALE: {
			const float percent = vr_scale.GetFloat();
			const float adjusted = percent + (float)adjustAmount * .01f;
			const float clamped = idMath::ClampFloat( 0.2f, 2.0f, adjusted );
			vr_scale.SetFloat( clamped );
			break;
		}
	}
	cvarSystem->ClearModifiedFlags( CVAR_ARCHIVE );
}

/*
========================
idMenuScreen_Shell_VR_Character_Options::idMenuDataSource_Shell_VR_Gameplay_Options::GetField	
========================
*/
idSWFScriptVar idMenuScreen_Shell_VR_Character_Options::idMenuDataSource_Shell_VR_Character_Options::GetField( const int fieldIndex ) const {
	switch ( fieldIndex )
	{
	
		case CHARACTER_OPTIONS_FIELD_BODY_MODE:
		{
			const int bm = vr_playerBodyMode.GetInteger();

			if ( bm == 0 )
			{
				return "Full Body + Arms";
			}

			if ( bm == 1 )
			{
				return "Hands + Weaps";
			}

			if ( bm == 2 )
			{
				return "Weapons Only";
			}
		}
			
		
		case CHARACTER_OPTIONS_FIELD_FLASH_MODE:
		{
			const int fm = vr_flashlightMode.GetInteger();

			if ( fm == 0 )
			{
				return "Body";
			}

			if ( fm == 1 )
			{
				return "Head";
			}

			if ( fm == 2 )
			{
				return "Weapon";
			}

			if ( fm == 3 )
			{
				return "Hand";
			}

		}
		
		case CHARACTER_OPTIONS_FIELD_WEAPON_HAND:
		{
			if ( vr_weaponHand.GetInteger() == 0 ) 
			{
				return "Right Hand";
			}
			else
			{
				return "Left Hand";
			}
		}
		
		case CHARACTER_OPTIONS_FIELD_HOLSTER_SLOTS:
		{
			if (vr_slotDisable.GetInteger() == 0)
			{
				return "#str_swf_enabled";
			}
			else
			{
				return "#str_swf_disabled";
			}
		}

		case CHARACTER_OPTIONS_FIELD_VIEW_HEIGHT:
		{
			return va("%.1f\" %d'%.1f\" %.1fcm", vr_normalViewHeight.GetFloat(), ((int)vr_normalViewHeight.GetFloat()) / 12, vr_normalViewHeight.GetFloat() - 12 * (((int)vr_normalViewHeight.GetFloat()) / 12), vr_normalViewHeight.GetFloat() * 2.54f);
		}

		case CHARACTER_OPTIONS_FIELD_USE_FLOOR_HEIGHT:
		{
			if (vr_useFloorHeight.GetInteger() == 1)
			{
				return "Marine eye height";
			}
			else if (vr_useFloorHeight.GetInteger() == 2)
			{
				return "Normal view height";
			}
			else if (vr_useFloorHeight.GetInteger() == 3)
			{
				return "Crouch to your height";
			}
			else if (vr_useFloorHeight.GetInteger() == 4)
			{
				return "Scale to your height";
			}
			else
			{
				return "Custom eye height";
			}
		}
		case CHARACTER_OPTIONS_FIELD_SCALE:
			return va("%.2f", vr_scale.GetFloat());

	}
	return false;
}

/*
========================
idMenuScreen_Shell_VR_Character_Options::idMenuDataSource_Shell_VR_Gameplay_Options::IsDataChanged	
========================
*/
bool idMenuScreen_Shell_VR_Character_Options::idMenuDataSource_Shell_VR_Character_Options::IsDataChanged() const {
	
	if ( originalBodyMode != vr_playerBodyMode.GetInteger() )
	{
		return true;
	}
	if ( originalFlashMode != vr_flashlightMode.GetInteger() )
	{
		return true;
	}
	if ( originalWeaponHand != vr_weaponHand.GetInteger() )
	{
		return true;
	}
	if ( originalSlotDisable != vr_slotDisable.GetFloat() )
	{
		return true;
	}
	if ( originalViewHeight != vr_normalViewHeight.GetFloat() )
	{
		return true;
	}
	if ( originalUseFloorHeight != vr_useFloorHeight.GetInteger() )
	{
		return true;
	}
	if ( originalScale != vr_scale.GetFloat() )
	{
		return true;
	}

	return false;
}
