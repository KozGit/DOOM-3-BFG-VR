#pragma hdrstop

#include"precompiled.h"
#include "d3xp/Game_local.h"

#undef strncmp
#undef vsnprintf
#undef _vsnprintf
#undef StrCmpN
#undef StrCmpNI
#undef StrCmpI

#include "Voice.h"
#include <sapi.h>
#include <sphelper.h>
#include "sys\win32\win_local.h"

ISpVoice * pVoice = NULL;
ISpRecognizer *pRecognizer = NULL;
ISpObjectToken *pObjectToken = NULL;
ISpRecoContext *pReco = NULL;
ISpRecoGrammar *pGrammar = NULL;
SPSTATEHANDLE rule = NULL;
SPSTATEHANDLE flicksyncRule = NULL;

extern iVoice voice;

vr_voiceAction_t voiceActionStrings[J_SAY_NUM] =
{
	{ "LIST", J_SAY_LIST },
	{ "LISTENSTART", J_SAY_LISTENSTART },
	{ "LISTENSTOP", J_SAY_LISTENSTOP },
	{ "PAUSE", J_SAY_PAUSE },
	{ "RESUME", J_SAY_RESUME },
	{ "EXIT", J_SAY_EXIT },
	{ "TALK", J_TALK },
	{ "MENU", J_SAY_MENU },
	{ "CANCEL", J_SAY_CANCEL },
	{ "RESETVIEW", J_SAY_RESET_VIEW },
	{ "RELOAD", J_SAY_RELOAD },
	{ "PDA", J_SAY_PDA },
	{ "FIST", J_SAY_FIST },
	{ "CHAINSAW", J_SAY_CHAINSAW },
	{ "FLASHLIGHT", J_SAY_FLASHLIGHT },
	{ "GRABBER", J_SAY_GRABBER },
	{ "PISTOL", J_SAY_PISTOL },
	{ "SHOTGUN", J_SAY_SHOTGUN },
	{ "SUPERSHOTGUN", J_SAY_SUPER_SHOTGUN },
	{ "MACHINEGUN", J_SAY_MACHINE_GUN },
	{ "CHAINGUN", J_SAY_CHAIN_GUN },
	{ "ROCKETLAUNCHER", J_SAY_ROCKET_LAUNCHER },
	{ "GRENADE", J_SAY_GRENADES },
	{ "PLASMAGUN", J_SAY_PLASMA_GUN },
	{ "BFG", J_SAY_BFG },
	{ "SOULCUBE", J_SAY_SOUL_CUBE },
	{ "ARTIFACT", J_SAY_ARTIFACT },
};
	
idList<idStr> voiceCommandStrings;
idList<int> voiceCommandActions;

	
const char* words[] = {
	"what can I say",
	"consecution", "start listening",
	"consentient", "stop listening",
	"pause game", "computer, freeze program",
	"resume game", "unpause game", "computer, resume program", "computer, play program", "computer, run program", "computer, continue program",
	"exit game", "computer, end program", "computer, exit", "computer, program complete", "computer, cancel program",
	"hey", "talk", "hello", "goodbye", "greets", "hey dickwad",
	"menu",
	"cancel",
	"reload",
	"PDA", "personal data assistant",
	"fist", "fists", "hands",
	"chainsaw", "beaver tooth", "beaver tooth chainsaw", "mixom beaver tooth", "mixom beaver tooth chainsaw",
	"flashlight", "torch",
	"grabber", "ionized plasma levitator", "IPL unit", "gravity gun",
	"pistol",
	"shotgun", "pump action shotgun", "single barreled shotgun",
	"super shotgun", "double barreled shotgun", "combat shotgun",
	"machine gun", "enforcer", "M G 88", "M G 88 enforcer", "M G",
	"chain gun", "mach 2 chain gun", "UAC Weapons division mach 2 chain gun", "saw", "mini gun", "gatling gun",
	"rocket launcher",
	"grenades", "grenade",
	"plasma gun", "plasma rifle",
	"BFG 9000", "BFG", "bio force gun", "big fucking gun", "big fragging gun", "big freaking gun",
	"soul cube",
	"the artifact", "artifact", "heart of hell", "blood stone",
};


