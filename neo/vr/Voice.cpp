#pragma hdrstop

#include"precompiled.h"

#undef strncmp
#undef vsnprintf
#undef _vsnprintf

#include "Voice.h"
#include <sapi.h>
#include "sys\win32\win_local.h"

ISpVoice * pVoice = NULL;
ISpRecognizer *pRecognizer = NULL;
ISpRecoContext *pReco = NULL;
ISpRecoGrammar *pGrammar = NULL;

extern iVoice voice;

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
}

bool in_phrase = false;

void MadeASound()
{

}

void StartedTalking()
{

}

void StoppedTalking()
{

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
				common->Printf("$ Sound start\n");
				break;
			case SPEI_SOUND_END:
				if (in_phrase)
					StoppedTalking();
				common->Printf("$ Sound end\n");
				in_phrase = false;
				break;
			case SPEI_PHRASE_START:
				in_phrase = true;
				StartedTalking();
				common->Printf("$ phrase start\n");
				break;
			case SPEI_RECOGNITION:
				common->Printf("$ Recognition\n");
				recoResult = reinterpret_cast<ISpRecoResult*>(event.lParam);
				if (recoResult)
				{
					wchar_t* text;
					hr = recoResult->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, FALSE, &text, NULL);
					voice.Say("You said %S.", text);

					CoTaskMemFree(text);
				}
				in_phrase = false;
				break;
			case SPEI_HYPOTHESIS:
				if (!in_phrase)
					StartedTalking();
				in_phrase = true;
				common->Printf("$ Hypothesis\n");
				recoResult = reinterpret_cast<ISpRecoResult*>(event.lParam);
				if (recoResult)
				{
					wchar_t* text;
					hr = recoResult->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, FALSE, &text, NULL);
					voice.Say("Maybe %S.", text);

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
				common->Printf("$ False Recognition\n");
				recoResult = reinterpret_cast<ISpRecoResult*>(event.lParam);
				if (recoResult)
				{
					wchar_t* text;
					hr = recoResult->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, FALSE, &text, NULL);
					voice.Say("You did not say that.", text);
					CoTaskMemFree(text);
				}
				in_phrase = false;
				break;
			case SPEI_INTERFERENCE:
				common->Printf("$ Interference\n");
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
				common->Printf("$ Start SR Stream\n");
				break;
			case SPEI_RECO_OTHER_CONTEXT:
				common->Printf("$ Reco Other Context\n");
				in_phrase = false;
				break;
			case SPEI_SR_AUDIO_LEVEL:
				common->Printf("$ SR Audio Level\n");
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


/*
==============
iVoice::VoiceInit
==============
*/

void iVoice::VoiceInit(void)
{
	// CoInitialize(NULL) is already called by  Sys_Init(), just make sure we call this after Sys_Init() 
	HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
	if (SUCCEEDED(hr))
	{
		common->Printf("\nISpVoice succeeded.\n");
		hr = pVoice->Speak(L"Hello world", 0, NULL);
	}
	else
	{
		common->Printf("\nISpVoice failed.\n");
		pVoice = NULL;
	}
	hr = CoCreateInstance(CLSID_SpSharedRecognizer, NULL, CLSCTX_ALL, IID_ISpRecognizer, (void **)&pRecognizer);
	if (SUCCEEDED(hr))
	{
		Say("Recognizer created.");
		//pRecognizer->SetInput();
		hr = pRecognizer->CreateRecoContext(&pReco);
		if (SUCCEEDED(hr))
		{
			Say("Context created.");
			hr = pReco->Pause(0);
			hr = pReco->CreateGrammar(1, &pGrammar);
			if (SUCCEEDED(hr))
			{
				Say("Grammar created.");
				SPSTATEHANDLE rule;
				pGrammar->GetRule(L"word", 0, SPRAF_TopLevel | SPRAF_Active, true, &rule);
				pGrammar->AddWordTransition(rule, NULL, L"hello", L" ", SPWT_LEXICAL, 1.0f, NULL);
				pGrammar->AddWordTransition(rule, NULL, L"goodbye", L" ", SPWT_LEXICAL, 1.0f, NULL);
				hr = pGrammar->Commit(NULL);
				if (SUCCEEDED(hr))
					Say("Compiled.");

				//hr = pReco->SetNotifyWindowMessage(win32.hWnd, WM_USER, 0, 0);
				hr = pReco->SetNotifyCallbackFunction(SpeechCallback, 0, 0);
				if (SUCCEEDED(hr))
					Say("Callback created.");
				ULONGLONG interest;
				interest = SPFEI_ALL_SR_EVENTS;
				hr = pReco->SetInterest(interest, interest);
				if (SUCCEEDED(hr))
					Say("Interested.");
				hr = pGrammar->SetRuleState(L"word", NULL, SPRS_ACTIVE);
				if (SUCCEEDED(hr))
					Say("Listening.");
				hr = pReco->Resume(0);
				if (SUCCEEDED(hr))
					Say("Resumed.");
			}
		}
	}
	else
	{
		Say("Recognizer failed.");
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
