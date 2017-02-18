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

const static int NUM_FLICKSYNC_OPTIONS = 8;

float LinearAdjust(const float input, const float currentMin, const float currentMax, const float desiredMin, float desiredMax);
int	AdjustOption(const int currentValue, const int values[], const int numValues, const int adjustment);

/*
========================
idMenuScreen_Shell_VR_Flicksync::Initialize
========================
*/
void idMenuScreen_Shell_VR_Flicksync::Initialize( idMenuHandler* data )
{
	idMenuScreen::Initialize( data );
	
	if (data != NULL) {
		menuGUI = data->GetGUI();
	}

	SetSpritePath("menuSystemOptions");

	options = new (TAG_SWF)idMenuWidget_DynamicList();
	options->SetNumVisibleOptions( NUM_FLICKSYNC_OPTIONS );
	options->SetSpritePath( GetSpritePath(), "info", "options" );
	options->SetWrappingAllowed( true );
	options->SetControlList( true );
	options->Initialize( data );

	btnBack = new (TAG_SWF)idMenuWidget_Button();
	btnBack->Initialize(data);
	btnBack->SetLabel("Campaign");
	btnBack->SetSpritePath(GetSpritePath(), "info", "btnBack");
	btnBack->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_GO_BACK);

	AddChild(options);
	AddChild(btnBack);

	idMenuWidget_ControlButton * control;

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel("Character");
	control->SetDataSource(&systemData, idMenuDataSource_Shell_VR_Flicksync::FLICKSYNC_FIELD_CHARACTER);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Flicksync::FLICKSYNC_FIELD_CHARACTER);
	options->AddChild(control);

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel("Cue Cards");
	control->SetDataSource(&systemData, idMenuDataSource_Shell_VR_Flicksync::FLICKSYNC_FIELD_CUECARDS);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Flicksync::FLICKSYNC_FIELD_CUECARDS);
	options->AddChild(control);

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel("Game Type");
	control->SetDataSource(&systemData, idMenuDataSource_Shell_VR_Flicksync::FLICKSYNC_FIELD_GAMETYPE);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Flicksync::FLICKSYNC_FIELD_GAMETYPE);
	options->AddChild(control);

	control = new (TAG_SWF)idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_BUTTON_TEXT);
	control->SetLabel("Play");
	control->SetDataSource(&systemData, idMenuDataSource_Shell_VR_Flicksync::FLICKSYNC_FIELD_NEWGAME);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, idMenuDataSource_Shell_VR_Flicksync::FLICKSYNC_FIELD_NEWGAME);
	options->AddChild(control);

	options->AddEventAction(WIDGET_EVENT_SCROLL_DOWN).Set(new (TAG_SWF)idWidgetActionHandler(options, WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER, WIDGET_EVENT_SCROLL_DOWN));
	options->AddEventAction(WIDGET_EVENT_SCROLL_UP).Set(new (TAG_SWF)idWidgetActionHandler(options, WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER, WIDGET_EVENT_SCROLL_UP));
	options->AddEventAction(WIDGET_EVENT_SCROLL_DOWN_RELEASE).Set(new (TAG_SWF)idWidgetActionHandler(options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_DOWN_RELEASE));
	options->AddEventAction(WIDGET_EVENT_SCROLL_UP_RELEASE).Set(new (TAG_SWF)idWidgetActionHandler(options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_UP_RELEASE));
	options->AddEventAction(WIDGET_EVENT_SCROLL_DOWN_LSTICK).Set(new (TAG_SWF)idWidgetActionHandler(options, WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER, WIDGET_EVENT_SCROLL_DOWN_LSTICK));
	options->AddEventAction(WIDGET_EVENT_SCROLL_UP_LSTICK).Set(new (TAG_SWF)idWidgetActionHandler(options, WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER, WIDGET_EVENT_SCROLL_UP_LSTICK));
	options->AddEventAction(WIDGET_EVENT_SCROLL_DOWN_LSTICK_RELEASE).Set(new (TAG_SWF)idWidgetActionHandler(options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_DOWN_LSTICK_RELEASE));
	options->AddEventAction(WIDGET_EVENT_SCROLL_UP_LSTICK_RELEASE).Set(new (TAG_SWF)idWidgetActionHandler(options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_UP_LSTICK_RELEASE));
}

