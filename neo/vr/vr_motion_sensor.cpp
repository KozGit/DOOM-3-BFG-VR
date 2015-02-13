#pragma hdrstop

#include "../../idlib/precompiled.h"

#undef strncmp
#undef vsnprintf		
#undef _vsnprintf		


#include "vr.h"
#include "d3xp\Game_local.h"
#include "d3xp\physics\Clip.h"
#include "..\dependencies\SixenseSDK_062612\include\sixense_utils\controller_manager\controller_manager.hpp"
#include "..\dependencies\LibOVR\Include\OVR.h"
//koz fixme #include "Kernel/OVR_Math.h"

using namespace OVR;

#define RADIANS_TO_DEGREES(rad) ((float) rad * (float) (180.0 / idMath::PI))

bool hasHMD = false;
bool hasOculusRift = false;
bool VR_GAME_PAUSED = false;
bool PDAforcetoggle = false;
bool PDAforced = false;
bool PDArising = false;
bool forceLeftStick = true;	// start the PDA in the left menu.
bool PDAclipModelSet = false;
int VR_USE_FBO = 0;
int VR_USE_HYDRA = 0;

bool vrIsBackgroundSaving = false;

fbo_t VR_FBO;
fbo_t VR_FullscreenFBO;
fbo_t VR_ResolveAAFBO;

ovrHmd hmd;
hmdEye_t hmdEye[2];
int currentRiftEye;
float hmdFovX; 
float hmdFovY; 
float hmdPixelScale;
float hmdAspect;

float oculusIPD = 64.0f;
float oculusHeight = 72.0f;

ovrTrackingState hmdTrackingState;
ovrFrameTiming hmdFrameTime;
bool hmdPositionTracked = false;
bool hmdInFrame = false;

idVec3 lastViewOrigin = vec3_zero;
idMat3 lastViewAxis = mat3_identity;
float lastHMDYaw = 0.0f;

float angles[3];
static hydraData leftHydraOffset = hydra_zero;		// koz base offset for left hydra 
static hydraData rightHydraOffset = hydra_zero;		// koz base offset for right hydra 

idQuat idQuat_zero = idQuat( 0.0f, 0.0f, 0.0f, 0.0f );

const int leftHydraIndex = 0;						// koz fixme should pull these from hydra sdk but it bails for some reason using sample code - fix later.
const int rightHydraIndex = 1;

static int frameDataCount = 0;
static int frameDataIndex[255];
static ovrPosef frameDataPose[255];

int VR_AAmode = 0;

float independentWeaponYaw = 0;
float independentWeaponPitch = 0;

