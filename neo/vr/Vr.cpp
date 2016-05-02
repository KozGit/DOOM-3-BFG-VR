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

idCVar vr_trackingPredictionAuto( "vr_trackingPredictionAuto", "1", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Use SDK tracking prediction.\n 1 = Auto, 0 = User defined." );
idCVar vr_trackingPredictionUserDefined( "vr_trackingPredictionUserDefined", "13", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "User defined tracking prediction in ms." );
idCVar vr_pixelDensity( "vr_pixelDensity", "1.25", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "" );
idCVar vr_vignette( "vr_vignette", "1", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, "Enable warp vignette. 0 = off 1 = on" );
idCVar vr_FBOAAmode( "vr_FBOAAmode", "1", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_RENDERER, "Antialiasing mode. 0 = Disabled 1 = MSAA 2= FXAA\n" );
idCVar vr_enable( "vr_enable", "1", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, "Enable VR mode. 0 = Disabled 1 = Enabled." );
idCVar vr_FBOscale( "vr_FBOscale", "1.0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_RENDERER, "FBO scaling factor." );
idCVar vr_scale( "vr_scale", "1.0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "VR World scale adjustment." );
idCVar vr_useOculusProfile( "vr_useOculusProfile", "1", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, "Use Oculus Profile values. 0 = use user defined profile, 1 = use Oculus profile." );
idCVar vr_manualIPD( "vr_manualIPD", "64", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "User defined IPD value in MM" );
idCVar vr_manualHeight( "vr_manualHeight", "70", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "User defined player height in inches" );
idCVar vr_minLoadScreenTime( "vr_minLoadScreenTime", "6000", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Min time to display load screens in ms.", 0.0f, 10000.0f );

idCVar vr_clipPositional( "vr_clipPositional", "1", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Clip positional tracking movement\n. 1 = Clip 0 = No clipping.\n" );

//koz cvars for hydra mods
idCVar vr_hydraEnable( "vr_hydraEnable", "1", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, " Enable Razer Hydra. 1 = enabled, 0 = disabled." );
idCVar vr_hydraForceDetect( "vr_hydraForceDetect", "0", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Force hydra detection. 0 = normal detection, 1 = force detection." );
idCVar vr_hydraMode( "vr_hydraMode", "0", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, "razer hydra mode. 0 = left hydra for positional tracking, 1 = left hydra as controller, 2 = left hydra as controller and flashlight" );

idCVar vr_armIKenable( "vr_armIKenable", "1", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Enable IK on arms when using motion controls and player body is visible.\n 1 = Enabled 0 = disabled\n" );
idCVar vr_hydraOffsetForward( "vr_hydraOffsetForward", "10", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_hydraOffsetHorizontal( "vr_hydraOffsetHorizontal", "7", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_hydraOffsetVertical( "vr_hydraOffsetVertical", "-22", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );

idCVar vr_hydraPitchOffset( "vr_hydraPitchOffset", "10", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Pitch offset for awkward hydra grip angle" );



//flashlight cvars

idCVar vr_flashlightMode( "vr_flashlightMode", "2", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, "Flashlight mount.\n0 = Body\n1 = Head\n2 = Gun\n3= Hand ( if motion controls available.)" );

//tweak flash position when aiming with hydra



idCVar vr_flashPitchAngle( "vr_flashPitchAngle", "90", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Pitch offset for flashlight using hydra. Default = 90" );

idCVar vr_flashlightBodyPosX( "vr_flashlightBodyPosX", "0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Flashlight vertical offset for body mount." );
idCVar vr_flashlightBodyPosY( "vr_flashlightBodyPosY", "0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Flashlight horizontal offset for body mount." );
idCVar vr_flashlightBodyPosZ( "vr_flashlightBodyPosZ", "0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Flashlight forward offset for body mount." );

idCVar vr_flashlightHelmetPosX( "vr_flashlightHelmetPosX", "6", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Flashlight vertical offset for helmet mount." );
idCVar vr_flashlightHelmetPosY( "vr_flashlightHelmetPosY", "-6", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Flashlight horizontal offset for helmet mount." );
idCVar vr_flashlightHelmetPosZ( "vr_flashlightHelmetPosZ", "-20", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Flashlight forward offset for helmet mount." );
idCVar vr_flashlightGunScale( "vr_flashlightGunScale", ".4", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Flashlight scale for gun mount." );

// Koz begin : these cvars just a tool to check model rotations in game
idCVar vr_offx( "vr_offx", "0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, " x offset" );
idCVar vr_offy( "vr_offy", "0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, " y offset" );
idCVar vr_offz( "vr_offz", "0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, " z offset" );
idCVar vr_rotateAxis( "vr_rotateAxis", "0", CVAR_FLOAT, "axis of rotation." );
idCVar vr_offsetYaw( "vr_offsetYaw", "0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "default yaw axis." );
idCVar vr_offsetPitch( "vr_offsetPitch", "0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "default pitch axis." );
idCVar vr_offsetRoll( "vr_offsetRoll", "0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "default roll axis." );

idCVar vr_off_leftx( "vr_off_leftx", "0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, " x offset" );
idCVar vr_off_lefty( "vr_off_lefty", "0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, " y offset" );
idCVar vr_off_leftz( "vr_off_leftz", "0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, " z offset" );

idCVar vr_offset_leftYaw( "vr_offset_leftYaw", "0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "default left yaw axis." );
idCVar vr_offset_leftPitch( "vr_offset_leftPitch", "0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "default left pitch axis." );
idCVar vr_offset_leftRoll( "vr_offset_leftRoll", "0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "default left roll axis." );


idCVar vr_forward_keyhole( "vr_forward_keyhole", "11.25", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Forward movement keyhole in deg. If view is inside body direction +/- this value, forward movement is in view direction, not body direction" );

idCVar vr_PDAscale( "vr_PDAscale", "3", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, " PDA scale factor" );
idCVar vr_PDAfixLocation( "vr_PDAfixLocation", "1", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Fix PDA position in space in front of player\n instead of holding in hand." );

idCVar vr_weaponPivotOffsetForward( "vr_weaponPivotOffsetForward", "3", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_weaponPivotOffsetHorizontal( "vr_weaponPivotOffsetHorizontal", "0", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_weaponPivotOffsetVertical( "vr_weaponPivotOffsetVertical", "0", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_weaponPivotForearmLength("vr_weaponPivotForearmLength", "16", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "");;

idCVar vr_guiScale( "vr_guiScale", "1", CVAR_FLOAT | CVAR_RENDERER | CVAR_ARCHIVE, "scale reduction factor for full screen menu/pda scale in VR", 0.0001f, 1.0f ); //koz allow scaling of full screen guis/pda
idCVar vr_guiSeparation( "vr_guiSeparation", ".01", CVAR_FLOAT | CVAR_ARCHIVE, " Screen separation value for fullscreen guis." );

idCVar vr_hudScale( "vr_hudScale", "1.0", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Hud scale", 0.1f, 2.0f ); 
idCVar vr_hudPosHor( "vr_hudPosHor", "0", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Hud Horizontal offset in inches" ); 
idCVar vr_hudPosVer( "vr_hudPosVer", "0", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Hud Vertical offset in inches" ); 
idCVar vr_hudPosDis( "vr_hudPosDis", "30", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Hud Distance from view in inches" );
idCVar vr_hudPosLock( "vr_hudPosLock", "1", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "Lock Hud to:  0 = Face, 1 = Body" );

idCVar vr_hudType( "vr_hudType", "2", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "VR Hud Type. 0 = Disable.\n1 = Full\n2=Look Activate", 0, 2 ); 
idCVar vr_hudAngle( "vr_hudAngle", "48", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "HMD pitch to reveal HUD in look activate mode." );
idCVar vr_hudTransparency( "vr_hudTransparency", "1", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, " Hud transparency. 0.0 = Invisible thru 1.0 = full", 0.0, 100.0 );
idCVar vr_hudOcclusion( "vr_hudOcclusion",	"1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, " Hud occlusion. 0 = Objects occlude HUD, 1 = No occlusion " );
idCVar vr_hudHealth(	"vr_hudHealth",		"1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show Armor/Health in Hud." );
idCVar vr_hudAmmo(		"vr_hudAmmo",		"1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show Ammo in Hud." );
idCVar vr_hudPickUps(	"vr_hudPickUps",	"1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show item pick ups in Hud." );
idCVar vr_hudTips(		"vr_hudTips",		"1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show tips Hud." );
idCVar vr_hudLocation(	"vr_hudLocation",	"1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show player location in Hud." );
idCVar vr_hudObjective(	"vr_hudObjective",	"1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show objectives in Hud." );
idCVar vr_hudStamina(	"vr_hudStamina",	"1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show stamina in Hud." );
idCVar vr_hudPills(		"vr_hudPills",		"1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show weapon pills in Hud." );
idCVar vr_hudComs(		"vr_hudComs",		"1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show communications in Hud." );
idCVar vr_hudWeap(		"vr_hudWeap",		"1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show weapon pickup/change icons in Hud." );
idCVar vr_hudNewItems(	"vr_hudNewItems",	"1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show new items acquired in Hud." );
idCVar vr_hudFlashlight("vr_hudFlashlight", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Show flashlight in Hud." );
idCVar vr_hudLowHealth( "vr_hudLowHealth",	"0", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, " 0 = Disable, otherwise show hud if heath below this value." );

idCVar vr_tweakTalkCursor( "vr_tweakTalkCursor", "41", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Tweak talk cursor y pos in VR. % val", 0, 99 );

idCVar vr_wristStatMon( "vr_wristStatMon", "1", CVAR_INTEGER | CVAR_ARCHIVE, "Use wrist status monitor. 0 = Disable 1 = Right Wrist 2 = Left Wrist " );

// koz display windows monitor name in the resolution selection menu, helpful to ID which is the rift if using extended mode
idCVar vr_listMonitorName( "vr_listMonitorName", "1", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "List monitor name with resolution." );

idCVar vr_viewModelArms( "vr_viewModelArms", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, " Display arms on view models in VR" );
idCVar vr_disableWeaponAnimation( "vr_disableWeaponAnimation", "1", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Disable weapon animations in VR. ( 1 = disabled )" );
idCVar vr_headKick( "vr_headKick", "0", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Damage can 'kick' the players view. 0 = Disabled in VR." );
idCVar vr_showBody( "vr_showBody", "0", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Show player body in VR." );
idCVar vr_joystickMenuMapping( "vr_joystickMenuMapping", "1", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, " Use alternate joy mapping\n in menus/PDA.\n 0 = D3 Standard\n 1 = VR Mode.\n(Both joys can nav menus,\n joy r/l to change\nselect area in PDA." );

idCVar vr_hmdHz("vr_hmdHz", "0", CVAR_INTEGER | CVAR_RENDERER | CVAR_ARCHIVE, " HMD refresh rate. 0 = Auto, othewise freq in Hz." );

idCVar vr_tweakx( "vr_tweakx", "1.0", CVAR_FLOAT, "xtweaking value.", 0.0f, 2.0f ); // deleteme for dev only
idCVar vr_tweaky( "vr_tweaky", "1.0", CVAR_FLOAT, "xtweaking value.", 0.0f, 2.0f ); // deleteme for dev only

idCVar vr_testWeaponModel( "vr_testWeaponModel", "0", CVAR_BOOL, "Free rotation of viewweapon models." ); // deleteme for dev only

idCVar	vr_aimMode( "vr_aimMode", "0", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "Aim Mode in VR", 0, 99 );
idCVar	vr_deadzonePitch( "vr_deadzonePitch", "90", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Vertical Aim Deadzone", 0, 180 );
idCVar	vr_deadzoneYaw( "vr_deadzoneYaw", "30", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Horizontal Aim Deadzone", 0, 180 );
idCVar	vr_comfortDelta( "vr_comfortDelta", "10", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Comfort Mode turning angle ", 0, 180 );

idCVar	vr_interactiveCinematic( "vr_interactiveCinematic", "1", CVAR_BOOL | CVAR_GAME | CVAR_ARCHIVE, "Interactive cinematics in VR ( no camera )" );

idCVar	vr_headingBeamWidth( "vr_headingBeamWidth", "12.0", CVAR_FLOAT | CVAR_ARCHIVE, "heading beam width" ); // Koz default was 2, IMO too big in VR.
idCVar	vr_headingBeamLength( "vr_headingBeamLength", "96", CVAR_FLOAT | CVAR_ARCHIVE, "heading beam length" ); // koz default was 250, but was to short in VR.  Length will be clipped if object is hit, this is max length for the hit trace. 
idCVar	vr_headingBeamMode( "vr_headingBeamMode", "3", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "0 = disabled, 1 = solid, 2 = arrows, 3 = scrolling arrows" );

idCVar	vr_weaponSight( "vr_weaponSight", "0", CVAR_INTEGER | CVAR_ARCHIVE, "Weapon Sight.\n 0 = Lasersight\n 1 = Red dot\n 2 = Circle dot\n 3 = Crosshair\n" );
idCVar	vr_weaponSightToSurface( "vr_weaponSightToSurface", "0", CVAR_INTEGER | CVAR_ARCHIVE, "Map sight to surface. 0 = Disabled 1 = Enabled\n" );


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
	PDAclipModelSet = false;
	useFBO = false;
	VR_USE_HYDRA = 0;
		
	vrIsBackgroundSaving = false;
	
	openVrIPD = 64.0f;
	openVrHeight = 72.0f;

	manualIPD = 64.0f;
	manualHeight = 72.0f;
			
	hmdPositionTracked = false;

	vrFrameNumber = 0;

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
	
	hmdWidth =  0;
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
			
}

/*
==============
iVr::HydraInit
==============
*/

void iVr::HydraInit(void) 
{
	
	int x = 0;
	VR_USE_HYDRA = FALSE;
	
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
			VR_USE_HYDRA = TRUE;
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
					if (  x > 0  )
					{
						common->Printf( "iVr::HydraInit : Hydra base 0 available, %d controllers active.\n", x );
						sixenseUtils::getTheControllerManager()->setGameType( sixenseUtils::ControllerManager::ONE_PLAYER_TWO_CONTROLLER );
						VR_USE_HYDRA = TRUE;
					}
					else 
					{
						common->Printf( "\nError: iVr::HydraInit : sixenseGetNumActiveControllers() reported %d controllers.\nHydra DISABLED\n\n ", x );
					}
				}
			}
		}
	}
	common->Printf("VR_USE_HYDRA = %d\n", VR_USE_HYDRA);
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

	common->Printf( "Getting driver info\n" );
	m_strDriver = "No Driver";
	m_strDisplay = "No Display";

	m_strDriver = GetTrackedDeviceString( m_pHMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String );
	m_strDisplay = GetTrackedDeviceString( m_pHMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String );

	// get this here so we have a resolution starting point for gl initialization.
	m_pHMD->GetRecommendedRenderTargetSize( &hmdWidth, &hmdHeight );

	commonVr->hmdHz = (int)m_pHMD->GetFloatTrackedDeviceProperty( vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float );
	
	openVrIPD = m_pHMD->GetFloatTrackedDeviceProperty( vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_UserIpdMeters_Float ) * 39.3701;

	com_engineHz.SetInteger( commonVr->hmdHz );

	common->Printf( "Hmd Driver: %s .\n", m_strDriver.c_str() );
	common->Printf( "Hmd Display: %s .\n", m_strDisplay.c_str() );
	common->Printf( "Hmd HZ %d, width %d, height %d\n", commonVr->hmdHz, hmdWidth, hmdHeight );
	common->Printf( "Hmd reported IPD in inches = %f = %f meters\n", openVrIPD, openVrIPD / 39.3701 );
	common->Printf( "\n\n HMD Initialized\n" );

	hasOculusRift = true;
	hasHMD = true;

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

	m_mat4ProjectionLeft = GetHMDMatrixProjectionEye( vr::Eye_Left );
	m_mat4ProjectionRight = GetHMDMatrixProjectionEye( vr::Eye_Right );
	m_mat4eyePosLeft = GetHMDMatrixPoseEye( vr::Eye_Left );
	m_mat4eyePosRight = GetHMDMatrixPoseEye( vr::Eye_Right );


	m_pHMD->GetProjectionRaw( vr::Eye_Left, &hmdEye[0].projection.projLeft, &hmdEye[0].projection.projRight, &hmdEye[0].projection.projUp, &hmdEye[0].projection.projDown );
	m_pHMD->GetProjectionRaw( vr::Eye_Right, &hmdEye[1].projection.projLeft, &hmdEye[1].projection.projRight, &hmdEye[1].projection.projUp, &hmdEye[1].projection.projDown );

	m_pHMD->GetRecommendedRenderTargetSize( &hmdWidth, &hmdHeight );

	for ( int eye = 0; eye < 2; eye++ )
	{
		hmdEye[eye].renderTargetRes.x = hmdWidth;// *vr_pixelDensity.GetFloat();
		hmdEye[eye].renderTargetRes.y = hmdHeight;// *vr_pixelDensity.GetFloat();
	}

	if ( !m_pHMD || !commonVr->hasOculusRift || !vr_enable.GetBool() )
	{
		game->isVR = false;
		return;
	}

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

		if ( !fboCreated  )
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
	
	wglSwapIntervalEXT( 0 );
	globalFramebuffers.primaryFBO->Bind();
	wglSwapIntervalEXT( 0 );// make sure vsync is off.
	r_swapInterval.SetModified();

	GL_CheckErrors();

	// call this once now so the oculus layer has valid values to start with
	// when rendering the intro bink and splash screen.
	FrameStart();




} 
		
/*
==============`
iVr::HMDGetOrientation
==============
*/

void iVr::HMDGetOrientation( idAngles &hmdAngles, idVec3 &headPositionDelta, idVec3 &bodyPositionDelta, bool immediate )
{
	
	static uint lastFrame = 0;
	static float lastRoll = 0.0f;
	static float lastPitch = 0.0f;
	static float lastYaw = 0.0f;
	static idVec3 lastPosition = idVec3( 0.0f, 0.0f, 0.0f );

	static idVec3 hmdPosition;
	static idMat3 hmdAxis = mat3_identity;
		
	static idVec3 currentNeckPosition = vec3_zero;
	static idVec3 lastNeckPosition = vec3_zero;
	
	static idVec3 lastHeadPositionDelta = vec3_zero;
	static idVec3 lastBodyPositionDelta = vec3_zero;
	
	if ( !hasOculusRift || !hasHMD ) 
	{
		hmdAngles.roll = 0.0f;
		hmdAngles.pitch = 0.0f;
		hmdAngles.yaw = 0.0f;
		headPositionDelta = vec3_zero;
		bodyPositionDelta = vec3_zero;
		return;
	}

    
	if ( commonVr->vrFrameNumber == lastFrame && immediate == false  )
	{
		//make sure to return the same values for this frame.
		hmdAngles.roll = lastRoll;
		hmdAngles.pitch = lastPitch;
		hmdAngles.yaw = lastYaw;
		headPositionDelta = lastHeadPositionDelta;
		bodyPositionDelta = lastBodyPositionDelta;
		return;
	}

	lastFrame = commonVr->vrFrameNumber;
	
	static vr::TrackedDevicePose_t lastTrackedPose = { 0.0f };
	static bool currentlyTracked;

	vr::VRCompositor()->WaitGetPoses( m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0 );

	int m_iValidPoseCount = 0;
	idStr m_strPoseClasses = "";
	for ( int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice )
	{
		if ( m_rTrackedDevicePose[nDevice].bPoseIsValid )
		{
			m_iValidPoseCount++;
			m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToidMat4( m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking );
			if ( m_rDevClassChar[nDevice] == 0 )
			{
				switch ( m_pHMD->GetTrackedDeviceClass( nDevice ) )
				{
				case vr::TrackedDeviceClass_Controller:        m_rDevClassChar[nDevice] = 'C'; break;
				case vr::TrackedDeviceClass_HMD:               m_rDevClassChar[nDevice] = 'H'; break;
				case vr::TrackedDeviceClass_Invalid:           m_rDevClassChar[nDevice] = 'I'; break;
				case vr::TrackedDeviceClass_Other:             m_rDevClassChar[nDevice] = 'O'; break;
				case vr::TrackedDeviceClass_TrackingReference: m_rDevClassChar[nDevice] = 'T'; break;
				default:                                       m_rDevClassChar[nDevice] = '?'; break;
				}
			}
			m_strPoseClasses += m_rDevClassChar[nDevice];

		}
	}

	if ( m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid )
	{
		static idQuat orientationPose;
		static idQuat poseRot;
		static idAngles poseAngles = ang_zero;

		static float x, y, z = 0.0f;

		hmdPosition.x = -m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd][3][2] * 39.3701;
		hmdPosition.y = -m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd][3][0] * 39.3701; // meters to inches
		hmdPosition.z = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd][3][1] * 39.3701;

		orientationPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd].ToMat3().ToQuat();

		poseRot.x = orientationPose.z;
		poseRot.y = orientationPose.x;
		poseRot.z = -orientationPose.y;
		poseRot.w = orientationPose.w;

		poseAngles = poseRot.ToAngles();

		hmdAngles.yaw = poseAngles.yaw;
		hmdAngles.roll = poseAngles.roll;
		hmdAngles.pitch = poseAngles.pitch;

		//	common->Printf( "Hmdangles yaw = %f pitch = %f roll = %f\n", poseAngles.yaw, poseAngles.pitch, poseAngles.roll );
		//	common->Printf( "Trans x = %f y = %f z = %f\n", hmdPosition.x, hmdPosition.y, hmdPosition.z );
	
		lastRoll = hmdAngles.roll;
		lastPitch = hmdAngles.pitch;
		lastYaw = hmdAngles.yaw;
		hmdPositionTracked = true;
	}
	
	if ( immediate == true )
	{
		headPositionDelta = hmdPosition;
		return;
	}

	idAngles hmd2 = hmdAngles;
	hmd2.yaw -= commonVr->bodyYawOffset;
		
	hmdAxis = hmd2.ToMat3();
	
	currentNeckPosition = hmdPosition + hmdAxis[0] * -3.0f /*+ hmdAxis[1] * 0.0f */ + hmdAxis[2] * -6.0f; // was -4, -6

	commonVr->hmdBodyTranslation = currentNeckPosition;

	bodyPositionDelta = currentNeckPosition - lastNeckPosition;
	lastBodyPositionDelta = bodyPositionDelta;
	lastNeckPosition = currentNeckPosition;
	
	headPositionDelta = hmdPosition - currentNeckPosition;
	headPositionDelta.z = hmdPosition.z;
	bodyPositionDelta.z = 0;

	lastBodyPositionDelta = bodyPositionDelta;
	lastHeadPositionDelta = headPositionDelta;

}

/*
==============`
iVr::HMDGetOrientation
==============
*/

void iVr::HMDGetOrientationAbsolute( idAngles &hmdAngles, idVec3 &position )
{
	/*
	static double time = 0.0;
	static ovrPosef translationPose;
	static ovrPosef	orientationPose;
	static ovrPosef lastTrackedPose = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	static bool currentlyTracked;
	static ovrVector3f  HmdToEyeOffset[2] = { hmdEye[0].eyeRenderDesc.HmdToEyeOffset, hmdEye[1].eyeRenderDesc.HmdToEyeOffset };


	if ( !hasOculusRift || !hasHMD )
	{
		hmdAngles.roll = 0.0f;
		hmdAngles.pitch = 0.0f;
		hmdAngles.yaw = 0.0f;
		position = vec3_zero;
		return;
	}

	if ( vr_trackingPredictionAuto.GetBool() )
	{
		time = ovr_GetPredictedDisplayTime( hmdSession, commonVr->vrFrameNumber  ); // renderSystem->GetFrameCount() );// renderSystem->GetFrameCount() );
	}
	else
	{
		time = ovr_GetTimeInSeconds() + (vr_trackingPredictionUserDefined.GetFloat() / 1000);
	}

	hmdTrackingState = ovr_GetTrackingState( hmdSession, time, false );

	ovr_GetEyePoses( hmdSession, 1 , ovrTrue, HmdToEyeOffset, EyeRenderPose, &sensorSampleTime );

	if ( hmdTrackingState.StatusFlags & (ovrStatus_OrientationTracked) )
	{

		orientationPose = hmdTrackingState.HeadPose.ThePose;
		
		static idQuat poseRot = idQuat_zero;
		static idAngles poseAng = ang_zero;
		
		poseRot.x = orientationPose.Orientation.z;	// x;
		poseRot.y = orientationPose.Orientation.x;	// y;
		poseRot.z = -orientationPose.Orientation.y;	// z;
		poseRot.w = orientationPose.Orientation.w;

		poseAng = poseRot.ToAngles();

		hmdAngles.roll = poseAng.roll;
		hmdAngles.pitch = poseAng.pitch;
		hmdAngles.yaw = poseAng.yaw;

	}
	
	// now read the HMD position if equiped

	currentlyTracked = hmdTrackingState.StatusFlags & (ovrStatus_PositionTracked);// ? 1 : 0;
	
	if ( currentlyTracked )
	{
		translationPose = hmdTrackingState.HeadPose.ThePose;
	}
	else
	{
		translationPose = lastTrackedPose;
	}

	position.x = -translationPose.Position.z * 39.3701f; // koz convert position (in meters) to inch (1 id unit = 1 inch).   
	position.y = -translationPose.Position.x * 39.3701f;
	position.z = translationPose.Position.y * 39.3701f;
	*/
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
	sixenseSetActiveBase(0);
	sixenseGetAllNewestData(&acd);
		
	if ( sixenseIsControllerEnabled(hydraIndex) ) 
	{
		
			sixtempq.x =  acd.controllers[hydraIndex].rot_quat[2];	// koz get hydra quat and convert to id coord space
			sixtempq.y =  acd.controllers[hydraIndex].rot_quat[0];
			sixtempq.z = -acd.controllers[hydraIndex].rot_quat[1];
			sixtempq.w =  acd.controllers[hydraIndex].rot_quat[3]; 
			
			//add pitch offset to the grip angle of the hydra to make more comfortable / correct for mounting orientation
			tempQuat = idAngles( vr_hydraPitchOffset.GetFloat(), 0.0f, 0.0f ).ToQuat();
			
			hydraData.hydraRotationQuat = tempQuat * sixtempq;
						
			hydraData.position.x =-acd.controllers[hydraIndex].pos[2] / 25.4f; // koz convert position (in MM) to inch (1 id unit = 1 inch). (mm/25.4 = inch)  
			hydraData.position.y =-acd.controllers[hydraIndex].pos[0] / 25.4f; 
			hydraData.position.z = acd.controllers[hydraIndex].pos[1] / 25.4f; 
							
			hydraData.buttons		=	acd.controllers[hydraIndex].buttons;
			hydraData.trigger		=	acd.controllers[hydraIndex].trigger;
			hydraData.joystick_x	=	acd.controllers[hydraIndex].joystick_x;
			hydraData.joystick_y	=	acd.controllers[hydraIndex].joystick_y;
				
	}

}

/*
==============
iVr::HydraGetRight
==============
*/
void iVr::HydraGetRight( hydraData &rightHydra ) 
{
	HydraGetData(hydraRightIndex, rightHydra);
}

/*
==============
iVr::HydraGetLeft
==============
*/

void iVr::HydraGetLeft( hydraData &leftHydra ) 
{
	HydraGetData( hydraLeftIndex, leftHydra);
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
		
	leftCurrent.position -= leftOffset.position ;
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

	rightCurrent.position -= rightOffset.position ;
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
iVr::GetHudAlpha
If in "look down" mode, hide weapon/health/armor stats until pitch threshold met.
If in "look down" and vr_lowHealth enabled, show health/ammo when health below threshold.
otherwise return default alpha.
==============
*/
float iVr::GetHudAlpha()
{
	static int lastFrame = idLib::frameNumber;
	static float currentAlpha = 0.0f;
	static float delta = 0.0f;
	
	idPlayer* player = gameLocal.GetLocalPlayer();

	delta = vr_hudTransparency.GetFloat() / (250 / (1000 / commonVr->hmdHz));
	
	if ( vr_hudType.GetInteger() != VR_HUD_LOOK_DOWN )
	{
		if ( player )
		{
			return player->hudActive ? vr_hudTransparency.GetFloat() : 0;
		}
		
		return vr_hudTransparency.GetFloat();
	}


	if ( lastFrame == idLib::frameNumber ) return currentAlpha;
	
	lastFrame = idLib::frameNumber;

	bool force = false;
		
	if ( player )
	{
		if ( vr_hudLowHealth.GetInteger() >= player->health && player->health >= 0 ) force = true;
	}
	
	if ( lastHMDPitch >= vr_hudAngle.GetFloat() || force ) // fade stats in
	{
		currentAlpha += delta;
		if ( currentAlpha > vr_hudTransparency.GetFloat() ) currentAlpha = vr_hudTransparency.GetFloat();
	}
	else 
	{
		currentAlpha -= delta;
		if ( currentAlpha < 0.0f ) currentAlpha = 0.0f;
	}

	return currentAlpha; 
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
	
	if ( commonVr->VR_USE_HYDRA ) // no independent aim or joystick pitch when using motion controllers.
	{
		pitchDelta = 0.0f;
		return;
	}
	
	
	float pitchDeadzone = vr_deadzonePitch.GetFloat();
	float yawDeadzone = vr_deadzoneYaw.GetFloat();


	commonVr->independentWeaponPitch += pitchDelta;
	commonVr->independentWeaponYaw += yawDelta;

	if ( vr_testWeaponModel.GetBool() ) // only used for rotating the viewmodels for testing.
	{
		if ( commonVr->independentWeaponPitch > 180.0 )	commonVr->independentWeaponPitch -= 360.0;
		if ( commonVr->independentWeaponPitch < -180.0 ) commonVr->independentWeaponPitch += 360.0;
		if ( commonVr->independentWeaponYaw > 180.0 )	commonVr->independentWeaponYaw -= 360.0;
		if ( commonVr->independentWeaponYaw < -180.0 ) commonVr->independentWeaponYaw += 360.0;
		yawDelta = 0;
		pitchDelta = 0;
		return;
	}
	
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

	static idVec3 pos = vec3_zero;
	static idVec3 pos1 = vec3_zero;
	static idAngles ang1 = ang_zero;

	commonVr->HMDGetOrientation( ang1, pos, pos1, false );

}