void __stdcall SpeechCallback(WPARAM wParam, LPARAM lParam)
{
	commonVoice->Event(wParam, lParam);
}



/*
==============
iVoice::iVoice()
==============
*/
iVoice::iVoice()
{
	currentVolume = 0.0f;
}

static bool in_phrase = false, spoke = false, listening = true;
static bool heard[J_SAY_MAX - J_SAY_MIN + 1] = {};

void MadeASound()
{

}

void StartedTalking()
{
	idPlayer* player = gameLocal.GetLocalPlayer();
	if( player != NULL && player->hudManager && (vr_flicksyncCharacter.GetInteger() || vr_voiceCommands.GetInteger() || vr_talkMode.GetInteger()) )
	{
		player->hudManager->SetRadioMessage( true );
	}
}

void StoppedTalking()
{
	spoke = true;

	idPlayer* player = gameLocal.GetLocalPlayer();
	if( player != NULL && player->hudManager && (vr_flicksyncCharacter.GetInteger() || vr_voiceCommands.GetInteger() || vr_talkMode.GetInteger()) )
	{
		player->hudManager->SetRadioMessage( false );
	}
	Flicksync_StoppedTalking();
}

bool iVoice::GetTalkButton()
{
	return in_phrase;
#if 0
	static int count = 0;
	if (spoke)
	{
		count = 20;
		spoke = false;
	}
	if (count > 0)
	{
		--count;
		return true;
	}
	return false;
#endif
}

idStr buildCmdString( int actionNum )
{
	idStr cmdStr;

	for ( int i = 0; i < voiceCommandActions.Num(); i++ )
	{
		if ( voiceCommandActions[i] == actionNum )
		{
			cmdStr += ", ," + voiceCommandStrings[i];
		}
	}
	return cmdStr;

}

bool iVoice::GetSayButton(int j)
{
	bool result = heard[j - J_SAY_MIN];
	heard[j - J_SAY_MIN] = false;
	return result;
}

void iVoice::ListVoiceCmds_f( const idCmdArgs& args )
{
	common->Printf( "Listing available voice commands\n" );
	
	for ( int i = 0; i < J_SAY_NUM; i++ )
	{
		common->Printf( "\nAction:  %s\n", voiceActionStrings[i].string.c_str() );

		int count = 0;
		for ( int j = 0; j < voiceCommandActions.Num(); j++ )
		{
			if ( voiceCommandActions[j] == voiceActionStrings[i].action )
			{
				count++;
				common->Printf( "    Phrase %d: %s\n", count, voiceCommandStrings[j].c_str() );
			}
		}
		if ( count == 0 ) common->Printf( "    No phrase defined\n" );

	}
}