//koz g_flash cvars allow tweaking of flash position when aiming with hydra
idCVar vr_flash_x(					"vr_flashX",				"12",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_flash_y(					"vr_flashY",				"-4",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_flash_z(					"vr_flashZ",				"-8",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_flashScale(				"vr_flashScale",			".8",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
//koz g_gun cvars allow tweaking of gun position when aiming with hydra
idCVar vr_gun_x(					"vr_gunX",					"12",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_gun_y(					"vr_gunY",					"4",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_gun_z(					"vr_gunZ",					"-8",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar vr_scaleGun(					"vr_scaleGun",				"1",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "" );

idCVar vr_hydraPitchOffset(			"vr_hydraPitchOffset",		"40",			CVAR_GAME | CVAR_ARCHIVE | CVAR_FLOAT, "Pitch offset for awkward hydra grip angle" );

// *** Oculus HMD Variables
    
idCVar vr_trackingPredictionAuto( "vr_useAutoTrackingPrediction", "1", CVAR_BOOL|CVAR_ARCHIVE|CVAR_GAME,"Use SDK tracking prediction.\n 1 = Auto, 0 = User defined.");
idCVar vr_trackingPredictionUserDefined( "vr_trackingPredictionUserDefined" , "50" , CVAR_FLOAT|CVAR_ARCHIVE|CVAR_GAME,"User defined tracking prediction in ms.");
idCVar vr_useOculusProjectionMatrix( "vr_useOculusProjectionMatrix" , "0" , CVAR_BOOL|CVAR_ARCHIVE|CVAR_GAME,"0(Default) Let engine calc projection Matrices. 1 = Oculus defined(testing only).");
idCVar vr_pixelDensity( "vr_pixelDensity", "1.33", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME,"");
idCVar vr_lowPersistence( "vr_lowPersistence" , "1", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, "Enable low persistence. 0 = off 1 = on");
idCVar vr_vignette( "vr_vignette" , "1", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, "Enable warp vignette. 0 = off 1 = on");
idCVar vr_FBOEnabled( "vr_FBOEnabled" , "0", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_RENDERER, "Use FBO rendering path." );
idCVar vr_FBOAAmode( "vr_FBOAAmode" , "2", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_RENDERER, "Antialiasing mode. 0 = Disabled 1 = FXAA 2=MSAA\n");
idCVar vr_enable( "vr_enable", "0", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, "Enable VR mode. 0 = Disabled 1 = Enabled."); 
idCVar vr_FBOscale( "vr_FBOscale", "1.0" , CVAR_FLOAT | CVAR_ARCHIVE | CVAR_RENDERER, "FBO scaling factor."); 
idCVar vr_scale( "vr_scale", "1.0", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "VR World scale adjustment.");
idCVar vr_useOculusProfile( "vr_useOculusProfile", "1", CVAR_INTEGER | CVAR_ARCHIVE | CVAR_GAME, "Use Oculus Profile values. 0 = use user defined profile, 1 = use Oculus profile.");
idCVar vr_manualIPD( "vr_manualIPD", "64", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "User defined IPD value in MM");
idCVar vr_manualHeight( "vr_manualHeight", "70", CVAR_FLOAT | CVAR_ARCHIVE | CVAR_GAME, "User defined player height in inches");

//===================================================================

/*
==============
VR_Hydra_Init
==============
*/

void VR_HydraInit(void) {
	
	VR_USE_HYDRA = FALSE;
	if ( vr_hydraEnable.GetInteger() != 0 ) {
		int x = sixenseInit();
		common->Printf("Sixense init = %d \n", x);
		if ( x == SIXENSE_SUCCESS /*sixenseInit()*/ ) { // koz init the hydra 
			x = sixenseSetActiveBase(0);
			common->Printf("Sixense sixenseSetActiveBase(0) = %d \n", x);
			if (x == SIXENSE_SUCCESS /*sixenseSetActiveBase( 0 )*/) {
				x = sixenseIsBaseConnected(0);
				common->Printf("SixensesixenseIsBaseConnected( 0 ) = %d \n", x);
				if ( 1 /* 1 sixenseIsBaseConnected( 0 )*/ ) {
					common->Printf( "VR_HydraInit: Hydra base 0 available.\n" );
					sixenseUtils::getTheControllerManager()->setGameType( sixenseUtils::ControllerManager::ONE_PLAYER_TWO_CONTROLLER );
					VR_USE_HYDRA = TRUE;
				}
			}
		}
	}
	common->Printf("VR_USE_HYDRA = %d\n", VR_USE_HYDRA);
}

/*
==============
VR_HMDInit
==============
*/

void VR_HMDInit(void) {
	
	// Oculus HMD Initialization
		
	ovr_Initialize();
	hmd = ovrHmd_Create( 0 );
    
	if ( hmd ) {
		
		// configure the tracking
								
		if ( ovrHmd_ConfigureTracking( hmd,	ovrTrackingCap_Orientation 
											| ovrTrackingCap_MagYawCorrection
											| ovrTrackingCap_Position, ovrTrackingCap_Orientation ) ) { 
			hasOculusRift = true;
			hasHMD = true;
			common->Printf( "\nOculus Rift HMD Initialized\n" );
			hmdInFrame = false;
			ovrHmd_ResetFrameTiming(hmd,0);
		
		
			currentRiftEye = 0;
		
			unsigned int caps = 0;
	
			//if ( hmd->HmdCaps & ovrHmdCap_DynamicPrediction )
			//caps |= ovrHmdCap_DynamicPrediction;

			if ( hmd->HmdCaps & ovrHmdCap_LowPersistence && vr_lowPersistence.GetInteger() )
			caps |= ovrHmdCap_LowPersistence;

			caps |= ovrHmdCap_NoVSync;  
		
			ovrHmd_SetEnabledCaps( hmd, caps );

			VR_FBO.valid = false;
			VR_ResolveAAFBO.valid = false;

			ovrHmd_RecenterPose( hmd ); // lets start looking forward.
			
		}
    	
	} else  {            
			common->Printf( "\nOculus Rift not detected.\n" );
			hasHMD = false;
			hasOculusRift = false;
	}

	
	
}

/*
==============
VR_GenerateDistortionMeshes
==============
*/

void VR_GenerateDistortionMeshes()  {
	
	if ( !hmd && !hasOculusRift) return;

	// koz : create distortion meshes for oculus - copy verts and indexes from oculus supplied mesh
	
	VR_USE_FBO = vr_FBOEnabled.GetInteger();
		
	int eye = 0;
	for ( eye = 0 ; eye < 2 ; eye++ ) {

		ovrDistortionMesh meshData = {};
		distortionVert_t *mesh = NULL;
		distortionVert_t *v = NULL;
		ovrDistortionVertex *ov = NULL;
		unsigned int vtex = 0;
		
		hmdEye[eye].eyeFov = hmd->DefaultEyeFov[ eye ];
		hmdEye[eye].eyeRenderDesc = ovrHmd_GetRenderDesc( hmd, ( ovrEyeType ) eye, hmdEye[ eye ].eyeFov );
		
		//oculus defaults znear 1 and positive zfar, id uses 1 znear, and the infinite z variation (-.999f) zfar
		//during cinematics znear is crammed to .25, so create a second matrix for cinematics
		Matrix4f pEye = ovrMatrix4f_Projection( hmdEye[ eye ].eyeRenderDesc.Fov, 1.0f, -0.9999999999f, true ); // nzear was 0.01f zfar was 10000
		Matrix4f pEyeCramZnear = ovrMatrix4f_Projection( hmdEye[ eye ].eyeRenderDesc.Fov, 0.25f, -0.999f, true ); // nzear was 0.01f zfar was 10000
		//common->Printf("\n\nHMD OFFSETS\n---------------\nhmdEye[%d].eyerenderdesc.viewadjust.x = %f, hmdEye[eye].viewoffset.x = %f \n",eye,hmdEye[eye].eyeRenderDesc.ViewAdjust.x,hmdEye[eye].viewOffset.x);
		int x,y;
					
		for ( x=0 ; x<4 ; x++ ) {
			for ( y = 0 ; y<4 ; y++ ) {
				hmdEye[eye].projectionRift[ y*4 + x ] = pEye.M[x][y];						// convert oculus matrices to something this engine likes 
				hmdEye[eye].projectionRiftCramZnear[ y*4 + x ] = pEyeCramZnear.M[x][y];	// convert oculus matrices to something this engine likes
				
			}
		}
		
		
		hmdEye[eye].projection.x.scale = 2.0f / ( hmdEye[eye].eyeFov.LeftTan + hmdEye[eye].eyeFov.RightTan );
		hmdEye[eye].projection.x.offset = ( hmdEye[eye].eyeFov.LeftTan - hmdEye[eye].eyeFov.RightTan ) * hmdEye[eye].projection.x.scale * 0.5f;
		hmdEye[eye].projection.y.scale = 2.0f / ( hmdEye[eye].eyeFov.UpTan + hmdEye[eye].eyeFov.DownTan );
		hmdEye[eye].projection.y.offset = ( hmdEye[eye].eyeFov.UpTan - hmdEye[eye].eyeFov.DownTan ) * hmdEye[eye].projection.y.scale * 0.5f;
	
		/*hmdEye[eye].viewOffset = (idVec3)( -hmdEye[eye].eyeRenderDesc.ViewAdjust.x ,	
												hmdEye[eye].eyeRenderDesc.ViewAdjust.y ,
												hmdEye[eye].eyeRenderDesc.ViewAdjust.z); */

		hmdEye[eye].viewOffset = (idVec3)(-hmdEye[eye].eyeRenderDesc.HmdToEyeViewOffset.x,
			hmdEye[eye].eyeRenderDesc.HmdToEyeViewOffset.y,
			hmdEye[eye].eyeRenderDesc.HmdToEyeViewOffset.z);
				
		common->Printf("EYE %d px.scale %f, px.offset %f, py.scale %f, py.offset %f\n",eye,hmdEye[eye].projection.x.scale,hmdEye[eye].projection.x.offset,hmdEye[eye].projection.y.scale,hmdEye[eye].projection.y.offset);
		common->Printf("EYE %d viewoffset viewadjust x %f y %f z %f\n",eye,hmdEye[eye].viewOffset.x,hmdEye[eye].viewOffset.y,hmdEye[eye].viewOffset.z);
		
		ovrSizei rendertarget;
		ovrRecti viewport = { 0, 0, 0 ,0 };
		//rendertarget = ovrHmd_GetFovTextureSize( hmd, (ovrEyeType) eye, hmdEye[eye].eyeFov, vr_pixelDensity.GetFloat() );
		rendertarget = ovrHmd_GetFovTextureSize( hmd, (ovrEyeType) 0, hmdEye[0].eyeFov, vr_pixelDensity.GetFloat() ); // make sure both eyes render to the same size target
				
		if ( VR_USE_FBO != 0  ) {
			common->Printf("Using FBOs \n");
			common->Printf("Requested pixel density = %f \n",vr_pixelDensity.GetFloat() );
			common->Printf("Eye %d Rendertaget Width x Height = %d x %d\n",eye,rendertarget.w, rendertarget.h);
			hmdEye[eye].renderTarget.h = rendertarget.w;
			hmdEye[eye].renderTarget.w = rendertarget.w;
									
			if ( !VR_FBO.valid ) { // create the FBOs if needed.
			
				VR_AAmode = vr_FBOAAmode.GetInteger();

				if ( VR_AAmode == 2 && r_multiSamples.GetInteger() /*vr_FBOMSAALevel.GetInteger()*/ == 0 ) VR_AAmode = VR_AA_NONE;
								
				if ( VR_AAmode == VR_AA_FXAA ) {// enable FXAA
					
					VR_GenFBO( rendertarget.w, rendertarget.h, VR_FBO, true, false  );
					VR_GenFBO( rendertarget.w , rendertarget.h , VR_ResolveAAFBO, false, false );
					VR_GenFBO( (rendertarget.w * 2) , rendertarget.h, VR_FullscreenFBO, false, false );
					if ( !VR_ResolveAAFBO.valid || !VR_FBO.valid || !VR_FullscreenFBO.valid ) {
						VR_DeleteFBO( VR_FBO );
						VR_DeleteFBO( VR_ResolveAAFBO );
						VR_DeleteFBO( VR_FullscreenFBO );
						common->Warning("Unable to create FBOs for FXAA antialiasing. Attempting to create FBOs with FXAA disabled.\n", rendertarget.w, rendertarget.h ); 
					} else {
						common->Printf("Successfully created two %d x %d FBOs for FXAA \n", rendertarget.w, rendertarget.h ); 
						VR_FBO.MSAAsamples = 0;
						VR_FBO.useFXAA = TRUE;
					}
				}

				if ( VR_AAmode == VR_AA_MSAA ) {// enable FXAA
					
					VR_GenFBO( rendertarget.w, rendertarget.h, VR_FBO, true, true  );
					VR_GenFBO( rendertarget.w , rendertarget.h , VR_ResolveAAFBO, true, false );
					VR_GenFBO( (rendertarget.w * 2) , rendertarget.h, VR_FullscreenFBO, false, false );
					if ( !VR_ResolveAAFBO.valid || !VR_FBO.valid || !VR_FullscreenFBO.valid ) {
						VR_DeleteFBO( VR_FBO );
						VR_DeleteFBO( VR_ResolveAAFBO );
						VR_DeleteFBO( VR_FullscreenFBO );
						common->Warning("Unable to create FBOs for MSAA antialiasing. Attempting to create FBOs with MSAA disabled.\n", rendertarget.w, rendertarget.h ); 
					} else {
						common->Printf("Successfully created %d x %d FBO for MSAA \n", rendertarget.w, rendertarget.h ); 
						VR_FBO.useFXAA = FALSE;
					}
				}

				if ( !VR_FBO.valid ) { // either AA disabled or AA buffer creation failed. Try creating unaliased FBOs.

					VR_GenFBO( rendertarget.w, rendertarget.h, VR_FBO, true, false );
					VR_GenFBO( (rendertarget.w * 2) , rendertarget.h, VR_FullscreenFBO, false, false );
					VR_FBO.useFXAA = false;
					
					if ( !VR_FBO.valid  || !VR_FullscreenFBO.valid ) {
						VR_DeleteFBO( VR_FBO );
						VR_DeleteFBO( VR_FullscreenFBO );
						VR_USE_FBO = false; // FBO creation failed, fallback to default rendering.
						common->Warning( "Unable to create FBO. Rendering to default framebuffer.\n" );
					} else common->Printf( "Succesfully created %d x %d FBO. Antialiasing DISABLED.\n", rendertarget.w, rendertarget.h ); 
				}
			}
		}

		if ( !VR_USE_FBO ) { // not using FBO's, will render to default framebuffer (screen) 
			
			rendertarget.w = renderSystem->GetNativeWidth() / 2 ;
			rendertarget.h = renderSystem->GetNativeHeight();
			hmdEye[eye].renderTarget = rendertarget;
			//hmdEye[eye].renderTarget.w = rendertarget.w;
			
		} 
		
		viewport.Size.w = rendertarget.w;
		viewport.Size.h = rendertarget.h;
				
		ovrHmd_GetRenderScaleAndOffset( hmdEye[eye].eyeFov, rendertarget, viewport, (ovrVector2f*) hmdEye[eye].UVScaleoffset );
		
		common->Printf( "Eye %d UVscale x %f y %f  offset x %f , y %f\n", eye,
			hmdEye[eye].UVScaleoffset[0].x,
			hmdEye[eye].UVScaleoffset[0].y,	
			hmdEye[eye].UVScaleoffset[1].x,	
			hmdEye[eye].UVScaleoffset[1].y ); 
						
		// create the distortion mesh for this eye
		ovrHmd_CreateDistortionMesh( hmd, 
									 hmdEye[eye].eyeRenderDesc.Eye, 
									 hmdEye[eye].eyeRenderDesc.Fov, 
									 ovrDistortionCap_Chromatic /* | ovrDistortionCap_SRGB | ovrDistortionCap_TimeWarp | ovrDistortionCap_Vignette */ ,
									 &meshData );

		hmdEye[eye].vbo.numVerts = meshData.VertexCount;
		hmdEye[eye].vbo.numIndexes = meshData.IndexCount;
		common->Printf(" %d vertexes  %d indexes", meshData.VertexCount, meshData.IndexCount);
		
		mesh = (distortionVert_t *) malloc( sizeof(distortionVert_t) * meshData.VertexCount );
		v = mesh;
		ov = meshData.pVertexData; 
		
		for ( vtex = 0; vtex < meshData.VertexCount; vtex++ )	{
				
			v->pos.x = ov->ScreenPosNDC.x;
			v->pos.y = ov->ScreenPosNDC.y;
			v->texR = ( *(idVec2*)&ov->TanEyeAnglesR ); 
			v->texG = ( *(idVec2*)&ov->TanEyeAnglesG );
			v->texB = ( *(idVec2*)&ov->TanEyeAnglesB ); 
			v->color[0] = v->color[1] = v->color[2] = (GLubyte)( ov->VignetteFactor * 255.99f );
			v->color[3] = (GLubyte)( ov->TimeWarpFactor * 255.99f );
			v++; ov++;
		}
				
		if ( !hmdEye[eye].vbo.vertBuffHandle ) {
			glGenBuffers( 1, &hmdEye[eye].vbo.vertBuffHandle );
			GL_CheckErrors();
		}
		
		if ( !hmdEye[eye].vbo.indexBuffHandle ) {	
			glGenBuffers( 1 , &hmdEye[eye].vbo.indexBuffHandle );
			GL_CheckErrors();
		}

		if ( hmdEye[eye].vbo.vertBuffHandle && hmdEye[eye].vbo.indexBuffHandle ) {
			//build the mesh vbo
			GLint oldVertArray , oldIndexArray = 0;
			glBindVertexArray(glConfig.global_vao);
			common->Printf("Bound VAO %d\n", glConfig.global_vao);
			glGetIntegerv( GL_ARRAY_BUFFER_BINDING , (GLint *) &oldVertArray ); 
			GL_CheckErrors();
			glGetIntegerv( GL_ELEMENT_ARRAY_BUFFER_BINDING, (GLint *) &oldIndexArray );
			GL_CheckErrors();
						
			glBindBuffer( GL_ARRAY_BUFFER, hmdEye[eye].vbo.vertBuffHandle );
			GL_CheckErrors();
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, hmdEye[eye].vbo.indexBuffHandle );
			GL_CheckErrors();
		
			glBufferData( GL_ARRAY_BUFFER, sizeof(distortionVert_t) * meshData.VertexCount, mesh,GL_STATIC_DRAW );
			GL_CheckErrors();
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * meshData.IndexCount, meshData.pIndexData, GL_STATIC_DRAW); GL_CheckErrors();
		
			glBindBuffer( GL_ARRAY_BUFFER, oldVertArray );
			GL_CheckErrors();
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, oldIndexArray );		
			GL_CheckErrors();
		
		} else {		
			common->Warning("ERROR: Unable to allocate vertex buffer data");
		}
		
		free(mesh);
		ovrHmd_DestroyDistortionMesh( &meshData ); 
	}
	
	// calculate fov for engine
	float combinedTanHalfFovHorizontal = std::max ( std::max ( hmdEye[0].eyeFov.LeftTan, hmdEye[0].eyeFov.RightTan ), std::max ( hmdEye[1].eyeFov.LeftTan, hmdEye[1].eyeFov.RightTan ) );
	float combinedTanHalfFovVertical = std::max ( std::max ( hmdEye[0].eyeFov.UpTan, hmdEye[0].eyeFov.DownTan ), std::max ( hmdEye[1].eyeFov.UpTan, hmdEye[1].eyeFov.DownTan ) );
	float horizontalFullFovInRadians = 2.0f * atanf ( combinedTanHalfFovHorizontal ); 
	
	hmdFovX = RAD2DEG( horizontalFullFovInRadians );
	hmdFovY = RAD2DEG( 2.0 * atanf(combinedTanHalfFovVertical ) );
	hmdAspect = combinedTanHalfFovHorizontal / combinedTanHalfFovVertical;
	hmdPixelScale = 1;//ovrScale * vid.width / (float) hmd->Resolution.w;	

	common->Warning( "Init Hmd FOV x,y = %f , %f. Aspect = %f, PixelScale = %f\n",hmdFovX,hmdFovY,hmdAspect,hmdPixelScale );
	GL_CheckErrors();
	if ( VR_USE_FBO ) VR_BindFBO( GL_FRAMEBUFFER, VR_FBO ); // make sure primary FBO active.
	GL_CheckErrors();
	} 

/*
==============
VR_SetFrameHMDData
==============
*/

void VR_SetFrameHMDData(int frameIndex, ovrPosef thePose) {
	
	if ( frameDataCount == 255 ) return;

	frameDataIndex[frameDataCount] = frameIndex;
	frameDataPose[frameDataCount] = thePose;

	frameDataCount++;
	
}

void VR_GetFrameHMDData(int &frameIndex, ovrPosef &thePose) {
	
	if ( frameDataCount ) {
		frameIndex = frameDataIndex[frameDataCount];
		thePose = frameDataPose[frameDataCount];
		frameDataCount--;
	} else {
		frameIndex = 0;
		ovrTrackingState currentTrackingState = ovrHmd_GetTrackingState(hmd, 0) ;
		thePose = currentTrackingState.HeadPose.ThePose;
	}

	return;
}
		
/*
==============`
VR_GetHMDOrientation
==============
*/

void VR_GetHMDOrientation(float &roll, float &pitch, float &yaw, idVec3 &hmdPosition) {
	
	if ( !hasOculusRift || !hasHMD ) {
		roll = 0;
		pitch =0;
		yaw = 0;
		hmdPosition = idVec3 ( 0, 0, 0 ) ;
		return;
	}
	static double time = 0.0;
	static ovrPosef translationPose;
	static Posef	orientationPose; 
	static ovrPosef lastTrackedPose = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } ;
	static bool currentlyTracked;
		
	ovrFrameTiming frameTiming;
	int frameIndex = tr.frameCount & 0xff;
	
	if ( hasOculusRift && hmd ) {
        
		if ( vr_trackingPredictionAuto.GetBool() ) {
			time = ( hmdFrameTime.EyeScanoutSeconds[ovrEye_Left] + hmdFrameTime.EyeScanoutSeconds[ovrEye_Right] ) / 2.0;
			frameTiming = ovrHmd_GetFrameTiming(hmd,frameIndex);		
		} else {
			time = ovr_GetTimeInSeconds() + ( vr_trackingPredictionUserDefined.GetFloat() / 1000 );
			frameTiming = ovrHmd_GetFrameTiming(hmd,frameIndex);	

		}

		//hmdTrackingState = ovrHmd_GetTrackingState( hmd, time );
		hmdTrackingState = ovrHmd_GetTrackingState(hmd, frameTiming.ScanoutMidpointSeconds) ;

		VR_SetFrameHMDData(frameIndex,hmdTrackingState.HeadPose.ThePose);
		
		if (hmdTrackingState.StatusFlags & ( ovrStatus_OrientationTracked ) ){
							
				orientationPose = hmdTrackingState.HeadPose.ThePose; 
				
				float y = 0.0f, p = 0.0f, r = 0.0f;
				
				orientationPose.Rotation.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&y, &p, &r);
							
				roll =   -RADIANS_TO_DEGREES(r); // ???
				pitch =  -RADIANS_TO_DEGREES(p); // should be degrees down
				yaw =     RADIANS_TO_DEGREES(y); // should be degrees left
			}
		
		} else {
			roll  = angles[ROLL];
			pitch = angles[PITCH];
			yaw   = angles[YAW];
		}
		
		// now read the HMD position if equiped

		currentlyTracked = hmdTrackingState.StatusFlags & ( ovrStatus_PositionTracked ) ? 1 : 0;
		//common->Printf("currently tracked = %d\n", currentlyTracked);

		if ( currentlyTracked ) {
				translationPose = hmdTrackingState.HeadPose.ThePose; 
				lastTrackedPose = translationPose;
		} else 	{	
				translationPose = lastTrackedPose;
		}
				
		hmdPosition.x = ( -translationPose.Position.z * 1000 ) / 25.4f ; // koz convert position (in meters) to inch (1 id unit = 1 inch). (mm/25.4 = inch)   
		hmdPosition.y = ( -translationPose.Position.x * 1000 ) / 25.4f ; 
		hmdPosition.z = ( translationPose.Position.y * 1000 ) / 25.4f ; 
			
		if ( hmd->TrackingCaps && (ovrTrackingCap_Position) ) {

			if ( currentlyTracked && !hmdPositionTracked ) {
				common->Warning( "Oculus Positional tracking established.\n" );
				hmdPositionTracked = true;
			} else if ( !currentlyTracked && hmdPositionTracked ) {
				common->Warning( "Oculus Positional tracking LOST.\n" );
				hmdPositionTracked = false;
			}
		}
}

/*
==============
VR_FrameStart
==============
*/
void VR_FrameStart(int index) {
		
	if ( vr_lowPersistence.IsModified() ) {
		
		unsigned int caps = 0;
		//if ( hmd->HmdCaps & ovrHmdCap_DynamicPrediction )
			//caps |= ovrHmdCap_DynamicPrediction;

		if ( hmd->HmdCaps & ovrHmdCap_LowPersistence && vr_lowPersistence.GetInteger() )
			caps |= ovrHmdCap_LowPersistence;

		caps |= ovrHmdCap_NoVSync;  
		
		ovrHmd_SetEnabledCaps( hmd, caps );
		vr_lowPersistence.ClearModified();
	
	} 

	if (!hmdInFrame)
	{
		hmdFrameTime = ovrHmd_BeginFrameTiming(hmd,index);
	}
	else
	{
		ovrHmd_EndFrameTiming(hmd);
		ovrHmd_ResetFrameTiming(hmd,index);
		hmdFrameTime = ovrHmd_BeginFrameTiming(hmd,index);
	}
	hmdInFrame = true;
}

/*
==============
VR_FrameEnd
==============
*/
void VR_FrameEnd() {
	ovrHmd_EndFrameTiming( hmd );
	hmdInFrame = false;
}

/*
==============
VR_SetLeftHydraOffset
==============
*/

void VR_SetLeftHydraOffset( hydraData hydraOffset ) {	//koz left hydra offset will store the initial/default delta between hydra and base for reference
																	//so we can get the actual movement deltas for positional body tracking.
	leftHydraOffset.position = hydraOffset.position;
	leftHydraOffset.hydraRotationQuat = hydraOffset.hydraRotationQuat;

}

/*
==============
VR_SetRightHydraOffset
==============
*/
void VR_SetRightHydraOffset( hydraData hydraOffset ) {	//koz right hydra offset will store the initial/default delta between hydra and base for reference
																	//so we can get the actual deltas for proper gun positioning in relation to body.
	rightHydraOffset.position = hydraOffset.position;
	rightHydraOffset.hydraRotationQuat = hydraOffset.hydraRotationQuat;

}

/*
==============
VR_GetLeftHydraOffset
==============
*/
void VR_GetLeftHydraOffset( hydraData &hydraOffset ) {
	
	hydraOffset.position = leftHydraOffset.position;
	hydraOffset.hydraRotationQuat = leftHydraOffset.hydraRotationQuat;

}

/*
==============
VR_GetRightHydraOffset
==============
*/
void VR_GetRightHydraOffset( hydraData &hydraOffset ) {
		
	hydraOffset.position = rightHydraOffset.position;
	hydraOffset.hydraRotationQuat = rightHydraOffset.hydraRotationQuat;

}

/*
==============
VR_GetHydraData
==============
*/
void VR_GetHydraData( int hydraIndex, hydraData &hydraData ) {
	
	idQuat sixtempq;
	sixenseAllControllerData acd;
	sixenseSetActiveBase(0);
	sixenseGetAllNewestData(&acd);

	static idQuat tempQuat = idQuat_zero;

	if ( sixenseIsControllerEnabled(hydraIndex)) {
		
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
VR_GetRightHydra
==============
*/
void VR_GetRightHydra( hydraData &rightHydra ) {
	VR_GetHydraData(rightHydraIndex, rightHydra);
}

/*
==============
VR_GetLeftHydra
==============
*/

void VR_GetLeftHydra( hydraData &leftHydra ) {
	VR_GetHydraData(leftHydraIndex, leftHydra);
}

/*
==============
VR_GetLeftHydraWithOffset
==============
*/

void VR_GetLeftHydraWithOffset( hydraData &leftOffsetHydra ) {  // will return the left hydra position with the offset subtracted

	static hydraData leftCurrent = hydra_zero;
	static hydraData leftOffset = hydra_zero;

	VR_GetLeftHydraOffset( leftOffset );
	VR_GetLeftHydra( leftCurrent );
	
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
VR_GetRightHydraWithOffset
==============
*/

void VR_GetRightHydraWithOffset( hydraData &rightOffsetHydra ) { // will return the right hydra position with the offset angle and position values.
		
	static hydraData rightCurrent = hydra_zero;
	static hydraData rightOffset = hydra_zero;
	
	VR_GetRightHydraOffset( rightOffset );
	VR_GetRightHydra( rightCurrent );
	
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
