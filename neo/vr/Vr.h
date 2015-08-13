/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "precompiled.h"
#pragma hdrstop

#include "..\LibOVR\Include\OVR_CAPI.h"
#include "..\LibOVR\Include\OVR_CAPI_GL.h"
#include "vr_hmd.h"
#include "vr_sixense.h"
#include "..\renderer\Framebuffer.h"


#ifndef __VR_H__
#define __VR_H__

typedef enum 
{
	VR_AA_NONE,
	VR_AA_MSAA,
	VR_AA_FXAA,
	NUM_VR_AA
} vr_aa_t;

typedef enum
{
	VR_HUD_NONE,
	VR_HUD_FULL,
	VR_HUD_LOOK_DOWN,
	VR_HUD_FLOATING
} vr_hud_t;

typedef enum
{
	RENDERING_NORMAL,
	RENDERING_PDA,
	RENDERING_HUD
} vr_swf_render_t;

typedef enum
{
	FLASH_BODY,
	FLASH_HEAD,
	FLASH_GUN,
	FLASH_HAND
} vr_flashlight_mode_t;


class iVr
{
public:
	
	iVr();
	
	void				HMDInit( void );
	void				HMDInitializeDistortion( void );
	void				HMDGetOrientation( float &roll, float &pitch, float &yaw, idVec3 &hmdPosition );
	void				HMDRender( idImage *leftCurrent, idImage *rightCurrent );
	void				HMDTrackStatic();
	void				HUDRender( idImage *image0, idImage *image1 );

	void				HydraInit( void );
	void				HydraSetLeftOffset( hydraData hydraOffset );
	void				HydraSetRightOffset( hydraData hydraOffset );
	void				HydraGetLeftOffset( hydraData &hydraOffset );
	void				HydraGetRightOffset( hydraData &hydraOffset );
	void				HydraGetLeft( hydraData &leftHydra );
	void				HydraGetRight( hydraData &rightHydra );
	void				HydraGetLeftWithOffset( hydraData &leftOffsetHydra );
	void				HydraGetRightWithOffset( hydraData &rightOffsetHydra );
		
	void				FrameStart( int index );
	void				FrameEnd();
	void				FrameWait();
	
	void				MSAAResolve( void );
	void				FXAAResolve( idImage * leftCurrent, idImage * rightCurrent );
	void				FXAASetUniforms( Framebuffer FBO );

	float				GetHudAlpha( void );

	void				CalcAimMove( float &yawDelta, float &pitchDelta );
		
	bool				VR_GAME_PAUSED;
	
	bool				PDAforcetoggle;
	bool				PDAforced;
	bool				PDArising;
	int					swfRenderMode;
	bool				PDAclipModelSet;
	bool				forceLeftStick;
	
	bool				vrIsBackgroundSaving;

	idVec3				lastViewOrigin;
	idMat3				lastViewAxis;
	float				lastHMDYaw;
	float				lastHMDPitch;
	float				lastHMDRoll;

	float				angles[3];
	
	int					hmdWidth;
	int					hmdHeight;
	int					hmdHz;
	
	int					useFBO;
	int					primaryFBOWidth;
	int					primaryFBOHeight;
	
	int					VR_AAmode;
	
	int					VR_USE_HYDRA;
	
	bool				hasHMD;
	bool				hasOculusRift;
	
	ovrHmd				hmd;
	float				hmdFovX;
	float				hmdFovY;
	float				hmdPixelScale;
	float				hmdAspect;
	hmdEye_t			hmdEye[2];
	int					eyeOrder[2];
	
	float				oculusIPD;
	float				oculusHeight;
	float				manualIPD;
	float				manualHeight;

	idImage*			hmdEyeImage[2];
	idImage*			hmdCurrentRender[2];
	
	idImage*			primaryFBOimage;
	idImage*			resolveFBOimage;
	idImage*			fullscreenFBOimage;

	ovrSwapTextureSet * oculusTextureSet[2];
	GLuint				oculusFboId;
	GLuint				ocululsDepthTexID;
	ovrGLTexture*		oculusMirrorTexture;
	GLuint				oculusMirrorFboId;

	ovrLayerEyeFov		oculusLayer;
		
	ovrTrackingState	hmdTrackingState;
	ovrFrameTiming		hmdFrameTime;
	bool				hmdPositionTracked;
	bool				hmdInFrame;
	
	bool				isBackgroundSaving;

	float				independentWeaponYaw;
	float				independentWeaponPitch;

	float				playerDead;

	
	//---------------------------
private:
	
	void				HydraGetData( int hydraIndex, hydraData &hydraData );
	
	int					hydraLeftIndex ;						
	int					hydraRightIndex ;
	
	hydraData			hydraLeftOffset;
	hydraData			hydraRightOffset;

	int					frameDataCount ;
	int					frameDataIndex[255];
	ovrPosef			frameDataPose[255];
};

#endif

