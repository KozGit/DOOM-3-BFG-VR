#pragma hdrstop

#include"precompiled.h"

#undef strncmp
#undef vsnprintf		
#undef _vsnprintf		

#include "vr.h"
#include "d3xp\Game_local.h"
#include "sys\win32\win_local.h"
#include "d3xp\physics\Clip.h"
#include "libs\SixenseSDK_062612\include\sixense_utils\controller_manager\controller_manager.hpp"
#include "..\renderer\Framebuffer.h"

#define RADIANS_TO_DEGREES(rad) ((float) rad * (float) (180.0 / idMath::PI))

// *** Oculus HMD Variables

idCVar vr_pixelDensity( "vr_pixelDensity", "1.25", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "" );
idCVar vr_vignette( "vr_vignette", "1", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, "unused" );
idCVar vr_FBOAAmode( "vr_FBOAAmode", "1", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_RENDERER, "Antialiasing mode. 0 = Disabled 1 = MSAA 2= FXAA\n" );
idCVar vr_enable( "vr_enable", "1", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, "Enable VR mode. 0 = Disabled 1 = Enabled." );
idCVar vr_FBOscale( "vr_FBOscale", "1.0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_RENDERER, "unused" );
idCVar vr_scale( "vr_scale", "1.0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "unused" );
idCVar vr_useOculusProfile( "vr_useOculusProfile", "1", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, "Use Oculus Profile values. 0 = use user defined profile, 1 = use Oculus profile." );
idCVar vr_manualIPDEnable( "vr_manualIPDEnable", "0", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, " Override the HMD provided IPD value with value in vr_manualIPD 0 = disable 1= use manual iPD\n" );
idCVar vr_manualIPD( "vr_manualIPD", "64", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "User defined IPD value in MM" );
idCVar vr_manualHeight( "vr_manualHeight", "70", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "User defined player height in inches" );
idCVar vr_minLoadScreenTime( "vr_minLoadScreenTime", "6000", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Min time to display load screens in ms.", 0.0f, 10000.0f );

idCVar vr_clipPositional( "vr_clipPositional", "1", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Clip positional tracking movement\n. 1 = Clip 0 = No clipping.\n" );

//koz cvars for hydra mods
idCVar vr_hydraEnable( "vr_hydraEnable", "0", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, " Enable Razer Hydra Native support. 1 = enabled, 0 = disabled." );
idCVar vr_hydraForceDetect( "vr_hydraForceDetect", "0", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Force hydra detection. 0 = normal detection, 1 = force detection." );
idCVar vr_hydraOffsetForward( "vr_hydraOffsetForward", "10", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_hydraOffsetHorizontal( "vr_hydraOffsetHorizontal", "7", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_hydraOffsetVertical( "vr_hydraOffsetVertical", "-22", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_hydraPitchOffset( "vr_hydraPitchOffset", "10", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Pitch offset for awkward hydra grip angle" );


idCVar vr_armIKenable( "vr_armIKenable", "1", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Enable IK on arms when using motion controls and player body is visible.\n 1 = Enabled 0 = disabled\n" );
idCVar vr_weaponHand( "vr_weaponHand", "0", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, "Which hand holds weapon.\n 0 = Right hand\n 1 = Left Hand\n", 0, 1 );

//flashlight cvars

idCVar vr_flashlightMode( "vr_flashlightMode", "3", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, "Flashlight mount.\n0 = Body\n1 = Head\n2 = Gun\n3= Hand ( if motion controls available.)" );

//tweak flash position when aiming with hydra

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

idCVar vr_PDAscale( "vr_PDAscale", "3", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, " unused" );
idCVar vr_PDAfixLocation( "vr_PDAfixLocation", "1", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Fix PDA position in space in front of player\n instead of holding in hand." );

idCVar vr_weaponPivotOffsetForward( "vr_weaponPivotOffsetForward", "3", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_weaponPivotOffsetHorizontal( "vr_weaponPivotOffsetHorizontal", "0", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_weaponPivotOffsetVertical( "vr_weaponPivotOffsetVertical", "0", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_weaponPivotForearmLength( "vr_weaponPivotForearmLength", "16", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );;

idCVar vr_guiScale( "vr_guiScale", "1", CVAR_FLOAT | CVAR_RENDERER | CVAR_ARCHIVE, "scale reduction factor for full screen menu/pda scale in VR", 0.0001f, 1.0f ); //koz allow scaling of full screen guis/pda
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
idCVar vr_hudLowHealth( "vr_hudLowHealth", "0", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, " 0 = Disable, otherwise force hud if heath below this value." );

idCVar vr_tweakTalkCursor( "vr_tweakTalkCursor", "25", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Tweak talk cursor y pos in VR. % val", 0, 99 );

idCVar vr_wristStatMon( "vr_wristStatMon", "1", CVAR_INTEGER | CVAR_ARCHIVE, "Use wrist status monitor. 0 = Disable 1 = Right Wrist 2 = Left Wrist " );

// koz display windows monitor name in the resolution selection menu, helpful to ID which is the rift if using extended mode
idCVar vr_listMonitorName( "vr_listMonitorName", "0", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "List monitor name with resolution." );

idCVar vr_viewModelArms( "vr_viewModelArms", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, " Dont change this, will be removed. Display arms on view models in VR" );
idCVar vr_disableWeaponAnimation( "vr_disableWeaponAnimation", "1", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Disable weapon animations in VR. ( 1 = disabled )" );
idCVar vr_headKick( "vr_headKick", "0", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Damage can 'kick' the players view. 0 = Disabled in VR." );
idCVar vr_showBody( "vr_showBody", "1", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Dont change this! Will be removed shortly, modifying will cause the player to have extra hands." );
idCVar vr_joystickMenuMapping( "vr_joystickMenuMapping", "1", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, " Use alternate joy mapping\n in menus/PDA.\n 0 = D3 Standard\n 1 = VR Mode.\n(Both joys can nav menus,\n joy r/l to change\nselect area in PDA." );


idCVar	vr_deadzonePitch( "vr_deadzonePitch", "90", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Vertical Aim Deadzone", 0, 180 );
idCVar	vr_deadzoneYaw( "vr_deadzoneYaw", "30", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Horizontal Aim Deadzone", 0, 180 );
idCVar	vr_comfortDelta( "vr_comfortDelta", "10", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Comfort Mode turning angle ", 0, 180 );

//idCVar	vr_interactiveCinematic( "vr_interactiveCinematic", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Interactive cinematics in VR ( no camera )" );

idCVar	vr_headingBeamWidth( "vr_headingBeamWidth", "12.0", CVAR_FLOAT | CVAR_ARCHIVE, "heading beam width" ); // Koz default was 2, IMO too big in VR.
idCVar	vr_headingBeamLength( "vr_headingBeamLength", "96", CVAR_FLOAT | CVAR_ARCHIVE, "heading beam length" ); // koz default was 250, but was to short in VR.  Length will be clipped if object is hit, this is max length for the hit trace. 
idCVar	vr_headingBeamMode( "vr_headingBeamMode", "3", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "0 = disabled, 1 = solid, 2 = arrows, 3 = scrolling arrows" );

idCVar	vr_weaponSight( "vr_weaponSight", "0", CVAR_INTEGER | CVAR_ARCHIVE, "Weapon Sight.\n 0 = Lasersight\n 1 = Red dot\n 2 = Circle dot\n 3 = Crosshair\n" );
idCVar	vr_weaponSightToSurface( "vr_weaponSightToSurface", "0", CVAR_INTEGER | CVAR_ARCHIVE, "Map sight to surface. 0 = Disabled 1 = Enabled\n" );

idCVar	vr_motionWeaponPitchAdj( "vr_motionWeaponPitchAdj", "40", CVAR_FLOAT | CVAR_ARCHIVE, "Weapon controller pitch adjust" );
idCVar	vr_motionFlashPitchAdj( "vr_motionFlashPitchAdj", "40", CVAR_FLOAT | CVAR_ARCHIVE, "Flash controller pitch adjust" );

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

idCVar vr_padDeadzone( "vr_padDeadzone", ".25", CVAR_FLOAT | CVAR_ARCHIVE, "Deadzone for steam pads.\n 0.0 = no deadzone 1.0 = dead\n" );
idCVar vr_padToButtonThreshold( "vr_padToButtonThreshold", ".7", CVAR_FLOAT | CVAR_ARCHIVE, "Threshold value for pad contact\n to register as button press\n .1 high sensitiveity thru\n .99 low sensitivity" );
idCVar vr_knockBack( "vr_knockBack", "0", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Enable damage knockback in VR. 0 = Disabled, 1 = Enabled" );
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

idCVar vr_movePoint( "vr_movePoint", "0", CVAR_INTEGER | CVAR_ARCHIVE, "If enabled, move in the direction the off hand is pointing." );
idCVar vr_moveClick( "vr_moveClick", "0", CVAR_INTEGER | CVAR_ARCHIVE, " 0 = Normal movement.\n 1 = Click and hold to walk, run button to run.\n 2 = Click to start walking, then touch only. Run btn to run.\n 3 = Click to start walking, hold click to run.\n 4 = Click to start walking, then click toggles run\n" );
idCVar vr_playerBodyMode( "vr_playerBodyMode", "0", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "Player body mode:\n0 = Display full body\n1 = Just Hands \n2 = Weapons only\n" );
idCVar vr_bodyToMove( "vr_bodyToMove", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Lock body orientaion to movement direction." );

idCVar vr_crouchTriggerDist( "vr_crouchTriggerDist", "10", CVAR_FLOAT | CVAR_ARCHIVE, " Distance ( in inches ) player must crouch in real life to toggle crouch\n" );
// Koz end
//===================================================================

int fboWidth;
int fboHeight;

iVr vrCom;
iVr* commonVr = &vrCom;

/*
====================
R_MakeFBOImage
//Koz deleteme, using renderbuffers instead of textures.
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
	gameSaving = false;
	forceLeftStick = true;	// start the PDA in the left menu.
	pdaToggleTime = Sys_Milliseconds();
	PDAclipModelSet = false;
	useFBO = false;
	VR_USE_MOTION_CONTROLS = 0;

	scanningPDA = false;

	vrIsBackgroundSaving = false;

	openVrIPD = 64.0f;
	openVrHeight = 72.0f;

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
	
	motionMoveDelta = vec3_zero;
	motionMoveVelocity = vec3_zero;
	leanOffset = vec3_zero;

	
	chestDefaultDefined = false;

	currentFlashlightPosition = FLASH_BODY;

	handInGui = false;

	handRoll[0] = 0.0f;
	handRoll[1] = 0.0f;

	angles[3] = { 0.0f };

	swfRenderMode = RENDERING_NORMAL;

	isWalking = false;

	hmdBodyTranslation = vec3_zero;

	hydraLeftOffset = hydra_zero;		// koz base offset for left hydra 
	hydraRightOffset = hydra_zero;		// koz base offset for right hydra 

	hydraLeftIndex = 0;						// koz fixme should pull these from hydra sdk but it bails for some reason using sample code - fix later.
	hydraRightIndex = 1;


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

	hmdAspect = 1.0f;

	mirrorW = 0;
	mirrorH = 0;

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

	currentFlashMode = vr_flashlightMode.GetInteger();
	renderingSplash = true;
	
}

/*
==============
iVr::HydraInit
==============
*/

void iVr::HydraInit( void )
{
	
	int x = 0;
	//VR_USE_MOTION_CONTROLS = FALSE;

	if ( vr_hydraEnable.GetInteger() != 0 )
	{
		x = sixenseInit();

		common->Printf( "\nvr_hydraEnable = %d\n", vr_hydraEnable.GetInteger() );
		common->Printf( "Initializing Hydra.\n" );

		// Koz for some reason, the rest of the init fails
		// if we don't give sixenseInit time to settle.
		// Not sure if its my code, my machine, or my Hydras ( poor abused things ) - previously this was not necessary.
		Sys_Sleep( 1000 );

		if ( vr_hydraForceDetect.GetBool() )
		{
			sixenseSetActiveBase( 0 );
			sixenseUtils::getTheControllerManager()->setGameType( sixenseUtils::ControllerManager::ONE_PLAYER_TWO_CONTROLLER );
			common->Printf( "Hydra detection forced.\n" );
			x = sixenseGetNumActiveControllers();
			common->Printf( "iVr::HydraInit : sixenseGetNumActiveControllers() reporting % controllers.\nHydra ENABLED\n\n ", x );
			VR_USE_MOTION_CONTROLS = TRUE;
			return;
		}

		if ( x == SIXENSE_SUCCESS )
		{
			common->Printf( "sixenseInit() = SIXENSE_SUCCESS\n" );
			x = sixenseSetActiveBase( 0 );
			if ( x == SIXENSE_SUCCESS )
			{
				common->Printf( "sixenseSetActiveBase( 0 ) = SIXENSE_SUCCESS\n" );
				x = sixenseIsBaseConnected( 0 );
				if ( x == 1 )
				{
					common->Printf( "sixenseIsBaseConnected( 0 ) = 1\n" );
					x = sixenseGetNumActiveControllers();
					if ( x > 0 )
					{
						common->Printf( "iVr::HydraInit : Hydra base 0 available, %d controllers active.\n", x );
						sixenseUtils::getTheControllerManager()->setGameType( sixenseUtils::ControllerManager::ONE_PLAYER_TWO_CONTROLLER );
						VR_USE_MOTION_CONTROLS = TRUE;
					}
					else
					{
						common->Printf( "\nError: iVr::HydraInit : sixenseGetNumActiveControllers() reported %d controllers.\nHydra DISABLED\n\n ", x );
					}
				}
			}
		}
	}
	common->Printf( "VR_USE_MOTION_CONTROLS = %d\n", VR_USE_MOTION_CONTROLS );
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

	vr::HmdMatrix44_t mat = commonVr->m_pHMD->GetProjectionMatrix( nEye, m_fNearClip, m_fFarClip, vr::API_OpenGL );

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

	return matrixObj.Inverse(); //.invert();
}


/*
==============
iVr::HMDInit
==============
*/

void iVr::HMDInit( void )
{
	hasHMD = false;
	hasOculusRift = false;
	game->isVR = false;

	if ( !vr::VR_IsHmdPresent() )
	{
		common->Printf( "No HMD detected.\n VR Disabled\n" );
		return;
	}

	vr::EVRInitError eError = vr::VRInitError_None;
	m_pHMD = vr::VR_Init( &eError, vr::VRApplication_Scene );

	if ( eError != vr::VRInitError_None )
	{
		m_pHMD = NULL;
		common->Printf( "\n Unable to init SteamVR runtime.\n VR disabled.\n" );
		return;
	}

	m_pRenderModels = (vr::IVRRenderModels *)vr::VR_GetGenericInterface( vr::IVRRenderModels_Version, &eError );

	if ( !m_pRenderModels )
	{
		m_pHMD = NULL;
		vr::VR_Shutdown();

		common->Printf( " Unable to get render model interface: %s\n VR Disabled\n", vr::VR_GetVRInitErrorAsEnglishDescription( eError ) );
		return;
	}

	if ( !vr::VRCompositor() )
	{
		common->Printf( "Compositor initialization failed. See log file for details\nVR Disabled\n" );
		return;
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

	commonVr->hmdHz = (int)m_pHMD->GetFloatTrackedDeviceProperty( vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float );
	commonVr->hmdHz;


	openVrIPD = m_pHMD->GetFloatTrackedDeviceProperty( vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_UserIpdMeters_Float ) * 100;

	com_engineHz.SetInteger( commonVr->hmdHz );

	common->Printf( "Hmd Driver: %s .\n", m_strDriver.c_str() );
	common->Printf( "Hmd Display: %s .\n", m_strDisplay.c_str() );
	common->Printf( "Hmd HZ %d, width %d, height %d\n", commonVr->hmdHz, hmdWidth, hmdHeight );
	common->Printf( "Hmd reported IPD in centimeters = %f \n", openVrIPD );
	common->Printf( "\n\n HMD Initialized\n" );

	hasOculusRift = true;
	hasHMD = true;
	common->Printf( "VR_USE_MOTION_CONTROLS Final = %d\n", VR_USE_MOTION_CONTROLS );
	
}


/*
==============
iVr::HMDShutdown
==============
*/

void iVr::HMDShutdown( void )
{
	vr::VR_Shutdown();
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

	if ( !m_pHMD || !commonVr->hasOculusRift || !vr_enable.GetBool() )
	{
		game->isVR = false;
		return;
	}


	m_mat4ProjectionLeft = GetHMDMatrixProjectionEye( vr::Eye_Left );
	m_mat4ProjectionRight = GetHMDMatrixProjectionEye( vr::Eye_Right );
	m_mat4eyePosLeft = GetHMDMatrixPoseEye( vr::Eye_Left );
	m_mat4eyePosRight = GetHMDMatrixPoseEye( vr::Eye_Right );


	m_pHMD->GetProjectionRaw( vr::Eye_Left, &hmdEye[0].projection.projLeft, &hmdEye[0].projection.projRight, &hmdEye[0].projection.projUp, &hmdEye[0].projection.projDown );
	m_pHMD->GetProjectionRaw( vr::Eye_Right, &hmdEye[1].projection.projLeft, &hmdEye[1].projection.projRight, &hmdEye[1].projection.projUp, &hmdEye[1].projection.projDown );

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


	game->isVR = true;
	common->Printf( "VR Mode ENABLED.\n" );

	if ( !glConfig.framebufferObjectAvailable )
	{
		common->Error( "Framebuffer object not available.  Framebuffer support required for VR.\n" );
	}

	bool fboCreated = false;

	primaryFBOWidth = hmdEye[0].renderTargetRes.x;
	primaryFBOHeight = hmdEye[0].renderTargetRes.y;
	fboWidth = primaryFBOWidth;
	fboHeight = primaryFBOHeight;

	if ( !fboCreated )
	{
		// create the FBOs 
		common->Printf( "Generating FBOs.\n" );
		common->Printf( "Default recommended resolution = %i %i \n", hmdWidth, hmdHeight );
		common->Printf( "Requested pixel density = %f \n", vr_pixelDensity.GetFloat() );
		common->Printf( "\nWorking resolution ( default * pixelDensity ) = %i %i \n", hmdEye[0].renderTargetRes.x, hmdEye[0].renderTargetRes.y );

		VR_AAmode = vr_FBOAAmode.GetInteger();

		common->Printf( "vr_FBOAAmode %d r_multisamples %d\n", VR_AAmode, r_multiSamples.GetInteger() );

		if ( VR_AAmode == VR_AA_FXAA )
		{
			// enable FXAA (removed, need to re-implement fxaa)
			VR_AAmode = VR_AA_NONE;
		}

		if ( VR_AAmode == VR_AA_MSAA )
		{
			// enable MSAA
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
			globalFramebuffers.resolveFBO = new Framebuffer( "_resolveFBO", primaryFBOWidth, primaryFBOHeight, false ); // koz
			common->Printf( "Adding Depth/Stencil attachments to framebuffer\n" );
			globalFramebuffers.resolveFBO->AddDepthStencilBuffer( GL_DEPTH24_STENCIL8 );
			common->Printf( "Adding color attachment to framebuffer\n" );
			globalFramebuffers.resolveFBO->AddColorBuffer( GL_RGBA, 0 );

			status = globalFramebuffers.resolveFBO->Check();
			globalFramebuffers.resolveFBO->Error( status );

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
			globalFramebuffers.primaryFBO = new Framebuffer( "_primaryFBO", primaryFBOWidth, primaryFBOHeight, false ); // koz
			common->Printf( "Adding Depth/Stencil attachments to framebuffer\n" );
			globalFramebuffers.primaryFBO->AddDepthStencilBuffer( GL_DEPTH24_STENCIL8 );
			common->Printf( "Adding color attachment to framebuffer\n" );
			globalFramebuffers.primaryFBO->AddColorBuffer( GL_RGBA8, 0 );

			int status = globalFramebuffers.primaryFBO->Check();
			globalFramebuffers.primaryFBO->Error( status );

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

	if ( !useFBO )
	{ // not using FBO's, will render to default framebuffer (screen) 

		primaryFBOWidth = renderSystem->GetNativeWidth() / 2;
		primaryFBOHeight = renderSystem->GetNativeHeight();

		for ( int eye = 0; eye < 2; eye++ )
		{
			hmdEye[eye].renderTargetRes.x = primaryFBOWidth;
			hmdEye[eye].renderTargetRes.y = primaryFBOHeight;
		}


	}

	globalImages->hudImage->Resize( primaryFBOWidth, primaryFBOHeight );
	globalImages->pdaImage->Resize( primaryFBOWidth, primaryFBOHeight );
	//globalImages->skyBoxFront->Resize( primaryFBOWidth, primaryFBOHeight );
	//globalImages->skyBoxSides->Resize( primaryFBOWidth, primaryFBOHeight );
	globalImages->currentRenderImage->Resize( primaryFBOWidth, primaryFBOHeight );
	globalImages->currentDepthImage->Resize( primaryFBOWidth, primaryFBOHeight );

	common->Printf( "pdaImage size %d %d\n", globalImages->pdaImage->GetUploadWidth(), globalImages->pdaImage->GetUploadHeight() );
	common->Printf( "Hudimage size %d %d\n", globalImages->hudImage->GetUploadWidth(), globalImages->hudImage->GetUploadHeight() );

	// calculate fov for engine
	float combinedTanHalfFovHorizontal = std::max( std::max( hmdEye[0].projection.projLeft, hmdEye[0].projection.projRight ), std::max( hmdEye[1].projection.projLeft, hmdEye[1].projection.projRight ) );
	float combinedTanHalfFovVertical = std::max( std::max( hmdEye[0].projection.projUp, hmdEye[0].projection.projDown ), std::max( hmdEye[1].projection.projUp, hmdEye[1].projection.projDown ) );
	float horizontalFullFovInRadians = 2.0f * atanf( combinedTanHalfFovHorizontal );

	hmdFovX = RAD2DEG( horizontalFullFovInRadians );
	hmdFovY = RAD2DEG( 2.0 * atanf( combinedTanHalfFovVertical ) );
	hmdAspect = combinedTanHalfFovHorizontal / combinedTanHalfFovVertical;

	common->Printf( "Init Hmd FOV x,y = %f , %f. Aspect = %f\n", hmdFovX, hmdFovY, hmdAspect );

	// override the default steam skybox, initially just set to back.  UpdateScreen can copy static images to skyBoxFront during level loads/saves 


	static vr::Texture_t * textures = new vr::Texture_t[6];
	for ( int i = 0; i < 6; i++ )
	{
		textures[i].handle = (unsigned int*)globalImages->skyBoxSides->texnum;
		textures[i].eType = vr::API_OpenGL;
		textures[i].eColorSpace = vr::ColorSpace_Auto;
	}

	//textures[0].handle = (unsigned int*)globalImages->skyBoxFront->texnum;
	textures[0].handle = (unsigned int*)globalImages->pdaImage->texnum;

	static vr::EVRCompositorError error = vr::VRCompositor()->SetSkyboxOverride( textures, 1 );

	common->Printf( "Compositor error = %d\n", error );
	if ( (int)error != vr::VRCompositorError_None )
	{
		gameLocal.Error( "Failed to set skybox override with error: %d\n", error );
	}

	common->Printf( "Finished setting skybox\n" );

	wglSwapIntervalEXT( 0 );
	globalFramebuffers.primaryFBO->Bind();
	wglSwapIntervalEXT( 0 );// make sure vsync is off.
	r_swapInterval.SetModified();

	GL_CheckErrors();

	// call this once now so the oculus layer has valid values to start with
	// when rendering the intro bink and splash screen.
	
	do
	{
		vr::VRCompositor()->WaitGetPoses( m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0 );
	} 
	while ( !m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid );


	//Seems to take a few frames before a vaild yaw is returned, so zero the current tracked player position by pulling multiple poses;
	for ( int t = 0; t < 20; t++ )
	{
		commonVr->HMDResetTrackingOriginOffset();
	}

}

/*
==============`
iVr::HMDGetOrientation
==============
*/

void iVr::HMDGetOrientation( idAngles &hmdAngles, idVec3 &headPositionDelta, idVec3 &bodyPositionDelta, idVec3 &absolutePosition, bool resetTrackingOffset )
{

	static uint lastFrame = -1;
	static uint lastIdFrame = -1;
	static float lastRoll = 0.0f;
	static float lastPitch = 0.0f;
	static float lastYaw = 0.0f;
	static idVec3 lastHmdPosition = vec3_zero;

	static idVec3 hmdPosition;
	static idVec3 lastHmdPos2 = vec3_zero;
	static idMat3 hmdAxis = mat3_identity;

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

	static vr::TrackedDevicePose_t lastTrackedPose = { 0.0f };
	static bool currentlyTracked;
		
	
	if ( !hasOculusRift || !hasHMD )
	{
		hmdAngles.roll = 0.0f;
		hmdAngles.pitch = 0.0f;
		hmdAngles.yaw = 0.0f;
		headPositionDelta = vec3_zero;
		bodyPositionDelta = vec3_zero;
		absolutePosition = vec3_zero;

		return;
	}
		
	if ( m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid )
	{
		//common->Printf( "Pose acquired %d\n", gameLocal.GetTime() );

		static idQuat orientationPose;
		static idQuat poseRot;
		static idAngles poseAngles = ang_zero;

		static float x, y, z = 0.0f;

		m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd] = ConvertSteamVRMatrixToidMat4( m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking );

		hmdPosition.x = -m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd][3][2] * 39.3701;
		hmdPosition.y = -m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd][3][0] * 39.3701; // meters to inches
		hmdPosition.z = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd][3][1] * 39.3701;

		lastHmdPosition = hmdPosition;

		orientationPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd].ToMat3().ToQuat();

		poseRot.x = orientationPose.z;
		poseRot.y = orientationPose.x;
		poseRot.z = -orientationPose.y;
		poseRot.w = orientationPose.w;

		poseAngles = poseRot.ToAngles();

		hmdAngles.yaw = poseAngles.yaw;
		hmdAngles.roll = poseAngles.roll;
		hmdAngles.pitch = poseAngles.pitch;

		hmdPosition += hmdForwardOffset * poseAngles.ToMat3()[0];

		if ( resetTrackingOffset == true )
		{

			trackingOriginOffset = lastHmdPosition;
			common->Printf( "Resetting tracking yaw offset.\n Yaw = %f old offset = %f ", hmdAngles.yaw, trackingOriginYawOffset );
			trackingOriginYawOffset = hmdAngles.yaw;
			common->Printf( "New Tracking yaw offset %f\n", hmdAngles.yaw, trackingOriginYawOffset );

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

		currentNeckPosition = hmdPosition + hmdAxis[0] * vr_nodalX.GetFloat() /*+ hmdAxis[1] * 0.0f */ + hmdAxis[2] * vr_nodalZ.GetFloat();
		
		currentNeckPosition.z = pm_normalviewheight.GetFloat() - (vr_nodalZ.GetFloat() + currentNeckPosition.z);
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

	
		bodyPositionDelta = currentNeckPosition - lastNeckPosition; // use this to base movement on neck model
	
		//bodyPositionDelta = currentChestPosition - lastChestPosition;
		lastBodyPositionDelta = bodyPositionDelta;
		
		lastNeckPosition = currentNeckPosition;
		lastChestPosition = currentChestPosition;
		
		headPositionDelta = hmdPosition - currentNeckPosition; // use this to base movement on neck model
		//headPositionDelta = hmdPosition - currentChestPosition;
		headPositionDelta.z = hmdPosition.z;
		bodyPositionDelta.z = 0;

		lastBodyPositionDelta = bodyPositionDelta;
		lastHeadPositionDelta = headPositionDelta;
	}
	else
	{
		common->Printf( "Pose invalid!!\n" );

		headPositionDelta = lastHeadPositionDelta;
		bodyPositionDelta = lastBodyPositionDelta;
		absolutePosition = lastAbsolutePosition;
		hmdAngles.roll = lastRoll;
		hmdAngles.pitch = lastPitch;
		hmdAngles.yaw = lastYaw;
	}

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
iVr::HydraSetOffset
==============
*/

void iVr::HydraSetOffset( void )
{
	static float hydraYawOffset = 0;
	hydraData currentHydra = hydra_zero;
	idQuat rotQuat;

	rotQuat = idAngles( 0.0f, hydraYawOffset, 0.0f ).Normalize180().ToQuat();

	commonVr->HydraGetLeft( currentHydra );
	commonVr->HydraSetLeftOffset( currentHydra );
	commonVr->HydraGetRight( currentHydra );
	commonVr->HydraSetRightOffset( currentHydra );
}

/*
==============
iVr::HydraSetRotationOffset
==============
*/

void iVr::HydraSetRotationOffset( void )
{
	hydraData currentHydra = hydra_zero;
	hydraData currentHydraOffset = hydra_zero;
	idQuat rotQuat;

	rotQuat = idAngles( 0.0f, (commonVr->lastHMDYaw - commonVr->bodyYawOffset), 0.0f ).Normalize180().ToQuat();


	commonVr->HydraGetLeftOffset( currentHydra );
	currentHydra.hydraRotationQuat *= rotQuat;
	commonVr->HydraSetLeftOffset( currentHydra );

	commonVr->HydraGetRightOffset( currentHydra );
	currentHydra.hydraRotationQuat *= rotQuat;
	commonVr->HydraSetRightOffset( currentHydra );
}

/*
==============
iVr::HydraSetLeftOffset
==============
*/

void iVr::HydraSetLeftOffset( hydraData hydraOffset )
{	//koz left hydra offset will store the initial/default delta between hydra and base for reference
	//so we can get the actual movement deltas for positional body tracking.
	hydraLeftOffset.position = hydraOffset.position;
	hydraLeftOffset.hydraRotationQuat = hydraOffset.hydraRotationQuat;

}

/*
==============
iVr::HydraSetRightOffset
==============
*/
void iVr::HydraSetRightOffset( hydraData hydraOffset )
{	// koz right hydra offset will store the initial/default delta between hydra and base for reference
	// so we can get the actual deltas for proper gun positioning in relation to body.
	hydraRightOffset.position = hydraOffset.position;
	hydraRightOffset.hydraRotationQuat = hydraOffset.hydraRotationQuat;

}

/*
==============
iVr::HydraGetLeftOffset
==============
*/
void iVr::HydraGetLeftOffset( hydraData &hydraOffset )
{

	hydraOffset.position = hydraLeftOffset.position;
	hydraOffset.hydraRotationQuat = hydraLeftOffset.hydraRotationQuat;

}

/*
==============
iVr::HydraGetRightOffset
==============
*/
void iVr::HydraGetRightOffset( hydraData &hydraOffset )
{

	hydraOffset.position = hydraRightOffset.position;
	hydraOffset.hydraRotationQuat = hydraRightOffset.hydraRotationQuat;

}

/*
==============
iVr::HydraGetData
==============
*/
void iVr::HydraGetData( int hydraIndex, hydraData &hydraData )
{

	static idQuat sixtempq;
	static idQuat tempQuat = idQuat_zero;

	sixenseAllControllerData acd;
	sixenseSetActiveBase( 0 );
	sixenseGetAllNewestData( &acd );

	if ( sixenseIsControllerEnabled( hydraIndex ) )
	{

		sixtempq.x = acd.controllers[hydraIndex].rot_quat[2];	// koz get hydra quat and convert to id coord space
		sixtempq.y = acd.controllers[hydraIndex].rot_quat[0];
		sixtempq.z = -acd.controllers[hydraIndex].rot_quat[1];
		sixtempq.w = acd.controllers[hydraIndex].rot_quat[3];

		//add pitch offset to the grip angle of the hydra to make more comfortable / correct for mounting orientation
		tempQuat = idAngles( vr_hydraPitchOffset.GetFloat(), 0.0f, 0.0f ).ToQuat();

		hydraData.hydraRotationQuat = tempQuat * sixtempq;

		hydraData.position.x = -acd.controllers[hydraIndex].pos[2] / 25.4f; // koz convert position (in MM) to inch (1 id unit = 1 inch). (mm/25.4 = inch)  
		hydraData.position.y = -acd.controllers[hydraIndex].pos[0] / 25.4f;
		hydraData.position.z = acd.controllers[hydraIndex].pos[1] / 25.4f;

		hydraData.buttons = acd.controllers[hydraIndex].buttons;
		hydraData.trigger = acd.controllers[hydraIndex].trigger;
		hydraData.joystick_x = acd.controllers[hydraIndex].joystick_x;
		hydraData.joystick_y = acd.controllers[hydraIndex].joystick_y;

	}

}

/*
==============
iVr::HydraGetRight
==============
*/
void iVr::HydraGetRight( hydraData &rightHydra )
{
	HydraGetData( hydraRightIndex, rightHydra );
}

/*
==============
iVr::HydraGetLeft
==============
*/

void iVr::HydraGetLeft( hydraData &leftHydra )
{
	HydraGetData( hydraLeftIndex, leftHydra );
}

/*
==============
iVr::HydraGetLeftWithOffset
==============
*/

void iVr::HydraGetLeftWithOffset( hydraData &leftOffsetHydra ) {  // will return the left hydra position with the offset subtracted

	static hydraData leftCurrent = hydra_zero;
	static hydraData leftOffset = hydra_zero;
	static idQuat offsetAngles;
	static idQuat currentAngles;
	static idAngles ca;
	static idAngles oa;

	static int lastFrame = -1;
	static hydraData lastHydraData = hydra_zero;
	static hydraData result = hydra_zero;


	// HydraGetLeftOffset returns a position of 0 when calibrated
	// baseoffset vector moves this to the hand/weapon position

	static idVec3 baseOffset;

	if ( lastFrame == commonVr->vrFrameNumber )
	{
		leftOffsetHydra = lastHydraData;
		return;
	}

	baseOffset = idVec3( vr_hydraOffsetForward.GetFloat(), vr_hydraOffsetHorizontal.GetFloat(), vr_hydraOffsetVertical.GetFloat() );

	HydraGetLeftOffset( leftOffset );
	HydraGetLeft( leftCurrent );

	offsetAngles = leftOffset.hydraRotationQuat;
	currentAngles = leftCurrent.hydraRotationQuat;

	ca = currentAngles.ToAngles();
	oa = offsetAngles.ToAngles();

	leftCurrent.position -= leftOffset.position;
	//leftCurrent.position -= commonVr->hmdBodyTranslation;
	leftCurrent.position *= offsetAngles.ToMat3().Inverse();
	leftCurrent.position -= commonVr->hmdBodyTranslation;

	ca.yaw -= oa.yaw;
	ca.Normalize180();

	leftCurrent.hydraRotationQuat = ca.ToQuat();
	result = leftCurrent;
	result.position += baseOffset;
	leftOffsetHydra = result;
	lastHydraData = result;
}

/*
==============
iVr::HydraGetRightWithOffset
==============
*/

void iVr::HydraGetRightWithOffset( hydraData &rightOffsetHydra ) { // will return the right hydra position with the offset angle and position values.

	static hydraData rightCurrent = hydra_zero;
	static hydraData rightOffset = hydra_zero;
	static idQuat offsetAngles;
	static idQuat currentAngles;
	static idAngles ca;
	static idAngles oa;

	static int lastFrame = -1;
	static hydraData lastHydraData = hydra_zero;

	static idVec3 baseOffset;

	if ( lastFrame == commonVr->vrFrameNumber )
	{
		rightOffsetHydra = lastHydraData;
		return;
	}

	static hydraData result = hydra_zero;

	lastFrame = commonVr->vrFrameNumber;

	// HydraGetLeftOffset returns a position of 0 when calibrated
	// baseoffset vector moves this to the hand/weapon position

	baseOffset = idVec3( vr_hydraOffsetForward.GetFloat(), -vr_hydraOffsetHorizontal.GetFloat(), vr_hydraOffsetVertical.GetFloat() );

	HydraGetRightOffset( rightOffset );
	HydraGetRight( rightCurrent );

	offsetAngles = rightOffset.hydraRotationQuat;
	currentAngles = rightCurrent.hydraRotationQuat;

	oa = offsetAngles.ToAngles();
	ca = currentAngles.ToAngles();

	rightCurrent.position -= rightOffset.position;
	//	rightCurrent.position -= commonVr->hmdBodyTranslation;
	rightCurrent.position *= offsetAngles.ToMat3().Inverse();
	rightCurrent.position -= commonVr->hmdBodyTranslation;

	ca.yaw -= oa.yaw;
	ca.Normalize180();

	rightCurrent.hydraRotationQuat = ca.ToQuat();
	result = rightCurrent;
	result.position += baseOffset;

	rightOffsetHydra = result;

	lastHydraData = result;

}

/*
==============
iVr::MotionControllSetRotationOffset;
==============
*/
void iVr::MotionControlSetRotationOffset()
{

	switch ( motionControlType )
	{

	case  MOTION_HYDRA:
	{
		HydraSetRotationOffset();
		break;
	}
	default:
		break;
	}

}

/*
==============
iVr::MotionControllSetOffset;
==============
*/
void iVr::MotionControlSetOffset()
{
	switch ( motionControlType )
	{

	case  MOTION_HYDRA:
	{
		HydraSetOffset();
		break;
	}
	default:
		break;
	}
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

	motionPosition.x = -m_rmat4DevicePose[3][2] * 39.3701;
	motionPosition.y = -m_rmat4DevicePose[3][0] * 39.3701; // meters to inches
	motionPosition.z = m_rmat4DevicePose[3][1] * 39.3701;

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
		idVec3 controlToHand = idVec3( vr_mountx.GetFloat(), vr_mounty.GetFloat(), vr_mountz.GetFloat() );
		idVec3 controlCenter = idVec3( vr_vcx.GetFloat(), vr_vcy.GetFloat(), vr_vcz.GetFloat() );

		motionPosition += ( controlToHand - controlCenter ) * motionRotation; // pivot around the new point
	}
	else
	{
		motionPosition += idVec3( vr_vcx.GetFloat(), vr_vcy.GetFloat(), vr_vcz.GetFloat() ) * motionRotation;
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

	case  MOTION_HYDRA:
	{
		hydraData leftHydra;
		commonVr->HydraGetLeftWithOffset( leftHydra );
		motionPosition = leftHydra.position;
		motionRotation = leftHydra.hydraRotationQuat;
		break;
	}
	case MOTION_STEAMVR:
	{
		//vr::TrackedDeviceIndex_t deviceNo = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole( vr::TrackedControllerRole_LeftHand );
		//MotionControlGetOpenVrController( deviceNo, motionPosition, motionRotation );
		MotionControlGetOpenVrController( leftControllerDeviceNo, motionPosition, motionRotation );

		//motionPosition += idVec3( vr_vcx.GetFloat(), vr_vcy.GetFloat(), vr_vcz.GetFloat() ) * motionRotation;

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

	case MOTION_HYDRA:
	{
		hydraData rightHydra;
		commonVr->HydraGetRightWithOffset( rightHydra );
		motionPosition = rightHydra.position;
		motionRotation = rightHydra.hydraRotationQuat;
		break;
	}
	case MOTION_STEAMVR:
	{
		//vr::TrackedDeviceIndex_t deviceNo = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole( vr::TrackedControllerRole_RightHand );
		//MotionControlGetOpenVrController( deviceNo, motionPosition, motionRotation );
		MotionControlGetOpenVrController( rightControllerDeviceNo, motionPosition, motionRotation );

		//motionPosition += idVec3( vr_vcx.GetFloat(), vr_vcy.GetFloat(), vr_vcz.GetFloat() ) * motionRotation;
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
void iVr::MotionControllerSetHaptic( int hand, unsigned short value )
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
	commonVr->independentWeaponYaw += yawDelta;


	if ( commonVr->independentWeaponPitch >= pitchDeadzone ) commonVr->independentWeaponPitch = pitchDeadzone;
	if ( commonVr->independentWeaponPitch < -pitchDeadzone ) commonVr->independentWeaponPitch = -pitchDeadzone;
	pitchDelta = 0;

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
iVr::GetCurrentFlashMode();
==============
*/

int iVr::GetCurrentFlashMode()
{
	//common->Printf( "Returning flashmode %d\n", currentFlashMode );
	return currentFlashMode;
}

/*
==============
iVr::GetCurrentFlashMode();
==============
*/
void iVr::NextFlashMode()
{
	currentFlashMode++;
	if ( currentFlashMode >= FLASH_MAX ) currentFlashMode = 0;
}

