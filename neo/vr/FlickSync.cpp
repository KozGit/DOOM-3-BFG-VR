#pragma hdrstop

#include"precompiled.h"

#include "FlickSync.h"
#include "Voice.h"

idCVar vr_flickCharacter( "vr_flickCharacter", "0", CVAR_INTEGER | CVAR_ARCHIVE, "FlickSync character. 0 = none, 1 = Betruger, 2 = Swan, 3 = Campbell, 4 = DarkStar, 5 = Tower, 6 = Reception, 7 = Kelly, 8 = Brooks, 9 = Mark Ryan, 10 = Ishii, 11 = Roland, 12 = McNeil, 13 = Marine w PDA, 14 = Marine w Torch, 15 = Point", 0, 15 );

typedef struct
{
	int character;
	const char* entity;
} character_map_t;

typedef struct
{
	const char* shader;
	const char* text;
} spoken_line_t;

static const character_map_t entityArray[] = {
// Mars City Intro
	{ FLICK_TOWER, "marscity_sec_window_1" },
	{ FLICK_BETRUGER, "marscity_cinematic_betruger_1" },
	{ FLICK_SWANN, "marscity_cinematic_swann_1" },
	{ FLICK_CAMPBELL, "marscity_cinematic_campbell_1" },
// Mars City Reception
	{ FLICK_RECEPTION, "marscity_receptionist_full_head" },
// Mars City Meeting
	{ FLICK_SWANN, "marscity_cinematic_swann_speech" },
	{ FLICK_BETRUGER, "marscity_cinematic_betruger_speech_head" },
// Mars City: Report to Sergeant Kelly
	{ FLICK_KELLY, "marscity_cinematic_sarge_1" },
	// { FLICK_KELLY, "sarge_secondary" },
// Mars City Underground, grab your gear
	{ FLICK_BROOKS, "underground_window_security_1_head" },
	{ FLICK_BROOKS, "underground_window_security_1" },
	{ FLICK_MARK_RYAN, "underground_security_helmet_mark_3" },
// Mars City Underground, all hell breaks loose
	{ FLICK_ISHII, "underground_crazy_sci_1" },
	{ FLICK_ISHII, "underground_crazy_zombie_1" },
	{ FLICK_ISHII, "underground_crazy_zombie_1_head" },
// Mars City 2: Ceiling guy
	{ FLICK_ROLAND, "marscity2_maint_ceiling_1_head" },

	// Admin
	{ FLICK_SWANN, "admin_overhear_swann_1" },
	{ FLICK_CAMPBELL, "admin_overhear_campbell_3" },

// ROE, Erebus1: Intro
	{ FLICK_TOWER, "erebus1_intro_scientist_1" },
	{ FLICK_MCNEIL, "erebus1_intro_mcneil_1" },
	{ FLICK_MCNEIL, "erebus1_intro_flash_1" },
	{ FLICK_NONE, "erebus1_intro_pda_1" },
	{ FLICK_MARINE_PDA, "erebus1_intro_marine1_1" },
	{ FLICK_MARINE_TORCH, "erebus1_intro_marine2_1" },
	{ FLICK_POINT, "erebus1_intro_detonate_1" },
	{ FLICK_MARINE_PDA, "erebus1_intro_marine3_1" },
	{ FLICK_MARINE_TORCH, "erebus1_intro_flash_1" },
	{ FLICK_BETRUGER, "maledict_intro_cinematic_1" },
};

static const character_map_t shaderArray[] = {
	// Admin
	{ FLICK_BETRUGER, "admin_betruger_operation_is_that" },
	{ FLICK_BETRUGER, "admin_betruger_its_not_out_of_control" },
	{ FLICK_BETRUGER, "admin_betruger_its_not_out_of_control" },
	{ FLICK_NONE, "swann_screen_off" },
	{ FLICK_NONE, "bfgcase_unlock" },

	// ROE, Erebus1: Intro
	{ FLICK_MARINE_TORCH, "e1_sci02_redteam" },
	{ FLICK_MARINE_PDA, "e1_marine_chamber" },
	{ FLICK_NONE, "e1_bomb_ticks" },
	{ FLICK_MARINE_PDA, "e1_engineer01_werein" },
	{ FLICK_MARINE_PDA, "e1_engineer01_areyouseeingthis" },
	{ FLICK_POINT, "e1_tango_chatter" },
	{ FLICK_NONE, "e1_tango_garbled" },
	{ FLICK_MARINE_PDA, "e1_mchatter_07" },
	{ FLICK_MCNEIL, "e1_mchatter_10" },
	{ FLICK_NONE, "e1_dscream_03" },

	// LE
	{ FLICK_MARINE_PDA, "enpro_soldier2_1" },
	{ FLICK_MARINE_TORCH, "enpro_soldier1_1" },
	{ FLICK_POINT, "enpro_soldier3_1" },
	{ FLICK_MARINE_TORCH, "enpro_soldier4_1" },
};