void iVoice::HearWord(const char *w, int confidence)
{

	int index = voiceCommandStrings.FindIndex( w );
	if ( index == -1 ) return; // not found;
	
	int action = voiceCommandActions[index];
		
	if (action == J_SAY_LIST ) 
	{		
		common->Printf( "J_SAY_LIST parsed" );
		Speed( 4 );
		if ( vr_talkMode.GetInteger() > 0 )
			Say( npc.c_str() );
			//Say( "You can say anything to NPC's." );

		if ( !listening )
		{
			Say( available.c_str() );// +", " + buildCmdString( J_SAY_LIST ) + buildCmdString( J_SAY_LISTENSTART );
			Say( buildCmdString( J_SAY_LIST ).c_str() );
			Say( buildCmdString( J_SAY_LISTENSTART ).c_str() );
			//Say( "You can say: what can I say, or start listening." );
		}
		else
		{
			if ( vr_voiceCommands.GetInteger() > 2 )
				Say( cmds2.c_str() );
				//Say( "You can use voice commands, weapon names, or holodeck commands." );
			else if ( vr_voiceCommands.GetInteger() == 1 )
				Say( cmds1.c_str() );
				//Say( "You can use voice commands or holodeck commands." );
			Speed( 6 );
						
			Say( available.c_str() );// +", " + buildCmdString( J_SAY_LIST ) + buildCmdString( J_SAY_LISTENSTART );
			Say( buildCmdString( J_SAY_LIST ).c_str() );
			Say( buildCmdString( J_SAY_LISTENSTOP ).c_str() );
			Say( buildCmdString( J_SAY_LISTENSTART ).c_str() );
			//Say( "You can say: What can I say, stop listening, start listening." );
			
			Speed( 7 );
			if ( vr_voiceCommands.GetInteger() >= 1 )
			{
				Say( buildCmdString( J_SAY_PAUSE ) );
				Say( buildCmdString( J_SAY_RESUME ) );
				Say( buildCmdString( J_SAY_EXIT ) );
				Say( buildCmdString( J_SAY_MENU ) );
				Say( buildCmdString( J_SAY_RESET_VIEW ) );
				Say( buildCmdString( J_SAY_CANCEL ) );
				Say( buildCmdString( J_SAY_PDA ) );
				//Say( "pause game, resume game, exit game, menu, cancel, PDA." );
			}
					
			if ( vr_voiceCommands.GetInteger() >= 2 )
			{
				Say( buildCmdString( J_SAY_RELOAD ) );
				Say( buildCmdString( J_SAY_FLASHLIGHT ) );
				Say( buildCmdString( J_SAY_FIST ) );
				Say( buildCmdString( J_SAY_CHAINSAW ) );
				Say( buildCmdString( J_SAY_GRABBER ) );
				Say( buildCmdString( J_SAY_PISTOL ) );
				Say( buildCmdString( J_SAY_SHOTGUN ) );
				Say( buildCmdString( J_SAY_SUPER_SHOTGUN ) );
				Say( buildCmdString( J_SAY_MACHINE_GUN ) );
				Say( buildCmdString( J_SAY_CHAIN_GUN ) );
				Say( buildCmdString( J_SAY_ROCKET_LAUNCHER ) );
				Say( buildCmdString( J_SAY_GRENADES ) );
				Say( buildCmdString( J_SAY_PLASMA_GUN ) );
				Say( buildCmdString( J_SAY_BFG ) );
				Say( buildCmdString( J_SAY_SOUL_CUBE ) );
				Say( buildCmdString( J_SAY_ARTIFACT ) );
										
				//Say( "reload, flashlight, fists, chainsaw, grabber, pistol, shotgun, super shotgun, machine gun, chain gun, rocket launcher, grenades, plasma gun, BFG 9000, soul cube, the artifact." );
			}
					
		}
	}

	else if ( action == J_SAY_LISTENSTART )

	{
		if ( !listening )
		{
			listening = true;
			Speed( 5 );
			Say( startListen.c_str() );
			//Say( "Started listening." );
		}
	}

	else if ( listening && confidence >= SP_NORMAL_CONFIDENCE )
	{
		int heardIndex = voiceCommandActions.FindIndex( action );

		if ( action == J_SAY_LISTENSTOP )
		{
			listening = false;
			Speed( 5 );
			Say( stopListen.c_str() );
			//Say( "Stopped listening." );
		}
		else
		{
			heard[action - J_SAY_MIN] = true;
		}

	}

	/*
#define ifw(s) if (!strcmp(w, s))
#define ifw2(s1, s2) if (!strcmp(w, s1) || !strcmp(w, s2))
#define ifw3(s1, s2, s3) if (!strcmp(w, s1) || !strcmp(w, s2) || !strcmp(w, s3))
#define ifw4(s1, s2, s3, s4) if (!strcmp(w, s1) || !strcmp(w, s2) || !strcmp(w, s3) || !strcmp(w, s4))
#define ifw5(s1, s2, s3, s4, s5) if (!strcmp(w, s1) || !strcmp(w, s2) || !strcmp(w, s3) || !strcmp(w, s4) || !strcmp(w, s5))
#define ifw6(s1, s2, s3, s4, s5, s6) if (!strcmp(w, s1) || !strcmp(w, s2) || !strcmp(w, s3) || !strcmp(w, s4) || !strcmp(w, s5) || !strcmp(w, s6))
#define elw(s) else ifw(s)
#define elw2(s1, s2) else ifw2(s1, s2)
#define elw3(s1, s2, s3) else ifw3(s1, s2, s3)
#define elw4(s1, s2, s3, s4) else ifw4(s1, s2, s3, s4)
#define elw5(s1, s2, s3, s4, s5) else ifw5(s1, s2, s3, s4, s5)
#define elw6(s1, s2, s3, s4, s5, s6) else ifw6(s1, s2, s3, s4, s5, s6)

	ifw("what can I say") {
		Speed(4);
		if (vr_talkMode.GetInteger() > 0)
			Say("You can say anything to NPC's.");
		if (!listening)
		{
			Say("You can say: what can I say, or start listening.");
		}
		else
		{
			if (vr_voiceCommands.GetInteger() > 2)
				Say("You can use voice commands, weapon names, or holodeck commands.");
			else if (vr_voiceCommands.GetInteger() == 1)
				Say("You can use voice commands or holodeck commands.");
			Speed(6);
			Say("You can say: What can I say, stop listening, start listening.");
			Speed(7);
			if (vr_voiceCommands.GetInteger() >= 1)
				Say("pause game, resume game, exit game, menu, cancel, PDA.");
			if (vr_voiceCommands.GetInteger() >= 2)
				Say("reload, flashlight, fists, chainsaw, grabber, pistol, shotgun, super shotgun, machine gun, chain gun, rocket launcher, grenades, plasma gun, BFG 9000, soul cube, the artifact.");
		}
	}
	else ifw2("consecution", "start listening") {
		if (!listening)
		{
			listening = true;
			Speed(5);
			Say("Started listening.");
		}
	}
	else if (listening && confidence >= SP_NORMAL_CONFIDENCE)
	{
		ifw2("consentient", "stop listening") {
			listening = false;
			Speed(5);
			Say("Stopped listening.");
		}
		elw2("pause game", "computer, freeze program") {
			heard[J_SAY_PAUSE - J_SAY_MIN] = true;
			//Say("Paused.");
		}
		elw6("resume game", "unpause game", "computer, resume program", "computer, play program", "computer, run program", "computer, continue program") {
			heard[J_SAY_RESUME - J_SAY_MIN] = true;
			//Say("Unpaused.");
		}
		elw5("exit game", "computer, end program", "computer, exit", "computer, program complete", "computer, cancel program") {
			heard[J_SAY_EXIT - J_SAY_MIN] = true;
			//Say("Exiting.");
		}
		elw6("hey", "talk", "hello", "goodbye", "greets", "hey dickwad") {
			//Say("Greets.");
		}
		elw2("menu", "main menu") {
			heard[J_SAY_MENU - J_SAY_MIN] = true;
			//Say("Menu.");
		}
		elw("cancel") {
			heard[J_SAY_CANCEL - J_SAY_MIN] = true;
			//Say("Cancelling.");
		}
		elw("reload") {
			heard[J_SAY_RELOAD - J_SAY_MIN] = true;
			//Say("Reloading.");
		}
		elw2("PDA", "personal data assistant") {
			heard[J_SAY_PDA - J_SAY_MIN] = true;
			//Say("PDA.");
		}
		elw3("fist", "fists", "hands") {
			heard[J_SAY_FIST - J_SAY_MIN] = true;
			//Say("fist.");
		}
		elw5("chainsaw", "beaver tooth", "beaver tooth chainsaw", "mixom beaver tooth", "mixom beaver tooth chainsaw") {
			heard[J_SAY_CHAINSAW - J_SAY_MIN] = true;
			//Say("chainsaw.");
		}
		elw2("flashlight", "torch") {
			heard[J_SAY_FLASHLIGHT - J_SAY_MIN] = true;
			//Say("flashlight.");
		}
		elw4("grabber", "ionized plasma levitator", "IPL unit", "gravity gun") {
			heard[J_SAY_GRABBER - J_SAY_MIN] = true;
			//Say("grabber.");
		}
		elw("pistol") {
			heard[J_SAY_PISTOL - J_SAY_MIN] = true;
			//Say("Pistol.");
		}
		elw3("shotgun", "pump action shotgun", "single barreled shotgun") {
			heard[J_SAY_SHOTGUN - J_SAY_MIN] = true;
			//Say("Shotgun.");
		}
		elw3("super shotgun", "double barreled shotgun", "combat shotgun") {
			heard[J_SAY_SUPER_SHOTGUN - J_SAY_MIN] = true;
			//Say("Super shotgun.");
		}
		elw5("machine gun", "enforcer", "M G 88", "M G 88 enforcer", "M G") {
			heard[J_SAY_MACHINE_GUN - J_SAY_MIN] = true;
			//Say("Machine gun.");
		}
		elw6("chain gun", "mach 2 chain gun", "UAC Weapons division mach 2 chain gun", "saw", "mini gun", "gatling gun") {
			heard[J_SAY_CHAIN_GUN - J_SAY_MIN] = true;
			//Say("Chain gun.");
		}
		elw("rocket launcher") {
			heard[J_SAY_ROCKET_LAUNCHER - J_SAY_MIN] = true;
			//Say("rocket launcher.");
		}
		elw2("grenades", "grenade") {
			heard[J_SAY_GRENADES - J_SAY_MIN] = true;
			//Say("Grenades.");
		}
		elw2("plasma gun", "plasma rifle") {
			heard[J_SAY_PLASMA_GUN - J_SAY_MIN] = true;
			//Say("Plasma gun.");
		}
		elw6("BFG 9000", "BFG", "bio force gun", "big fucking gun", "big fragging gun", "big freaking gun") {
			heard[J_SAY_BFG - J_SAY_MIN] = true;
			//Say("BFG 9000.");
		}
		elw("soul cube") {
			heard[J_SAY_SOUL_CUBE - J_SAY_MIN] = true;
			//Say("soul cube.");
		}
		elw4("the artifact", "artifact", "heart of hell", "blood stone") {
			heard[J_SAY_ARTIFACT - J_SAY_MIN] = true;
			//Say("the artifact.");
		}
		else
		{
			//Say("Hear Word is missing %s", w);
		}
	}

#undef ifw
*/
}