//koz g_flash cvars allow tweaking of flash position when aiming with hydra
extern idCVar	vr_flashHand_x;
extern idCVar	vr_flashHand_y;
extern idCVar	vr_flashHand_z;
extern idCVar	vr_flashScale;
//koz g_gun cvars allow tweaking of gun position when aiming with hydra
extern idCVar	vr_gunHand_x;
extern idCVar	vr_gunHand_y;
extern idCVar	vr_gunHand_z;
extern idCVar	vr_scaleGun;

extern idCVar	vr_hydraPitchOffset;
extern idCVar	vr_vignette;
extern idCVar	vr_overdriveEnable;
extern idCVar	vr_scale;
extern idCVar	vr_useOculusProfile;
extern idCVar	vr_manualIPD;
extern idCVar	vr_manualHeight;
extern idCVar   vr_timewarp;
extern idCVar	vr_chromaCorrection;
extern idCVar	vr_oculusHmdDirectMode;

extern idCVar	vr_showBody;
extern idCVar	vr_viewModelArms;
extern idCVar	vr_disableWeaponAnimation;
extern idCVar	vr_headKick;

extern idCVar	vr_hydraEnable;
extern idCVar	vr_hydraForceDetect;
extern idCVar	vr_hydraMode;

extern idCVar	vr_flashPitchAngle;
extern idCVar	vr_flashlightMode;

extern idCVar	vr_flashlightBodyPosX;
extern idCVar	vr_flashlightBodyPosY;
extern idCVar	vr_flashlightBodyPosZ;

extern idCVar	vr_flashlightHelmetPosX;
extern idCVar	vr_flashlightHelmetPosY;
extern idCVar	vr_flashlightHelmetPosZ;
extern idCVar	vr_flashlightGunScale;

extern idCVar	vr_offx;
extern idCVar	vr_offy;
extern idCVar	vr_offz;

extern idCVar	vr_rotateAxis;

extern idCVar	vr_offsetYaw;
extern idCVar	vr_offsetPitch;
extern idCVar	vr_offsetRoll;

extern idCVar	vr_forward_keyhole;

extern idCVar	vr_PDAscale;
extern idCVar	vr_PDAfixLocation;

extern idCVar	vr_mouse_gunx;
extern idCVar	vr_mouse_guny;
extern idCVar	vr_mouse_gunz;
extern idCVar	vr_mouse_gun_forearm;

extern idCVar	vr_chromaCorrection;
extern idCVar	vr_timewarp;
extern idCVar	vr_overdrive;
extern idCVar	vr_overdriveEnable;

extern idCVar	vr_guiScale; 
extern idCVar	vr_guiSeparation;

extern idCVar	vr_hudScale;
extern idCVar	vr_hudPosX; 
extern idCVar	vr_hudPosY; 
extern idCVar	vr_hudType;
extern idCVar	vr_hudAngle;
extern idCVar	vr_hudTransparency;

extern idCVar	vr_hudHealth;
extern idCVar	vr_hudAmmo;
extern idCVar	vr_hudPickUps;
extern idCVar	vr_hudTips;
extern idCVar	vr_hudLocation;
extern idCVar	vr_hudObjective;
extern idCVar	vr_hudStamina;
extern idCVar	vr_hudPills;
extern idCVar	vr_hudComs;
extern idCVar	vr_hudWeap;
extern idCVar	vr_hudNewItems;
extern idCVar	vr_hudFlashlight;
extern idCVar	vr_hudLowHealth;

extern idCVar	vr_tweakTalkCursor; 

extern idCVar	vr_listMonitorName;

extern idCVar	vr_enable;
extern idCVar	vr_FBOscale;
extern idCVar	vr_hydraEnable;
extern idCVar	vr_joystickMenuMapping;

extern idCVar	vr_trackingPredictionAuto;
extern idCVar	vr_trackingPredictionUserDefined;

extern idCVar	vr_minLoadScreenTime;

extern idCVar	vr_tweakx;
extern idCVar	vr_tweaky;

extern idCVar	vr_testWeaponModel;

extern idCVar	vr_aimMode;
extern idCVar	vr_deadzonePitch;
extern idCVar	vr_deadzoneYaw;
extern idCVar	vr_comfortDelta;

extern idCVar	vr_interactiveCinematic;

extern idCVar	vr_headingBeamLength;
extern idCVar	vr_headingBeamWidth;


extern iVr* vr;

/*
extern bool		VR_GAME_PAUSED;
extern bool		PDAforcetoggle;
extern bool		PDAforced;
extern bool		PDArising;
extern int		swfRenderMode;
extern idVec3	lastViewOrigin;
extern idMat3	lastViewAxis;
extern float	lastHMDYaw;

extern int		useFBO;
extern int		VR_USE_HYDRA;
extern int		VR_AAmode;

extern bool		PDAclipModelSet;

extern ovrHmd	hmd;
extern hmdEye_t hmdEye[2];
extern float	hmdFovX;
extern float	hmdFovY;

extern float	oculusIPD;
extern float	oculusHeight;

extern idImage * hmdEyeImage[2];
extern idImage * hmdCurrentRender[2];

extern bool vrIsBackgroundSaving;

extern float independentWeaponYaw;
extern float independentWeaponPitch;

extern bool hasHMD;
extern bool hasOculusRift;
*/
