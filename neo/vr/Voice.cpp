#pragma hdrstop

#include"precompiled.h"

#undef strncmp
#undef vsnprintf		
#undef _vsnprintf		

#include "Voice.h"
#include <sapi.h>

/*
==============
iVoice::iVoice()
==============
*/
iVoice::iVoice()
{
}


/*
==============
iVoice::VoiceInit
==============
*/

void iVoice::VoiceInit(void)
{
	// CoInitialize(NULL) is already called by  Sys_Init(), just make sure we call this after Sys_Init() 
	ISpVoice * pVoice = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
	if (SUCCEEDED(hr))
	{
		common->Printf("\nISpVoice succeeded.\n");
		hr = pVoice->Speak(L"Hello world", 0, NULL);
		pVoice->Release();
		pVoice = NULL;
	}
	else
	{
		common->Printf("\nISpVoice failed.\n");
	}
}


/*
==============
iVoice::VoiceShutdown
==============
*/

void iVoice::VoiceShutdown(void)
{
}

