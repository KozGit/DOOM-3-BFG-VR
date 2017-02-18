#pragma hdrstop

#include"precompiled.h"

#include "framework/Common_local.h"

#include "d3xp/Game_local.h"

#include "FlickSync.h"
#include "Voice.h"

idCVar vr_flicksyncCharacter( "vr_flicksyncCharacter", "0", CVAR_INTEGER | CVAR_ARCHIVE, "Flicksync character. 0 = none, 1 = Betruger, 2 = Swan, 3 = Campbell, 4 = DarkStar, 5 = Tower, 6 = Reception, 7 = Kelly, 8 = Brooks, 9 = Mark Ryan, 10 = Ishii, 11 = Roland, 12 = McNeil, 13 = Marine w PDA, 14 = Marine w Torch, 15 = Point, 16 = Bravo Lead, 17 = Player", 0, FLICK_PLAYER );
idCVar vr_flicksyncCueCards( "vr_flicksyncCueCards", "0", CVAR_INTEGER | CVAR_ARCHIVE, "How many Cue Card Power-Ups to start with. Default = 0, max = 5", 0, 5 );
idCVar vr_cutscenesOnly( "vr_cutscenesOnly", "0", CVAR_INTEGER | CVAR_ARCHIVE, "Skip action and only show cutscenes. 0 = normal game, 1 = cutscenes only, 2 = action only", 0, 2 );

extern idCVar timescale;

int Flicksync_Score = 0;
int Flicksync_CueCards = 0;	// How many Cue Card Power-Ups we have in our inventory. 0 to 5.
int Flicksync_CorrectInARow = 0;	// 7 correct in a row will give us another Cue Card Power-Up
int Flicksync_FailsInARow = 0;	// 3 fails in a row is Game Over
idStr Flicksync_CueCardText = "";	// What our cue card would say if we used it
bool Flicksync_CueCardActive = false;	// Are we currently using one of our Cue Card Power-Ups?
idStr Flicksync_CueText = "";	// What cue line we need to respond to
bool Flicksync_CueActive = false;	// Are we playing and displaying the cue line subtitle?
int Flicksync_CheatCount = 0;	// Cheat once = warning, cheat twice it's GAME OVER!
bool Flicksync_GameOver = false;
bool Flicksync_InCutscene = false;
t_cutscene Flicksync_skipToCutscene;
t_cutscene Flicksync_currentCutscene;

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

typedef struct
{
	uint64 startTime;
	uint32 length;
	int confidence;
	const char* shader;
	idStr entity;
	char text[1024];
} timed_spoken_line_t;

typedef struct
{
	t_cutscene cutscene;
	const char *camera;
} cutscene_camera_t;

#define MAX_HEARD_LINES 3
static timed_spoken_line_t linesHeard[MAX_HEARD_LINES] = {};
static int firstLineHeard = 0, lastLineHeard = -1;

static timed_spoken_line_t waitingLine = {};
static bool hasWaitingLine = false;

static timed_spoken_line_t pausedLine = {};
static bool hasPausedLine = false, endAfterPause = false;

static timed_spoken_line_t cueLine = {};
static bool hasCueLine = false, needCue = false;

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
	{ FLICK_SARGE, "marscity_cinematic_sarge_1" },
	{ FLICK_TOWER, "marscity_sec_window2_1" },
	// { FLICK_SARGE, "sarge_secondary" },
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
	{ FLICK_POINT, "erebus1_cinematic_marine_gravitygun_end_1" },

	{ FLICK_MARINE_PDA, "enpro_soldier2_1" },
	{ FLICK_POINT, "enpro_soldier1_1" },
	{ FLICK_BRAVO_LEAD, "enpro_soldier3_1" },
	{ FLICK_MARINE_TORCH, "enpro_soldier4_1" }
	
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
	{ FLICK_BRAVO_LEAD, "enpro_give_status" },
};

