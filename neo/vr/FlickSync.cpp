#pragma hdrstop

#include"precompiled.h"

#include "FlickSync.h"
#include "Voice.h"

idCVar vr_flickCharacter( "vr_flickCharacter", "0", CVAR_INTEGER | CVAR_ARCHIVE, "FlickSync character. 0 = none, 1 = Betruger, 2 = Swan, 3 = Campbell, 4 = DarkStar, 5 = Tower, 6 = Reception, 7 = Kelly, 8 = McNeil", 0, 8 );

int EntityToCharacter( const char* entity )
{
	if ( idStr::Cmp(entity, "marscity_cinematic_betruger_1") == 0 || idStr::Cmp(entity, "marscity_cinematic_betruger_speech_head") == 0 )
		return FLICK_BETRUGER;
	else if ( idStr::Cmp(entity, "marscity_cinematic_swann_1") == 0 || idStr::Cmp(entity, "marscity_cinematic_swann_speech") == 0 )
		return FLICK_SWAN;
	else if ( idStr::Cmp(entity, "marscity_cinematic_campbell_1") == 0 )
		return FLICK_CAMPBELL;
	else if ( idStr::Cmp(entity, "marscity_sec_window_1") == 0 )
		return FLICK_TOWER;
	else if ( idStr::Cmp(entity, "marscity_receptionist_full_head") == 0 )
		return FLICK_RECEPTION;
	else if ( idStr::Cmp(entity, "marscity_cinematic_sarge_1") == 0 || idStr::Cmp(entity, "sarge_secondary") == 0 )
		return FLICK_KELLY;
	else
		return FLICK_NONE;
}

// return true if the game is allowed to play this line, or false if the user is going to say it.
bool FlickSync_Voice( const char* entity, const char* animation, const char* lineName )
{
	int character = EntityToCharacter( entity );
	return character != vr_flickCharacter.GetInteger();
}
