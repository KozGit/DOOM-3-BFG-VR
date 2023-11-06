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

const static int NUM_SYSTEM_VR_SAFETY_OPTIONS = 8;


float LinearAdjust( const float input, const float currentMin, const float currentMax, const float desiredMin,  float desiredMax );
int	AdjustOption( const int currentValue, const int values[], const int numValues, const int adjustment );

/*
========================
idMenuScreen_Shell_VR_Safety_Protocols::Initialize
========================
*/
void idMenuScreen_Shell_VR_Safety_Protocols::Initialize( idMenuHandler * data ) {
	idMenuScreen::Initialize( data );

	if ( data != NULL ) {
		menuGUI = data->GetGUI();
	}

	SetSpritePath( "menuSystemOptions" );
	
	options = new (TAG_SWF) idMenuWidget_DynamicList();
	options->SetNumVisibleOptions( NUM_SYSTEM_VR_SAFETY_OPTIONS );
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
	
	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "Teleport" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Safety_Protocols::SAFETY_PROTOCOLS_FIELD_TELEPORTATION );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Safety_Protocols::SAFETY_PROTOCOLS_FIELD_TELEPORTATION );
	options->AddChild( control );

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel("Teleport Mode");
	control->SetDataSource(&systemData, idMenuDataSource_Shell_VR_Safety_Protocols::SAFETY_PROTOCOLS_FIELD_TELEPORTATION_MODE);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Safety_Protocols::SAFETY_PROTOCOLS_FIELD_TELEPORTATION_MODE);
	options->AddChild(control);

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "Turning" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Safety_Protocols::SAFETY_PROTOCOLS_FIELD_SNAP_TURNS );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Safety_Protocols::SAFETY_PROTOCOLS_FIELD_SNAP_TURNS );
	options->AddChild( control );

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel("JetStrafe Turning");
	control->SetDataSource(&systemData, idMenuDataSource_Shell_VR_Safety_Protocols::SAFETY_PROTOCOLS_FIELD_JET_SNAP_TURNS);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Safety_Protocols::SAFETY_PROTOCOLS_FIELD_JET_SNAP_TURNS);
	options->AddChild(control);

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "Walk Speed Adj" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Safety_Protocols::SAFETY_PROTOCOLS_FIELD_WALK_SPEED_ADJUST );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Safety_Protocols::SAFETY_PROTOCOLS_FIELD_WALK_SPEED_ADJUST );
	options->AddChild( control );

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "Motion Sickness Aid" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Safety_Protocols::SAFETY_PROTOCOLS_FIELD_MOTION_SICKNESS );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Safety_Protocols::SAFETY_PROTOCOLS_FIELD_MOTION_SICKNESS );
	options->AddChild( control );

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "KnockBack & Head Kick" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Safety_Protocols::SAFETY_PROTOCOLS_FIELD_KNOCKBACK );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Safety_Protocols::SAFETY_PROTOCOLS_FIELD_KNOCKBACK );
	options->AddChild( control );

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_TEXT );
	control->SetLabel( "Step Smooth & Jump Bounce" );
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Safety_Protocols::SAFETY_PROTOCOLS_FIELD_HEADBOB );
	control->SetupEvents( DEFAULT_REPEAT_TIME, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Safety_Protocols::SAFETY_PROTOCOLS_FIELD_HEADBOB );
	options->AddChild( control );

	
	control = new(TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType( OPTION_SLIDER_BAR );
	control->SetLabel( "Shake Amplitude" );	// Brightness
	control->SetDataSource( &systemData, idMenuDataSource_Shell_VR_Safety_Protocols::SAFETY_PROTOCOLS_FIELD_SHAKE_AMPLITUDE );
	control->SetupEvents( 2, options->GetChildren().Num() );
	control->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Safety_Protocols::SAFETY_PROTOCOLS_FIELD_SHAKE_AMPLITUDE );
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
idMenuScreen_Shell_VR_Safety_Protocols::Update
========================
*/
void idMenuScreen_Shell_VR_Safety_Protocols::Update() {

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
			heading->SetText( "VR Comfort + Safety" );
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
idMenuScreen_Shell_VR_Safety_Protocols::ShowScreen
========================
*/
void idMenuScreen_Shell_VR_Safety_Protocols::ShowScreen( const mainMenuTransition_t transitionType ) {
	
	systemData.LoadData();
	
	idMenuScreen::ShowScreen( transitionType );
}

/*
========================
idMenuScreen_Shell_VR_Safety_Protocols::HideScreen
========================
*/
void idMenuScreen_Shell_VR_Safety_Protocols::HideScreen( const mainMenuTransition_t transitionType ) {

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
idMenuScreen_Shell_VR_Safety_Protocols::HandleAction h
========================
*/
bool idMenuScreen_Shell_VR_Safety_Protocols::HandleAction( idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled ) {

	if ( menuData == NULL ) {
		return true;
	}
	
	if ( menuData->ActiveScreen() != SHELL_AREA_VR_SAFETY_PROTOCOLS ) {
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
idMenuScreen_Shell_VR_Safety_Protocols::idMenuDataSource_Shell_VR_Gameplay_Options::idMenuDataSource_Shell_VR_Gameplay_Options
========================
*/
idMenuScreen_Shell_VR_Safety_Protocols::idMenuDataSource_Shell_VR_Safety_Protocols::idMenuDataSource_Shell_VR_Safety_Protocols() {
}

/*
========================
idMenuScreen_Shell_VR_Safety_Protocols::idMenuDataSource_Shell_VR_Gameplay_Options::LoadData
========================
*/
void idMenuScreen_Shell_VR_Safety_Protocols::idMenuDataSource_Shell_VR_Safety_Protocols::LoadData() {
	
	originalComfortDelta = vr_comfortDelta.GetFloat();
	originalComfortJetStrafeDelta = vr_comfortJetStrafeDelta.GetFloat();
	originalTeleport = vr_teleport.GetInteger();
	originalTeleportMode = vr_teleportMode.GetInteger();
	originalMotionSickness = vr_motionSickness.GetInteger();
	originalWalkSpeedAdjust = vr_walkSpeedAdjust.GetFloat();
	originalKnockBack = vr_knockBack.GetInteger();
	originalShakeAmplitude = vr_shakeAmplitude.GetFloat();
	originalHeadKick = vr_headKick.GetInteger();
	originalStepSmooth = vr_stepSmooth.GetFloat();
	originalJumpBounce = vr_jumpBounce.GetFloat();

}

/*
========================
idMenuScreen_Shell_VR_Safety_Protocols::idMenuDataSource_Shell_VR_Gameplay_Options::IsRestartRequired
========================
*/
bool idMenuScreen_Shell_VR_Safety_Protocols::idMenuDataSource_Shell_VR_Safety_Protocols::IsRestartRequired() const {
		
	return false;
}

/*
========================
idMenuScreen_Shell_VR_Safety_Protocols::idMenuDataSource_Shell_VR_Gameplay_Options::CommitData
========================
*/
void idMenuScreen_Shell_VR_Safety_Protocols::idMenuDataSource_Shell_VR_Safety_Protocols::CommitData() {
	cvarSystem->SetModifiedFlags( CVAR_ARCHIVE );
}

/*
========================
idMenuScreen_Shell_VR_Safety_Protocols::idMenuDataSource_Shell_VR_Gameplay_Options::AdjustField
========================
*/
void idMenuScreen_Shell_VR_Safety_Protocols::idMenuDataSource_Shell_VR_Safety_Protocols::AdjustField( const int fieldIndex, const int adjustAmount ) {
	switch ( fieldIndex ) {
		case SAFETY_PROTOCOLS_FIELD_TELEPORTATION:
		{
			static const int numValues = 5;
			static const int values[numValues] = { 0, 1, 2, 3, 4 };
			vr_teleport.SetInteger( AdjustOption( vr_teleport.GetInteger(), values, numValues, adjustAmount ) );
			break;
		}

		case SAFETY_PROTOCOLS_FIELD_TELEPORTATION_MODE:
		{
			static const int numValues = 3;
			static const int values[numValues] = { 0, 1, 2 };
			vr_teleportMode.SetInteger(AdjustOption(vr_teleportMode.GetInteger(), values, numValues, adjustAmount));
			break;
		}

		case SAFETY_PROTOCOLS_FIELD_SNAP_TURNS:
		{
			static const int numValues = 7;
			static const int values[numValues] = { 0, 1, 10, 30, 45, 90, 180 };
			int value = (int)(vr_comfortDelta.GetFloat() + 0.5f);
			int comfortCount = 0, analogCount = 0;
			for (int k = K_JOY17; k < K_R_STEAMVRTRIG; k++)
			{
				// Don't count gamepad, because it has both modes mapped at once
				if (k < K_JOY_STICK1_UP || k > K_JOY_TRIGGER2)
				{
					if ( idStr::Icmp( idKeyInput::GetBinding( k ), "_impulse34" ) == 0 || idStr::Icmp( idKeyInput::GetBinding( k ), "_impulse35" ) == 0 )
						comfortCount++;
					if ( idStr::Icmp( idKeyInput::GetBinding( k ), "_right" ) == 0 || idStr::Icmp( idKeyInput::GetBinding( k ), "_left" ) == 0 )
						analogCount++;
				}
			}
			if ( analogCount > comfortCount )
				value = 1;
			else if ( value > 180 )
				value = 180;
			else
			{
				for (int i = 0; i < numValues - 1; i++)
				{
					if ( value > values[i] && value < values[i + 1] )
					{
						value = values[i + 1];
						break;
					}
				}
			}
			value = AdjustOption( value, values, numValues, adjustAmount );
			if ( value == 1 )
			{
				// continuous: unbind comfort mode turns and rebind normal turns
				vr_comfortDelta.SetFloat( 10.0f );
				for (int k = K_JOY17; k < K_R_STEAMVRTRIG; k++)
				{
					// Don't change gamepad, because it has both modes mapped at once
					if (k < K_JOY_STICK1_UP || k > K_JOY_TRIGGER2)
					{
						if ( idStr::Icmp( idKeyInput::GetBinding( k ), "_impulse34" ) == 0 )
							idKeyInput::SetBinding( k, "_right" );
						else if ( idStr::Icmp( idKeyInput::GetBinding( k ), "_impulse35" ) == 0 )
							idKeyInput::SetBinding( k, "_left" );
					}
				}
			}
			else
			{
				// enable snap turns
				// unbind normal turns and rebind comfort mode turns
				vr_comfortDelta.SetFloat( value );
				for (int k = K_JOY17; k < K_R_STEAMVRTRIG; k++)
				{
					// Don't change gamepad, because it has both modes mapped at once
					if (k < K_JOY_STICK1_UP || k > K_JOY_TRIGGER2)
					{
						if ( idStr::Icmp( idKeyInput::GetBinding( k ), "_right" ) == 0 )
							idKeyInput::SetBinding( k, "_impulse34" );
						else if ( idStr::Icmp(idKeyInput::GetBinding( k ), "_left" ) == 0 )
							idKeyInput::SetBinding( k, "_impulse35" );
					}
				}
			}
			break;
		}

		case SAFETY_PROTOCOLS_FIELD_JET_SNAP_TURNS:
		{
			static const int numValues = 6;
			static const int values[numValues] = { 0, 1, 10, 30, 45, 90 };
			int value = (int)(vr_comfortJetStrafeDelta.GetFloat() + 0.5f);
			if (value < 1)
				value = 1;
			else if (value > 90)
				value = 90;
			else
			{
				for (int i = 0; i < numValues - 1; i++)
				{
					if (value > values[i] && value < values[i + 1])
					{
						value = values[i + 1];
						break;
					}
				}
			}
			value = AdjustOption(value, values, numValues, adjustAmount);
			vr_comfortJetStrafeDelta.SetFloat(value);
			break;
		}

		case SAFETY_PROTOCOLS_FIELD_WALK_SPEED_ADJUST:
		{
			float ws = vr_walkSpeedAdjust.GetFloat();
			ws += adjustAmount;
			if ( ws < -100 ) ws = -100;
			if ( ws > 150 ) ws = 150;
			vr_walkSpeedAdjust.SetFloat( ws );
			break;
		}

		case SAFETY_PROTOCOLS_FIELD_MOTION_SICKNESS:
		{
			static const int numValues = 13;
			// 0 = None, 1 = Chaperone, 2 = Reduce FOV, 3 = Black Screen, 4 = Black & Chaperone, 5 = Reduce FOV & Chaperone, 6 = Slow Mo, 7 = Slow Mo & Chaperone, 8 = Slow Mo & Reduce FOV, 9 = Slow Mo, Chaperone, Reduce FOV, 10 = Third Person, 11 = Particles, 12 = Particles & Chaperone
			static const int values[numValues] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
			vr_motionSickness.SetInteger(AdjustOption(vr_motionSickness.GetInteger(), values, numValues, adjustAmount));
			break;
		}

		case SAFETY_PROTOCOLS_FIELD_KNOCKBACK:
		{
			static const int numValues = 4;
			static const int values[numValues] = { 0, 1, 2, 3 };
			int value = vr_knockBack.GetInteger() + ( vr_headKick.GetInteger() << 1 );
			value = AdjustOption( value, values, numValues, adjustAmount );
			vr_knockBack.SetInteger( value & 1 );
			vr_headKick.SetInteger( value >> 1 );
			break;
		}

		case SAFETY_PROTOCOLS_FIELD_HEADBOB:
		{
			static const int numValues = 4;
			static const int values[numValues] = { 0, 1, 2, 3 };
			int value = ((vr_stepSmooth.GetFloat() > 0) ? 1 : 0) | ((vr_jumpBounce.GetFloat() > 0) ? 2 : 0);
			value = AdjustOption( value, values, numValues, adjustAmount );
			vr_stepSmooth.SetInteger( value & 1 );
			vr_jumpBounce.SetInteger( value >> 1 );
			break;
		}

		case SAFETY_PROTOCOLS_FIELD_SHAKE_AMPLITUDE: {
			const float percent = vr_shakeAmplitude.GetFloat();;
			const float adjusted = percent + (float)adjustAmount * .01f;
			const float clamped = idMath::ClampFloat( 0.0f, 1.0f, adjusted );
			vr_shakeAmplitude.SetFloat( clamped );
			break;
		}

	
	}
	cvarSystem->ClearModifiedFlags( CVAR_ARCHIVE );
}

/*
========================
idMenuScreen_Shell_VR_Safety_Protocols::idMenuDataSource_Shell_VR_Gameplay_Options::GetField	
========================
*/
idSWFScriptVar idMenuScreen_Shell_VR_Safety_Protocols::idMenuDataSource_Shell_VR_Safety_Protocols::GetField( const int fieldIndex ) const {
	switch ( fieldIndex )
	{
	
		case SAFETY_PROTOCOLS_FIELD_TELEPORTATION:
		{
			const char* names[] = { "Disabled", "Gun Sight", "Right Hand", "Left Hand", "Head" };
			return names[vr_teleport.GetInteger()];
		}

		case SAFETY_PROTOCOLS_FIELD_TELEPORTATION_MODE:
		{
			const char* names[] = { "Blink", "Doom VFR", "Doom VFR + JetStrafe"};
			return names[vr_teleportMode.GetInteger()];
		}

		case SAFETY_PROTOCOLS_FIELD_SNAP_TURNS:
		{
			float f = vr_comfortDelta.GetFloat();
			int comfortCount = 0, analogCount = 0;
			for (int k = K_JOY17; k < K_R_STEAMVRTRIG; k++)
			{
					// Don't count gamepad, because it has both modes mapped at once
				if (k < K_JOY_STICK1_UP || k > K_JOY_TRIGGER2)
				{
					if ( idStr::Icmp( idKeyInput::GetBinding( k ), "_impulse34" ) == 0 || idStr::Icmp( idKeyInput::GetBinding(k), "_impulse35" ) == 0 )
						comfortCount++;
					if ( idStr::Icmp( idKeyInput::GetBinding( k ), "_right" ) == 0 || idStr::Icmp( idKeyInput::GetBinding(k), "_left" ) == 0 )
						analogCount++;
				}
			}

			if (f < 0.5f || (analogCount==0 && comfortCount==0))
				return "Real Life"; 
			if ( analogCount >= comfortCount )
				return "Analog";
			else
				return va( "Snap %.0f degrees", f );
		}	

		case SAFETY_PROTOCOLS_FIELD_JET_SNAP_TURNS:
		{
			float f = vr_comfortJetStrafeDelta.GetFloat();

			if (f < 0.5f)
				return "Real Life";
			if (f == 1.0f)
				return "Analog";
			else
				return va("Snap %.0f degrees", f);
		}

		case SAFETY_PROTOCOLS_FIELD_WALK_SPEED_ADJUST:
			return va("%.0f", vr_walkSpeedAdjust.GetFloat());

		case SAFETY_PROTOCOLS_FIELD_MOTION_SICKNESS:
		{
			const char* names[] = { "None", "Chaperone", "Reduce FOV", "Black Screen", "Black + Chaperone", "FOV + Chaperone", "Slow Mo", "Slow Mo + Chaperone", "Slow Mo + FOV", "Slow+FOV+Chaperone", "Third Person", "Particles", "Particles & Chaperone" };
			return names[vr_motionSickness.GetInteger()];
		}

		case SAFETY_PROTOCOLS_FIELD_KNOCKBACK:
		{
			if ( !vr_knockBack.GetBool() && !vr_headKick.GetBool() )
				return "#str_swf_disabled";
			else if ( vr_knockBack.GetBool() && vr_headKick.GetBool() )
				return "#str_swf_enabled";
			else if ( vr_knockBack.GetBool() )
				return "KnockBack only";
			else if ( vr_headKick.GetBool() )
				return "HeadKick only";
		}

		case SAFETY_PROTOCOLS_FIELD_HEADBOB:
		{
			if ( !vr_stepSmooth.GetFloat() && !vr_jumpBounce.GetFloat() )
				return "#str_swf_disabled";
			else if ( vr_stepSmooth.GetFloat() && vr_jumpBounce.GetFloat() )
				return "#str_swf_enabled";
			else if ( vr_stepSmooth.GetFloat() )
				return "Step Smooth only";
			else if ( vr_jumpBounce.GetFloat() )
				return "Jump Bounce only";
		}

		case SAFETY_PROTOCOLS_FIELD_SHAKE_AMPLITUDE:
			return LinearAdjust( vr_shakeAmplitude.GetFloat(), 0.0f, 1.0f, 0.0f, 100.0f );

	}
	return false;
}

/*
========================
idMenuScreen_Shell_VR_Safety_Protocols::idMenuDataSource_Shell_VR_Gameplay_Options::IsDataChanged	
========================
*/
bool idMenuScreen_Shell_VR_Safety_Protocols::idMenuDataSource_Shell_VR_Safety_Protocols::IsDataChanged() const {
	
	if ( originalTeleport != vr_teleport.GetInteger() )
	{
		return true;
	}
	if (originalTeleportMode != vr_teleportMode.GetInteger())
	{
		return true;
	}
	if ( originalComfortDelta != vr_comfortDelta.GetFloat() )
	{
		return true;
	}
	if (originalComfortJetStrafeDelta != vr_comfortJetStrafeDelta.GetFloat())
	{
		return true;
	}
	if ( originalMotionSickness != vr_motionSickness.GetInteger() )
	{
		return true;
	}
	if ( originalKnockBack != vr_knockBack.GetInteger() )
	{
		return true;
	}
	if ( originalShakeAmplitude != vr_shakeAmplitude.GetFloat() )
	{
		return true;
	}
	if ( originalHeadKick != vr_headKick.GetInteger() )
	{
		return true;
	}
	if (originalStepSmooth != vr_stepSmooth.GetFloat())
	{
		return true;
	}
	if (originalJumpBounce != vr_jumpBounce.GetFloat())
	{
		return true;
	}

	return false;
}