void iVoice::HearWord(const wchar_t *w, int confidence)
{
	char buffer[1024];
	WideCharToMultiByte(CP_ACP, 0, w, -1, buffer, sizeof(buffer) / sizeof(buffer[0]),"'", NULL);
	HearWord(buffer, confidence);
}

void iVoice::Event(WPARAM wParam, LPARAM lParam)
{
	SPEVENT event;
	HRESULT hr;
	while (pReco->GetEvents(1, &event, NULL) == S_OK)
	{
		ISpRecoResult* recoResult;
		switch (event.eEventId)
		{
			case SPEI_END_SR_STREAM:
				in_phrase = false;
				common->Printf("$ End SR Stream\n");
				break;
			case SPEI_SOUND_START:
				in_phrase = false;
				MadeASound();
				//common->Printf("$ Sound start\n");
				break;
			case SPEI_SOUND_END:
				if (in_phrase)
					StoppedTalking();
				//common->Printf("$ Sound end\n");
				in_phrase = false;
				currentVolume = 0;
				break;
			case SPEI_PHRASE_START:
				in_phrase = true;
				StartedTalking();
				//common->Printf("$ phrase start\n");
				break;
			case SPEI_RECOGNITION:
				//common->Printf("$ Recognition\n");
				recoResult = reinterpret_cast<ISpRecoResult*>(event.lParam);
				if (recoResult)
				{
					ULONG isLine = false;
					ULONGLONG startTime = 0;
					ULONG length = 0;
					wchar_t* text;
					SPPHRASE* pPhrase = NULL;
					hr = recoResult->GetPhrase(&pPhrase);
					int confidence = 0;
					if SUCCEEDED(hr) {
						confidence = pPhrase->Rule.Confidence; // -1, 0, or 1
						isLine = pPhrase->Rule.ulId;
						startTime = pPhrase->ftStartTime;
						length = pPhrase->ulAudioSizeTime;
					}
					const char* confidences[3] = { "low", "medium", "high" };
					hr = recoResult->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, FALSE, &text, NULL);
					//voice.Say("%s: You said %S.", confidences[confidence + 1], text);
					if (isLine)
					{
						if ( vr_flicksyncCharacter.GetInteger() )
						{
							char buffer[1024];
							WideCharToMultiByte(CP_ACP, 0, text, -1, buffer, sizeof(buffer) / sizeof(buffer[0]), "'", NULL);
							Flicksync_HearLine(buffer, confidence, startTime, length);
						}
					}
					else
					{
						HearWord(text, confidence);
						Flicksync_StoppedTalking();
					}
					CoTaskMemFree(text);
					CoTaskMemFree(pPhrase);
				}
				in_phrase = false;
				break;
			case SPEI_HYPOTHESIS:
				if (!in_phrase)
					StartedTalking();
				in_phrase = true;
				//common->Printf("$ Hypothesis\n");
				recoResult = reinterpret_cast<ISpRecoResult*>(event.lParam);
				if (recoResult)
				{
					ULONG isLine = false;
					
					wchar_t* text;
					SPPHRASE* pPhrase = NULL;
					hr = recoResult->GetPhrase(&pPhrase);
					int confidence = 0;
					if SUCCEEDED(hr) {
						confidence = pPhrase->Rule.Confidence; // -1, 0, or 1
						isLine = pPhrase->Rule.ulId;
					}
					const char* confidences[3] = { "low", "medium", "high" };
					hr = recoResult->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, FALSE, &text, NULL);
					if (isLine)
					{
						//voice.Say("Maybe %s: %S", confidences[confidence + 1], text);
					}
					else
					{
						//voice.Say("Maybe %S.", text);
					}
					CoTaskMemFree(text);
				}
				break;
			case SPEI_SR_BOOKMARK:
				common->Printf("$ Bookmark\n");
				break;
			case SPEI_PROPERTY_NUM_CHANGE:
				common->Printf("$ Num change\n");
				break;
			case SPEI_PROPERTY_STRING_CHANGE:
				common->Printf("$ property string change\n");
				break;
			case SPEI_FALSE_RECOGNITION:
				//common->Printf("$ False Recognition\n");
				recoResult = reinterpret_cast<ISpRecoResult*>(event.lParam);
				if (recoResult)
				{
					wchar_t* text;
					hr = recoResult->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, FALSE, &text, NULL);
					//voice.Say("You did not say that.");
					CoTaskMemFree(text);
				}
				in_phrase = false;
				break;
			case SPEI_INTERFERENCE:
				//common->Printf("$ Interference\n");
				break;
			case SPEI_REQUEST_UI:
				common->Printf("$ Request UI\n");
				break;
			case SPEI_RECO_STATE_CHANGE:
				in_phrase = false;
				common->Printf("$ Reco State Change\n");
				break;
			case SPEI_ADAPTATION:
				common->Printf("$ Adaptation\n");
				break;
			case SPEI_START_SR_STREAM:
				in_phrase = false;
				currentVolume = 0;
				common->Printf("$ Start SR Stream\n");
				break;
			case SPEI_RECO_OTHER_CONTEXT:
				//common->Printf("$ Reco Other Context\n");
				in_phrase = false;
				break;
			case SPEI_SR_AUDIO_LEVEL:
				currentVolume = event.wParam / 100.0f;
				//common->Printf("$ SR Audio Level: %3d / 100\n", event.wParam);
				break;
			case SPEI_SR_RETAINEDAUDIO:
				common->Printf("$ SR Retained Audio\n");
				break;
			case SPEI_SR_PRIVATE:
				common->Printf("$ SR Private\n");
				break;
			case SPEI_ACTIVE_CATEGORY_CHANGED:
				common->Printf("$ Active Category changed\n");
				break;
		}
	}
}