static const spoken_line_t lineArray[] = {
	// Mars City Intro
	{ "", "Incoming transport detected." },
	{ "", "Mars approach, Darkstar with you, zero seven zero, 63, passing through 38 thousand." },
	{ "", "Roger, Darkstar. Descend to 2 thousand, set speed, contact ground on 2 6 9 7 2." },
	{ "", "Roger that, Tower." },
	{ "marscity_cin_marine1_1", "We have them on radar, sir. They'll be landing in a few moments." },
	{ "marscity_cin_bertruger1_1", "Excellent. See that councillor Swann is sent directly to me." },
	{ "marscity_cin_marine1_2", "Here, sir." },
	{ "", "Tower. Darkstar on final." },
	{ "", "We've got you, Darkstar, you are set for lockdown. Welcome back." },
	{ "marscity_cin_swann1_1", "I can't believe it's come to this. I didn't want to come here." },
	{ "marscity_cin_campbell1_1", "He left you no choice." },
	{ "marscity_cin_swann1_2", "True, but this is the last time. I'm tired of running damage control every time he makes a mess." },
	{ "marscity_cin_campbell1_2", "Right. You're the control. And if that fails, I'm the damage." },
	{ "marscity_cin_swann1_3", "If that's what it takes. Betruger is going to start doing things our way." },
	{ "marscity_cin_campbell1_3", "Whatever you say, councillor." },
	// Mars City Reception
	{ "marscity_receptionist_trigger_1", "Welcome to Mars." },
	{ "marscity_receptionist_trigger_2", "First time?" },
	{ "marscity_receptionist_trigger_3", "You can just leave your bag there. I'll have it sent up to your quarters." },
	{ "marscity_receptionist_trigger_4", "OK, there's a few things we need to take care of first. This is your personal data assistant. You'll need this to access all secure areas." },
	{ "marscity_receptionist_trigger_5", "If you get clearance for any security zones, it'll download directly." },
	{ "marscity_receptionist_trigger_6", "It's important, so don't lose it." },
	{ "marscity_receptionist_trigger_7", "I see here that sergeant Kelly has requested your immediate attention." },
	{ "marscity_receptionist_trigger_8", "Head directly to Marine Command." },
	{ "marscity_receptionist_trigger_9", "It's just that way. Follow the signs." },
	// Mars City Meeting
	{ "marscity_speech_swann1", "I'm here because there seems to be some very serious problems." },
	{ "marscity_speech_bertruger1", "Oh really? Do I need to remind you of the groundbreaking work that we're doing here?" },
	{ "marscity_speech_swann2", "No. But I've been authorized by the board to look at everything." },
	{ "marscity_speech_bertruger2", "The board authorized you? Hmm. The board doesn't know the first thing about science. All they want is something to make them more money. Some product. Don't worry, they'll get their product." },
	{ "marscity_speech_swann3", "After how many accidents? Tell me, Doctor Betruger. Why are so many workers spooked, complaining, requesting transfers off Mars?" },
	{ "marscity_speech_bertruger3", "They simply can't handle life here. They're exhausted and overworked. If I had a larger, more competent staff, and bigger budget, even these few accidents could have been avoided." },
	{ "marscity_speech_swann4", "I'm afraid you'll get nothing more until my report is filed with the board. I will need full access, Doctor Betruger, Delta included. I won't have any difficulties doing that, will I?" },
	{ "marscity_speech_bertruger4", "Only if you get lost, Swann. Just stay out of my way. Amazing things will happen here soon. You just wait." },
	{ "marscity_speech_swann5", "Let's go." },
	// Mars City Sergeant Kelly
	{ "snd_sargecin1", "Took your sweet time, Marine. Now, here's the situation." },
	{ "snd_sargecin2", "Another member of the science team has gone missing. Since you're the ranking FNG, you get to find him." },
	{ "snd_sargecin3", "I want you to check out the old decommissioned comm facility. We heard he might be heading that way." },
	{ "snd_sargecin4", "The only way there is through the service passageway under mars city." },
	{ "snd_sargecin5", "I programmed this sentry to guide you to the maintenance elevator. I hope you follow the sentry better than you've followed orders so far." },
	{ "snd_sargecin6", "You can pick up some gear at the security checkpoint at the bottom of the elevator." },
	{ "snd_sargecin7", "Oh, and when you find him, just bring him back. Do not hurt him. Now move out." },
	{ "", "When you complete your mission, report back here. Move out." },
	{ "", "You've got a mission to accomplish. Start moving, Marine." },
	{ "", "Are you deaf, Marine? When I say move out, you'd better move." },
	// { "marscity_sarge_cough", "Ahem" },

	// Mars City Underground Security
	//Voice underground_window_security_1_head: talk_trigger:
	{ "snd_window1", "" },
	//Voice underground_window_security_1_head: window_b:
	{ "snd_window2", "" },
	//Voice underground_window_security_1_head: window_c:
	{ "snd_window3", "" },
	//Voice underground_window_security_1_head: window_k:
	{ "snd_window11", "" },
	//Voice underground_window_security_1_head: window_d:
	{ "snd_window4", "" },
	//Voice underground_window_security_1: window_e:
	{ "brooks05radiocheck", "Mars Sec radio check. Excellent. Good signal." },
	//Voice underground_window_security_1: window_f:
	{ "brooks06mission", "Looks like you've been assigned the decommissioned comm facility. Just follow the main passage through the underground junction and then straight out. You'll have a quick evac. And what better way to see the Martian surface than to run across it." },
	//Voice underground_window_security_1: window_g:
	{ "brooks07allset", "OK. You're all set." },
	//Voice underground_window_security_1: window_h:
	{ "brooks08dontshootcivilians", "Oh yeah, keep in mind, civillians are working down here. Don't get excited and shoot any." },
	//Voice underground_window_security_1_head: window_l:
	{ "snd_window12", "You don't have a lot of time, Marine. You need to move out." },

	//Voice underground_security_helmet_mark_3: talk_trigger:
	{ "snd_talk_trigger", "Your suit's got plenty of oxygen." },

	//Voice underground_crazy_sci_1: crazy_a:
	{ "snd_crazy1", "" },
	//Voice underground_crazy_sci_1: crazy_a:
	{ "snd_crazy1", "" },
	//Voice underground_crazy_sci_1: crazy_a:
	{ "snd_crazy1", "" },
	//Voice underground_crazy_sci_1: crazy_c:
	{ "snd_crazy2", "" },
	//Voice underground_crazy_sci_1: crazy_c:
	{ "snd_crazy2", "" },
	//Voice underground_crazy_sci_1: crazy_c:
	{ "snd_crazy2", "" },
	//Voice underground_crazy_sci_1: crazy_c:
	{ "snd_crazy3", "" },
	//Voice underground_crazy_sci_1: crazy_c:
	{ "snd_crazy3", "" },
	//Voice underground_crazy_sci_1: crazy_c:
	{ "snd_crazy3", "" },
	//Voice underground_crazy_sci_1: crazy_c:
	{ "snd_crazy4", "" },
	//Voice underground_crazy_sci_1: crazy_c:
	{ "snd_crazy4", "" },
	//Voice underground_crazy_sci_1: crazy_c:
	{ "snd_crazy4", "" },
	//Voice underground_crazy_sci_1: crazy_c:
	{ "snd_crazy5", "" },
	//Voice underground_crazy_sci_1: crazy_c:
	{ "snd_crazy5", "" },
	//Voice underground_crazy_sci_1: crazy_c:
	{ "snd_crazy5", "" },
	//Voice2 underground_crazy_zombie_1: backup:
	{ "jonathan_aya", "ay ah" },
	//Voice monster_zsec_shotgun_2: windowstart:
	//{ "monster_zombie_security_melee", "" },
	//Voice2 underground_crazy_zombie_1: backup:
	{ "jonathan_lord_help_us", "Lord help us." },
	//Voice2 underground_invasion_chestskull_2: floorskull:
	//{ "mc_skull_shriek", "" },
	//Voice underground_invasion_chestskull_2: floorskull:
	//{ "mc_skull_passby", "" },
	//Voice underground_crazy_zombie_1: transform:
	{ "jonathan_huh", "huh?" },
	//Voice2 underground_crazy_zombie_1: transform:
	{ "jonathan_no", "no" },
	//Voice underground_invasion_chestskull_1: transform:
	//{ "mc_skull_passby", "" },
	//Voice2 underground_crazy_zombie_1_head: transform:
	//{ "mc_leatherface", "" },
		//Voice underground_crazy_zombie_1_head: transform:
	{ "zombiesting", "roar" },


	// Admin overheard
	//Voice admin_overhear_swann_1: overhear_a:
	{ "admin_swann_im_telling_you", "I'm telling you now, doctor. The UAC is taking over this operation." },
	//Voice admin_overhear_swann_1: overhear_a:
	{ "admin_betruger_operation_is_that", "Operation? Is that what you're calling it?" },
	//Voice admin_overhear_swann_1: overhear_b:
	{ "admin_swann_the_situation", "The situation is out of control." },
	//Voice admin_overhear_swann_1: overhear_b:
	{ "admin_betruger_its_not_out_of_control", "It's not out of control, Swann. You are. I'll manage this, and you and your flunky will be taking control of nothing. Do you understand?" },
	//Voice admin_overhear_swann_1: overhear_c:
	{ "admin_swann_yes_bertruger", "Yes, Betruger." },
	//Voice2 admin_overhear_swann_1: overhear_c:
	//{ "swann_screen_off", "" },
	//Voice admin_overhear_swann_1: overhear_c:
	{ "admin_swann_i_understand", "I think I do understand." },
	//Voice2 admin_overhear_campbell_3: overhear3:
	//{ "bfgcase_unlock", "" },
	//Voice admin_overhear_campbell_3: overhear3:
	{ "admin_campbell_planb", "OK. Plan B." },


	// ROE Intro
	{ "e1_sci01_thereclose", "Dr. McNeil, they're close." },
	{ "e1_sci02_redteam", "" },
	{ "e1_mcneil_redteam_alt", "Red Team, we're showing your position less than one hundred metres from the signal. There's no data suggesting any ancient civ development in that area." },
	{ "e1_mchatter_01", "I think they're up to something they just don't want us to know about." },
	{ "e1_mchatter_02", "Hmph. Some things never change around here." },
	{ "e1_marine_chamber", "There's some sort of chamber beyond this wall. Point, we need a charge here." },
	{ "e1_marine_onmyway", "Roger that. On my way." },
	{ "e1_marine_backitup", "OK, back it up." },
	{ "e1_sci02_troublestabilizing", "I'm having trouble stabilising the signals." },
	{ "e1_engineer01_werein", "We're in." },
	{ "e1_mchatter_06", "Someone didn't want this place to be found." },
	{ "e1_engineer01_areyouseeingthis", "Doctor McNeil, are you seeing this?" },
	{ "e1_tango_chatter", "We have reached target and are now preparing to secure the area." },
	{ "e1_tango_garbled", "We have reached target and are now preparing to secure the area." },
	{ "e1_mcneil_troublewithtransmission", "We're having trouble with your transmission. I need that stream brought back online now. Damn it. I can't see a thing." },
	{ "e1_sci_channel", "" },
	{ "e1_mchatter_08", "Go slowly." },
	{ "e1_mchatter_04", "Do you hear that sound?" },
	{ "e1_mchatter_07", "What the hell is that?" },
	{ "e1_mchatter_10", "Look at that." },
	{ "snd_hellreachesout", "Hell reaches out for what is ours." },
	{ "snd_unbound", "We have been unbound." },
	{ "snd_theyhavearrived", "They have arrived, my children." },
	{ "snd_asipromised", "As I promised." },
	{ "snd_riseup", "Rise up." },
	{ "snd_awaken", "Awaken." },
	{ "snd_huntthemdown", "Hunt them down." },
	{ "", "Our new reign begins now." },

	//Voice enpro_soldier2_1: shot_a:
	{ "snd_move_in", "" },
	//Voice enpro_soldier2_1: shot_c:
	{ "snd_quiet", "" },
	//Voice enpro_soldier3_1: shot_b:
	{ "snd_status", "" },
	//Voice enpro_soldier1_1: shot_b:
	{ "snd_hallway", "" },
	//Voice enpro_soldier3_1: shot_d:
	{ "snd_nothing_here", "" },
	//Voice enpro_soldier3_1: shot_d:
	{ "snd_pain", "" },
	//Voice enpro_soldier1_1: shot_d:
	{ "snd_what", "" },
	//Voice enpro_soldier2_1: shot_h:
	{ "snd_son", "" },
	//Voice enpro_soldier4_1: shot_c:
	{ "snd_swing", "" },
	//Voice enpro_soldier4_1: shot_c:
	{ "snd_down", "" },
};
/*
//Voice erebus1_intro_scientist_1: intro_scientist_a:
{ "e1_sci01_thereclose", "." }
//Voice2 erebus1_intro_scientist_1: intro_scientist_a:
{ "e1_sci02_redteam", "." }
//Voice erebus1_intro_mcneil_1: intro_mcneil_a:
{ "e1_mcneil_redteam_alt", "." }
//Voice2 erebus1_intro_pda_1: intro_pda_d:
{ "e1_scanner_pings", "." }
//Voice erebus1_intro_marine1_1: intro_marine1_d:
{ "e1_mchatter_01", "." }
//Voice2 erebus1_intro_marine2_1: intro_marine2_e:
{ "e1_mchatter_02", "." }
//Voice erebus1_intro_pda_1: intro_pda_h:
{ "e1_marine_chamber", "." }
//Voice2 erebus1_intro_pda_1: intro_pda_h:
{ "e1_scanner_keypresses", "." }
//Voice erebus1_intro_detonate_1: intro_detonate_h:
{ "e1_marine_onmyway", "." }
//Voice erebus1_intro_detonate_1: intro_detonate_j:
{ "e1_marine_backitup", "." }
//Voice2 erebus1_intro_detonate_1: intro_detonate_j:
{ "e1_bomb_ticks", "." }
//Voice erebus1_intro_scientist_1: intro_scientist_l:
{ "e1_sci02_troublestabilizing", "." }
//Voice erebus1_intro_pda_1: intro_pda_l:
{ "e1_engineer01_werein", "." }
//Voice erebus1_intro_marine3_1: intro_marine3_n:
{ "e1_mchatter_06", "." }
//Voice erebus1_intro_pda_1: intro_pda_n:
{ "e1_engineer01_areyouseeingthis", "." }
//Voice erebus1_intro_detonate_1: intro_detonate_o:
{ "e1_tango_chatter", "." }
//Voice2 erebus1_intro_mcneil_1: intro_mcneil_o:
{ "e1_tango_garbled", "." }
//Voice erebus1_intro_mcneil_1: intro_mcneil_p:
{ "e1_mcneil_troublewithtransmission", "." }
//Voice erebus1_intro_scientist_1: intro_scientist_p:
{ "e1_sci_channel", "." }
//Voice erebus1_intro_flash_1: intro_flash_s:
{ "e1_mchatter_08", "." }
//Voice erebus1_intro_flash_1: intro_flash_u:
{ "e1_mchatter_04", "." }
//Voice erebus1_intro_flash_1: intro_flash_v:
{ "e1_mchatter_07", "." }
//Voice2 erebus1_intro_flash_1: intro_flash_w:
{ "e1_mchatter_10", "." }
//Voice erebus1_intro_plyr_helmet_1: intro_plyr_helmet_w:
{ "e1_helmet_removal", "." }
//Voice maledict_intro_cinematic_1: maledict_intro:
{ "snd_hellreachesout", "." }
//Voice2 maledict_intro_cinematic_1: maledict_intro:
{ "snd_unbound", "." }
//Voice maledict_intro_cinematic_1: maledict_intro:
{ "snd_theyhavearrived", "." }
//Voice2 maledict_intro_cinematic_1: maledict_intro:
{ "snd_asipromised", "." }
//Voice maledict_intro_cinematic_1: maledict_intro:
{ "snd_riseup", "." }
//Voice maledict_intro_cinematic_1: maledict_intro:
{ "snd_awaken", "." }
//Voice2 maledict_intro_cinematic_1: maledict_intro:
{ "e1_dscream_03", "." }
//Voice monster_hunter_invul_1: idle:
{ "snd_idle", "." }
//Voice maledict_intro_cinematic_1: maledict_intro:
{ "snd_huntthemdown", "." }

*/


int EntityToCharacter( const char* entity, const char* lineName )
{
	// Some lines come from the same entity, but are different characters. Check these exceptions first.
	for (int i = 0; i < sizeof(shaderArray) / sizeof(*shaderArray); i++)
	{
		if (idStr::Cmp(lineName, shaderArray[i].entity) == 0)
			return shaderArray[i].character;
	}
	// Check which entity maps to which character
	for (int i = 0; i < sizeof(entityArray) / sizeof(*entityArray); i++)
	{
		if (idStr::Cmp(entity, entityArray[i].entity) == 0)
			return entityArray[i].character;
	}
	// not found
	return FLICK_NONE;
}

// return true if the game is allowed to play this line, or false if the user is going to say it.
bool FlickSync_Voice( const char* entity, const char* animation, const char* lineName )
{
	int character = EntityToCharacter( entity, lineName );
	return character != vr_flickCharacter.GetInteger();
}