/*
========================
idMenuScreen_Shell_VR_Flicksync::Update
========================
*/
void idMenuScreen_Shell_VR_Flicksync::Update()
{
	if (menuData != NULL) {
		idMenuWidget_CommandBar * cmdBar = menuData->GetCmdBar();
		if (cmdBar != NULL) {
			cmdBar->ClearAllButtons();
			idMenuWidget_CommandBar::buttonInfo_t * buttonInfo;
			buttonInfo = cmdBar->GetButton(idMenuWidget_CommandBar::BUTTON_JOY2);
			if (menuData->GetPlatform() != 2) {
				buttonInfo->label = "#str_00395";
			}
			buttonInfo->action.Set(WIDGET_ACTION_GO_BACK);

			buttonInfo = cmdBar->GetButton(idMenuWidget_CommandBar::BUTTON_JOY1);
			buttonInfo->action.Set(WIDGET_ACTION_PRESS_FOCUSED);
		}
	}

	idSWFScriptObject & root = GetSWFObject()->GetRootObject();
	if (BindSprite(root)) {
		idSWFTextInstance * heading = GetSprite()->GetScriptObject()->GetNestedText("info", "txtHeading");
		if (heading != NULL) {
			heading->SetText("Flicksync");
			heading->SetStrokeInfo(true, 0.75f, 1.75f);
		}

		idSWFSpriteInstance * gradient = GetSprite()->GetScriptObject()->GetNestedSprite("info", "gradient");
		if (gradient != NULL && heading != NULL) {
			gradient->SetXPos(heading->GetTextLength());
		}
	}

	if (btnBack != NULL) {
		btnBack->BindSprite(root);
	}

	idMenuScreen::Update();
}

/*
========================
idMenuScreen_Shell_VR_Flicksync::ShowScreen
========================
*/
void idMenuScreen_Shell_VR_Flicksync::ShowScreen( const mainMenuTransition_t transitionType )
{
	systemData.LoadData();

	idMenuScreen::ShowScreen( transitionType );
}