void iVoice::AddWord(const char* word)
{
	wchar_t wbuffer[1024];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, word, -1, wbuffer, sizeof(wbuffer) / sizeof(wbuffer[0]));
	pGrammar->AddWordTransition(rule, NULL, wbuffer, L" ", SPWT_LEXICAL, 1.0f, NULL);
}

void iVoice::AddWord(const wchar_t* word)
{
	pGrammar->AddWordTransition(rule, NULL, word, L" ", SPWT_LEXICAL, 1.0f, NULL);
}

void iVoice::AddFlicksyncLine(const char* line)
{
	wchar_t wbuffer[1024];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, line, -1, wbuffer, sizeof(wbuffer) / sizeof(wbuffer[0]));
	pGrammar->AddWordTransition(flicksyncRule, NULL, wbuffer, L" ", SPWT_LEXICAL, 1.0f, NULL);
}

void iVoice::AddFlicksyncLine(const wchar_t* line)
{
	pGrammar->AddWordTransition(flicksyncRule, NULL, line, L" ", SPWT_LEXICAL, 1.0f, NULL);
}


/*
==============
iVoice::InitVoiceDictionary
==============
*/

bool iVoice::InitVoiceDictionary( void )
{
	idLexer		parser( LEXFL_NOSTRINGESCAPECHARS | LEXFL_NOSTRINGCONCAT  );
	idToken		token;
	idStr		filename;

	int maxActions = J_SAY_NUM;
	int numActions;
	int numEntries;
	int currentAction;
	
	voiceCommandStrings.Clear();
	voiceCommandActions.Clear();

	filename = "/dict/voice";
	filename.SetFileExtension( "dict" );
	if ( !parser.LoadFile( filename ) )
	{
		gameLocal.Error( "Error initializing voice commands.\nUnable to load '%s'",filename.c_str());
	}

	parser.ExpectTokenString( "numActions" );
	numActions = parser.ParseInt();

	if ( numActions > maxActions )
	{
		parser.Error( "Invalid number of actions: %d\nMax defined actions = %d\n", numActions,maxActions );
	}
	
	parser.ExpectTokenString( "available" );
	parser.ReadToken( &token );
	available = token;

	parser.ExpectTokenString( "npc" );
	parser.ReadToken( &token );
	npc = token;

	parser.ExpectTokenString( "cmds1" );
	parser.ReadToken( &token );
	cmds1 = token;

	parser.ExpectTokenString( "cmds2" );
	parser.ReadToken( &token );
	cmds2 = token;

	parser.ExpectTokenString( "start" );
	parser.ReadToken( &token );
	startListen = token;

	parser.ExpectTokenString( "stop" );
	parser.ReadToken( &token );
	stopListen = token;
	
	for ( int i = 0; i < numActions; i++ )
	{
		parser.ExpectTokenString( "action" );
		parser.ExpectTokenString( "{" );
		parser.ExpectTokenString( "name" );
		parser.ReadToken( &token );

		//common->Printf( "Read token %s \n", token.c_str() );
		
		for ( int j = 0; j < maxActions; j++ )
		{
			if ( !token.Cmp( voiceActionStrings[j].string ) )
			{
				currentAction = voiceActionStrings[j].action;
				//common->Printf( "Token identified as action %d \n", currentAction );
				break;
			}
		}
				
		parser.ExpectTokenString( "entries" );
		numEntries = parser.ParseInt();

		for ( int k = 0; k < numEntries; k++ )
		{
			parser.ReadToken( &token );
			//common->Printf( "Current Action %d Entry %d : %s\n",currentAction, k + 1, token.c_str() );
						
			voiceCommandStrings.Append( token );
			voiceCommandActions.Append( currentAction );
			
		}
		parser.ExpectTokenString( "}" );
	}
	
	/*
	for ( int i = 0; i < voiceCommandStrings.Num(); i++ )
	{
		common->Printf( "Voice command %d i: %s %d\n", i + 1, voiceCommandStrings[i].c_str(), voiceCommandActions[i] );
	}
	
	common->Printf("PDA looked up command action = %d\n", voiceCommandActions[voiceCommandStrings.FindIndex( "PDA" )]);
	*/

	cmdSystem->AddCommand( "vr_listVoiceCommands", ListVoiceCmds_f, CMD_FL_SYSTEM, "lists voice activated commands" );

	return true;
}


