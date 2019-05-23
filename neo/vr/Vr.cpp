#pragma hdrstop

#include"precompiled.h"

#undef strncmp
#undef vsnprintf		
#undef _vsnprintf		

#include "Vr.h"
#include "Voice.h"
#include "d3xp/Game_local.h"
#ifdef _WIN32
#include "sys\win32\win_local.h"
#endif
#include "d3xp/physics/Clip.h"
#ifdef USE_OVR
#include "libs\LibOVR\Include\OVR_CAPI_GL.h"
#endif
#include "../renderer/Framebuffer.h"

#define RADIANS_TO_DEGREES(rad) ((float) rad * (float) (180.0 / idMath::PI))

// *** Oculus HMD Variables

idCVar vr_pixelDensity( "vr_pixelDensity", "1.25", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "" );
idCVar vr_enable( "vr_enable", "1", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, "Enable VR mode. 0 = Disabled 1 = Enabled." );
idCVar vr_scale( "vr_scale", "1.0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "World scale. Everything virtual is this times as big." );
idCVar vr_useOculusProfile( "vr_useOculusProfile", "1", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, "Use official profile values. 0 = use user defined profile, 1 = use official profile." );
idCVar vr_manualIPDEnable( "vr_manualIPDEnable", "0", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, " Override the HMD provided IPD value with value in vr_manualIPD 0 = disable 1= use manual iPD\n" );
idCVar vr_manualIPD( "vr_manualIPD", "64", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "User defined IPD value in MM" );
idCVar vr_manualHeight( "vr_manualHeight", "70", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Unused. User defined player height in inches" );
idCVar vr_minLoadScreenTime( "vr_minLoadScreenTime", "6000", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Min time to display load screens in ms.", 0.0f, 10000.0f );
idCVar vr_useFloorHeight( "vr_useFloorHeight", "0", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, "0 = Custom eye height. 1 = Marine Eye Height. 2 = Normal View Height. 3 = make floor line up by Doomguy crouching. 4 = make everything line up by scaling world to your height.", 0, 4 );
idCVar vr_normalViewHeight( "vr_normalViewHeight", "73", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Height of player's view while standing, in real world inches." );

idCVar vr_weaponHand( "vr_weaponHand", "0", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, "Which hand holds weapon.\n 0 = Right hand\n 1 = Left Hand\n", 0, 1 );

//flashlight cvars

idCVar vr_flashlightMode( "vr_flashlightMode", "3", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, "Flashlight mount.\n0 = Body\n1 = Head\n2 = Gun\n3= Hand ( if motion controls available.)" );
idCVar vr_flashlightStrict( "vr_flashlightStrict", "0", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Flashlight location should be strictly enforced." ); // Carl

idCVar vr_flashlightBodyPosX( "vr_flashlightBodyPosX", "0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Flashlight vertical offset for body mount." );
idCVar vr_flashlightBodyPosY( "vr_flashlightBodyPosY", "0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Flashlight horizontal offset for body mount." );
idCVar vr_flashlightBodyPosZ( "vr_flashlightBodyPosZ", "0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Flashlight forward offset for body mount." );

idCVar vr_flashlightHelmetPosX( "vr_flashlightHelmetPosX", "6", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Flashlight vertical offset for helmet mount." );
idCVar vr_flashlightHelmetPosY( "vr_flashlightHelmetPosY", "-6", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Flashlight horizontal offset for helmet mount." );
idCVar vr_flashlightHelmetPosZ( "vr_flashlightHelmetPosZ", "-20", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Flashlight forward offset for helmet mount." );

idCVar vr_offHandPosX( "vr_offHandPosX", "0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "X position for off hand when not using motion controls." );
idCVar vr_offHandPosY( "vr_offHandPosY", "0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Y position for off hand when not using motion controls." );
idCVar vr_offHandPosZ( "vr_offHandPosZ", "0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Z position for off hand when not using motion controls." );

idCVar vr_forward_keyhole( "vr_forward_keyhole", "11.25", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Forward movement keyhole in deg. If view is inside body direction +/- this value, forward movement is in view direction, not body direction" );

idCVar vr_PDAfixLocation( "vr_PDAfixLocation", "0", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Fix PDA position in space in front of player\n instead of holding in hand." );

idCVar vr_weaponPivotOffsetForward( "vr_weaponPivotOffsetForward", "4", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_weaponPivotOffsetHorizontal( "vr_weaponPivotOffsetHorizontal", "-4", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_weaponPivotOffsetVertical( "vr_weaponPivotOffsetVertical", "-12", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_weaponPivotForearmLength( "vr_weaponPivotForearmLength", "16", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );

idCVar vr_guiScale( "vr_guiScale", "1", CVAR_FLOAT | CVAR_RENDERER | CVAR_ARCHIVE, "scale reduction factor for full screen menu/pda scale in VR", 0.0001f, 1.0f ); // Koz allow scaling of full screen guis/pda
idCVar vr_guiSeparation( "vr_guiSeparation", ".01", CVAR_FLOAT | CVAR_ARCHIVE, " Screen separation value for fullscreen guis." );

idCVar vr_guiMode( "vr_guiMode", "2", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "Gui interaction mode.\n 0 = Weapon aim as cursor\n 1 = Look direction as cursor\n 2 = Touch screen\n" );

idCVar vr_hudScale( "vr_hudScale", "1.0", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Hud scale", 0.1f, 2.0f );
idCVar vr_hudPosHor( "vr_hudPosHor", "0", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Hud Horizontal offset in inches" );
idCVar vr_hudPosVer( "vr_hudPosVer", "7", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Hud Vertical offset in inches" );
idCVar vr_hudPosDis( "vr_hudPosDis", "32", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Hud Distance from view in inches" );
idCVar vr_hudPosAngle( "vr_hudPosAngle", "30", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Hud View Angle" );
idCVar vr_hudPosLock( "vr_hudPosLock", "1", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "Lock Hud to:  0 = Face, 1 = Body" );


idCVar vr_hudType( "vr_hudType", "2", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "VR Hud Type. 0 = Disable.\n1 = Full\n2=Look Activate", 0, 2 );
idCVar vr_hudRevealAngle( "vr_hudRevealAngle", "48", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "HMD pitch to reveal HUD in look activate mode." );
idCVar vr_hudTransparency( "vr_hudTransparency", "1", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, " Hud transparency. 0.0 = Invisible thru 1.0 = full", 0.0, 100.0 );
idCVar vr_hudOcclusion( "vr_hudOcclusion", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, " Hud occlusion. 0 = Objects occlude HUD, 1 = No occlusion " );
idCVar vr_hudHealth( "vr_hudHealth", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show Armor/Health in Hud." );
idCVar vr_hudAmmo( "vr_hudAmmo", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show Ammo in Hud." );
idCVar vr_hudPickUps( "vr_hudPickUps", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show item pick ups in Hud." );
idCVar vr_hudTips( "vr_hudTips", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show tips Hud." );
idCVar vr_hudLocation( "vr_hudLocation", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show player location in Hud." );
idCVar vr_hudObjective( "vr_hudObjective", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show objectives in Hud." );
idCVar vr_hudStamina( "vr_hudStamina", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show stamina in Hud." );
idCVar vr_hudPills( "vr_hudPills", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show weapon pills in Hud." );
idCVar vr_hudComs( "vr_hudComs", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show communications in Hud." );
idCVar vr_hudWeap( "vr_hudWeap", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show weapon pickup/change icons in Hud." );
idCVar vr_hudNewItems( "vr_hudNewItems", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show new items acquired in Hud." );
idCVar vr_hudFlashlight( "vr_hudFlashlight", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show flashlight in Hud." );
idCVar vr_hudLowHealth( "vr_hudLowHealth", "20", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, " 0 = Disable, otherwise force hud if heath below this value." );

idCVar vr_voiceRepeat( "vr_voiceRepeat", "0", CVAR_BOOL, "1 = computer speaks back whatever commands or lines you say" );
idCVar vr_voiceMinVolume( "vr_voiceMinVolume", "2", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "Minimum volume required to recognise voice commands. Out of 100. Set this higher if background sounds trigger voice commands.", 0, 100 );
idCVar vr_voiceCommands( "vr_voiceCommands", "2", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "Enable voice commands. 0 = none, 1 = menus, 2 = menus and weapons", 0, 2 );
idCVar vr_voicePushToTalk( "vr_voicePushToTalk", "0", CVAR_INTEGER | CVAR_ARCHIVE, "'Push to Talk' button must be pressed before voice commands recognized\n 0 = disabled, 1 = enabled \n", 0, 1 );
idCVar vr_talkWakeMonsters( "vr_talkWakeMonsters", "1", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "Talking wakes monsters. 0 = no, 1 = both methods, 2 = like flashlight, 3 = like weapon", 0, 3 );
idCVar vr_talkWakeMonsterRadius( "vr_talkWakeMonsterRadius", "120", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Radius in inches within which talking quietly can wake enemies. Talking louder wakes monsters further than this." );
idCVar vr_talkMode( "vr_talkMode", "2", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "Talk to NPC 0 = buttons, 1 = buttons or voice, 2 = voice only, 3 = voice no cursor", 0, 3 );
idCVar vr_tweakTalkCursor( "vr_tweakTalkCursor", "25", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Tweak talk cursor y pos in VR. % val", 0, 99 );

idCVar vr_wristStatMon( "vr_wristStatMon", "1", CVAR_INTEGER | CVAR_ARCHIVE, "Use wrist status monitor. 0 = Disable 1 = Right Wrist 2 = Left Wrist " );

// Koz display windows monitor name in the resolution selection menu, helpful to ID which is the rift if using extended mode
idCVar vr_listMonitorName( "vr_listMonitorName", "0", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "List monitor name with resolution." );

//idCVar vr_viewModelArms( "vr_viewModelArms", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, " Dont change this, will be removed. Display arms on view models in VR" );
idCVar vr_disableWeaponAnimation( "vr_disableWeaponAnimation", "1", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Disable weapon animations in VR. ( 1 = disabled )" );
idCVar vr_headKick( "vr_headKick", "0", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Damage can 'kick' the players view. 0 = Disabled in VR." );
//idCVar vr_showBody( "vr_showBody", "1", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Dont change this! Will be removed shortly, modifying will cause the player to have extra hands." );
idCVar vr_joystickMenuMapping( "vr_joystickMenuMapping", "1", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, " Use alternate joy mapping\n in menus/PDA.\n 0 = D3 Standard\n 1 = VR Mode.\n(Both joys can nav menus,\n joy r/l to change\nselect area in PDA." );


idCVar	vr_deadzonePitch( "vr_deadzonePitch", "90", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Vertical Aim Deadzone", 0, 180 );
idCVar	vr_deadzoneYaw( "vr_deadzoneYaw", "30", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Horizontal Aim Deadzone", 0, 180 );
idCVar	vr_comfortDelta( "vr_comfortDelta", "10", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Comfort Mode turning angle ", 0, 180 );

//idCVar	vr_interactiveCinematic( "vr_interactiveCinematic", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Interactive cinematics in VR ( no camera )" );

idCVar	vr_headingBeamMode( "vr_headingBeamMode", "3", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "0 = disabled, 1 = solid, 2 = arrows, 3 = scrolling arrows" );

idCVar	vr_weaponSight( "vr_weaponSight", "0", CVAR_INTEGER | CVAR_ARCHIVE, "Weapon Sight.\n 0 = Lasersight\n 1 = Red dot\n 2 = Circle dot\n 3 = Crosshair\n 4 = Beam + Dot\n" );
idCVar	vr_weaponSightToSurface( "vr_weaponSightToSurface", "1", CVAR_INTEGER | CVAR_ARCHIVE, "Map sight to surface. 0 = Disabled 1 = Enabled\n" );

idCVar	vr_motionWeaponPitchAdj( "vr_motionWeaponPitchAdj", "40", CVAR_FLOAT | CVAR_ARCHIVE, "Weapon controller pitch adjust" );
idCVar	vr_motionFlashPitchAdj( "vr_motionFlashPitchAdj", "40", CVAR_FLOAT | CVAR_ARCHIVE, "Flashlight controller pitch adjust" );

idCVar	vr_nodalX( "vr_nodalX", "-3", CVAR_FLOAT | CVAR_ARCHIVE, "Forward offset from eyes to neck" );
idCVar	vr_nodalZ( "vr_nodalZ", "-6", CVAR_FLOAT | CVAR_ARCHIVE, "Vertical offset from neck to eye height" );


idCVar vr_vcx( "vr_vcx", "-3.5", CVAR_FLOAT | CVAR_ARCHIVE, "Controller X offset to handle center" ); // these values work for steam
idCVar vr_vcy( "vr_vcy", "0", CVAR_FLOAT | CVAR_ARCHIVE, "Controller Y offset to handle center" );
idCVar vr_vcz( "vr_vcz", "-.5", CVAR_FLOAT | CVAR_ARCHIVE, "Controller Z offset to handle center" );

idCVar vr_mountx( "vr_mountx", "0", CVAR_FLOAT | CVAR_ARCHIVE, "If motion controller mounted on object, X offset from controller to object handle.\n (Eg controller mounted on Topshot)" );
idCVar vr_mounty( "vr_mounty", "0", CVAR_FLOAT | CVAR_ARCHIVE, "If motion controller mounted on object, Y offset from controller to object handle.\n (Eg controller mounted on Topshot)" );
idCVar vr_mountz( "vr_mountz", "0", CVAR_FLOAT | CVAR_ARCHIVE, "If motion controller mounted on object, Z offset from controller to object handle.\n (Eg controller mounted on Topshot)" );

idCVar vr_mountedWeaponController( "vr_mountedWeaponController", "0", CVAR_BOOL | CVAR_ARCHIVE, "If physical controller mounted on object (eg topshot), enable this to apply mounting offsets\n0=disabled 1 = enabled" );

idCVar vr_3dgui( "vr_3dgui", "1", CVAR_BOOL | CVAR_ARCHIVE, "3d effects for in game guis. 0 = disabled 1 = enabled\n" );
idCVar vr_shakeAmplitude( "vr_shakeAmplitude", "1.0", CVAR_FLOAT | CVAR_ARCHIVE, "Screen shake amplitude 0.0 = disabled to 1.0 = full\n", 0.0f, 1.0f );


idCVar vr_controllerStandard( "vr_controllerStandard", "0", CVAR_INTEGER | CVAR_ARCHIVE, "If 1, use standard controller, not motion controllers\nRestart after changing\n" );

idCVar vr_padDeadzone("vr_padDeadzone", ".25", CVAR_FLOAT | CVAR_ARCHIVE, "Deadzone for steam pads.\n 0.0 = no deadzone 1.0 = dead\n");
idCVar vr_jsDeadzone("vr_jsDeadzone", ".25", CVAR_FLOAT | CVAR_ARCHIVE, "Deadzone for steam joysticks.\n 0.0 = no deadzone 1.0 = dead\n");
idCVar vr_padToButtonThreshold( "vr_padToButtonThreshold", ".7", CVAR_FLOAT | CVAR_ARCHIVE, "Threshold value for pad contact\n to register as button press\n .1 high sensitiveity thru\n .99 low sensitivity" );
idCVar vr_knockBack( "vr_knockBack", "0", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Enable damage knockback in VR. 0 = Disabled, 1 = Enabled" );
idCVar vr_jumpBounce( "vr_jumpBounce", "0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Enable view bounce after jumping. 0 = Disabled, 1 = Full", 0.0f, 1.0f ); // Carl
idCVar vr_stepSmooth( "vr_stepSmooth", "1", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Enable smoothing when climbing stairs. 0 = Disabled, 1 = Full", 0.0f, 1.0f ); // Carl
idCVar vr_walkSpeedAdjust( "vr_walkSpeedAdjust", "-20", CVAR_FLOAT | CVAR_ARCHIVE, "Player walk speed adjustment in VR. (slow down default movement)" );

idCVar vr_wipPeriodMin( "vr_wipPeriodMin", "10.0", CVAR_FLOAT | CVAR_ARCHIVE, "" );
idCVar vr_wipPeriodMax( "vr_wipPeriodMax", "2000.0", CVAR_FLOAT | CVAR_ARCHIVE, "" );

idCVar vr_wipVelocityMin( "vr_wipVelocityMin", ".05", CVAR_FLOAT | CVAR_ARCHIVE, "" );
idCVar vr_wipVelocityMax( "vr_wipVelocityMax", "2.0", CVAR_FLOAT | CVAR_ARCHIVE, "" );

idCVar vr_headbbox( "vr_headbbox", "10.0", CVAR_FLOAT | CVAR_ARCHIVE, "" );

idCVar vr_pdaPosX( "vr_pdaPosX", "20", CVAR_FLOAT | CVAR_ARCHIVE, "" );
idCVar vr_pdaPosY( "vr_pdaPosY", "0", CVAR_FLOAT | CVAR_ARCHIVE, "" );
idCVar vr_pdaPosZ( "vr_pdaPosZ", "-11", CVAR_FLOAT | CVAR_ARCHIVE, "" );

idCVar vr_pdaPitch( "vr_pdaPitch", "30", CVAR_FLOAT | CVAR_ARCHIVE, "" );

idCVar vr_movePoint( "vr_movePoint", "4", CVAR_INTEGER | CVAR_ARCHIVE, "0: Standard Stick Move, 1: Off Hand = Forward, 2: Look = forward, 3: Weapon Hand = Forward, 4: Left Hand = Forward, 5: Right Hand = Forward", 0, 5 );
idCVar vr_moveClick( "vr_moveClick", "0", CVAR_INTEGER | CVAR_ARCHIVE, " 0 = Normal movement.\n 1 = Click and hold to walk, run button to run.\n 2 = Click to start walking, then touch only. Run btn to run.\n 3 = Click to start walking, hold click to run.\n 4 = Click to start walking, then click toggles run\n" );
idCVar vr_playerBodyMode( "vr_playerBodyMode", "0", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "Player body mode:\n0 = Display full body\n1 = Just Hands \n2 = Weapons only\n" );
idCVar vr_bodyToMove( "vr_bodyToMove", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Lock body orientaion to movement direction." );
idCVar vr_moveThirdPerson( "vr_moveThirdPerson", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Artifical movement will user 3rd person perspective." );

idCVar vr_crouchMode( "vr_crouchMode", "0", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "Crouch Mode:\n 0 = Full motion crouch (In game matches real life)\n 1 = Crouch anim triggered by smaller movement." );
idCVar vr_crouchTriggerDist( "vr_crouchTriggerDist", "7", CVAR_FLOAT | CVAR_ARCHIVE, " Distance ( in real-world inches ) player must crouch in real life to toggle crouch\n" );
idCVar vr_crouchHideBody( "vr_crouchHideBody", "0", CVAR_FLOAT | CVAR_ARCHIVE, "Hide body ( if displayed )  when crouching. 0 = Dont hide, 1 = hide." );
idCVar vr_frameCheck( "vr_frameCheck", "0", CVAR_INTEGER | CVAR_ARCHIVE, "0 = bypass frame check" );

idCVar vr_forceOculusAudio( "vr_forceOculusAudio", "1", CVAR_BOOL | CVAR_ARCHIVE, "Request openAL to search for Rift headphones instead of default device\nFails to default device if rift not found." );
idCVar vr_stereoMirror( "vr_stereoMirror", "1", CVAR_BOOL | CVAR_ARCHIVE, "Render mirror window with stereo views. 0 = Mono , 1 = Stereo Warped" );

idCVar vr_APISelect( "vr_APISelect", "0", CVAR_INTEGER | CVAR_ARCHIVE, "VR API Select:\n 0 = Autodetect ( Oculus Native then OpenVR ) ,\n 1 = Oculus Native Only\n 2 = OpenVR only\n" );


idCVar vr_teleportSkipHandrails( "vr_teleportSkipHandrails", "0", CVAR_INTEGER | CVAR_ARCHIVE, "Teleport aim ingnores handrails. 1 = true" );
idCVar vr_teleportShowAimAssist( "vr_teleportShowAimAssist", "0", CVAR_INTEGER | CVAR_ARCHIVE, "Move telepad target to reflect aim assist. 1 = true" );
idCVar vr_teleportButtonMode( "vr_teleportButtonMode", "0", CVAR_BOOL | CVAR_ARCHIVE, "0 = Press aim, release teleport.\n1 = 1st press aim, 2nd press teleport" );
idCVar vr_teleportHint( "vr_teleportHint", "0", CVAR_BOOL | CVAR_ARCHIVE, "" ); // Koz blech hack - used for now to keep track if the game has issued the player the hint about ducking when the teleport target is red.

idCVar vr_useHandPoses( "vr_useHandPoses", "0", CVAR_BOOL | CVAR_ARCHIVE, "If using oculus touch, enable finger poses when hands are empty or in guis" );
// Koz end
// Carl
idCVar vr_teleport( "vr_teleport", "2", CVAR_INTEGER | CVAR_ARCHIVE, "Player can teleport at will. 0 = disabled, 1 = gun sight, 2 = right hand, 3 = left hand, 4 = head", 0, 4 );
idCVar vr_teleportMode("vr_teleportMode", "0", CVAR_INTEGER | CVAR_ARCHIVE, "Teleport Mode. 0 = Blink (default), 1 = Doom VFR style (slow time and warp speed), 2 = Doom VFR style + jet strafe)", 0, 2);
idCVar vr_teleportMaxTravel( "vr_teleportMaxTravel", "950", CVAR_INTEGER | CVAR_ARCHIVE, "Maximum teleport path length/complexity/time. About 250 or 500 are good choices, but must be >= about 950 to use tightrope in MC Underground.", 150, 5000 );
idCVar vr_teleportThroughDoors( "vr_teleportThroughDoors", "0", CVAR_BOOL | CVAR_ARCHIVE, "Player can teleport somewhere visible even if the path to get there takes them through closed (but not locked) doors." );
idCVar vr_motionSickness( "vr_motionSickness", "10", CVAR_INTEGER | CVAR_ARCHIVE, "Motion sickness prevention aids. 0 = None, 1 = Chaperone, 2 = Reduce FOV, 3 = Black Screen, 4 = Black & Chaperone, 5 = Reduce FOV & Chaperone, 6 = Slow Mo, 7 = Slow Mo & Chaperone, 8 = Slow Mo & Reduce FOV, 9 = Slow Mo, Chaperone, Reduce FOV, 10 = Third Person, 11 = Particles, 12 = Particles & Chaperone", 0, 12 );

idCVar vr_strobeTime( "vr_strobeTime", "500", CVAR_INTEGER | CVAR_ARCHIVE, "Time in ms between flashes when blacking screen. 0 = no strobe" );
idCVar vr_chaperone( "vr_chaperone", "2", CVAR_INTEGER | CVAR_ARCHIVE, "Chaperone/Guardian mode. 0 = when near, 1 = when throwing, 2 = when melee, 3 = when dodging, 4 = always", 0, 4 );
idCVar vr_chaperoneColor( "vr_chaperoneColor", "0", CVAR_INTEGER | CVAR_ARCHIVE, "Chaperone colour. 0 = default, 1 = black, 2 = grey, 3 = white, 4 = red, 5 = green, 6 = blue, 7 = yellow, 8 = cyan, 9 = magenta, 10 = purple", 0, 10 );

idCVar vr_handSwapsAnalogs( "vr_handSwapsAnalogs", "0", CVAR_BOOL | CVAR_ARCHIVE, "Should swapping the weapon hand affect analog controls (stick or touchpad) or just buttons/triggers? 0 = only swap buttons, 1 = swap all controls" );
idCVar vr_autoSwitchControllers( "vr_autoSwitchControllers", "1", CVAR_BOOL | CVAR_ARCHIVE, "Automatically switch to/from gamepad mode when using gamepad/motion controller. Should be true unless you're trying to use both together, or you get false detections. 0 = no, 1 = yes." );

idCVar vr_cinematics("vr_cinematics", "0", CVAR_INTEGER | CVAR_ARCHIVE, "Cinematic type. 0 = Immersive, 1 = Cropped, 2 = Projected");

idCVar vr_instantAccel( "vr_instantAccel", "1", CVAR_BOOL | CVAR_ARCHIVE, "Instant Movement Acceleration. 0 = Disabled 1 = Enabled" );
idCVar vr_shotgunChoke( "vr_shotgunChoke", "0", CVAR_FLOAT | CVAR_ARCHIVE, "% To choke shotgun. 0 = None, 100 = Full Choke\n" );
idCVar vr_headshotMultiplier( "vr_headshotMultiplier", "2.5", CVAR_FLOAT | CVAR_ARCHIVE, "Damage multiplier for headshots when using Fists,Pistol,Shotgun,Chaingun or Plasmagun.", 1, 5 );

// Carl
idCVar vr_weaponCycleMode( "vr_weaponCycleMode", "0", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "When cycling through weapons\n0 = skip holstered weapons, 1 = include holstered weapons, 2 = flashlight but not holstered, 3 = holstered+flashlight, 4 = holstered+flashlight+pda" );
idCVar vr_gripMode( "vr_gripMode", "0", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "How the grip button works\n0 = context sensitive toggle, 1 = context sensitive toggle no surface, 2 = toggle for weapons/items hold for physics objects, 3 = toggle for weapons hold for physics/items, 4 = always toggle (can drop), 5 = Dead and Burried, 6 = hold to hold, 7 = hold to hold squeeze for action" );
idCVar vr_doubleClickGrip( "vr_doubleClickGrip", "0", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "Double-clicking grip 0 = does nothing, 1 = drops or does action (depending on grip mode). Not implemented!" );
idCVar vr_pickUpMode( "vr_pickUpMode", "1", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "How to pick up/collect/use items and powerups 0 = walk over, 1 = walk/touch, 2 = touch, 3 = manual grip, 4 = put in body, 5 = put in properly, 6 = hold and press trigger" );
idCVar vr_reloadMode( "vr_reloadMode", "0", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "How to reload your weapon\n0 = button, 1 = with other hand, 2 = with other empty hand, 3 = Dead and Burried" );
idCVar vr_mustEmptyHands( "vr_mustEmptyHands", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Do you need to have an empty hand to interact with things?\n0 = no, it works automatically; 1 = yes, your hand must be empty" );
idCVar vr_contextSensitive( "vr_contextSensitive", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Are buttons context sensitive?\n0 = no just map the buttons in the binding window, 1 = yes, context sensitive buttons (default)" );
idCVar vr_dualWield( "vr_dualWield", "8", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "Can you use two weapons at once?\n0 = not even fists, 1 = nothing, 2 = only flashlight, 3 = only grenades (VFR), 4 = only grenades/flashlight, 5 = only pistols, 6 = only pistols/flashlight, 7 = only pistols/grenades/flashlight, 8 = yes" );
idCVar vr_voiceMicLocation( "vr_voiceMicLocation", "0", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "Where is the virtual VR microphone you need to hold up to your mouth to speak into for voice commands to work?\n0 = helmet (always works), 1 = StatWatch, 2 = left hand, 3 = right hand, 4 = either hand, 5 = push-to-talk hand, 6 = weapon, 7 = flashlight, 8 = PDA" );
idCVar vr_debugHands( "vr_debugHands", "0", CVAR_BOOL | CVAR_GAME, "Enable hand/weapon/dual wielding debugging" );
idCVar vr_rumbleChainsaw( "vr_rumbleChainsaw", "0", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Enable weapon (currently chainsaw only) constant haptic feedback in VR. Not recommended for wireless VR controllers." );

//===================================================================

int fboWidth;
int fboHeight;

iVr vrCom;
iVr* commonVr = &vrCom;

iVoice _voice; //avoid nameclash with timidity
iVoice* commonVoice = &_voice;

void SwapBinding(int Old, int New)
{
	idStr s = idKeyInput::GetBinding(New);
	idKeyInput::SetBinding(New, idKeyInput::GetBinding(Old));
	idKeyInput::SetBinding(Old, s.c_str());
}

void SwapWeaponHand()
{
	vr_weaponHand.SetInteger(1 - vr_weaponHand.GetInteger());
	// swap teleport hand
	if (vr_teleport.GetInteger() == 2)
		vr_teleport.SetInteger( 3 );
	else if (vr_teleport.GetInteger() == 3)
		vr_teleport.SetInteger( 2 );
	// swap motion controller bindings to other hand
	for (int k = K_JOY17; k <= K_JOY18; k++)
		SwapBinding(k, k + 7);
	// JOY19 is the Touch menu button, which only exists on the left hand
	for (int k = K_JOY20; k <= K_JOY23; k++)
		SwapBinding(k, k + 7);
	for (int k = K_JOY31; k <= K_JOY48; k++)
		SwapBinding(k, k + 18);
	SwapBinding(K_L_TOUCHTRIG, K_R_TOUCHTRIG);
	SwapBinding(K_L_STEAMVRTRIG, K_R_STEAMVRTRIG);
	if (vr_handSwapsAnalogs.GetBool())
	{
		for (int k = K_TOUCH_LEFT_STICK_UP; k <= K_TOUCH_LEFT_STICK_RIGHT; k++)
			SwapBinding(k, k + 4);
		for (int k = K_STEAMVR_LEFT_PAD_UP; k <= K_STEAMVR_LEFT_PAD_RIGHT; k++)
			SwapBinding(k, k + 4);
		for (int k = K_STEAMVR_LEFT_JS_UP; k <= K_STEAMVR_LEFT_JS_RIGHT; k++)
			SwapBinding(k, k + 4);
	}
}

/*
====================
R_MakeFBOImage
// Koz deleteme, using renderbuffers instead of textures.
====================
*/
static void R_MakeFBOImage( idImage* image )
{
	idImageOpts	opts;
	opts.width = fboWidth;
	opts.height = fboHeight;
	opts.numLevels = 1;
	opts.format = FMT_RGBA8;
	image->AllocImage( opts, TF_LINEAR, TR_CLAMP );
}

/*
==============
iVr::iVr()
==============
*/
iVr::iVr()
{
	hasHMD = false;
	hasOculusRift = false;
	VR_GAME_PAUSED = false;
	PDAforcetoggle = false;
	PDAforced = false;
	PDArising = false;
	gameSavingLoading = false;
	showingIntroVideo = true;
	forceLeftStick = true;	// start the PDA in the left menu.
	pdaToggleTime = Sys_Milliseconds();
	lastSaveTime = Sys_Milliseconds();
	wasSaved = false;
	wasLoaded = false;
	shouldRecenter = false;

	PDAclipModelSet = false;
	useFBO = false;
	VR_USE_MOTION_CONTROLS = 0;

	scanningPDA = false;

	vrIsBackgroundSaving = false;

	VRScreenSeparation = 0.0f;

	officialIPD = 64.0f;
	officialHeight = 72.0f;

	manualIPD = 64.0f;
	manualHeight = 72.0f;

	hmdPositionTracked = false;

	vrFrameNumber = 0;
	lastPostFrame = 0;


	lastViewOrigin = vec3_zero;
	lastViewAxis = mat3_identity;

	lastCenterEyeOrigin = vec3_zero;
	lastCenterEyeAxis = mat3_identity;

	bodyYawOffset = 0.0f;
	lastHMDYaw = 0.0f;
	lastHMDPitch = 0.0f;
	lastHMDRoll = 0.0f;
	lastHMDViewOrigin = vec3_zero;
	lastHMDViewAxis = mat3_identity;
	headHeightDiff = 0;

	motionMoveDelta = vec3_zero;
	motionMoveVelocity = vec3_zero;
	leanOffset = vec3_zero;
	leanBlankOffset = vec3_zero;
	leanBlankOffsetLengthSqr = 0.0f;
	leanBlank = false;
	isLeaning = false;
	
	thirdPersonMovement = false;
	thirdPersonDelta = 0.0f;
	thirdPersonHudAxis = mat3_identity;
	thirdPersonHudPos = vec3_zero;
	
	chestDefaultDefined = false;

	currentFlashlightPosition = FLASHLIGHT_BODY;

	handInGui = false;

	handRoll[0] = 0.0f;
	handRoll[1] = 0.0f;

	fingerPose[0] = 0;
	fingerPose[1] = 0;
	
	angles[3] = { 0.0f };

	swfRenderMode = RENDERING_NORMAL;

	isWalking = false;

	forceRun = false;

	hmdBodyTranslation = vec3_zero;

	VR_AAmode = 0;

	independentWeaponYaw = 0;
	independentWeaponPitch = 0;

	playerDead = false;

	hmdWidth = 0;
	hmdHeight = 0;

	primaryFBOWidth = 0;
	primaryFBOHeight = 0;
	hmdHz = 90;

	hmdFovX = 0.0f;
	hmdFovY = 0.0f;

	hmdPixelScale = 1.0f;
	hmdAspect = 1.0f;

#ifdef USE_OVR
	hmdSession = nullptr;
	ovrLuid.Reserved[0] = { 0 };

	oculusSwapChain[0] = nullptr;
	oculusSwapChain[1] = nullptr;
	oculusFboId = 0;
	ocululsDepthTexID = 0;
	oculusMirrorFboId = 0;
	oculusMirrorTexture = 0;
	mirrorTexId = 0;
	mirrorW = 0;
	mirrorH = 0;
#endif


	hmdEyeImage[0] = 0;
	hmdEyeImage[1] = 0;
	hmdCurrentRender[0] = 0;
	hmdCurrentRender[1] = 0;

	// wip stuff
	// wip stuff
	wipNumSteps = 0;
	wipStepState = 0;
	wipLastPeriod = 0;
	wipCurrentDelta = 0.0f;
	wipCurrentVelocity = 0.0f;

	wipTotalDelta = 0.0f;
	wipLastAcces = 0.0f;
	wipAvgPeriod = 0.0f;
	wipTotalDeltaAvg = 0.0f;

	hmdFrameTime = 0;
	
	lastRead = 0;
	currentRead = 0;
	updateScreen = false;

	motionControlType = MOTION_NONE;

	bodyMoveAng = 0.0f;
	teleportButtonCount = 0;


	currentFlashlightMode = vr_flashlightMode.GetInteger();
	renderingSplash = true;

	currentBindingDisplay = "";

	cinematicStartViewYaw = 0.0f;
	cinematicStartPosition = vec3_zero;
	
	didTeleport = false;
	teleportDir = 0.0f;

	currentHandWorldPosition[0] = vec3_zero;
	currentHandWorldPosition[1] = vec3_zero;


	
}

idMat4 ConvertSteamVRMatrixToidMat4( const vr::HmdMatrix34_t &matPose )
{
	idMat4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], matPose.m[3][0],
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], matPose.m[3][1],
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], matPose.m[3][2],
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
		);
	return matrixObj;
}

//==============
// Purpose: Helper to get a string from a tracked device property and turn it
//			into an idStr
//==============
idStr GetTrackedDeviceString( vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL )
{
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty( unDevice, prop, NULL, 0, peError );
	if ( unRequiredBufferLen == 0 )
		return "";

	char *pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty( unDevice, prop, pchBuffer, unRequiredBufferLen, peError );
	idStr sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}

//==============
// Purpose:
//==============

idMat4 GetHMDMatrixProjectionEye( vr::Hmd_Eye nEye )
{
	if ( !commonVr->m_pHMD )
		return mat4_default;

	float m_fNearClip = 0.1f;
	float m_fFarClip = 30.0f;
	
	vr::HmdMatrix44_t mat = commonVr->m_pHMD->GetProjectionMatrix( nEye, m_fNearClip, m_fFarClip ); // , vr::API_OpenGL );

	return idMat4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
		);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
idMat4 GetHMDMatrixPoseEye( vr::Hmd_Eye nEye )
{
	if ( !commonVr->m_pHMD )
		return  mat4_default;

	vr::HmdMatrix34_t matEyeRight = commonVr->m_pHMD->GetEyeToHeadTransform( nEye );
	idMat4 matrixObj(
		matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
		matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
		matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
		matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
		);

	return matrixObj.Inverse(); 
}


/*
==============
iVr::OculusInit
==============
*/

bool iVr::OculusInit( void )
{
#ifndef USE_OVR
	hasOculusRift = false;
	return false;
#else

	if ( vr_APISelect.GetInteger() == 2 ) // only use OpenVr;
	{
		common->Printf( "OculusInit: vr_vrAPISelect set to only use OpenVR API. Returning false." );
		hasOculusRift = false;
		return false;
	}
	
	
	hasOculusRift = false;
	// Oculus HMD Initialization
	ovrResult result = ovr_Initialize( nullptr );

	if ( OVR_FAILURE( result ) )
	{
		common->Printf( "\nOculus Rift not detected.\n" );
		return false;
	}


	common->Printf( "ovr_Initialize was successful.\n" );

	ovr_IdentifyClient( "EngineName: id Tech 4\n"
		"EngineVersion: 1.0.3\n"
		"EngineEditor: false" );

	result = ovr_Create( &hmdSession, &ovrLuid );

	if ( OVR_FAILURE( result ) )
	{
		common->Printf( "\nFailed to initialize Oculus Rift.\n" );
		ovr_Shutdown();
		return false;
	}
	
	hmdDesc = ovr_GetHmdDesc( hmdSession );
	
	//ovrSizei resoultion = hmdDesc.Resolution;

	common->Printf( "\n\nOculus Rift HMD Initialized\n" );
	//ovr_RecenterPose( hmdSession ); // lets start looking forward.
	
	if ( vr_useFloorHeight.GetInteger() >= 3 )
	{
		ovr_SetTrackingOriginType( hmdSession, ovrTrackingOrigin_FloorLevel );
	}
	else
	{
		ovr_SetTrackingOriginType( hmdSession, ovrTrackingOrigin_EyeLevel );
	}

	ovr_RecenterTrackingOrigin( hmdSession );
	hmdWidth = hmdDesc.Resolution.w;
	hmdHeight = hmdDesc.Resolution.h;
	hmdHz = hmdDesc.DisplayRefreshRate + 0.5f; // Carl: This was 89 because we were rounding down when converting to int
	com_engineHz.SetInteger( hmdHz );
	common->Printf( "Hmd: %s .\n", hmdDesc.ProductName );
	common->Printf( "Hmd HZ %d, width %d, height %d\n", hmdHz, hmdWidth, hmdHeight );
	
	ovr_GetAudioDeviceOutGuid( &oculusGuid );
	ovr_GetAudioDeviceOutGuidStr( oculusGuidStr );

	common->Printf( "Oculus sound guid " );
	for ( int c = 0; c < 128; c++ )
	{
		common->Printf( "%c", oculusGuidStr[c] );
	}
	common->Printf( "Oculus sound guid\n" );
	/*
	vr::TrackedDeviceIndex_t deviceLeft,deviceRight;

	deviceLeft = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole( vr::TrackedControllerRole_LeftHand );
	deviceRight = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole( vr::TrackedControllerRole_RightHand );

	common->Printf( "Left Controller %d right Controller %d\n", deviceLeft, deviceRight );

	if ( deviceLeft != -1 || deviceRight != -1  )
	{
	common->Printf( "Tracked controllers detected. MOTION CONTROLS ENABLED\n" );
	VR_USE_MOTION_CONTROLS = true;
	}
	else
	{
	VR_USE_MOTION_CONTROLS = false;
	}
	*/

	VR_USE_MOTION_CONTROLS = false;

	unsigned int ctrlrs = ovr_GetConnectedControllerTypes( hmdSession );
	if( ( ctrlrs & ovrControllerType_Touch ) != 0 )
	{
		VR_USE_MOTION_CONTROLS = true;
		motionControlType = MOTION_OCULUS;
	}

	// Carl: TODO
	VRScreenSeparation = 0.0f;

	hasOculusRift = true;
	return true;
#endif
}

/*
==============
iVr::OpenVRInit
==============
*/

bool iVr::OpenVRInit(void)
{
	
	if ( vr_APISelect.GetInteger() == 1 ) // Only use Oculus API
	{
		common->Printf( "OpenVRInit: vr_vrAPISelect set to only use Oculus API. Returning false." );
		return false;
	}
	
	
	
	
	if (!vr::VR_IsHmdPresent())
	{
		common->Printf("No OpenVR HMD detected.\n");
		return false;
	}

	vr::EVRInitError eError = vr::VRInitError_None;
	m_pHMD = vr::VR_Init( &eError, vr::VRApplication_Scene );

	if ( eError != vr::VRInitError_None )
	{
		m_pHMD = NULL;
		common->Printf( "\n Unable to init SteamVR runtime.\n" );
		return false;
	}

	m_pRenderModels = (vr::IVRRenderModels *)vr::VR_GetGenericInterface( vr::IVRRenderModels_Version, &eError );

	if ( !m_pRenderModels )
	{
		m_pHMD = NULL;
		vr::VR_Shutdown();

		common->Printf( " Unable to get render model interface: %s\n", vr::VR_GetVRInitErrorAsEnglishDescription( eError ) );
		return false;
	}

	if ( !vr::VRCompositor() )
	{
		common->Printf( "Compositor initialization failed. See log file for details\n" );
		return false;
	}

	vr::VRCompositor()->SetTrackingSpace( vr::TrackingUniverseStanding );

	motionControlType = MOTION_STEAMVR;


	/*
	vr::TrackedDeviceIndex_t deviceLeft,deviceRight;

	deviceLeft = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole( vr::TrackedControllerRole_LeftHand );
	deviceRight = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole( vr::TrackedControllerRole_RightHand );

	common->Printf( "Left Controller %d right Controller %d\n", deviceLeft, deviceRight );

	if ( deviceLeft != -1 || deviceRight != -1  )
	{
	common->Printf( "Tracked controllers detected. MOTION CONTROLS ENABLED\n" );
	VR_USE_MOTION_CONTROLS = true;
	}
	else
	{
	VR_USE_MOTION_CONTROLS = false;
	}
	*/

	VR_USE_MOTION_CONTROLS = true;

	common->Printf( "Getting driver info\n" );
	m_strDriver = "No Driver";
	m_strDisplay = "No Display";

	m_strDriver = GetTrackedDeviceString( m_pHMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String );
	m_strDisplay = GetTrackedDeviceString( m_pHMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String );

	// get this here so we have a resolution starting point for gl initialization.
	m_pHMD->GetRecommendedRenderTargetSize( &hmdWidth, &hmdHeight );

	commonVr->hmdHz = (int)(m_pHMD->GetFloatTrackedDeviceProperty( vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float ) + 0.5f);

	officialIPD = m_pHMD->GetFloatTrackedDeviceProperty( vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_UserIpdMeters_Float ) * 100;

	// Leyland's code, used for Reduce FOV motion sickness fix
	float				openVRfovEye[2][4];
	m_pHMD->GetProjectionRaw(vr::Eye_Left,
		&openVRfovEye[1][0], &openVRfovEye[1][1],
		&openVRfovEye[1][2], &openVRfovEye[1][3]);

	m_pHMD->GetProjectionRaw(vr::Eye_Right,
		&openVRfovEye[0][0], &openVRfovEye[0][1],
		&openVRfovEye[0][2], &openVRfovEye[0][3]);

	VRScreenSeparation = 
		0.5f * (openVRfovEye[1][1] + openVRfovEye[1][0])
		/ (openVRfovEye[1][1] - openVRfovEye[1][0])
		- 0.5f * (openVRfovEye[0][1] + openVRfovEye[0][0])
		/ (openVRfovEye[0][1] - openVRfovEye[0][0]);

	VRScreenSeparation = fabs( VRScreenSeparation ) / 2.0f ;
	com_engineHz.SetInteger( commonVr->hmdHz );

	common->Printf( "Hmd Driver: %s .\n", m_strDriver.c_str() );
	common->Printf( "Hmd Display: %s .\n", m_strDisplay.c_str() );
	common->Printf( "Hmd HZ %d, width %d, height %d\n", commonVr->hmdHz, hmdWidth, hmdHeight );
	common->Printf( "Hmd reported IPD in centimeters = %f \n", officialIPD );

	common->Printf( "HMD Left Eye leftTan %f\n", openVRfovEye[1][0] );
	common->Printf( "HMD Left Eye rightTan %f\n", openVRfovEye[1][1] );
	common->Printf( "HMD Left Eye upTan %f\n", openVRfovEye[1][2] );
	common->Printf( "HMD Left Eye downTan %f\n", openVRfovEye[1][3] );

	common->Printf( "HMD Right Eye leftTan %f\n", openVRfovEye[0][0] );
	common->Printf( "HMD Right Eye rightTan %f\n", openVRfovEye[0][1] );
	common->Printf( "HMD Right Eye upTan %f\n", openVRfovEye[0][2] );
	common->Printf( "HMD Right Eye downTan %f\n", openVRfovEye[0][3] );
	common->Printf( "OpenVR HMD Screen separation = %f\n", VRScreenSeparation );
	return true;
}

/*
==============
iVr::HMDInit
==============
*/

void iVr::HMDInit( void )
{
	hasHMD = false;
	game->isVR = false;

	if ( !OculusInit() && !OpenVRInit() )
	{
		common->Printf( "No HMD detected.\n VR Disabled\n" );
		return;
	}
	common->Printf( "\n\n HMD Initialized\n" );
	hasHMD = true;
	game->isVR = true;
	common->Printf( "VR_USE_MOTION_CONTROLS Final = %d\n", VR_USE_MOTION_CONTROLS );
	
}


/*
==============
iVr::HMDShutdown
==============
*/

void iVr::HMDShutdown( void )
{
#ifdef USE_OVR
	if ( hasOculusRift )
	{
		ovr_DestroyTextureSwapChain( hmdSession, oculusSwapChain[0] );
		ovr_DestroyTextureSwapChain( hmdSession, oculusSwapChain[1] );

		ovr_Destroy( hmdSession );
		ovr_Shutdown();
		hmdSession = NULL;
	}
	else
#endif
	{
		vr::VR_Shutdown();
	}
	m_pHMD = NULL;
	return;
}

/*
==============
iVr::HMDInitializeDistortion
==============
*/

void iVr::HMDInitializeDistortion()
{

	if ( 
#ifdef USE_OVR
	( !commonVr->hmdSession || !commonVr->hasOculusRift || !vr_enable.GetBool() ) 
		&&
#endif
		( !m_pHMD || !commonVr->hasHMD || !vr_enable.GetBool() ) )
	{
		game->isVR = false;
		return;
	}


	if ( !commonVr->hasOculusRift )
	{
		m_mat4ProjectionLeft = GetHMDMatrixProjectionEye( vr::Eye_Left );
		m_mat4ProjectionRight = GetHMDMatrixProjectionEye( vr::Eye_Right );
		m_mat4eyePosLeft = GetHMDMatrixPoseEye( vr::Eye_Left );
		m_mat4eyePosRight = GetHMDMatrixPoseEye( vr::Eye_Right );


		m_pHMD->GetProjectionRaw( vr::Eye_Left, &hmdEye[0].projectionOpenVR.projLeft, &hmdEye[0].projectionOpenVR.projRight, &hmdEye[0].projectionOpenVR.projUp, &hmdEye[0].projectionOpenVR.projDown );
		m_pHMD->GetProjectionRaw( vr::Eye_Right, &hmdEye[1].projectionOpenVR.projLeft, &hmdEye[1].projectionOpenVR.projRight, &hmdEye[1].projectionOpenVR.projUp, &hmdEye[1].projectionOpenVR.projDown );

		m_pHMD->GetRecommendedRenderTargetSize( &hmdWidth, &hmdHeight );

		for ( int eye = 0; eye < 2; eye++ )
		{
			hmdEye[eye].renderTargetRes.x = hmdWidth  * vr_pixelDensity.GetFloat();
			hmdEye[eye].renderTargetRes.y = hmdHeight * vr_pixelDensity.GetFloat();
		}

		vr::HmdMatrix34_t EyeToHeadTransform;
		EyeToHeadTransform = m_pHMD->GetEyeToHeadTransform( vr::Eye_Right );

		hmdForwardOffset = -EyeToHeadTransform.m[2][3];
		singleEyeIPD = EyeToHeadTransform.m[0][3];


	}

	game->isVR = true;
	common->Printf( "VR Mode ENABLED.\n" );

	if ( !glConfig.framebufferObjectAvailable )
	{
		common->Error( "Framebuffer object not available.  Framebuffer support required for VR.\n" );
	}

	bool fboCreated = false;

	ovrSizei rendertarget;
#ifdef USE_OVR
	ovrRecti viewport = { 0, 0, 0, 0 };
	if (hasOculusRift)
	{
		for (int eye = 0; eye < 2; eye++ )
		{

			hmdEye[eye].eyeFov = commonVr->hmdDesc.DefaultEyeFov[eye];
			hmdEye[eye].eyeRenderDesc = ovr_GetRenderDesc( commonVr->hmdSession, (ovrEyeType)eye, hmdEye[eye].eyeFov );

			common->Printf("Eye[%d] viewOffset %s\n", eye,hmdEye[eye].viewOffset.ToString());
			common->Printf("Eye[%d] uvOffset[0] x = %f y = %f\n", eye, hmdEye[eye].UVScaleoffset[0].x, hmdEye[eye].UVScaleoffset[0].y);
			common->Printf("Eye[%d] uvOffset[1] x = %f y = %f\n", eye, hmdEye[eye].UVScaleoffset[1].x, hmdEye[eye].UVScaleoffset[1].y);

			common->Printf("Eye[%d] fovLeftTan = %f\n", eye, hmdEye[eye].eyeFov.LeftTan);
			common->Printf("Eye[%d] fovRightTan = %f\n", eye, hmdEye[eye].eyeFov.RightTan);
			common->Printf( "Eye[%d] fovUpTan = %f\n", eye, hmdEye[eye].eyeFov.UpTan );
			common->Printf( "Eye[%d] fovDownTan = %f\n", eye, hmdEye[eye].eyeFov.DownTan );

			ovrMatrix4f pEye = ovrMatrix4f_Projection( hmdEye[eye].eyeRenderDesc.Fov, 1.0f, -0.9999999999f, true );
			int x, y;
			for ( x = 0; x < 4; x++ )
			{
				for ( y = 0; y < 4; y++ )
				{
					hmdEye[eye].projectionHmd[y * 4 + x] = pEye.M[x][y];
				}
			}
			hmdEye[eye].viewOffset.x = hmdEye[eye].eyeRenderDesc.HmdToEyePose.Position.x;
			hmdEye[eye].viewOffset.y = hmdEye[eye].eyeRenderDesc.HmdToEyePose.Position.y;
			hmdEye[eye].viewOffset.z = hmdEye[eye].eyeRenderDesc.HmdToEyePose.Position.z;
			common->Printf( "EYE %d HmdToEyeOffset x %f y %f z %f\n", eye, hmdEye[eye].eyeRenderDesc.HmdToEyePose.Position.x, hmdEye[eye].eyeRenderDesc.HmdToEyePose.Position.y, hmdEye[eye].eyeRenderDesc.HmdToEyePose.Position.z );

			rendertarget = ovr_GetFovTextureSize( commonVr->hmdSession, (ovrEyeType)eye, commonVr->hmdEye[eye].eyeFov, vr_pixelDensity.GetFloat() ); // make sure both eyes render to the same size target
			hmdEye[eye].renderTarget.h = rendertarget.h; // Koz was height?
			hmdEye[eye].renderTarget.w = rendertarget.w;
			common->Printf( "Eye %d Rendertaget Width x Height = %d x %d\n", eye, rendertarget.w, rendertarget.h );
		}
	}

	if (hasOculusRift)
	{
		primaryFBOWidth = rendertarget.w;
		primaryFBOHeight = rendertarget.h;
	}
	else
#endif
	{
		primaryFBOWidth = hmdEye[0].renderTargetRes.x;
		primaryFBOHeight = hmdEye[0].renderTargetRes.y;
	}
	fboWidth = primaryFBOWidth;
	fboHeight = primaryFBOHeight;


	if ( !fboCreated )
	{
		// create the FBOs 
		common->Printf( "Generating FBOs.\n" );
		common->Printf( "Default recommended resolution = %i %i \n", hmdWidth, hmdHeight );
		common->Printf( "Requested pixel density = %f \n", vr_pixelDensity.GetFloat() );
		common->Printf( "\nWorking resolution ( default * pixelDensity ) = %i %i \n", primaryFBOWidth, primaryFBOHeight );

		

		VR_AAmode = r_multiSamples.GetInteger() == 0 ? VR_AA_NONE : VR_AA_MSAA;

		common->Printf( "vr_FBOAAmode %d r_multisamples %d\n", VR_AAmode, r_multiSamples.GetInteger() );

		/*
		if ( VR_AAmode == VR_AA_FXAA )
		{// enable FXAA

			VR_AAmode = VR_AA_NONE;

		}
		*/

		if ( VR_AAmode == VR_AA_MSAA )
		{	// enable MSAA
			GL_CheckErrors();

			common->Printf( "Creating %d x %d MSAA framebuffer\n", primaryFBOWidth, primaryFBOHeight );
			globalFramebuffers.primaryFBO = new Framebuffer( "_primaryFBO", primaryFBOWidth, primaryFBOHeight, true );
			common->Printf( "Adding Depth/Stencil attachments to MSAA framebuffer\n" );
			globalFramebuffers.primaryFBO->AddDepthStencilBuffer( GL_DEPTH24_STENCIL8 );
			common->Printf( "Adding color attachment to MSAA framebuffer\n" );
			globalFramebuffers.primaryFBO->AddColorBuffer( GL_RGBA, 0 );

			int status = globalFramebuffers.primaryFBO->Check();
			globalFramebuffers.primaryFBO->Error( status );

			common->Printf( "Creating resolve framebuffer\n" );
			globalFramebuffers.resolveFBO = new Framebuffer( "_resolveFBO", primaryFBOWidth, primaryFBOHeight, false ); // Koz
			common->Printf( "Adding Depth/Stencil attachments to framebuffer\n" );
			globalFramebuffers.resolveFBO->AddDepthStencilBuffer( GL_DEPTH24_STENCIL8 );
			common->Printf( "Adding color attachment to framebuffer\n" );
			globalFramebuffers.resolveFBO->AddColorBuffer( GL_RGBA, 0 );

			status = globalFramebuffers.resolveFBO->Check();
			globalFramebuffers.resolveFBO->Error( status );
#ifdef USE_OVR
			if ( hasOculusRift )
			{
				fboWidth = globalFramebuffers.primaryFBO->GetWidth();// rendertarget.w;
				fboHeight = globalFramebuffers.primaryFBO->GetHeight();
				common->Printf( "Globalframebuffer w x h  = %d x %d\n", fboWidth, fboHeight );
				rendertarget.w = fboWidth;
				rendertarget.h = fboHeight;
			}
#endif

			if ( status = GL_FRAMEBUFFER_COMPLETE )
			{
				useFBO = true;
				fboCreated = true;
			}
			else
			{
				useFBO = false;
				fboCreated = false;
			}

		}

		if ( !fboCreated )
		{ // either AA disabled or AA buffer creation failed. Try creating unaliased FBOs.

			//primaryFBOimage = globalImages->ImageFromFunction( "_primaryFBOimage", R_MakeFBOImage );
			common->Printf( "Creating framebuffer\n" );
			globalFramebuffers.primaryFBO = new Framebuffer( "_primaryFBO", primaryFBOWidth, primaryFBOHeight, false ); // Koz
			common->Printf( "Adding Depth/Stencil attachments to framebuffer\n" );
			globalFramebuffers.primaryFBO->AddDepthStencilBuffer( GL_DEPTH24_STENCIL8 );
			common->Printf( "Adding color attachment to framebuffer\n" );
			globalFramebuffers.primaryFBO->AddColorBuffer( GL_RGBA8, 0 );

			int status = globalFramebuffers.primaryFBO->Check();
			globalFramebuffers.primaryFBO->Error( status );

#ifdef USE_OVR
			if ( hasOculusRift )
			{
				fboWidth = globalFramebuffers.primaryFBO->GetWidth();// rendertarget.w;
				fboHeight = globalFramebuffers.primaryFBO->GetHeight();
				common->Printf("Globalframebuffer w x h  = %d x %d\n", fboWidth, fboHeight);
				rendertarget.w = fboWidth;
				rendertarget.h = fboHeight;
			}
#endif

			if ( status = GL_FRAMEBUFFER_COMPLETE )
			{
				useFBO = true;
				fboCreated = true;
			}
			else
			{
				useFBO = false;
				fboCreated = false;
			}
		}

	}

	if ( !useFBO ) { // not using FBO's, will render to default framebuffer (screen) 

		primaryFBOWidth = renderSystem->GetNativeWidth() / 2;
		primaryFBOHeight = renderSystem->GetNativeHeight();
		rendertarget.w = primaryFBOWidth;
		rendertarget.h = primaryFBOHeight;
		for ( int eye = 0; eye < 2; eye++ )
		{
			hmdEye[eye].renderTargetRes.x = primaryFBOWidth;
			hmdEye[eye].renderTargetRes.y = primaryFBOHeight;
#ifdef USE_OVR
			//TODO LINUX
			hmdEye[eye].renderTarget = rendertarget;
#endif
		}

	}

#ifdef USE_OVR
	if ( hasOculusRift )
	{
		viewport.Size.w = rendertarget.w;
		viewport.Size.h = rendertarget.h;

		globalImages->hudImage->Resize( rendertarget.w, rendertarget.h );
		globalImages->pdaImage->Resize( rendertarget.w, rendertarget.h );
		globalImages->currentRenderImage->Resize( rendertarget.w, rendertarget.h );
		globalImages->currentDepthImage->Resize( rendertarget.w, rendertarget.h );
	}
	else
#endif
	{
		globalImages->hudImage->Resize( primaryFBOWidth, primaryFBOHeight );
		globalImages->pdaImage->Resize( primaryFBOWidth, primaryFBOHeight );
		//globalImages->skyBoxFront->Resize( primaryFBOWidth, primaryFBOHeight );
		//globalImages->skyBoxSides->Resize( primaryFBOWidth, primaryFBOHeight );
		globalImages->currentRenderImage->Resize( primaryFBOWidth, primaryFBOHeight );
		globalImages->currentDepthImage->Resize( primaryFBOWidth, primaryFBOHeight );
	}

	common->Printf( "pdaImage size %d %d\n", globalImages->pdaImage->GetUploadWidth(), globalImages->pdaImage->GetUploadHeight() );
	common->Printf( "Hudimage size %d %d\n", globalImages->hudImage->GetUploadWidth(), globalImages->hudImage->GetUploadHeight() );

	if ( hasOculusRift )
	{
#ifdef USE_OVR
		// total IPD in mm
		officialIPD = ( fabs( hmdEye[0].viewOffset.x ) + fabs( hmdEye[1].viewOffset.x ) ) * 1000.0f;
		common->Printf( "Oculus IPD : %f\n", officialIPD );
	
		VRScreenSeparation = 0.5f * ( hmdEye[1].eyeFov.RightTan + -hmdEye[1].eyeFov.LeftTan)
			/ (hmdEye[1].eyeFov.RightTan - -hmdEye[1].eyeFov.LeftTan)
			- 0.5f * (hmdEye[0].eyeFov.RightTan + -hmdEye[0].eyeFov.LeftTan)
			/ (hmdEye[0].eyeFov.RightTan - -hmdEye[0].eyeFov.LeftTan);
		
		VRScreenSeparation /= 2.0f;
		
		
		VRScreenSeparation = (fabs( hmdEye[0].eyeFov.LeftTan ) - fabs( hmdEye[1].eyeFov.LeftTan )) / ( fabs( hmdEye[0].eyeFov.LeftTan ) + fabs( hmdEye[1].eyeFov.LeftTan ) );
		VRScreenSeparation *= 0.5f;

		common->Printf( "Oculus HMD Screen separation = %f\n", VRScreenSeparation );
		
		// calculate fov for engine
	
		float combinedTanHalfFovHorizontal = std::max( std::max( hmdEye[0].eyeFov.LeftTan, hmdEye[0].eyeFov.RightTan ), std::max( hmdEye[1].eyeFov.LeftTan, hmdEye[1].eyeFov.RightTan ) );
		float combinedTanHalfFovVertical = std::max( std::max( hmdEye[0].eyeFov.UpTan, hmdEye[0].eyeFov.DownTan ), std::max( hmdEye[1].eyeFov.UpTan, hmdEye[1].eyeFov.DownTan ) );
		float horizontalFullFovInRadians = 2.0f * atanf( combinedTanHalfFovHorizontal );

		hmdFovX = RAD2DEG( horizontalFullFovInRadians );
		hmdFovY = RAD2DEG( 2.0 * atanf( combinedTanHalfFovVertical ) );
		hmdAspect = combinedTanHalfFovHorizontal / combinedTanHalfFovVertical;
		hmdPixelScale = 1;//ovrScale * vid.width / (float) hmd->Resolution.w;	

		hmdEye[0].renderTarget.w = globalFramebuffers.primaryFBO->GetWidth();
		hmdEye[0].renderTarget.h = globalFramebuffers.primaryFBO->GetHeight();


		hmdEye[1].renderTarget = hmdEye[0].renderTarget;

		common->Printf("Init Hmd FOV x,y = %f , %f. Aspect = %f, PixelScale = %f\n", hmdFovX, hmdFovY, hmdAspect, hmdPixelScale);
		common->Printf("Creating oculus texture set width = %d height = %d.\n", hmdEye[0].renderTarget.w, hmdEye[0].renderTarget.h);
#endif
	}
	else
	{
		float combinedTanHalfFovHorizontal = std::max( std::max(hmdEye[0].projectionOpenVR.projLeft, hmdEye[0].projectionOpenVR.projRight ), std::max( hmdEye[1].projectionOpenVR.projLeft, hmdEye[1].projectionOpenVR.projRight ) );
		float combinedTanHalfFovVertical = std::max( std::max(hmdEye[0].projectionOpenVR.projUp, hmdEye[0].projectionOpenVR.projDown ), std::max( hmdEye[1].projectionOpenVR.projUp, hmdEye[1].projectionOpenVR.projDown ) );
		float horizontalFullFovInRadians = 2.0f * atanf( combinedTanHalfFovHorizontal );

		hmdFovX = RAD2DEG( horizontalFullFovInRadians );
		hmdFovY = RAD2DEG( 2.0 * atanf( combinedTanHalfFovVertical ) );
		hmdAspect = combinedTanHalfFovHorizontal / combinedTanHalfFovVertical;

		common->Printf( "Init Hmd FOV x,y = %f , %f. Aspect = %f\n", hmdFovX, hmdFovY, hmdAspect );
	}

#ifdef USE_OVR
	if ( hasOculusRift )
	{
		ovrTextureSwapChainDesc desc = {};
		desc.Type = ovrTexture_2D;
		desc.ArraySize = 1;
		desc.Width = hmdEye[0].renderTarget.w;
		desc.Height = hmdEye[0].renderTarget.h;
		desc.MipLevels = 1;
		desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.SampleCount = 1;
		desc.StaticImage = ovrFalse;


		// create the swap texture sets 
		if ( ovr_CreateTextureSwapChainGL(hmdSession, &desc, &oculusSwapChain[0]) != ovrSuccess ||
			ovr_CreateTextureSwapChainGL(hmdSession, &desc, &oculusSwapChain[1]) != ovrSuccess )
		{
			ovrErrorInfo errorInfo;
			ovr_GetLastErrorInfo(&errorInfo);
			common->Warning("OveErrorInfo: %d / %s", errorInfo.Result, errorInfo.ErrorString);
			common->Error(" iVr::HMDInitializeDistortion unable to create OVR swap texture set.\n ovr_CreateTextureSwapChainGL failed.\n (If there are multiple video cards installed, make sure the rift is attached to the primary card. This is an issue with the oculus SDK.)");
			game->isVR = false;

		}

		unsigned int texId = 0;
		int length = 0;

		for (int j = 0; j < 2; j++)
		{
			ovr_GetTextureSwapChainLength( hmdSession, oculusSwapChain[j], &length );
			for (int i = 0; i < length; ++i )
			{
				ovr_GetTextureSwapChainBufferGL( hmdSession, oculusSwapChain[j], 0, &texId );
				//oculusSwapChainTexId[j] = texId;

				glBindTexture( GL_TEXTURE_2D, texId );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			}
		}

		ovr_CommitTextureSwapChain( hmdSession, oculusSwapChain[0] );
		ovr_CommitTextureSwapChain( hmdSession, oculusSwapChain[1] );

		glGenFramebuffers( 1, &oculusFboId );
		glGenTextures( 1, &ocululsDepthTexID );

		glBindTexture( GL_TEXTURE_2D, ocululsDepthTexID );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

		glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, hmdEye[0].renderTarget.w, hmdEye[0].renderTarget.h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL );


		//int ww = glConfig.nativeScreenWidth / 2;
		//int wh = glConfig.nativeScreenHeight / 2;

		int ww = hmdDesc.Resolution.w / 2;
		int wh = hmdDesc.Resolution.h / 2;

		ovrMirrorTextureDesc mirrorDesc;
		memset( &mirrorDesc, 0, sizeof( mirrorDesc ) );
		mirrorDesc.Width = ww;
		mirrorDesc.Height = wh;
		mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		mirrorW = ww;
		mirrorH = wh;

		common->Printf( "Creating oculus mirror texture %d x %d\n", ww, wh );
		ovr_CreateMirrorTextureGL( hmdSession, &mirrorDesc, &oculusMirrorTexture );
		ovr_GetMirrorTextureBufferGL( hmdSession, oculusMirrorTexture, &mirrorTexId );
		glGenFramebuffers( 1, &oculusMirrorFboId );
		glBindFramebuffer( GL_READ_FRAMEBUFFER, oculusMirrorFboId );
		glFramebufferTexture2D( GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTexId, 0 );
		glFramebufferRenderbuffer( GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0 );
		glBindFramebuffer( GL_READ_FRAMEBUFFER, 0 );

		wglSwapIntervalEXT( 0 );

		oculusLayer.Header.Type = ovrLayerType_EyeFov;
		oculusLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;
		oculusLayer.ColorTexture[0] = oculusSwapChain[0];
		oculusLayer.ColorTexture[1] = oculusSwapChain[1];
		oculusLayer.Fov[0] = hmdEye[0].eyeRenderDesc.Fov;
		oculusLayer.Fov[1] = hmdEye[1].eyeRenderDesc.Fov;
		oculusLayer.Viewport[0].Pos.x = 0;
		oculusLayer.Viewport[0].Pos.y = 0;
		oculusLayer.Viewport[0].Size.h = hmdEye[0].renderTarget.h;
		oculusLayer.Viewport[0].Size.w = hmdEye[0].renderTarget.w;

		oculusLayer.Viewport[1].Pos.x = 0;
		oculusLayer.Viewport[1].Pos.y = 0;
		oculusLayer.Viewport[1].Size.h = hmdEye[1].renderTarget.h;
		oculusLayer.Viewport[1].Size.w = hmdEye[1].renderTarget.w;
	}
#endif
	if ( !hasOculusRift )
	{
		// override the default steam skybox, initially just set to black.  UpdateScreen can copy static images to skyBoxFront during level loads/saves 


		static vr::Texture_t * textures = new vr::Texture_t[6];
		for ( int i = 0; i < 6; i++ )
		{
			textures[i].handle = (unsigned int*)(size_t)globalImages->skyBoxSides->texnum;
			textures[i].eType = vr::TextureType_OpenGL;
			textures[i].eColorSpace = vr::ColorSpace_Auto;
		}

		//textures[0].handle = (unsigned int*)(size_t)globalImages->skyBoxFront->texnum;
		textures[0].handle = (unsigned int*)(size_t)globalImages->pdaImage->texnum;

		static vr::EVRCompositorError error = vr::VRCompositor()->SetSkyboxOverride(textures, 1);

		common->Printf( "Compositor error = %d\n", error );
		if ( (int)error != vr::VRCompositorError_None )
		{
			gameLocal.Error( "Failed to set skybox override with error: %d\n", error );
		}

		common->Printf( "Finished setting skybox\n" );
	}

#ifdef _WIN32
	if ( !hasOculusRift )
		wglSwapIntervalEXT( 0 );
#endif

	globalFramebuffers.primaryFBO->Bind();

	if ( !hasOculusRift )
	{
#ifdef _WIN32
		wglSwapIntervalEXT( 0 );// make sure vsync is off.
#endif
		r_swapInterval.SetModified();
	}

	GL_CheckErrors();

	// call this once now so the oculus layer has valid values to start with
	// when rendering the intro bink and splash screen.
	if ( hasOculusRift )
	{
		idAngles angTemp = ang_zero;
		idVec3 headPosTemp = vec3_zero;
		idVec3 bodyPosTemp = vec3_zero;
		idVec3 absTemp = vec3_zero;


		HMDGetOrientation( angTemp, headPosTemp, bodyPosTemp, absTemp, false );
	}
	else
	{
		do
		{
			vr::VRCompositor()->WaitGetPoses( m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0 );
		} while ( !m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid );


		//Seems to take a few frames before a vaild yaw is returned, so zero the current tracked player position by pulling multiple poses;
		for ( int t = 0; t < 20; t++ )
		{
			commonVr->HMDResetTrackingOriginOffset();
		}
	}
}

/*
==============`
iVr::HMDGetOrientation
==============
*/

void iVr::HMDGetOrientation( idAngles &hmdAngles, idVec3 &headPositionDelta, idVec3 &bodyPositionDelta, idVec3 &absolutePosition, bool resetTrackingOffset )
{
	static int lastFrame = -1;
	static double time = 0.0;
	static int currentlyTracked;
	static int lastFrameReturned = -1;
	static uint lastIdFrame = -1;
	static float lastRoll = 0.0f;
	static float lastPitch = 0.0f;
	static float lastYaw = 0.0f;
	static idVec3 lastHmdPosition = vec3_zero;

	static idVec3 hmdPosition;
	static idVec3 lastHmdPos2 = vec3_zero;
	static idMat3 hmdAxis = mat3_identity;

	static bool	neckInitialized = false;
	static idVec3 initialNeckPosition = vec3_zero;
	static idVec3 currentNeckPosition = vec3_zero;
	static idVec3 lastNeckPosition = vec3_zero;

	static idVec3 currentChestPosition = vec3_zero;
	static idVec3 lastChestPosition = vec3_zero;

	static float chestLength = 0;
	static bool chestInitialized = false;

	idVec3 neckToChestVec = vec3_zero;
	idMat3 neckToChestMat = mat3_identity;
	idAngles neckToChestAng = ang_zero;

	static idVec3 lastHeadPositionDelta = vec3_zero;
	static idVec3 lastBodyPositionDelta = vec3_zero;
	static idVec3 lastAbsolutePosition = vec3_zero;

	static vr::TrackedDevicePose_t lastTrackedPoseOpenVR = { 0.0f };
		
	
	if ( !hasHMD )
	{
		hmdAngles.roll = 0.0f;
		hmdAngles.pitch = 0.0f;
		hmdAngles.yaw = 0.0f;
		headPositionDelta = vec3_zero;
		bodyPositionDelta = vec3_zero;
		absolutePosition = vec3_zero;
		return;
	}

	lastBodyYawOffset = bodyYawOffset;
	poseLastHmdAngles = poseHmdAngles;
	poseLastHmdHeadPositionDelta = poseHmdHeadPositionDelta;
	poseLastHmdBodyPositionDelta = poseHmdBodyPositionDelta;
	poseLastHmdAbsolutePosition = poseHmdAbsolutePosition;
	
	
	if ( vr_frameCheck.GetInteger() == 1 && idLib::frameNumber == lastFrame )//&& !commonVr->renderingSplash )
	{
		//make sure to return the same values for this frame.
		hmdAngles.roll = lastRoll;
		hmdAngles.pitch = lastPitch;
		hmdAngles.yaw = lastYaw;
		headPositionDelta = lastHeadPositionDelta;
		bodyPositionDelta = lastBodyPositionDelta;

		if ( resetTrackingOffset == true )
		{

			trackingOriginOffset = lastHmdPosition;
			trackingOriginHeight = trackingOriginOffset.z;
			if (vr_useFloorHeight.GetInteger() == 0)
				trackingOriginOffset.z += pm_normalviewheight.GetFloat() + 5 + CM_CLIP_EPSILON - vr_normalViewHeight.GetFloat() / vr_scale.GetFloat();
			else if (vr_useFloorHeight.GetInteger() == 2)
				trackingOriginOffset.z += 5;
			else if (vr_useFloorHeight.GetInteger() == 3)
				trackingOriginOffset.z = pm_normalviewheight.GetFloat() + 5 + CM_CLIP_EPSILON;
			else if (vr_useFloorHeight.GetInteger() == 4)
			{
				float oldScale = vr_scale.GetFloat();
				float h = trackingOriginHeight * oldScale;
				float newScale = h / 73.0f;
				trackingOriginHeight *= oldScale / newScale;
				trackingOriginOffset *= oldScale / newScale;
				vr_scale.SetFloat( newScale );
			}
			common->Printf( "Resetting tracking yaw offset.\n Yaw = %f old offset = %f ", hmdAngles.yaw, trackingOriginYawOffset );
			trackingOriginYawOffset = hmdAngles.yaw;
			common->Printf( "New Tracking yaw offset %f\n", hmdAngles.yaw, trackingOriginYawOffset );
			neckInitialized = false;

			cinematicStartViewYaw = trackingOriginYawOffset;
			
		}
		common->Printf( "HMDGetOrientation FramCheck Bail == idLib:: framenumber  lf %d  ilfn %d  rendersplash = %d\n", lastFrame, idLib::frameNumber, commonVr->renderingSplash );
		return;
	}

	lastFrame = idLib::frameNumber;
		
	

#ifdef USE_OVR
	static ovrPosef translationPose;
	static ovrPosef	orientationPose;
	static ovrPosef cameraPose;
	static ovrPosef lastTrackedPoseOculus = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

	if ( hasOculusRift )
	{
		hmdFrameTime = ovr_GetPredictedDisplayTime( hmdSession, lastFrame ); 
		
		//common->Printf( "HMDGetOrientation lastframe idLib::framenumber = %d\n", lastFrame );
		
		time = hmdFrameTime;

		hmdTrackingState = ovr_GetTrackingState( hmdSession, time, false );

		currentlyTracked = hmdTrackingState.StatusFlags & ( ovrStatus_PositionTracked );

		if (currentlyTracked)
		{
			translationPose = hmdTrackingState.HeadPose.ThePose;
			lastTrackedPoseOculus = translationPose;
		}
		else
		{
			translationPose = lastTrackedPoseOculus;
		}

		commonVr->handPose[1] = hmdTrackingState.HandPoses[ovrHand_Left].ThePose;
		commonVr->handPose[0] = hmdTrackingState.HandPoses[ovrHand_Right].ThePose;

		for (int i = 0; i < 2; i++)
		{
			MotionControlGetHand(i, poseHandPos[i], poseHandRotationQuat[i]);
			poseHandRotationMat3[i] = poseHandRotationQuat[i].ToMat3();
			poseHandRotationAngles[i] = poseHandRotationQuat[i].ToAngles();
		}
	}
	else
#endif
	{
		if (!m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
		{
			common->Printf("Pose invalid!!\n");

			headPositionDelta = lastHeadPositionDelta;
			bodyPositionDelta = lastBodyPositionDelta;
			absolutePosition = lastAbsolutePosition;
			hmdAngles.roll = lastRoll;
			hmdAngles.pitch = lastPitch;
			hmdAngles.yaw = lastYaw;
			return;
		}
		//common->Printf( "Pose acquired %d\n", gameLocal.GetTime() );
	}

#ifdef USE_OVR
	if (hasOculusRift)
	{
		hmdPosition.x = -translationPose.Position.z * (100.0f / 2.54f) / vr_scale.GetFloat(); // Koz convert position (in meters) to inch (1 id unit = 1 inch).   
		hmdPosition.y = -translationPose.Position.x * (100.0f / 2.54f) / vr_scale.GetFloat();
		hmdPosition.z = translationPose.Position.y * (100.0f / 2.54f) / vr_scale.GetFloat();
	}
	else
#endif
	{
		m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd] = ConvertSteamVRMatrixToidMat4( m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking );

		hmdPosition.x = -m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd][3][2] * (100.0f / 2.54f) / vr_scale.GetFloat();
		hmdPosition.y = -m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd][3][0] * (100.0f / 2.54f) / vr_scale.GetFloat(); // meters to inches
		hmdPosition.z = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd][3][1] * (100.0f / 2.54f) / vr_scale.GetFloat();
	}

	lastHmdPosition = hmdPosition;

	static idQuat poseRot;// = idQuat_zero;
	static idAngles poseAngles = ang_zero;

#ifdef USE_OVR
	if (hasOculusRift)
	{
		static ovrPosef	orientationPose;
		orientationPose = hmdTrackingState.HeadPose.ThePose;

		poseRot.x = orientationPose.Orientation.z;	// x;
		poseRot.y = orientationPose.Orientation.x;	// y;
		poseRot.z = -orientationPose.Orientation.y;	// z;
		poseRot.w = orientationPose.Orientation.w;
	}
	else
#endif
	{
		static idQuat orientationPose;
		orientationPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd].ToMat3().ToQuat();

		poseRot.x = orientationPose.z;
		poseRot.y = orientationPose.x;
		poseRot.z = -orientationPose.y;
		poseRot.w = orientationPose.w;
	}

	poseAngles = poseRot.ToAngles();

	hmdAngles.yaw = poseAngles.yaw;
	hmdAngles.roll = poseAngles.roll;
	hmdAngles.pitch = poseAngles.pitch;

	lastRoll = hmdAngles.roll;
	lastPitch = hmdAngles.pitch;
	lastYaw = hmdAngles.yaw;

	hmdPosition += hmdForwardOffset * poseAngles.ToMat3()[0];

	if ( resetTrackingOffset == true )
	{

		trackingOriginOffset = lastHmdPosition;
		trackingOriginHeight = trackingOriginOffset.z;
		if (vr_useFloorHeight.GetInteger() == 0)
			trackingOriginOffset.z += pm_normalviewheight.GetFloat() + 5 + CM_CLIP_EPSILON - vr_normalViewHeight.GetFloat() / vr_scale.GetFloat();
		else if (vr_useFloorHeight.GetInteger() == 2)
			trackingOriginOffset.z += 5;
		else if (vr_useFloorHeight.GetInteger() == 3)
			trackingOriginOffset.z = pm_normalviewheight.GetFloat() + 5 + CM_CLIP_EPSILON;
		else if (vr_useFloorHeight.GetInteger() == 4)
		{
			float oldScale = vr_scale.GetFloat();
			float h = trackingOriginHeight * oldScale;
			float newScale = h / (pm_normalviewheight.GetFloat() + 5 + CM_CLIP_EPSILON);
			trackingOriginHeight *= oldScale / newScale;
			trackingOriginOffset *= oldScale / newScale;
			vr_scale.SetFloat(newScale);
		}
		common->Printf("Resetting tracking yaw offset.\n Yaw = %f old offset = %f ", hmdAngles.yaw, trackingOriginYawOffset);
		trackingOriginYawOffset = hmdAngles.yaw;
		common->Printf( "New Tracking yaw offset %f\n", hmdAngles.yaw, trackingOriginYawOffset );
		neckInitialized = false;
		cinematicStartViewYaw = trackingOriginYawOffset;
		return;
	}

	hmdPosition -= trackingOriginOffset;

	hmdPosition *= idAngles( 0.0f, -trackingOriginYawOffset, 0.0f ).ToMat3();

	absolutePosition = hmdPosition;

	hmdAngles.yaw -= trackingOriginYawOffset;
	hmdAngles.Normalize360();

	//	common->Printf( "Hmdangles yaw = %f pitch = %f roll = %f\n", poseAngles.yaw, poseAngles.pitch, poseAngles.roll );
	//	common->Printf( "Trans x = %f y = %f z = %f\n", hmdPosition.x, hmdPosition.y, hmdPosition.z );

	lastRoll = hmdAngles.roll;
	lastPitch = hmdAngles.pitch;
	lastYaw = hmdAngles.yaw;
	lastAbsolutePosition = absolutePosition;
	hmdPositionTracked = true;

	commonVr->hmdBodyTranslation = absolutePosition;

	idAngles hmd2 = hmdAngles;
	hmd2.yaw -= commonVr->bodyYawOffset;

	//hmdAxis = hmd2.ToMat3();
	hmdAxis = hmdAngles.ToMat3();

	currentNeckPosition = hmdPosition + hmdAxis[0] * vr_nodalX.GetFloat() / vr_scale.GetFloat() /*+ hmdAxis[1] * 0.0f */ + hmdAxis[2] * vr_nodalZ.GetFloat() / vr_scale.GetFloat();

//	currentNeckPosition.z = pm_normalviewheight.GetFloat() - (vr_nodalZ.GetFloat() + currentNeckPosition.z);

	/*
	if ( !chestInitialized )
	{
		if ( chestDefaultDefined )
		{
				
			neckToChestVec = currentNeckPosition - gameLocal.GetLocalPlayer()->chestPivotDefaultPos;
			chestLength = neckToChestVec.Length();
			chestInitialized = true;
			common->Printf( "Chest Initialized, length %f\n", chestLength );
			common->Printf( "Chest default position = %s\n", gameLocal.GetLocalPlayer()->chestPivotDefaultPos.ToString() );
		}
	}

	if ( chestInitialized )
	{
		neckToChestVec = currentNeckPosition - gameLocal.GetLocalPlayer()->chestPivotDefaultPos;
		neckToChestVec.Normalize();

		idVec3 chesMove = chestLength * neckToChestVec;
		currentChestPosition = currentNeckPosition - chesMove;

		common->Printf( "Chest length %f angles roll %f pitch %f yaw %f \n", chestLength, neckToChestVec.ToAngles().roll, neckToChestVec.ToAngles().pitch, neckToChestVec.ToAngles().yaw );
		common->Printf( "CurrentNeckPos = %s\n", currentNeckPosition.ToString() );
		common->Printf( "CurrentChestPos = %s\n", currentChestPosition.ToString() );
		common->Printf( "ChestMove = %s\n", chesMove.ToString() );

		idAngles chestAngles = ang_zero;
		chestAngles.roll = neckToChestVec.ToAngles().yaw + 90.0f;
		chestAngles.pitch = 0;// neckToChestVec.ToAngles().yaw;            //chest angles.pitch rotates the chest.
		chestAngles.yaw = 0;


		//lastView = commonVr->lastHMDViewAxis.ToAngles();
		//headAngles.roll = lastView.pitch;
		//headAngles.pitch = commonVr->lastHMDYaw - commonVr->bodyYawOffset;
		//headAngles.yaw = lastView.roll;
		//headAngles.Normalize360();
		//gameLocal.GetLocalPlayer()->GetAnimator()->SetJointAxis( gameLocal.GetLocalPlayer()->chestPivotJoint, JOINTMOD_LOCAL, chestAngles.ToMat3() );
	}
	*/
	if ( !neckInitialized )
	{
		lastNeckPosition = currentNeckPosition;
		initialNeckPosition = currentNeckPosition;
		if ( vr_useFloorHeight.GetInteger() != 1 )
			initialNeckPosition.z = vr_nodalZ.GetFloat() / vr_scale.GetFloat();
		neckInitialized = true;
	}
	
	bodyPositionDelta = currentNeckPosition - lastNeckPosition; // use this to base movement on neck model
	bodyPositionDelta.z = currentNeckPosition.z - initialNeckPosition.z;

	//bodyPositionDelta = currentChestPosition - lastChestPosition;
	lastBodyPositionDelta = bodyPositionDelta;
	
	lastNeckPosition = currentNeckPosition;
	lastChestPosition = currentChestPosition;
	
	headPositionDelta = hmdPosition - currentNeckPosition; // use this to base movement on neck model
	//headPositionDelta = hmdPosition - currentChestPosition;
	headPositionDelta.z = hmdPosition.z;
	//bodyPositionDelta.z = 0;
	// how many game units the user has physically ducked in real life from their calibrated position
	userDuckingAmount = (trackingOriginHeight - trackingOriginOffset.z) - hmdPosition.z;

	lastBodyPositionDelta = bodyPositionDelta;
	lastHeadPositionDelta = headPositionDelta;
}

/*
==============
iVr::HMDGetOrientation
==============
*/

void iVr::HMDGetOrientationAbsolute( idAngles &hmdAngles, idVec3 &position )
{

}

/*
==============
iVr::HMDResetTrackingOriginOffset
==============
*/
void iVr::HMDResetTrackingOriginOffset( void )
{
	static idVec3 body = vec3_zero;
	static idVec3 head = vec3_zero;
	static idVec3 absPos = vec3_zero;
	static idAngles rot = ang_zero;

	common->Printf( "HMDResetTrackingOriginOffset called\n " );

	HMDGetOrientation( rot, head, body, absPos, true );

	common->Printf( "New Yaw offset = %f\n", commonVr->trackingOriginYawOffset );
}

/*
==============
iVr::MotionControllSetRotationOffset;
==============
*/
void iVr::MotionControlSetRotationOffset()
{

	/*
	switch ( motionControlType )
	{
	
	default:
		break;
	}
	*/
}

/*
==============
iVr::MotionControllSetOffset;
==============
*/
void iVr::MotionControlSetOffset()
{
	/*
	switch ( motionControlType )
	{

		default:
			break;
	}
	*/
	return;
}

/*
==============
iVr::MotionControlGetOpenVrController
==============
*/
void iVr::MotionControlGetOpenVrController( vr::TrackedDeviceIndex_t deviceNum, idVec3 &motionPosition, idQuat &motionRotation )
{

	idMat4 m_rmat4DevicePose = ConvertSteamVRMatrixToidMat4( m_rTrackedDevicePose[(int)deviceNum].mDeviceToAbsoluteTracking );
	static idQuat orientationPose;
	static idQuat poseRot;
	static idAngles poseAngles = ang_zero;
	static idAngles angTemp = ang_zero;

	motionPosition.x = -m_rmat4DevicePose[3][2] * (100.0f / 2.54f) / vr_scale.GetFloat();
	motionPosition.y = -m_rmat4DevicePose[3][0] * (100.0f / 2.54f) / vr_scale.GetFloat(); // meters to inches
	motionPosition.z = m_rmat4DevicePose[3][1] * (100.0f / 2.54f) / vr_scale.GetFloat();

	motionPosition -= trackingOriginOffset;
	motionPosition *= idAngles( 0.0f, (-trackingOriginYawOffset ) , 0.0f ).ToMat3();// .Inverse();

	orientationPose = m_rmat4DevicePose.ToMat3().ToQuat();

	poseRot.x = orientationPose.z;
	poseRot.y = orientationPose.x;
	poseRot.z = -orientationPose.y;
	poseRot.w = orientationPose.w;

	poseAngles = poseRot.ToAngles();

	angTemp.yaw = poseAngles.yaw;
	angTemp.roll = poseAngles.roll;
	angTemp.pitch = poseAngles.pitch;

	motionPosition -= commonVr->hmdBodyTranslation;

	angTemp.yaw -= trackingOriginYawOffset;// + bodyYawOffset;
	angTemp.Normalize360();

	motionRotation = angTemp.ToQuat();
}

void iVr::MotionControlGetTouchController( int hand, idVec3 &motionPosition, idQuat &motionRotation )
{
	
#ifdef USE_OVR //TODO: ovr only?

	static idQuat poseRot;
	static idAngles poseAngles = ang_zero;
	static idAngles angTemp = ang_zero;

	motionPosition.x = -handPose[hand].Position.z * (100.0f / 2.54f) / vr_scale.GetFloat();// Koz convert position (in meters) to inch (1 id unit = 1 inch).   
	
	motionPosition.y = -handPose[hand].Position.x * (100.0f / 2.54f) / vr_scale.GetFloat();
	
	motionPosition.z = handPose[hand].Position.y * (100.0f / 2.54f) / vr_scale.GetFloat();
			
	motionPosition -= trackingOriginOffset;
	
	motionPosition *= idAngles( 0.0f, (-trackingOriginYawOffset), 0.0f ).ToMat3();

	poseRot.x = handPose[hand].Orientation.z;	// x;
	poseRot.y = handPose[hand].Orientation.x;	// y;
	poseRot.z = -handPose[hand].Orientation.y;	// z;
	poseRot.w = handPose[hand].Orientation.w;
	
	poseAngles = poseRot.ToAngles();

	angTemp.yaw = poseAngles.yaw;
	angTemp.roll = poseAngles.roll;
	angTemp.pitch = poseAngles.pitch;

	motionPosition -= commonVr->hmdBodyTranslation;

	angTemp.yaw -= trackingOriginYawOffset;// + bodyYawOffset;
	angTemp.Normalize360();

	motionRotation = angTemp.ToQuat();
#endif
}
/*
==============
iVr::MotionControllGetHand;
==============
*/
void iVr::MotionControlGetHand( int hand, idVec3 &motionPosition, idQuat &motionRotation )
{
	if ( hand == HAND_LEFT )
	{
		MotionControlGetLeftHand( motionPosition, motionRotation );
	}
	else
	{
		MotionControlGetRightHand( motionPosition, motionRotation );
	}

	// apply weapon mount offsets
	
	if ( hand == vr_weaponHand.GetInteger() && vr_mountedWeaponController.GetBool() )
	{
		idVec3 controlToHand = idVec3( vr_mountx.GetFloat() / vr_scale.GetFloat(), vr_mounty.GetFloat() / vr_scale.GetFloat(), vr_mountz.GetFloat()  / vr_scale.GetFloat() );
		idVec3 controlCenter = idVec3( vr_vcx.GetFloat() / vr_scale.GetFloat(), vr_vcy.GetFloat() / vr_scale.GetFloat(), vr_vcz.GetFloat()  / vr_scale.GetFloat() );

		motionPosition += ( controlToHand - controlCenter ) * motionRotation; // pivot around the new point
	}
	else
	{
		motionPosition += idVec3( vr_vcx.GetFloat()  / vr_scale.GetFloat(), vr_vcy.GetFloat() / vr_scale.GetFloat(), vr_vcz.GetFloat() / vr_scale.GetFloat() ) * motionRotation;
	}
}


/*
==============
iVr::MotionControllGetLeftHand;
==============
*/
void iVr::MotionControlGetLeftHand( idVec3 &motionPosition, idQuat &motionRotation )
{
	static idAngles angles = ang_zero;
	switch ( motionControlType )
	{

		case MOTION_STEAMVR:
		{
			//vr::TrackedDeviceIndex_t deviceNo = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole( vr::TrackedControllerRole_LeftHand );
			//MotionControlGetOpenVrController( deviceNo, motionPosition, motionRotation );
			MotionControlGetOpenVrController( leftControllerDeviceNo, motionPosition, motionRotation );

			//motionPosition += idVec3( vr_vcx.GetFloat(), vr_vcy.GetFloat(), vr_vcz.GetFloat() ) * motionRotation;

			break;
		}
		
		case MOTION_OCULUS:
		{

			MotionControlGetTouchController(1, motionPosition, motionRotation);
			break;
		}
		
		default:
			break;
	}
}

/*
==============
iVr::MotionControllGetRightHand;
==============
*/
void iVr::MotionControlGetRightHand( idVec3 &motionPosition, idQuat &motionRotation )
{
	static idAngles angles = ang_zero;
	switch ( motionControlType )
	{
			
		case MOTION_STEAMVR:
		{
			//vr::TrackedDeviceIndex_t deviceNo = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole( vr::TrackedControllerRole_RightHand );
			//MotionControlGetOpenVrController( deviceNo, motionPosition, motionRotation );
			MotionControlGetOpenVrController( rightControllerDeviceNo, motionPosition, motionRotation );

			//motionPosition += idVec3( vr_vcx.GetFloat(), vr_vcy.GetFloat(), vr_vcz.GetFloat() ) * motionRotation;
			break;
		}
		
		case MOTION_OCULUS:
		{

			MotionControlGetTouchController(0, motionPosition, motionRotation);
			break;
		}
		
		default:
			break;
	}
}

/*
==============
iVr::MotionControllSetHaptic
==============
*/
void iVr::MotionControllerSetHapticOculus( float low, float hi )
{
	
	float beat;
	float enable;
	
	beat = fabs( low - hi ) / 65535;
	
	enable = ( beat > 0.0f) ? 1.0f : 0.0f;

#ifdef USE_OVR
	if ( vr_weaponHand.GetInteger() == HAND_RIGHT )
	{
		ovr_SetControllerVibration( hmdSession, ovrControllerType_RTouch, beat, enable );
	}
	else
	{
		ovr_SetControllerVibration( hmdSession, ovrControllerType_LTouch, beat, enable );
	}
#endif

	return;
}

/*
==============
iVr::MotionControllSetHaptic
==============
*/
void iVr::MotionControllerSetHapticOpenVR( int hand, unsigned short value )
{
	vr::TrackedDeviceIndex_t deviceNo;

	if ( hand == HAND_RIGHT )
	{
		deviceNo = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole( vr::TrackedControllerRole_RightHand );
	}
	else
	{
		deviceNo = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole( vr::TrackedControllerRole_LeftHand );
	}

	m_pHMD->TriggerHapticPulse( deviceNo, 0, value );
	return;
}

/*
==============
iVr::CalcAimMove
Pass the controller yaw & pitch changes.
Indepent weapon view angles will be updated,
and the correct yaw & pitch movement values will
be returned based on the current user aim mode.
==============
*/

void iVr::CalcAimMove( float &yawDelta, float &pitchDelta )
{

	if ( commonVr->VR_USE_MOTION_CONTROLS ) // no independent aim or joystick pitch when using motion controllers.
	{
		pitchDelta = 0.0f;
		return;
	}
		

	float pitchDeadzone = vr_deadzonePitch.GetFloat();
	float yawDeadzone = vr_deadzoneYaw.GetFloat();
	
	commonVr->independentWeaponPitch += pitchDelta;

	if ( commonVr->independentWeaponPitch >= pitchDeadzone ) commonVr->independentWeaponPitch = pitchDeadzone;
	if ( commonVr->independentWeaponPitch < -pitchDeadzone ) commonVr->independentWeaponPitch = -pitchDeadzone;
	pitchDelta = 0;

	// if moving the character in third person, just turn immediately, no deadzones.
	if ( commonVr->thirdPersonMovement ) return;

	
	commonVr->independentWeaponYaw += yawDelta;

	if ( commonVr->independentWeaponYaw >= yawDeadzone )
	{
		yawDelta = commonVr->independentWeaponYaw - yawDeadzone;
		commonVr->independentWeaponYaw = yawDeadzone;
		return;
	}

	if ( commonVr->independentWeaponYaw < -yawDeadzone )
	{
		yawDelta = commonVr->independentWeaponYaw + yawDeadzone;
		commonVr->independentWeaponYaw = -yawDeadzone;
		return;
	}

	yawDelta = 0.0f;

}



/*
==============
iVr::FrameStart
==============
*/
void iVr::FrameStart( void )
{
	//common->Printf( "Framestart called from frame %d\n", idLib::frameNumber );
	
	if ( hasOculusRift )
	{
		HMDGetOrientation( poseHmdAngles, poseHmdHeadPositionDelta, poseHmdBodyPositionDelta, poseHmdAbsolutePosition, false );
		remainingMoveHmdBodyPositionDelta = poseHmdBodyPositionDelta;
		return;
	}

	static int lastFrame = -1;

	if ( idLib::frameNumber == lastFrame && !commonVr->renderingSplash ) return;
	lastFrame = idLib::frameNumber;

	lastBodyYawOffset = bodyYawOffset;
	poseLastHmdAngles = poseHmdAngles;
	poseLastHmdHeadPositionDelta = poseHmdHeadPositionDelta;
	poseLastHmdBodyPositionDelta = poseHmdBodyPositionDelta;
	poseLastHmdAbsolutePosition = poseHmdAbsolutePosition;

	vr::VRCompositor()->WaitGetPoses( m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0 );
	

	leftControllerDeviceNo = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole( vr::TrackedControllerRole_LeftHand );
	rightControllerDeviceNo = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole( vr::TrackedControllerRole_RightHand );
	
	/*
	vr::VRControllerState_t& currentStateL = commonVr->pControllerStateL;
	m_pHMD->GetControllerState( commonVr->leftControllerDeviceNo, &currentStateL );

	vr::VRControllerState_t& currentStateR = commonVr->pControllerStateR;
	m_pHMD->GetControllerState( commonVr->rightControllerDeviceNo, &currentStateR );
	*/
	//Sys_PollJoystickInputEvents( 5 ); // 5 is the device num for the steam vr controller, pull once per frame for now :(


	HMDGetOrientation( poseHmdAngles, poseHmdHeadPositionDelta, poseHmdBodyPositionDelta, poseHmdAbsolutePosition, false );
	remainingMoveHmdBodyPositionDelta = poseHmdBodyPositionDelta;
	
	for ( int i = 0; i < 2; i++ )
	{
		MotionControlGetHand( i, poseHandPos[i], poseHandRotationQuat[i] );
		poseHandRotationMat3[i] = poseHandRotationQuat[i].ToMat3();
		poseHandRotationAngles[i] = poseHandRotationQuat[i].ToAngles();
	}
	return;
}

/*
==============
iVr::GetCurrentFlashlightMode();
==============
*/

int iVr::GetCurrentFlashlightMode()
{
	//common->Printf( "Returning flashlightmode %d\n", currentFlashlightMode );
	return currentFlashlightMode;
}

/*
==============
iVr::GetCurrentFlashlightMode();
==============
*/
void iVr::NextFlashlightMode()
{
	currentFlashlightMode++;
	if ( currentFlashlightMode >= FLASHLIGHT_MAX ) currentFlashlightMode = 0;
}

bool iVr::ShouldQuit()
{
#ifdef USE_OVR
	if (hasOculusRift)
	{
		ovrSessionStatus ss;
		ovrResult result = ovr_GetSessionStatus(hmdSession, &ss);
		if (ss.ShouldQuit)
			return true;
		if (ss.ShouldRecenter)
			shouldRecenter = true;
	}
#endif
	return false;
}

void iVr::ForceChaperone(int which, bool force)
{
	static bool chaperones[2] = {};
	chaperones[which] = force;
	force = chaperones[0] || chaperones[1];

#ifdef USE_OVR
	if (hasOculusRift)
	{
		ovr_RequestBoundaryVisible(hmdSession, force);
	}
	else 
#endif	
	if (hasHMD)
	{
		vr::VRChaperone()->ForceBoundsVisible(force);
	}
}