/*
========================
idMenuScreen_Shell_VR_Flicksync::HideScreen
========================
*/
void idMenuScreen_Shell_VR_Flicksync::HideScreen( const mainMenuTransition_t transitionType )
{

	if (systemData.IsRestartRequired()) {
		class idSWFScriptFunction_Restart : public idSWFScriptFunction_RefCounted {
		public:
			idSWFScriptFunction_Restart(gameDialogMessages_t _msg, bool _restart) {
				msg = _msg;
				restart = _restart;
			}
			idSWFScriptVar Call(idSWFScriptObject * thisObject, const idSWFParmList & parms) {
				common->Dialog().ClearDialog(msg);
				if (restart) {
					idStr cmdLine = Sys_GetCmdLine();
					if (cmdLine.Find("com_skipIntroVideos") < 0) {
						cmdLine.Append(" +set com_skipIntroVideos 1");
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
		callbacks.Append(new idSWFScriptFunction_Restart(GDM_GAME_RESTART_REQUIRED, false));
		callbacks.Append(new idSWFScriptFunction_Restart(GDM_GAME_RESTART_REQUIRED, true));
		optionText.Append(idStrId("#str_00100113")); // Continue
		optionText.Append(idStrId("#str_02487")); // Restart Now
		common->Dialog().AddDynamicDialog(GDM_GAME_RESTART_REQUIRED, callbacks, optionText, true, idStr());
	}

	if (systemData.IsDataChanged()) {
		systemData.CommitData();
	}

	idMenuScreen::HideScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_VR_Flicksync::HandleAction
========================
*/
bool idMenuScreen_Shell_VR_Flicksync::HandleAction( idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled )
{

	if ( menuData == NULL ) {
		return true;
	}
	
	if ( menuData->ActiveScreen() != SHELL_AREA_VR_FLICKSYNC ) {
		return false;
	}

	widgetAction_t actionType = action.GetType();
	const idSWFParmList & parms = action.GetParms();

	switch ( actionType ) {
		case WIDGET_ACTION_GO_BACK: {
			if ( menuData != NULL ) {
				menuData->SetNextScreen( SHELL_AREA_CAMPAIGN, MENU_TRANSITION_SIMPLE );
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

			switch ( selectionIndex ) {
			  case idMenuDataSource_Shell_VR_Flicksync::FLICKSYNC_FIELD_NEWGAME:
				{
					// reset score to 0
					Flicksync_NewGame();
					Flicksync_skipToCutscene = CUTSCENE_NONE;
					switch (vr_flicksyncCharacter.GetInteger())
					{
						case FLICK_RECEPTION:
							Flicksync_skipToCutscene = CUTSCENE_RECEPTION;
							cmdSystem->AppendCommandText("devmap game/mars_city1\n");
							break;
						case FLICK_SARGE:
							Flicksync_skipToCutscene = CUTSCENE_SARGE;
							cmdSystem->AppendCommandText("devmap game/mars_city1\n");
							break;
						case FLICK_BROOKS:
							cmdSystem->AppendCommandText("devmap game/mc_underground\n");
							break;
						case FLICK_MARK_RYAN:
							Flicksync_skipToCutscene = ACTING_AIRLOCK;
							cmdSystem->AppendCommandText("devmap game/mc_underground\n");
							break;
						case FLICK_ISHII:
							Flicksync_skipToCutscene = CUTSCENE_ISHII;
							cmdSystem->AppendCommandText("devmap game/mc_underground\n");
							break;
						case FLICK_ROLAND:
							cmdSystem->AppendCommandText("devmap game/mars_city2\n");
							break;
						case FLICK_MCNEIL:
						case FLICK_MARINE_PDA:
						case FLICK_MARINE_TORCH:
						case FLICK_POINT:
							cmdSystem->AppendCommandText("devmap game/erebus1\n");
							break;
						case FLICK_BRAVO_LEAD:
							cmdSystem->AppendCommandText("devmap game/le_enpro1\n");
							break;
						case FLICK_NONE:
						case FLICK_DARKSTAR:
						case FLICK_TOWER:
						case FLICK_BETRUGER:
						case FLICK_SWANN:
						case FLICK_CAMPBELL:
						default:
							cmdSystem->AppendCommandText("devmap game/mars_city1\n");
							break;
					}
					break;
				}
				default: {
					systemData.AdjustField( selectionIndex , 1);
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
idMenuScreen_Shell_VR_Flicksync::idMenuDataSource_Shell_VR_Flicksync::idMenuDataSource_Shell_VR_Flicksync
========================
*/
idMenuScreen_Shell_VR_Flicksync::idMenuDataSource_Shell_VR_Flicksync::idMenuDataSource_Shell_VR_Flicksync() {
}

/*
========================
idMenuScreen_Shell_VR_Flicksync::idMenuDataSource_Shell_VR_Gameplay_Options::LoadData
========================
*/
void idMenuScreen_Shell_VR_Flicksync::idMenuDataSource_Shell_VR_Flicksync::LoadData() {

	originalFlicksyncCharacter = vr_flicksyncCharacter.GetInteger();
	originalFlicksyncCueCards = vr_flicksyncCueCards.GetInteger();
	originalCutscenesOnly = vr_cutscenesOnly.GetInteger();
}

/*
========================
idMenuScreen_Shell_VR_Flicksync::idMenuDataSource_Shell_VR_Gameplay_Options::IsRestartRequired
========================
*/
bool idMenuScreen_Shell_VR_Flicksync::idMenuDataSource_Shell_VR_Flicksync::IsRestartRequired() const {

	return false;
}

/*
========================
idMenuScreen_Shell_VR_Flicksync::idMenuDataSource_Shell_VR_Gameplay_Options::CommitData
========================
*/
void idMenuScreen_Shell_VR_Flicksync::idMenuDataSource_Shell_VR_Flicksync::CommitData() {
	cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
}

/*
========================
idMenuScreen_Shell_VR_Flicksync::idMenuDataSource_Shell_VR_Gameplay_Options::AdjustField
========================
*/
void idMenuScreen_Shell_VR_Flicksync::idMenuDataSource_Shell_VR_Flicksync::AdjustField( const int fieldIndex, const int adjustAmount ) {
	switch ( fieldIndex ) {
	case FLICKSYNC_FIELD_CHARACTER:
	{
		static const int numValues = 14;
		static const int values[numValues] = { FLICK_NONE, FLICK_BETRUGER, FLICK_SWANN, FLICK_CAMPBELL, FLICK_TOWER, FLICK_RECEPTION, FLICK_SARGE, FLICK_ISHII, FLICK_MCNEIL, FLICK_MARINE_PDA, FLICK_MARINE_TORCH, FLICK_POINT, FLICK_BRAVO_LEAD, FLICK_PLAYER };
		vr_flicksyncCharacter.SetInteger( AdjustOption( vr_flicksyncCharacter.GetInteger(), values, numValues, adjustAmount ) );
		break;

	}
	case FLICKSYNC_FIELD_CUECARDS:
	{
		static const int numValues = 6;
		static const int values[numValues] = { 0, 1, 2, 3, 4, 5 };
		vr_flicksyncCueCards.SetInteger( AdjustOption( vr_flicksyncCueCards.GetInteger(), values, numValues, adjustAmount ) );
		break;

	}
	case FLICKSYNC_FIELD_GAMETYPE:
	{
		static const int numValues = 2;
		static const int values[numValues] = { 0, 1 };
		vr_cutscenesOnly.SetInteger( AdjustOption( vr_cutscenesOnly.GetInteger(), values, numValues, adjustAmount ) );
		break;

	}

	case FLICKSYNC_FIELD_NEWGAME:
	{
		break;
	}

	}
	cvarSystem->ClearModifiedFlags( CVAR_ARCHIVE );
}

/*
========================
idMenuScreen_Shell_VR_Flicksync::idMenuDataSource_Shell_VR_Gameplay_Options::GetField
========================
*/
idSWFScriptVar idMenuScreen_Shell_VR_Flicksync::idMenuDataSource_Shell_VR_Flicksync::GetField(const int fieldIndex) const {
	switch (fieldIndex)
	{

	case FLICKSYNC_FIELD_CHARACTER:
	{
		const char* names[] = { "None", "Dr. Betruger", "Elliot Swann", "Jack Campbell", "Darkstar", "S.L. Medley (Tower)", "Reception", "Sergeant Kelly", "Brooks (Mars Sec)", "Mark Ryan (airlock)", "Ishii (missing scientist)", "Roland (ceiling)", "Dr. Elizabeth McNeil", "Marine with PDA", "Marine with Torch", "Point (explosives)", "Bravo Lead", "Player" };
		return names[vr_flicksyncCharacter.GetInteger()];
	}
	case FLICKSYNC_FIELD_CUECARDS:
	{
		const char* names[] = { "0 (default)", "1", "2", "3", "4", "5" };
		return names[vr_flicksyncCueCards.GetInteger()];
	}
	case FLICKSYNC_FIELD_GAMETYPE:
	{
		const char* names[] = { "Cutscenes + Game", "Cutscenes Only", "Game Only" };
		return names[vr_cutscenesOnly.GetInteger()];
	}

	}
	return false;
}

/*
========================
idMenuScreen_Shell_VR_Flicksync::idMenuDataSource_Shell_VR_Gameplay_Options::IsDataChanged
========================
*/
bool idMenuScreen_Shell_VR_Flicksync::idMenuDataSource_Shell_VR_Flicksync::IsDataChanged() const {

	if (originalFlicksyncCharacter != vr_flicksyncCharacter.GetInteger())
	{
		return true;
	}
	if (originalFlicksyncCueCards != vr_flicksyncCueCards.GetInteger())
	{
		return true;
	}
	if (originalCutscenesOnly != vr_cutscenesOnly.GetInteger())
	{
		return true;
	}

	return false;
}