/*
==============
iVoice::VoiceInit
==============
*/

void iVoice::VoiceInit(void)
{
	
	if ( !InitVoiceDictionary() )
	{
		pVoice = NULL;
		return;

	}
		
	
	// CoInitialize(NULL) is already called by  Sys_Init(), just make sure we call this after Sys_Init() 
	HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
	if (SUCCEEDED(hr))
	{
		Speed(7);
		common->Printf("\nISpVoice succeeded.\n");
		//hr = pVoice->Speak(L"Hello world", 0, NULL);
	}
	else
	{
		common->Printf("\nISpVoice failed.\n");
		pVoice = NULL;
	}
	hr = CoCreateInstance(CLSID_SpInprocRecognizer, NULL, CLSCTX_ALL, IID_ISpRecognizer, (void **)&pRecognizer);
	if (SUCCEEDED(hr))
	{
		//Say("Recognizer created.");
		// Get the default audio input token.
		hr = SpGetDefaultTokenFromCategoryId(SPCAT_AUDIOIN, &pObjectToken);

		if (SUCCEEDED(hr))
		{
			// Set the audio input to our token.
			hr = pRecognizer->SetInput(pObjectToken, TRUE);
		}

		hr = pRecognizer->CreateRecoContext(&pReco);
		if (SUCCEEDED(hr))
		{
			//Say("Context created.");
			hr = pReco->Pause(0);
			hr = pReco->CreateGrammar(1, &pGrammar);
			if (SUCCEEDED(hr))
			{
				//Say("Grammar created.");
				pGrammar->GetRule(L"word", 0, SPRAF_TopLevel | SPRAF_Active, true, &rule);
				
				
				//koz begin
				
				/*
				for (int i = 0; i < sizeof(words) / sizeof(words[0]); ++i)
				{
					AddWord(words[i]);
				}
				*/

				for ( int i = 0; i < voiceCommandStrings.Num(); i++ )
				{
					AddWord( voiceCommandStrings[i].c_str() );
				}
				// koz end

				pGrammar->GetRule(L"line", 1, SPRAF_TopLevel | SPRAF_Active, true, &flicksyncRule);
				Flicksync_AddVoiceLines();

				hr = pGrammar->Commit(NULL);
				//if (SUCCEEDED(hr))
				//	Say("Compiled.");

				//hr = pReco->SetNotifyWindowMessage(win32.hWnd, WM_USER, 0, 0);
				hr = pReco->SetNotifyCallbackFunction(SpeechCallback, 0, 0);
				//if (SUCCEEDED(hr))
				//	Say("Callback created.");
				ULONGLONG interest;
				interest = SPFEI_ALL_SR_EVENTS;
				hr = pReco->SetInterest(interest, interest);
				//if (SUCCEEDED(hr))
				//	Say("Interested.");
				hr = pGrammar->SetRuleState(L"word", NULL, SPRS_ACTIVE);
				hr = pGrammar->SetRuleState(L"line", NULL, SPRS_ACTIVE);
				//if (SUCCEEDED(hr))
				//	Say("Listening.");
				hr = pReco->Resume(0);
				//if (SUCCEEDED(hr))
				//	Say("Resumed.");
			}
		}
	}
	else
	{
		//Say("Recognizer failed.");
	}
}


/*
==============
iVoice::VoiceShutdown
==============
*/

void iVoice::VoiceShutdown(void)
{
	if (pVoice)
	{
		pVoice->Release();
		pVoice = NULL;
	}
}

// speed must be -10 to 10
void iVoice::Speed(int talkingSpeed)
{
	pVoice->SetRate(talkingSpeed);
}

void iVoice::Say(VERIFY_FORMAT_STRING const char* fmt, ...)
{
	va_list argptr;
	va_start(argptr, fmt);
	common->Printf("$ ");
	common->VPrintf(fmt, argptr);
	common->Printf("\n");
	char buffer[1024];
	wchar_t wbuffer[1024];
	vsprintf_s(buffer, 1023, fmt, argptr);
	for (int i = 0; i < sizeof(buffer); ++i)
		wbuffer[i] = buffer[i];
	pVoice->Speak(wbuffer, SPF_ASYNC, NULL);
	va_end(argptr);
}
