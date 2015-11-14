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
#include "libs\LibOVR\Include\OVR_CAPI_GL.h"
#include "..\renderer\Framebuffer.h"

// koz delete using namespace OVR;

#define RADIANS_TO_DEGREES(rad) ((float) rad * (float) (180.0 / idMath::PI))


// *** Oculus HMD Variables

idCVar vr_trackingPredictionAuto( "vr_useAutoTrackingPrediction", "1", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Use SDK tracking prediction.\n 1 = Auto, 0 = User defined." );
idCVar vr_trackingPredictionUserDefined( "vr_trackingPredictionUserDefined", "50", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "User defined tracking prediction in ms." );
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

idCVar vr_wristStatMon( "vr_wristStatMon", "1", CVAR_INTEGER | CVAR_ARCHIVE, "Use wrist status monitor. 0 = Disable 1 = Right Wrist 2 = Left Wrist " );

//koz cvars for hydra mods
idCVar vr_hydraEnable( "vr_hydraEnable", "1", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, " Enable Razer Hydra. 1 = enabled, 0 = disabled." );
idCVar vr_hydraForceDetect( "vr_hydraForceDetect", "0", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Force hydra detection. 0 = normal detection, 1 = force detection." );;
idCVar vr_hydraMode( "vr_hydraMode", "0", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, "razer hydra mode. 0 = left hydra for positional tracking, 1 = left hydra as controller, 2 = left hydra as controller and flashlight" );

//koz g_flash cvars allow tweaking of flash position when aiming with hydra
idCVar vr_flashHand_x( "vr_flashX", "12", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_flashHand_y( "vr_flashY", "-4", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_flashHand_z( "vr_flashZ", "-8", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_flashScale( "vr_flashScale", ".8", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );

//koz g_gun cvars allow tweaking of gun position when aiming with hydra
idCVar vr_gunHand_x( "vr_gunX", "12", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_gunHand_y( "vr_gunY", "4", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_gunHand_z( "vr_gunZ", "-8", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_scaleGun( "vr_scaleGun", "1", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );

idCVar vr_hydraPitchOffset( "vr_hydraPitchOffset", "40", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Pitch offset for awkward hydra grip angle" );

idCVar vr_flashPitchAngle( "vr_flashPitchAngle", "90", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Pitch offset for flashlight using hydra. Default = 90" );
idCVar vr_flashlightMode( "vr_flashlightMode", "2", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, "Flashlight mount.\n0 = Body\n1 = Head\n2 = Gun\n3= Hand ( if motion controls available.)" );

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
idCVar vr_offsetYaw( "vr_offsetYaw", "0", CVAR_FLOAT, "default yaw axis." );
idCVar vr_offsetPitch( "vr_offsetPitch", "0", CVAR_FLOAT, "default pitch axis." );
idCVar vr_offsetRoll( "vr_offsetRoll", "0", CVAR_FLOAT, "default roll axis." );

idCVar vr_forward_keyhole( "vr_forward_keyhole", "11.25", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "Forward movement keyhole in deg. If view is inside body direction +/- this value, forward movement is in view direction, not body direction" );

idCVar vr_PDAscale( "vr_PDAscale", "3", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, " PDA scale factor" );
idCVar vr_PDAfixLocation( "vr_PDAfixLocation", "1", CVAR_BOOL | CVAR_ARCHIVE | CVAR_GAME, "Fix PDA position in space in front of player\n instead of holding in hand." );

idCVar vr_mouse_gunx( "vr_mouse_gunX", "3", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_mouse_guny( "vr_mouse_gunY", "0", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_mouse_gunz( "vr_mouse_gunZ", "0", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_mouse_gun_forearm("vr_mouse_gun_forearm", "16", CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "");;

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
idCVar vr_hudLowHealth( "vr_hudLowHealth",	"0", CVAR_INTEGER | CVAR_GAME | CVAR_ARCHIVE, "When health low, reveal hud if hidden." );

idCVar vr_tweakTalkCursor( "vr_tweakTalkCursor", "41", CVAR_FLOAT | CVAR_GAME | CVAR_ARCHIVE, "Tweak talk cursor y pos in VR. % val", 0, 99 );

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



// Koz end
//===================================================================

int fboWidth;
int fboHeight;

iVr vrLocal;
iVr* vr = &vrLocal;

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
	forceLeftStick = true;	// start the PDA in the left menu.
	PDAclipModelSet = false;
	useFBO = false;
	VR_USE_HYDRA = 0;
		
	isBackgroundSaving = false;
	
	oculusIPD = 64.0f;
	oculusHeight = 72.0f;
		
	hmdPositionTracked = false;
	hmdInFrame = false;

	lastViewOrigin = vec3_zero;
	lastViewAxis = mat3_identity;
	lastHMDYaw = 0.0f;
	lastHMDPitch = 0.0f;
	lastHMDRoll = 0.0f;
	lastHMDViewOrigin = vec3_zero;
	lastHMDViewAxis = mat3_identity;
	
	hydraLeftOffset = hydra_zero;		// koz base offset for left hydra 
	hydraRightOffset = hydra_zero;		// koz base offset for right hydra 
	
	hydraLeftIndex = 0;						// koz fixme should pull these from hydra sdk but it bails for some reason using sample code - fix later.
	hydraRightIndex = 1;

	frameDataCount = 0;
	frameDataIndex[255];
	
	VR_AAmode = 0;

	independentWeaponYaw = 0;
	independentWeaponPitch = 0;

	playerDead = false;
	
	hmdWidth =  0;
	hmdHeight = 0;
					
	primaryFBOWidth = 0;
	primaryFBOHeight = 0;
	hmdHz = 75; 
	
	hmd = nullptr;
	oculusTextureSet[0] = 0;
	oculusTextureSet[1] = 0;
	oculusFboId = 0;
	ocululsDepthTexID = 0;
	oculusMirrorFboId = 0;
	oculusMirrorTexture = 0;

	inFrame = false;
	
	bool loading = false;
	int lastRead = 0;
	int currentRead = 0;
	bool updateScreen = false;

	hmdEyeImage[0] = 0;
	hmdEyeImage[1] = 0;
	hmdCurrentRender[0] = 0;
	hmdCurrentRender[1] = 0;
			
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

/*
==============
iVr::HMDInit
==============
*/

void iVr::HMDInit( void ) 
{
	
	// Oculus HMD Initialization
	
	if ( ovr_Initialize( nullptr ) == ovrSuccess )
	{
		common->Printf( "ovr_Initialize was successful.\n" );
		ovrResult result  = ovr_Create( &hmd, &luid );
		if ( result == ovrSuccess )
		
		{
			
			hmdDesc = ovr_GetHmdDesc( hmd );
						
			// configure the tracking
			common->Printf( "ovrHmd_Create was successful.\n" );
			if ( ovr_ConfigureTracking( hmd, ovrTrackingCap_Orientation
												| ovrTrackingCap_MagYawCorrection
												| ovrTrackingCap_Position, ovrTrackingCap_Orientation ) == ovrSuccess )
			{
				
				common->Printf( "ovrHmd_ConfigureTracking was successful.\n" );
				hasOculusRift = true;
				hasHMD = true;
				common->Printf( "\n\nOculus Rift HMD Initialized\n" );
				hmdInFrame = false;
				
				unsigned int caps = 0;
									
				ovr_SetEnabledCaps( hmd, caps );
				ovr_RecenterPose( hmd ); // lets start looking forward.
									
				hmdWidth = hmdDesc.Resolution.w;
				hmdHeight = hmdDesc.Resolution.h;
								
				vr->hmdHz = hmdDesc.DisplayRefreshRate + 1 ;
				com_engineHz.SetInteger( vr->hmdHz );
				
				common->Printf( "Hmd: %s .\n", hmdDesc.ProductName );
				common->Printf( "Hmd HZ %d, width %d, height %d\n", vr->hmdHz, hmdWidth, hmdHeight );
				return;
			}

		}
	}
	
	common->Printf( "\nOculus Rift not detected.\n" );
	hasHMD = false;
	hasOculusRift = false;
	game->isVR = false;

}



/*
==============
iVr::HMDInitializeDistortion
==============
*/

void iVr::HMDInitializeDistortion()
{

	if ( !vr->hmd || !vr->hasOculusRift || !vr_enable.GetBool() )
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


	int eye = 0;
	for ( int eye = 0; eye < 2; eye++ )
	{
		
		hmdEye[eye].eyeFov = vr->hmdDesc.DefaultEyeFov[eye];
		hmdEye[eye].eyeRenderDesc = ovr_GetRenderDesc( vr->hmd, (ovrEyeType)eye, hmdEye[eye].eyeFov );

		//oculus defaults znear 1 and positive zfar, id uses 1 znear, and the infinite z variation (-.999f) zfar
		//during cinematics znear is crammed to .25, so create a second matrix for cinematics
		ovrMatrix4f pEye = ovrMatrix4f_Projection( hmdEye[eye].eyeRenderDesc.Fov, 1.0f, -0.9999999999f, true ); // nzear was 0.01f zfar was 10000
		int x, y;

		for ( x = 0; x < 4; x++ )
		{
			for ( y = 0; y < 4; y++ )
			{
				hmdEye[eye].projectionRift[y * 4 + x] = pEye.M[x][y];						// convert oculus matrices to something this engine likes 
			}
		}

		hmdEye[eye].projection.x.scale = 2.0f / (hmdEye[eye].eyeFov.LeftTan + hmdEye[eye].eyeFov.RightTan);
		hmdEye[eye].projection.x.offset = (hmdEye[eye].eyeFov.LeftTan - hmdEye[eye].eyeFov.RightTan) * hmdEye[eye].projection.x.scale * 0.5f;
		hmdEye[eye].projection.y.scale = 2.0f / (hmdEye[eye].eyeFov.UpTan + hmdEye[eye].eyeFov.DownTan);
		hmdEye[eye].projection.y.offset = (hmdEye[eye].eyeFov.UpTan - hmdEye[eye].eyeFov.DownTan) * hmdEye[eye].projection.y.scale * 0.5f;

		hmdEye[eye].viewOffset = (idVec3)(-hmdEye[eye].eyeRenderDesc.HmdToEyeViewOffset.x,
			hmdEye[eye].eyeRenderDesc.HmdToEyeViewOffset.y,
			hmdEye[eye].eyeRenderDesc.HmdToEyeViewOffset.z);

		common->Printf( "EYE %d px.scale %f, px.offset %f, py.scale %f, py.offset %f\n", eye, hmdEye[eye].projection.x.scale, hmdEye[eye].projection.x.offset, hmdEye[eye].projection.y.scale, hmdEye[eye].projection.y.offset );
		common->Printf( "EYE %d viewoffset viewadjust x %f y %f z %f\n", eye, hmdEye[eye].viewOffset.x, hmdEye[eye].viewOffset.y, hmdEye[eye].viewOffset.z );

		ovrSizei rendertarget;
		ovrRecti viewport = { 0, 0, 0, 0 };

		rendertarget = ovr_GetFovTextureSize( vr->hmd, (ovrEyeType)eye, vr->hmdEye[eye].eyeFov, vr_pixelDensity.GetFloat() ); // make sure both eyes render to the same size target
		hmdEye[eye].renderTarget.h = rendertarget.h; // koz was height?
		hmdEye[eye].renderTarget.w = rendertarget.w;
		common->Printf( "Eye %d Rendertaget Width x Height = %d x %d\n", eye, rendertarget.w, rendertarget.h );

		if ( !fboCreated )
		{
			common->Printf( "Generating FBOs.\n" );
			common->Printf( "Requested pixel density = %f \n", vr_pixelDensity.GetFloat() );
			primaryFBOWidth = rendertarget.w;
			primaryFBOHeight = rendertarget.h;
			fboWidth = rendertarget.w;
			fboHeight = rendertarget.h;


			if ( !fboCreated )
			{ // create the FBOs if needed.

				VR_AAmode = vr_FBOAAmode.GetInteger();

				common->Printf( "vr_FBOAAmode %d r_multisamples %d\n", VR_AAmode, r_multiSamples.GetInteger() );
				if ( VR_AAmode == VR_AA_MSAA && r_multiSamples.GetInteger() == 0 ) VR_AAmode = VR_AA_NONE;

				if ( VR_AAmode == VR_AA_FXAA )
				{// enable FXAA

					VR_AAmode = VR_AA_NONE;
										
				}

				if ( VR_AAmode == VR_AA_MSAA )
				{// enable MSAA
					GL_CheckErrors();

					common->Printf( "Creating %d x %d MSAA framebuffer\n", rendertarget.w, rendertarget.h );
					globalFramebuffers.primaryFBO = new Framebuffer( "_primaryFBO", rendertarget.w, rendertarget.h, true ); // koz
					common->Printf( "Adding Depth/Stencil attachments to MSAA framebuffer\n" );
					globalFramebuffers.primaryFBO->AddDepthStencilBuffer( GL_DEPTH24_STENCIL8 );
					common->Printf( "Adding color attachment to MSAA framebuffer\n" );
					globalFramebuffers.primaryFBO->AddColorBuffer( GL_RGBA, 0 );
					
					int status = globalFramebuffers.primaryFBO->Check();
					globalFramebuffers.primaryFBO->Error( status );
				
					common->Printf( "Creating resolve framebuffer\n" );
					globalFramebuffers.resolveFBO = new Framebuffer( "_resolveFBO", rendertarget.w, rendertarget.h, false ); // koz
					common->Printf( "Adding Depth/Stencil attachments to framebuffer\n" );
					globalFramebuffers.resolveFBO->AddDepthStencilBuffer( GL_DEPTH24_STENCIL8 );
					common->Printf( "Adding color attachment to framebuffer\n" );
					globalFramebuffers.resolveFBO->AddColorBuffer( GL_RGBA, 0 );

					status = globalFramebuffers.resolveFBO->Check();
					globalFramebuffers.resolveFBO->Error( status );
					
					fboWidth = globalFramebuffers.primaryFBO->GetWidth();// rendertarget.w;
					fboHeight = globalFramebuffers.primaryFBO->GetHeight();
					common->Printf( "Globalframebuffer w x h  = %d x %d\n", fboWidth, fboHeight );
					rendertarget.w = fboWidth;
					rendertarget.h = fboHeight;
					
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

				if ( !fboCreated /*!VR_FBO.valid*/ )
				{ // either AA disabled or AA buffer creation failed. Try creating unaliased FBOs.

					//primaryFBOimage = globalImages->ImageFromFunction( "_primaryFBOimage", R_MakeFBOImage );
					common->Printf( "Creating framebuffer\n" );
					globalFramebuffers.primaryFBO = new Framebuffer( "_primaryFBO", rendertarget.w, rendertarget.h, false ); // koz
					common->Printf( "Adding Depth/Stencil attachments to framebuffer\n" );
					globalFramebuffers.primaryFBO->AddDepthStencilBuffer( GL_DEPTH24_STENCIL8 );
					common->Printf( "Adding color attachment to framebuffer\n" );
					globalFramebuffers.primaryFBO->AddColorBuffer( GL_RGBA8, 0 );
				
					
					//globalFramebuffers.primaryFBO->AttachImage2D( GL_TEXTURE_2D, primaryFBOimage, 0 );
					//primaryFBOimage->Bind();

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
		}

		if ( !useFBO ) { // not using FBO's, will render to default framebuffer (screen) 

			rendertarget.w = renderSystem->GetNativeWidth() / 2;
			rendertarget.h = renderSystem->GetNativeHeight();
			hmdEye[eye].renderTarget = rendertarget;


		}

		viewport.Size.w = rendertarget.w;
		viewport.Size.h = rendertarget.h;

		
	}

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

	common->Printf( "Init Hmd FOV x,y = %f , %f. Aspect = %f, PixelScale = %f\n", hmdFovX, hmdFovY, hmdAspect, hmdPixelScale );
	common->Printf( "Creating oculus texture set width = %d height = %d.\n", hmdEye[0].renderTarget.w , hmdEye[0].renderTarget.h );
	
	// create the swap texture sets 
	if ( (ovr_CreateSwapTextureSetGL( hmd, GL_SRGB8_ALPHA8, hmdEye[0].renderTarget.w, hmdEye[0].renderTarget.h,
		&oculusTextureSet[0] ) != ovrSuccess) || ovr_CreateSwapTextureSetGL( hmd, GL_SRGB8_ALPHA8, hmdEye[1].renderTarget.w, hmdEye[1].renderTarget.h,
		&oculusTextureSet[1] ) != ovrSuccess ) {
		common->Warning( "iVr::HMDInitializeDistortion unable to create OVR swap texture set.\n VR mode is DISABLED.\n" );
		game->isVR = false;

	}

	for ( int j = 0; j < 2; j++ )
	{
	
		for ( int i = 0; i < oculusTextureSet[j]->TextureCount; ++i )
		{
			ovrGLTexture* tex = (ovrGLTexture*)&oculusTextureSet[j]->Textures[i];
			glBindTexture( GL_TEXTURE_2D, tex->OGL.TexId );

			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}
	}
	glGenFramebuffers( 1, &oculusFboId );

	glGenTextures( 1, &ocululsDepthTexID );
		
	glBindTexture( GL_TEXTURE_2D, ocululsDepthTexID );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, hmdEye[0].renderTarget.w, hmdEye[0].renderTarget.h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL );


	ovr_CreateMirrorTextureGL( hmd, GL_RGBA , hmdDesc.Resolution.w / 2, hmdDesc.Resolution.h / 2, (ovrTexture**)&oculusMirrorTexture );
	glGenFramebuffers( 1, &oculusMirrorFboId );
	glBindFramebuffer( GL_READ_FRAMEBUFFER, oculusMirrorFboId );
	glFramebufferTexture2D( GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, oculusMirrorTexture->OGL.TexId, 0 );
	glFramebufferRenderbuffer( GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0 );
	glBindFramebuffer( GL_READ_FRAMEBUFFER, 0 );

	oculusLayer.Header.Type = ovrLayerType_EyeFov;
	oculusLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;
	oculusLayer.ColorTexture[0] = oculusTextureSet[0];
	oculusLayer.ColorTexture[1] = oculusTextureSet[1];
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

	globalFramebuffers.primaryFBO->Bind();

	GL_CheckErrors();
} 
		
/*
==============`
iVr::HMDGetOrientation
==============
*/

void iVr::HMDGetOrientation(float &roll, float &pitch, float &yaw, idVec3 &hmdPosition) 
{
	
	if ( !hasOculusRift || !hasHMD ) 
	{
		roll = 0;
		pitch =0;
		yaw = 0;
		hmdPosition = idVec3 ( 0, 0, 0 ) ;
		return;
	}
	static double time = 0.0;
	static ovrPosef translationPose;
	static ovrPosef	orientationPose; 
	static ovrPosef lastTrackedPose = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } ;
	static bool currentlyTracked;
		
	if ( hasOculusRift && hmd ) 
	{
        
		if ( vr_trackingPredictionAuto.GetBool() ) 
		{
			hmdFrameTime = ovr_GetFrameTiming( hmd, renderSystem->GetFrameCount() );// idLib::frameNumber );
			time = hmdFrameTime.DisplayMidpointSeconds;
		} 
		else 
		{
			time = ovr_GetTimeInSeconds() + ( vr_trackingPredictionUserDefined.GetFloat() / 1000 );
		}
			
		hmdTrackingState = ovr_GetTrackingState( hmd, time );
				
		if (hmdTrackingState.StatusFlags & ( ovrStatus_OrientationTracked ) )
		{
							
				orientationPose = hmdTrackingState.HeadPose.ThePose; 
				
				//float y = 0.0f, p = 0.0f, r = 0.0f;
				static idQuat poseRot;
				static idAngles poseAng;

				//orientationPose.Rotation.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&y, &p, &r);
				
				poseRot.x = orientationPose.Orientation.z;// x;
				poseRot.y = orientationPose.Orientation.x;// y;
				poseRot.z = -orientationPose.Orientation.y;// z;
				poseRot.w = orientationPose.Orientation.w;

				poseAng = poseRot.ToAngles();

				roll = poseAng.roll;
				pitch = poseAng.pitch;
				yaw = poseAng.yaw;
						

				//roll =   -RADIANS_TO_DEGREES(r); // ???
				//pitch =  -RADIANS_TO_DEGREES(p); // should be degrees down
				//yaw =     RADIANS_TO_DEGREES(y); // should be degrees left
		}
		
	}
	else 
	{
			roll  = angles[ROLL];
			pitch = angles[PITCH];
			yaw   = angles[YAW];
	}
		
	// now read the HMD position if equiped

	currentlyTracked = hmdTrackingState.StatusFlags & ( ovrStatus_PositionTracked ) ? 1 : 0;
	//common->Printf("currently tracked = %d\n", currentlyTracked);

	if ( currentlyTracked ) 
	{
			translationPose = hmdTrackingState.HeadPose.ThePose; 
			lastTrackedPose = translationPose;
	} 
	else
	{	
			translationPose = lastTrackedPose;
	}
				
	hmdPosition.x = ( -translationPose.Position.z * 1000 ) / 25.4f ; // koz convert position (in meters) to inch (1 id unit = 1 inch). (mm/25.4 = inch)   
	hmdPosition.y = ( -translationPose.Position.x * 1000 ) / 25.4f ; 
	hmdPosition.z = ( translationPose.Position.y * 1000 ) / 25.4f ; 
			
	if ( hmdDesc.AvailableTrackingCaps && (ovrTrackingCap_Position) ) 
	{
		if ( currentlyTracked && !hmdPositionTracked ) 
		{
			common->Warning( "Oculus Positional tracking established.\n" );
			hmdPositionTracked = true;
		} 
		else if ( !currentlyTracked && hmdPositionTracked ) 
		{
			common->Warning( "Oculus Positional tracking LOST.\n" );
			hmdPositionTracked = false;
		}
	}
}

/*
==============
iVr::FrameStart
==============
*/
void iVr::FrameStart( int index ) 
{
	// koz fixme move these checks 

	

	if ( vr_hydraEnable.IsModified() )
	{
		vr->HydraInit();
		vr_hydraEnable.ClearModified();
	}
	static idVec3 pos;
	static float roll = 0;
	static float pitch = 0;
	static float yaw = 0;

	vr->HMDGetOrientation( roll, pitch, yaw, pos );

}



/*
==============
iVr::FrameEnd
==============
*/
void iVr::FrameEnd() 
{
	// koz fixme

	//ovrHmd_EndFrameTiming( vr->hmd );
	//hmdInFrame = false;

}

/*
==============
iVr::FrameWait
==============
*/
void iVr::FrameWait()
{
	return;
	// ovr_WaitTillTime( hmdFrameTime. TimewarpPointSeconds );

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
	
	idQuat sixtempq;
	sixenseAllControllerData acd;
	sixenseSetActiveBase(0);
	sixenseGetAllNewestData(&acd);

	static idQuat tempQuat = idQuat_zero;
		
	if ( sixenseIsControllerEnabled(hydraIndex) ) 
	{
		
			sixtempq.x =  acd.controllers[hydraIndex].rot_quat[2];	// koz get hydra quat and convert to id coord space
			sixtempq.y =  acd.controllers[hydraIndex].rot_quat[0];
			sixtempq.z = -acd.controllers[hydraIndex].rot_quat[1];
			sixtempq.w =  acd.controllers[hydraIndex].rot_quat[3]; 
			
			//add pitch offset to the grip angle of the hydra to make mor comfortable / correct for mounting orientation
			tempQuat = idAngles(vr_hydraPitchOffset.GetFloat(),0.0f,0.0f).ToQuat();
			hydraData.hydraRotationQuat = tempQuat * sixtempq;

			//hydraData.hydraRotation = sixtempq.ToMat3(); // koz convert quat to rotation matrix in id space
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

	HydraGetLeftOffset( leftOffset );
	HydraGetLeft( leftCurrent );
	
	idQuat offsetAngles = leftOffset.hydraRotationQuat;
	idQuat currentAngles = leftCurrent.hydraRotationQuat;

	idAngles ca = currentAngles.ToAngles();
	idAngles oa = offsetAngles.ToAngles();
		
	leftCurrent.position -= leftOffset.position ;
	leftCurrent.position *= offsetAngles.ToMat3().Inverse();
		
	ca.yaw -= oa.yaw;
	ca.Normalize180();

	leftCurrent.hydraRotationQuat = ca.ToQuat();
	leftOffsetHydra = leftCurrent;
}

/*
==============
iVr::HydraGetRightWithOffset
==============
*/

void iVr::HydraGetRightWithOffset( hydraData &rightOffsetHydra ) { // will return the right hydra position with the offset angle and position values.
		
	static hydraData rightCurrent = hydra_zero;
	static hydraData rightOffset = hydra_zero;
	
	HydraGetRightOffset( rightOffset );
	HydraGetRight( rightCurrent );
	
	idQuat offsetAngles = rightOffset.hydraRotationQuat;
	idQuat currentAngles = rightCurrent.hydraRotationQuat;
	
	idAngles ca = currentAngles.ToAngles();
	idAngles oa = offsetAngles.ToAngles();
	
	rightCurrent.position -= rightOffset.position ;
	rightCurrent.position *= offsetAngles.ToMat3().Inverse();
		
	ca.yaw -= oa.yaw;
	ca.Normalize180();
		
	rightCurrent.hydraRotationQuat = ca.ToQuat(); 
	rightOffsetHydra = rightCurrent;
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

	delta = vr_hudTransparency.GetFloat() / (250 / (1000 / vr->hmdHz));
	
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
	
	if ( vr->VR_USE_HYDRA ) // no independent aim or joystick pitch when using motion controllers.
	{
		pitchDelta = 0.0f;
		return;
	}
	
	
	float pitchDeadzone = vr_deadzonePitch.GetFloat();
	float yawDeadzone = vr_deadzoneYaw.GetFloat();


	vr->independentWeaponPitch += pitchDelta;
	vr->independentWeaponYaw += yawDelta;

	if ( vr_testWeaponModel.GetBool() ) // only used for rotating the viewmodels for testing.
	{
		if ( vr->independentWeaponPitch > 180.0 )	vr->independentWeaponPitch -= 360.0;
		if ( vr->independentWeaponPitch < -180.0 ) vr->independentWeaponPitch += 360.0;
		if ( vr->independentWeaponYaw > 180.0 )	vr->independentWeaponYaw -= 360.0;
		if ( vr->independentWeaponYaw < -180.0 ) vr->independentWeaponYaw += 360.0;
		yawDelta = 0;
		pitchDelta = 0;
		return;
	}
	
	if ( vr->independentWeaponPitch >= pitchDeadzone ) vr->independentWeaponPitch = pitchDeadzone;
	if ( vr->independentWeaponPitch < -pitchDeadzone ) vr->independentWeaponPitch = -pitchDeadzone;
	pitchDelta = 0;
	
	if ( vr->independentWeaponYaw >= yawDeadzone )
	{
		yawDelta = vr->independentWeaponYaw - yawDeadzone;
		vr->independentWeaponYaw = yawDeadzone;
		return;
	}

	if ( vr->independentWeaponYaw < -yawDeadzone )
	{
		yawDelta = vr->independentWeaponYaw + yawDeadzone;
		vr->independentWeaponYaw = -yawDeadzone;
		return;
	}
		
	yawDelta = 0.0f;
	
}