static const spoken_line_t lineArray[] = {
	// Mars City Intro
	{ NULL, "Incoming transport detected." },
	{ NULL, "Mars approach, Darkstar with you, zero seven zero, 63, passing through 38 thousand." },
	{ NULL, "Roger, Darkstar. Descend to 2 thousand, set speed, contact ground on 2 6 9 7 2." },
	{ NULL, "Roger that, Tower." },
	{ "marscity_cin_marine1_1", "We have them on radar, sir. They'll be landing in a few moments." },
	{ "marscity_cin_bertruger1_1", "Excellent. See that councillor Swan is sent directly to me." },
	{ "marscity_cin_marine1_2", "Here, sir." },
	{ NULL, "Tower. Darkstar on final." },
	{ NULL, "We've got you, Darkstar, you are set for lockdown. Welcome back." },
	{ "marscity_cin_swann1_1", "I can't believe it's come to this. I didn't want to come here." },
	{ "marscity_cin_campbell1_1", "He left you no choice." },
	{ "marscity_cin_swann1_2", "True, but this is the last time. I'm tired of running damage control every time he makes a mess." },
	{ "marscity_cin_campbell1_2", "Right. You're the control. And if that fails, I'm the damage." },
	{ "marscity_cin_swann1_3", "If that's what it takes. Betroogger is going to start doing things our way." },
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
	{ "marscity_speech_swann3", "After how many accidents? Tell me, Doctor Betroogger. Why are so many workers spooked, complaining, requesting transfers off Mars?" },
	{ "marscity_speech_bertruger3", "They simply can't handle life here. They're exhausted and overworked. If I had a larger, more competent staff, and bigger budget, even these few accidents could have been avoided." },
	{ "marscity_speech_swann4", "I'm afraid you'll get nothing more until my report is filed with the board. I will need full access, Doctor Betroogger, Delta included. I won't have any difficulties doing that, will I?" },
	{ "marscity_speech_bertruger4", "Only if you get lost, Swan. Just stay out of my way. Amazing things will happen here soon. You just wait." },
	{ "marscity_speech_swann5", "Let's go." },
	// Mars City Sergeant Kelly
	{ "marscity_cin_sarge1", "Took your sweet time, Marine. Now, here's the situation." },
	{ "marscity_cin_sarge2", "Another member of the science team's gone missing. Since you're the ranking FNG, you get to find him." },
	{ "marscity_cin_sarge3", "I want you to check out the old decommissioned comm facility. We heard he might be heading that way." },
	{ "marscity_cin_sarge4", "The only way there is through the service passage under mars city." },
	{ "marscity_cin_sarge5", "I programmed this sentry to guide you to the maintenance elevator. I hope you follow the sentry better than you've followed orders so far." },
	{ "marscity_cin_sarge6", "You can pick up some gear at the security checkpoint at the bottom of the elevator." },
	{ "marscity_cin_sarge7", "Oh, and when you find him, just bring him back. Do not hurt him. Now move out." },
	{ "marscity_sarge_primary", "When you complete your mission, report back here. Move out." },
	{ "marscity_sarge_secondary", "You've got a mission to accomplish. Start moving, Marine." },
	{ "marscity_sarge_secondary2", "Are you deaf, Marine? When I say move out, you'd better move." },
	// { "marscity_sarge_cough", "Ahem" },

	// Mars City Underground Security
	//Voice underground_window_security_1_head: talk_trigger:
	{ "brooks01welcome", "Welcome to the dungeon, marine. The most unexciting place on Mars." },
	//Voice underground_window_security_1_head: window_b:
	{ "brooks02imgonnaneed", "I'm gonna need you to grab some armor and secure your pistol before I can pass you through security." },
	//Voice underground_window_security_1_head: window_c:
	{ "brooks03grabyourgear", "OK. Grab your gear." },
	//Voice underground_window_security_1_head: window_i:
	{ "brooks09re_secureyourgear", "You need to secure your gear, marine." },
	//Voice underground_window_security_1_head: window_j:
	{ "brooks13re_storagecabinet", "Your stuff's in the storage cabinet." },
	//Voice underground_window_security_1_head: window_k:
	{ "brooks14readyforcombat", "Now you're ready for combat." },
	//Voice underground_window_security_1_head: window_d:
	{ "brooks04letmeradiotest", "Let me do a radio test." },
	//Voice underground_window_security_1: window_e:
	{ "brooks05radiocheck", "Mars Sec radio check. Excellent. Good signal." },
	//Voice underground_window_security_1: window_f:
	{ "brooks06mission", "Looks like you've been assigned the decommissioned comm facility. Just follow the main passage through the underground junction and then straight out. You'll have a quick evac. And what better way to see the Martian surface than to run across it." },
	//Voice underground_window_security_1: window_g:
	{ "brooks07allset", "OK. You're all set." },
	//Voice underground_window_security_1: window_h:
	{ "brooks08dontshootcivilians", "Oh yeah, keep in mind, civillians are working down here. Don't get excited and shoot any." },
	//Voice underground_window_security_1_head: window_l:
	{ "brooks18re_moveout", "You don't have a lot of time, marine. You need to move out." },

/*
	//Voice charles_head: talk_trigger:
	{ "charles_mcu_hey", "Hey! You're looking for the scientist, right?" },
	//Voice charles_head: talk_trigger:
	{ "charles_mcu_notsure", "I'm not sure you want to find him. You see. Er, never mind." },

	//Voice underground_maint_bald_ross_1_head: rosstalk1:
	{ "ross_mcu_listenscotty", "Listen, Scotty. I've done this a million times. It's not that hard." },
	//Voice underground_maint_bald_ross_1_head: rosstalk2:
	{ "ross_mcu_because", "Because I'm getting paid to make sure you do it." },
	//Voice underground_maint_bald_ross_1_head: rosstalk3:
	{ "ross_mcu_justfinish", "Just finish the coupling so we can continue." },

	//Voice todd: toddtalk1:
	{ "todd_mcu_talk1", "Can you maintenance guys not keep any of these machines running." },
	//Voice eric: worktalk1:
	{ "eric_mcu_worktalk1", "Yes sir. It's just that the science team demands a lot out of these things." },
	//Voice todd: toddtalk2:
	{ "todd_mcu_talk2", "Enough excuses. Just do your job and get the science team what they need." },
	//Voice eric: worktalk2:
	{ "eric_mcu_worktalk2", "Well, I'm doing everything. But I just can't explain some of the things that have been happening to the systems. It's weird." },
	//Voice todd: toddtalk3:
	{ "todd_mcu_talk3", "Just get it done." },

	//Voice ian_head: talk_trigger:
	{ "ian_mcu_trigger", "Jeez. Huh. Do you make a habit of sneaking up on people? Everyone's already on edge down here with all the strange things that have been going on." },
	//Voice ian_head: talk_primary:
	{ "ian_mcu_primary", "The garage area is right through that door. Now stop bothering me." },
	//Voice ian_head: talk_secondary1:
	{ "ian_mcu_secondary", "The equipment seems to be affected by something unseen." },
*/

	//Voice underground_security_helmet_mark_3: talk_trigger:
	{ "mark_mcu_triggered", "Hey. You're that new guy headed for the old comm center, right? "
	"You'd better hustle up pal. This passage doesn't go all the way there. You'll have a quick walk outside to the center's airlock. "
	"Hey, don't sweat it. Your suit's got plenty of oxygen." },
	//Voice underground_security_helmet_mark_3: talk_primary:
	{ "mark_mcu_primary", "Well? What the hell are you waiting for, marine?" },
	//Voice underground_security_helmet_mark_3: talk_secondary1:
	{ "mark_mcu_secondary1", "Get going." },
	//Voice underground_security_helmet_mark_3: talk_secondary2:
	{ "mark_mcu_secondary2", "Well? What the hell are you waiting for, marine?" },

	//Voice underground_crazy_sci_1: crazy_a:
	{ "scinuts1a", "Huh? No, no. Please. You must let me get this communication out. They have to be warned while there is still time. I can't let. I..." },
	//Voice underground_crazy_sci_1: crazy_c:
	{ "scinuts2a", "You don't know what I've seen. You can't possibly understand or comprehend." },
	//Voice underground_crazy_sci_1: crazy_c:
	{ "scinuts3a", "The devil is real." },
	//Voice underground_crazy_sci_1: crazy_c:
	{ "scinuts4a", "I know. I built his cage." },
	//Voice underground_crazy_sci_1: crazy_c:
	{ "scinuts5a", "Oh, God!" },
	//Voice2 underground_crazy_zombie_1: backup:
	{ "jonathan_aya", "ay ah" },
	//Voice monster_zsec_shotgun_2: windowstart:
	//{ "monster_zombie_security_melee", NULL },
	//Voice2 underground_crazy_zombie_1: backup:
	{ "jonathan_lord_help_us", "Lord help us!" },
	//Voice2 underground_invasion_chestskull_2: floorskull:
	//{ "mc_skull_shriek", NULL },
	//Voice underground_invasion_chestskull_2: floorskull:
	//{ "mc_skull_passby", NULL },
	//Voice underground_crazy_zombie_1: transform:
	{ "jonathan_huh", "huh?" },
	//Voice2 underground_crazy_zombie_1: transform:
	{ "jonathan_no", "no!" },
	//Voice underground_invasion_chestskull_1: transform:
	//{ "mc_skull_passby", NULL },
	//Voice2 underground_crazy_zombie_1_head: transform:
	//{ "mc_leatherface", NULL },
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
	{ "admin_betruger_its_not_out_of_control", "It's not out of control, Swan. You are. I'll manage this, and you and your flunky will be taking control of nothing. Do you understand?" },
	//Voice admin_overhear_swann_1: overhear_c:
	{ "admin_swann_yes_bertruger", "Yes, Betroogger." },
	//Voice2 admin_overhear_swann_1: overhear_c:
	//{ "swann_screen_off", NULL },
	//Voice admin_overhear_swann_1: overhear_c:
	{ "admin_swann_i_understand", "I think I do understand." },
	//Voice2 admin_overhear_campbell_3: overhear3:
	//{ "bfgcase_unlock", NULL },
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
	//Voice2 erebus1_intro_flash_1: intro_flash_w:
	{ "e1_mchatter_10", "Look at that." },
	//Voice maledict_intro_cinematic_1: maledict_intro:
	{ "e1_bet_hellreachesout", "Hell reaches out for what is ours." },
	//Voice2 maledict_intro_cinematic_1: maledict_intro:
	{ "e1_bet_unbound", "We have been unbound." },
	//Voice maledict_intro_cinematic_1: maledict_intro:
	{ "e1_bet_arrived", "They have arrived, my children." },
	//Voice2 maledict_intro_cinematic_1: maledict_intro:
	{ "e1_bet_asipromised", "As I promised." },
	//Voice maledict_intro_cinematic_1: maledict_intro:
	{ "e1_bet_riseup", "Rise up." },
	//Voice maledict_intro_cinematic_1: maledict_intro:
	{ "e1_bet_awaken", "Awaken." },
	//Voice maledict_intro_cinematic_1: maledict_intro:
	{ "e1_bet_huntthemdown", "Hunt them down." },
	{ NULL, "Our new reign begins now." },

	// Grabber cutscene
	/*
	//Voice erebus1_cinematic_marine_gravitygun_1: ggun_a:
	{ "grabber_cin_idle", "" },
	//Voice2 erebus1_cinematic_marine_gravitygun_1: ggun_a:
	{ "grabber_marine_breathe", "" },
	//Voice2 erebus1_cinematic_imp_2: imp_b:
	{ "grabber_imp_thud", "" },
	//Voice2 erebus1_cinematic_imp_2: imp_c:
	{ "grabber_imp_yell", "" },
	//Voice erebus1_cinematic_imp_2: imp_c:
	{ "grabber_imp_attack", "" },
	//Voice erebus1_cinematic_imp_2: imp_d:
	{ "monster_demon_imp_fireball_flight", "" },
	//Voice erebus1_cinematic_marine_gravitygun_1: ggun_d:
	{ "grabber_cin_electro", "" },
	//Voice2 erebus1_cinematic_marine_gravitygun_1: ggun_d:
	{ "grabber_cin_fireloop", "" },
	//Voice2 erebus1_cinematic_imp_2: imp_e:
	{ "grabber_imp_idle2", "" },
	//Voice erebus1_cinematic_marine_gravitygun_1: ggun_f:
	{ "grabber_cin_release", "" },
	//Voice2 erebus1_cinematic_marine_gravitygun_1: ggun_f:
	{ "grabber_cin_silent", "" },
	//Voice2 erebus1_cinematic_imp_2: imp_g:
	{ "monster_demon_imp_fireball_explode", "" },
	//Voice erebus1_cinematic_imp_2: imp_g:
	{ "monster_demon_imp_die", "" },
	//Voice erebus1_cinematic_marine_gravitygun_1: ggun_h:
	{ "e1_fall_01", "" },
	*/
	//Voice erebus1_cinematic_marine_gravitygun_end_1: ggun_end_b:
	{ "e1_dying_marine_grabber", "He tried to hit me with a fireball. But I grabbed it and threw it right back at him. You're not going to get far with that pistol. Take this grabber. It's more useful than you think." },

	  //Voice enpro_soldier2_1: shot_a:
	{ "enpro_move_in", "Bravo team. Entry secure. Move in and take positions." },
	  //Voice enpro_soldier2_1: shot_c:
	{ "enpro_quiet", "Quiet. Did you hear that?" },
	  //Voice enpro_soldier3_1: shot_b:
	{ "enpro_give_status", "This is Leed. Give me status." },
	  //Voice enpro_soldier1_1: shot_b:
	{ "enpro_hallway_clear", "Hallway's clear." },
	//Voice enpro_soldier3_1: shot_d:
	{ "enpro_nothing_here", "There's nothing here, Sam." },
	//Voice enpro_soldier3_1: shot_d:
	{ "enpro_pain", "oowahg" },
	  //Voice enpro_soldier1_1: shot_d:
	{ "enpro_what_the_hell", "What the hell?" },
	  //Voice enpro_soldier2_1: shot_h:
	{ "enpro_son_of_a", "Son of a bitch!" },

	  //Voice enpro_soldier4_1: shot_c:
	{ "enpro_swinging_left", "Leed, I'm swinging around left." },
	  //Voice enpro_soldier4_1: shot_c:
	{ "enpro_im_down", "Ah! No!" },
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


static const cutscene_camera_t cameraArray[] = {
	// Mars City 1
	{ CUTSCENE_DARKSTAR, "marscity_cinematic_cam_2" },
	{ CUTSCENE_RECEPTION, "marscity_recep_cam_a" },
	{ CUTSCENE_MEETING, "betrugerspeech" },
	{ CUTSCENE_SARGE, "marscity_cinematic_cam_sarge" },
	// Mars City Underground
	{ CUTSCENE_ISHII, "underground_invasion_cam_9" },
	{ CUTSCENE_IMP, "underground_impintro_cam_2" },
	// Admin
	{ CUTSCENE_ADMIN, "admin_overhear_cam_1" },
	{ CUTSCENE_PINKY, "admin_pinkyattack_cam_1" },
	// Alpha Labs 1
	{ CUTSCENE_ALPHALABS1, "alphalabs1_cam_2" },
	// Alpha Labs 4
	{ CUTSCENE_VAGARY, "alphalabs3_vagaryintro_cam_1" },
	// Enpro TODO (lots of enpro cameras)
	//{ CUTSCENE_ENPRO, "" },
	// CPU 1
	{ CUTSCENE_CAMPHUNT, "cpu1_camphunt_cam_1" },
	// CPU Boss
	{ CUTSCENE_CPU_BOSS, "cin_cpu_boss_cam_1" },
	// Delta 2a
	{ CUTSCENE_DELTA_SCIENTIST, "delta2a_scientist_cam_1" },
	{ CUTSCENE_DELTA_TELEPORTER, "delta2a_teleporter_cam_2" },
	// Delta 4
	{ CUTSCENE_DELTA_HKINTRO, "delta4_hkintro_cam_1" },
	// Erebus 1
	{ CUTSCENE_ARTIFACT, "erebus1_intro_camera_1" },
	{ CUTSCENE_BLOOD, "func_cameraview_1" },
	{ CUTSCENE_GRABBER, "erebus1_cinematic_camera_15" },

};

t_cutscene CameraToCutscene(idStr & name)
{
	if (g_debugCinematic.GetBool())
		gameLocal.Printf("%d: CameraToCutscene():\n\t{ CUTSCENE_, \"%s\" },\n", gameLocal.framenum, name.c_str());
	for (int i = 0; i < sizeof(cameraArray) / sizeof(*cameraArray); i++)
	{
		if (name.Cmp(cameraArray[i].camera) == 0)
			return cameraArray[i].cutscene;
	}
	return CUTSCENE_NONE;
}

void Flicksync_DoGameOver()
{
	if (g_debugCinematic.GetBool())
		gameLocal.Printf("%d: Flicksync_GameOver()\n", gameLocal.framenum);
	Flicksync_GameOver = true;
	commonVoice->Say("Game Over");
}

void Flicksync_ScoreFail()
{
	if (g_debugCinematic.GetBool())
		gameLocal.Printf("%d: Flicksync_ScoreFail()\n", gameLocal.framenum);
	needCue = false;
	Flicksync_CueActive = false;
	Flicksync_Score -= 10; // Chapter 11 says you lose points, but doesn't say how many.
	Flicksync_CorrectInARow = 0;
	Flicksync_FailsInARow++;
	if (Flicksync_FailsInARow == 2)
		commonVoice->Say("Final warning");
	else if (Flicksync_FailsInARow == 3)
		Flicksync_DoGameOver();
	else
		commonVoice->Say("Miss");
}

void Flicksync_ScoreLine(int confidence, uint64 ourStartTime, uint64 realStartTime, uint32 ourLength, uint32 realLength)
{
	const int64 seconds = 10000000;
	int64 startDelay = (int64)(ourStartTime - realStartTime);
	int64 endDelay = (int64)(ourStartTime - realStartTime) + (int32)(ourLength - realLength);
	float speed = (float)ourLength / (float)realLength;

	Flicksync_GameOver = false; // for now, we can't actually end the game
	Flicksync_CueActive = false;
	Flicksync_CueCardText = "";
	Flicksync_CueCardActive = false;
	needCue = false;

	Flicksync_Score += 100; // specified in Chapter 11
	Flicksync_CorrectInARow++; // specified in Chapter 11
	Flicksync_FailsInARow = 0;
	if (Flicksync_CorrectInARow >= 7 && Flicksync_CueCards < 5) // specified in Chapter 11
	{
		Flicksync_CueCards++;
		Flicksync_CorrectInARow = 0;
	}

	if (startDelay < -2 * seconds && endDelay < -2 * seconds)
	{
		// Early!
		if (g_debugCinematic.GetBool())
			gameLocal.Printf("%d: Flicksync_ScoreLine() early\n", gameLocal.framenum);
		commonVoice->Say("Early!");
		if (confidence < 0)
			commonVoice->Say("Unclear.");
		else if (confidence > 0)
			commonVoice->Say("Clear.");
	}
	else if (startDelay > 2 * seconds && endDelay > 2 * seconds)
	{
		// Late!
		if (g_debugCinematic.GetBool())
			gameLocal.Printf("%d: Flicksync_ScoreLine() late\n", gameLocal.framenum);
		commonVoice->Say("Late!");
		if (confidence < 0)
			commonVoice->Say("Unclear.");
		else if (confidence > 0)
			commonVoice->Say("Clear.");
	}
	else
	{
		// Good!
		if (g_debugCinematic.GetBool())
			gameLocal.Printf("%d: Flicksync_ScoreLine() good\n", gameLocal.framenum);
		if (confidence < 0)
			commonVoice->Say("Unclear!");
		else if (confidence > 0) {
			Flicksync_Score += 50; // arbitrary bonus points for speaking clearly with OK timing
			commonVoice->Say("Bonus!");
		} 
		else
			commonVoice->Say("Good!");
	}

}

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

const char* Flicksync_LineNameToLine(const char* shader)
{
	if (!shader)
		return NULL;
	for (int i = 0; i < sizeof(lineArray) / sizeof(*lineArray); i++)
	{
		if (lineArray[i].shader && idStr::Cmp(shader, lineArray[i].shader) == 0)
			return lineArray[i].text;
	}
	return NULL;
}

int Flicksync_AlreadyHeardLine(const char* line)
{
	// if heard line list is empty
	if (lastLineHeard < 0)
		return -1;

	// search backwards through circular buffer for line
	for (int i = lastLineHeard; true; i = (i - 1 + MAX_HEARD_LINES) % MAX_HEARD_LINES)
	{
		if (idStr::Cmp(line, linesHeard[i].text) == 0)
			return i;
		if (i == firstLineHeard)
			break;
	}

	return -1;
}

bool Flicksync_WaitingOnLineThatIsLate(const char* lineName, uint64 startTime)
{
	if (!hasWaitingLine)
		return false;
	// if they're asking us to wait for the same line we're already waiting for, ignore it.
	if (idStr::Cmp(waitingLine.shader, lineName) == 0)
		return false;
	// if the new line is supposed to start before our line is finished, then no need to wait
	if (startTime < (waitingLine.startTime + waitingLine.length))
		return false;
	return true;
}

void Flicksync_SayPausedLine()
{
	if (!hasPausedLine)
	{
		if (g_debugCinematic.GetBool())
			gameLocal.Printf("%d: Flicksync_SayPausedLine() but there's no paused line\n", gameLocal.framenum);
		return;
	}
	if (g_debugCinematic.GetBool())
		gameLocal.Printf("%d: Flicksync_SayPausedLine(\"%s\", \"%s\")\n", gameLocal.framenum, pausedLine.entity.c_str(), pausedLine.shader );
	idEntity* ent = NULL;
	ent = gameLocal.FindEntity(pausedLine.entity);
	if (ent != NULL)
	{
		const idSoundShader* shader;
		shader = declManager->FindSound(pausedLine.shader);
		if (shader)
			ent->StartSoundShader(shader, SND_CHANNEL_VOICE, 0, false, NULL);
		else
			ent->StartSound(pausedLine.shader, SND_CHANNEL_VOICE, 0, false, NULL);
	}
	// if our character has a line after this, then this becomes the cue line
	cueLine = pausedLine;
	hasCueLine = true;
	hasPausedLine = false;
}

void Flicksync_SayCueLine()
{
	needCue = false;
	Flicksync_CueActive = true;
	if (!hasCueLine)
	{
		if (g_debugCinematic.GetBool())
			gameLocal.Printf("%d: Flicksync_SayCueLine() but there's no cue line\n", gameLocal.framenum);
		Flicksync_CueText = "( You speak first )";
		return;
	}
	idEntity* ent = NULL;
	ent = gameLocal.FindEntity(cueLine.entity);
	if (ent != NULL)
	{
		const idSoundShader* shader;
		shader = declManager->FindSound(cueLine.shader);
		if (shader)
			ent->StartSoundShader(shader, SND_CHANNEL_VOICE, 0, false, NULL);
		else
			ent->StartSound(cueLine.shader, SND_CHANNEL_VOICE, 0, false, NULL);
	}
	Flicksync_CueText = Flicksync_LineNameToLine(cueLine.shader);
	if (g_debugCinematic.GetBool())
		gameLocal.Printf("%d: Flicksync_SayCueLine(\"%s\")\n", gameLocal.framenum, Flicksync_CueText.c_str());
}

void Flicksync_StoppedTalking()
{
	if (needCue)
	{
		if (g_debugCinematic.GetBool())
			gameLocal.Printf("%d: Flicksync_StoppedTalking(), so say the cue line we couldn't say before.\n", gameLocal.framenum);
		Flicksync_SayCueLine();
	}
}

void Flicksync_PauseCutscene()
{
	if( g_debugCinematic.GetBool() )
		gameLocal.Printf("%d: Flicksync_PauseCutscene()\n", gameLocal.framenum);
	g_stopTime.SetBool(true);
	// This makes background sounds slow mo, which is really only good for the long DarkStar landing sound.
	// But it messes up our FINAL DIALOGUE WARNING.
	//timescale.SetFloat( 0.2f );
}

void Flicksync_ResumeCutscene()
{
	if (g_debugCinematic.GetBool())
		gameLocal.Printf("%d: Flicksync_ResumeCutscene()\n", gameLocal.framenum);
	timescale.SetFloat(1.0f);
	g_stopTime.SetBool(false);
	Flicksync_SayPausedLine();
	if (endAfterPause)
	{
		// try again to end the cutscene
		Flicksync_EndCutscene();
	}
}

// return true if the game is allowed to play this line, or false if the user is going to say it.
// length is in FileTime, which is 1/10,000 of a millisecond, or 1/10,000,000 of a second
bool Flicksync_Voice( const char* entity, const char* animation, const char* lineName, uint32 length )
{
	// if we're not in flicksync mode, then play it like normal
	if( !vr_flicksyncCharacter.GetInteger() || ( !Flicksync_InCutscene && !gameLocal.inCinematic ) )
		return true;

	SYSTEMTIME systime;
	GetSystemTime(&systime);
	// startTime is also in FileTime
	uint64 startTime = 0;
	SystemTimeToFileTime(&systime, (LPFILETIME)&startTime);

	int character = EntityToCharacter(entity, lineName);

	// If the next character tries to speak before we finished our line, pause the cutscene to wait for us.
	if (Flicksync_WaitingOnLineThatIsLate(lineName, startTime))
	{
		if (g_debugCinematic.GetBool())
			gameLocal.Printf("%d: new Flicksync_Voice() while waiting for line. Pausing to wait for \"%s\"\n", gameLocal.framenum, waitingLine.text);
		//commonVoice->Say("pausing to wait for %s", waitingLine.text);
		// pause cutscene until we hear the line we are waiting for
		pausedLine.entity = entity;
		pausedLine.shader = lineName;
		pausedLine.startTime = startTime;
		pausedLine.length = length;
		hasPausedLine = true;
		Flicksync_PauseCutscene();
		if ( commonVoice->GetTalkButton() )
			needCue = true;
		else
			Flicksync_SayCueLine();
		// don't let them say the next line until we have said ours.
		return false;
	}

	// I don't know why, but sometimes this function is called 3 times for the same line.
	static const char* previousLineName = "";
	if (idStr::Cmp(lineName, previousLineName) == 0)
		return character != vr_flicksyncCharacter.GetInteger();
	previousLineName = lineName;

	if (character != vr_flicksyncCharacter.GetInteger())
	{
		if (g_debugCinematic.GetBool())
			gameLocal.Printf("%d: Flicksync_Voice(): This is a different character speaking, so set cure line\n", gameLocal.framenum);
		// this is a different character speaking
		cueLine.entity = entity;
		cueLine.shader = lineName;
		cueLine.startTime = startTime;
		cueLine.length = length;
		hasCueLine = true;
		return true;
	}

	if (Flicksync_WaitingOnLineThatIsLate(lineName, startTime))
	{
		Flicksync_ScoreFail();
	}

	const char *line = Flicksync_LineNameToLine(lineName);

	int index;
	if ((index = Flicksync_AlreadyHeardLine(line)) > 0)
	{
		if (g_debugCinematic.GetBool())
			gameLocal.Printf("%d: Flicksync_Voice(): We already said this line early: %s\n", gameLocal.framenum, line);
		//commonVoice->Say("Already heard %s", line);
		// score it based on timing
		Flicksync_ScoreLine(linesHeard[index].confidence, linesHeard[index].startTime, startTime, linesHeard[index].length, length);
		//   clear any older lines than this line from list of heard lines
		if (lastLineHeard == firstLineHeard)
			lastLineHeard = -1;
		else
			firstLineHeard = (lastLineHeard + 1) % MAX_HEARD_LINES;
		// If our character has another line straight after this, this is our cue
		cueLine.entity = entity;
		cueLine.shader = lineName;
		cueLine.startTime = startTime;
		cueLine.length = length;
		hasCueLine = true;
	}
	else
	{
		if (g_debugCinematic.GetBool())
			gameLocal.Printf("%d: Flicksync_Voice(): Starting to wait for: %s\n", gameLocal.framenum, line);
		//commonVoice->Say("Wait for %s", line);
		//   set waiting line to this line
		idStr::Copynz(waitingLine.text, line, 1024);
		waitingLine.entity = entity;
		waitingLine.length = length;
		waitingLine.startTime = startTime;
		waitingLine.shader = lineName;
		hasWaitingLine = true;
		// if we used up our cue card
		if ( Flicksync_CueCardText != "" )
			Flicksync_CueCardActive = false;
		Flicksync_CueCardText = line;
	}
	return false;
}

void Flicksync_AddVoiceLines()
{
	for (int i = 0; i < sizeof(lineArray) / sizeof(*lineArray); i++)
	{
		if (lineArray[i].text)
			commonVoice->AddFlicksyncLine(lineArray[i].text);
	}
}

// startTime & length are in FileTime, which is 1/10,000 of a millisecond, or 1/10,000,000 of a second
void Flicksync_HearLine( const char* line, int confidence, uint64 startTime, uint32 length )
{
	if( !startTime )
	{
		SYSTEMTIME systime;
		uint64 filetime;
		GetSystemTime( &systime );
		SystemTimeToFileTime( &systime, (LPFILETIME)&filetime );
		startTime = filetime - length;
	}
	const char* confidences[3] = { "low", "medium", "high" };
	//commonVoice->Say("%s: %s", confidences[confidence + 1], line);

	// if we are waiting for this line
	if (hasWaitingLine && idStr::Cmp(waitingLine.text, line) == 0)
	{
		if (g_debugCinematic.GetBool())
			gameLocal.Printf("%d: Flicksync_HearLine(\"%s\") we were waiting for\n", gameLocal.framenum, line);
		// score it based on timing
		Flicksync_ScoreLine(confidence, startTime, waitingLine.startTime, length, waitingLine.length);
		// if our character has another line straight after this, this becomes our cue
		cueLine = waitingLine;
		hasCueLine = true;

		// if we were waiting on a line that is late, unpause cutscene
		hasWaitingLine = false;
		Flicksync_ResumeCutscene();
	}
	// if we spoke before they requested the line
	else
	{
		if (hasWaitingLine)
		{
			if (g_debugCinematic.GetBool())
				gameLocal.Printf("%d: Flicksync_HearLine(\"%s\") heard wrong line!\n", gameLocal.framenum, line);
			Flicksync_StoppedTalking();
			//commonVoice->Say("Sorry, was waiting to hear %s.", waitingLine.text);
		}
		else
		{
			if (g_debugCinematic.GetBool())
				gameLocal.Printf("%d: Flicksync_HearLine(\"%s\") adding it to heard line list\n", gameLocal.framenum, line);
			//commonVoice->Say("Add to heard list %s.", line);
		}

		// add this line to list of heard lines
		int index = (lastLineHeard + 1) % MAX_HEARD_LINES;
		if (lastLineHeard < 0)
			firstLineHeard = 0;
		else if (index == firstLineHeard)
			firstLineHeard++;
		idStr::Copynz(linesHeard[index].text, line, 1024);
		linesHeard[index].length = length;
		linesHeard[index].startTime = startTime;
		linesHeard[index].confidence = confidence;
		//linesHeard[index].shader = NULL;
		lastLineHeard = index;
	}
}

// reset score to 0
void Flicksync_NewGame()
{
	if (g_debugCinematic.GetBool())
		gameLocal.Printf("%d: Flicksync_NewGame()\n", gameLocal.framenum);
	hasWaitingLine = false;
	hasPausedLine = false;
	hasCueLine = false;
	needCue = false;
	lastLineHeard = -1; // empty ring buffer of heard lines
	Flicksync_Score = 0;
	Flicksync_FailsInARow = 0;
	Flicksync_CorrectInARow = 0;
	Flicksync_CueCards = vr_flicksyncCueCards.GetInteger(); // allow them to start with cue cards
	Flicksync_CueCardText = "";
}

bool Flicksync_EndCutscene()
{
	if (hasWaitingLine)
	{
		if (g_debugCinematic.GetBool())
			gameLocal.Printf("%d: Flicksync_EndCutscene() while waiting for line\n", gameLocal.framenum);
		hasPausedLine = false;
		endAfterPause = true;
		Flicksync_PauseCutscene();
		if (commonVoice->GetTalkButton())
			needCue = true;
		else
			Flicksync_SayCueLine();
		// don't let them end the cutscene until we have said our line.
		return false;
	}
	else
	{
		if (g_debugCinematic.GetBool())
			gameLocal.Printf("%d: Flicksync_EndCutscene()\n", gameLocal.framenum);
		endAfterPause = false;
		hasCueLine = false;
		needCue = false;
		Flicksync_ResumeCutscene();
		lastLineHeard = -1; // empty ring buffer of heard lines
		Flicksync_CueCardText = "";
		timescale.SetFloat(1.0f);
		Flicksync_InCutscene = false;

		// Check if we need to skip to another cutscene after this
		if (vr_cutscenesOnly.GetInteger() == 1 && Flicksync_skipToCutscene == CUTSCENE_NONE)
			Flicksync_skipToCutscene = Flicksync_GetNextCutscene();
		if (Flicksync_skipToCutscene != CUTSCENE_NONE && Flicksync_skipToCutscene != Flicksync_currentCutscene)
			Flicksync_GoToCutscene( Flicksync_skipToCutscene );

		return true;
	}
}

void Flicksync_StartCutscene()
{
	if ( Flicksync_InCutscene )
	{
		if (g_debugCinematic.GetBool())
			gameLocal.Printf("%d: change cutscene camera angle\n", gameLocal.framenum);
		return;
	}
	t_cutscene c = CameraToCutscene(gameLocal.GetCamera()->name);
	if (c)
		Flicksync_currentCutscene = c;
	if (g_debugCinematic.GetBool())
		gameLocal.Printf("%d: Flicksync_StartCutscene()\n", gameLocal.framenum);
	Flicksync_InCutscene = true;
	endAfterPause = false;
	hasPausedLine = false;
	hasWaitingLine = false;
	hasCueLine = false;
	needCue = false;
	Flicksync_ResumeCutscene();
	lastLineHeard = -1; // empty ring buffer of heard lines
	Flicksync_CueCardText = "";
	timescale.SetFloat( 1.0f );

	// If this is the right cutscene, we're done. If wrong cutscene, skip it until we get to the one we want.
	if (Flicksync_skipToCutscene == Flicksync_currentCutscene)
		Flicksync_skipToCutscene = CUTSCENE_NONE;
	else if (Flicksync_skipToCutscene != CUTSCENE_NONE)
		gameLocal.SkipCinematicScene();
}

bool Flicksync_UseCueCard()
{
	if( Flicksync_CueCards > 0 && !Flicksync_CueCardActive )
	{
		if (g_debugCinematic.GetBool())
			gameLocal.Printf("%d: Flicksync_UseCueCard() successful\n", gameLocal.framenum);
		--Flicksync_CueCards;
		Flicksync_CueCardActive = true;
		return true;
	}
	else
	{
		if (g_debugCinematic.GetBool())
			gameLocal.Printf("%d: Flicksync_UseCueCard() not possible\n", gameLocal.framenum);
		return false;
	}
}

void Flicksync_Cheat()
{
	++Flicksync_CheatCount;
	if( Flicksync_CheatCount >= 2 )
	{
		Flicksync_DoGameOver();
	}
}

void Flicksync_GiveUp()
{
	if ( hasPausedLine || g_stopTime.GetBool() )
	{
		if (g_debugCinematic.GetBool())
			gameLocal.Printf("%d: Flicksync_GiveUp() was paused\n", gameLocal.framenum);
		hasWaitingLine = false;
		Flicksync_ScoreFail();
		Flicksync_ResumeCutscene();
	}
	else if ( hasWaitingLine )
	{
		if (g_debugCinematic.GetBool())
			gameLocal.Printf("%d: Flicksync_GiveUp() was waiting\n", gameLocal.framenum);
		hasWaitingLine = false;
		Flicksync_ScoreFail();
	}
	else
	{
		if (g_debugCinematic.GetBool())
			gameLocal.Printf("%d: Flicksync_GiveUp() skipping\n", gameLocal.framenum);
		gameLocal.SkipCinematicScene();
	}
}

idStr CutsceneToMapName( t_cutscene c )
{
	common->Printf("CutsceneToMapName() current = \"%s\"", commonLocal.GetCurrentMapName());
	if (!c)
		return "";
	if (c >= FMV_UAC && c <= CUTSCENE_SARGE)
		return "game/mars_city1";
	else if (c <= CUTSCENE_IMP)
		return "game/mc_underground";
	else if (c <= ACTING_SARGE_VIDEO)
		return "game/mars_city2";
	else if (c <= ACTING_OVERHEAR)
		return "game/admin";
	else if (c <= CUTSCENE_ALPHALABS1)
		return "game/alphalabs1";
	else if (c <= CUTSCENE_VAGARY)
		return "game/alphalabs4";
	else if (c <= CUTSCENE_DELTA_TELEPORTER)
		return "game/delta2a";
	else if (c <= CUTSCENE_DELTA_HKINTRO)
		return "game/delta4";
	else if (c <= CUTSCENE_CAMPHUNT)
		return "game/cpu";
	else if (c <= CUTSCENE_CPU_BOSS)
		return "game/cpuboss";
	else if (c <= CUTSCENE_GRABBER)
		return "game/erebus1";
	else
		return "game/le_enpro1";
}

// Note: use the console command "teleport trigger_once_8" to skip to the Betruger meeting, and "teleport trigger_once_40" for Sergeant Kelly
void Flicksync_GoToCutscene( t_cutscene scene )
{
	if (g_debugCinematic.GetBool())
		gameLocal.Printf("%d: Flicksync_GoToCutscene(%d)\n", gameLocal.framenum, scene);
	//Flicksync_currentCutscene = scene;
	//Flicksync_skipToCutscene = CUTSCENE_NONE;

	// check we're on the correct map first
	idStr map = CutsceneToMapName(scene);
	if( map.Cmp( commonLocal.GetCurrentMapName() )!=0 )
	{
		gameLocal.sessionCommand = "map " + map;
	}

	// go to cutscene
	idPlayer *player = gameLocal.GetLocalPlayer();
	if (!player)
		return;
	idEntity *ent = NULL;
	idVec3 origin;
	idAngles angles;
	idMat3 axis;
	switch (scene)
	{
	case CUTSCENE_RECEPTION:
		ent = gameLocal.FindEntity("tim_func_door_70");
		break;
	case CUTSCENE_MEETING:
		// if they don't have a PDA, give them a PDA
		if ( player->inventory.pdas.Num() == 0 )
			player->GivePDA( NULL, NULL );
		ent = gameLocal.FindEntity("trigger_once_8");
		break;
	case CUTSCENE_SARGE:
		// if they don't have a PDA, give them a PDA
		if (player->inventory.pdas.Num() == 0)
			player->GivePDA(NULL, NULL);
		// Trigger entities on our way
		ent = gameLocal.FindEntity("trigger_once_29");
		if (ent)
		{
			origin = ent->GetPhysics()->GetOrigin();
			player->GetPhysics()->SetOrigin( origin );
			player->TouchTriggers();
		}
		ent = gameLocal.FindEntity("trigger_once_40");
		break;
	case ACTING_AIRLOCK:
		ent = gameLocal.FindEntity("trigger_once_93");
		break;
	case CUTSCENE_ISHII:
		ent = gameLocal.FindEntity("trigger_once_120");
		break;
	case CUTSCENE_IMP:
		ent = gameLocal.FindEntity("impintrotrigger");
		break;
	case CUTSCENE_ADMIN:
		ent = gameLocal.FindEntity("trigger_once_21");
		break;
	case CUTSCENE_PINKY:
		ent = gameLocal.FindEntity("tim_trigger_once_11"); // This is triggered by a GUI, not sure how to handle it.
		break;
	case CUTSCENE_ALPHALABS1:
		ent = gameLocal.FindEntity("trigger_once_12");
		break;
	case CUTSCENE_VAGARY:
		ent = gameLocal.FindEntity("func_door_438"); // triggered by a door? how to handle it?
		break;
	case CUTSCENE_CAMPHUNT: // trigger on level load
		break;
	case CUTSCENE_BLOOD:
		ent = gameLocal.FindEntity("trigger_once_56");
		break;
	case CUTSCENE_GRABBER:
		ent = gameLocal.FindEntity("trigger_once_88");
		break;
	}

	if (ent)
	{
		angles.Zero();
		angles.yaw = ent->GetPhysics()->GetAxis()[0].ToYaw();
		origin = ent->GetPhysics()->GetOrigin();
		player->Teleport( origin, angles, ent );
	}
}

t_cutscene Flicksync_GetNextCutscene()
{
	if (g_debugCinematic.GetBool())
		gameLocal.Printf("%d: Flicksync_GetNextCutscene()\n", gameLocal.framenum);
	int c = vr_flicksyncCharacter.GetInteger();
	switch (Flicksync_currentCutscene)
	{
	case FMV_UAC:
		return CUTSCENE_DARKSTAR;
	case CUTSCENE_DARKSTAR:
	case ACTING_BIOSCAN:
		if( c == FLICK_TOWER || c == FLICK_BETRUGER || c == FLICK_SWANN || c == FLICK_CAMPBELL )
			return CUTSCENE_MEETING;
		else if( c == FLICK_SARGE )
			return CUTSCENE_SARGE;
		else
			return CUTSCENE_RECEPTION;
	case CUTSCENE_RECEPTION:
		return CUTSCENE_MEETING;
	case CUTSCENE_MEETING:
	case ACTING_SUITS:
	case ACTING_KITCHEN:
	case ACTING_BEFORE_SARGE:
		return CUTSCENE_SARGE;
	case CUTSCENE_SARGE:
		if ( c == FLICK_TOWER )
			return CUTSCENE_ARTIFACT;
		else
			return CUTSCENE_ISHII;

	case ACTING_GEARUP:
	case ACTING_CRANE:
	case ACTING_HEY_YOURE_LOOKING:
	case ACTING_SCOTTY:
	case ACTING_SNEAKING:
	case ACTING_MAINTENANCE:
	case ACTING_AIRLOCK:
		return CUTSCENE_ISHII;
	case CUTSCENE_ISHII:
		if ( c == FLICK_NONE || c == FLICK_PLAYER || c == FLICK_RECEPTION || c == FLICK_SARGE )
			return CUTSCENE_IMP;
		else
			return CUTSCENE_ADMIN;
	case CUTSCENE_IMP:
	case ACTING_CEILING:
	case ACTING_SARGE_VIDEO:
		return CUTSCENE_ADMIN;

	case CUTSCENE_ADMIN:
		return CUTSCENE_PINKY;
	case CUTSCENE_PINKY:
	case ACTING_OVERHEAR:
		return CUTSCENE_ALPHALABS1;

	case CUTSCENE_ALPHALABS1:
		return CUTSCENE_VAGARY;

	case CUTSCENE_VAGARY:
	case FMV_ROE:
		return CUTSCENE_ARTIFACT;

	case CUTSCENE_ARTIFACT:
	case FMV_LOST_MISSIONS:
		return CUTSCENE_BLOOD;
	case CUTSCENE_BLOOD:
		return CUTSCENE_GRABBER;
	case CUTSCENE_GRABBER:
		return CUTSCENE_BRAVO_TEAM;

	case CUTSCENE_BRAVO_TEAM:
	default:
		return CUTSCENE_NONE;
	}
}
