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
#include "..\..\\dependencies\LibOVR\Src\OVR_CAPI.h"
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
}  vr_aa_t;


class iVr
{
public:
	
	iVr();
	
	void				HMDInit( void );
	void				HMDInitializeDistortion( void );
	void				HMDGetOrientation( float &roll, float &pitch, float &yaw, idVec3 &hmdPosition );
	void				HMDRender( idImage *leftCurrent, idImage *rightCurrent, idImage *leftLast, idImage *rightLast );
	void				HMDTrackStatic();
	void				HMDGetFrameData( int &frameIndex, ovrPosef &thePose );
	void				HMDSetFrameData( int frameIndex, ovrPosef thePose );
	
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
	
	void				MSAAResolve( void );
	void				FXAAResolve( idImage * leftCurrent, idImage * rightCurrent );
	void				FXAASetUniforms( Framebuffer FBO );
	
	bool				VR_GAME_PAUSED;
	
	bool				PDAforcetoggle;
	bool				PDAforced;
	bool				PDArising;
	bool				renderingPDA;
	bool				PDAclipModelSet;
	bool				forceLeftStick;
	
	bool				vrIsBackgroundSaving;

	idVec3				lastViewOrigin;
	idMat3				lastViewAxis;
	float				lastHMDYaw;
	float				angles[3];
	
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
	int					currentRiftEye;
	int					eyeOrder[2];
	
	float				oculusIPD;
	float				oculusHeight;
	float				manualIPD;
	float				manualHeight;

	idImage*			hmdEyeImage[2];
	idImage*			hmdCurrentRender[2];
	idImage*			hmdPreviousRender[2];

	idImage*			primaryFBOimage;
	idImage*			resolveFBOimage;
	idImage*			fullscreenFBOimage;
	
	ovrTrackingState	hmdTrackingState;
	ovrFrameTiming		hmdFrameTime;
	bool				hmdPositionTracked;
	bool				hmdInFrame;
	
	bool				isBackgroundSaving;

	float				independentWeaponYaw;
	float				independentWeaponPitch;

	
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


/*
void VR_HydraInit( void );
void VR_HMDInit( void );
void VR_GenerateDistortionMeshes( void );
void VR_GetHMDOrientation( float &roll, float &pitch, float &yaw, idVec3 &hmdPosition );
void VR_SetLeftHydraOffset( hydraData hydraOffset );
void VR_SetRightHydraOffset( hydraData hydraOffset );
void VR_GetLeftHydraOffset( hydraData &hydraOffset );
void VR_GetRightHydraOffset( hydraData &hydraOffset );
void VR_GetLeftHydra( hydraData &leftHydra );
void VR_GetRightHydra( hydraData &rightHydra );
void VR_GetLeftHydraWithOffset( hydraData &leftOffsetHydra );
void VR_GetRightHydraWithOffset( hydraData &rightOffsetHydra );
void VR_FrameStart( int index );
void VR_FrameEnd();
void VR_GetFrameHMDData( int &frameIndex, ovrPosef &thePose );
void VR_SetFrameHMDData( int frameIndex, ovrPosef thePose );
void VR_ResolveMSAA( void );
void VR_ResolveFXAA( void );
void VR_SetFXAAUniforms( Framebuffer FBO );
void VR_HUDRender( idImage *image0, idImage *image1 );
void VR_HMDRender( idImage *leftCurrent, idImage *rightCurrent, idImage *leftLast, idImage *rightLast );
void VR_HMDTrackStatic();
*/


#endif

//koz g_flash cvars allow tweaking of flash position when aiming with hydra
extern idCVar	vr_flash_x;
extern idCVar	vr_flash_y;
extern idCVar	vr_flash_z;
extern idCVar	vr_flashScale;
//koz g_gun cvars allow tweaking of gun position when aiming with hydra
extern idCVar	vr_gun_x;
extern idCVar	vr_gun_y;
extern idCVar	vr_gun_z;
extern idCVar	vr_scaleGun;

extern idCVar	vr_hydraPitchOffset;
extern idCVar	vr_vignette;
extern idCVar	vr_overdriveEnable;
extern idCVar	vr_scale;
extern idCVar	vr_useOculusProfile;
extern idCVar	vr_manualIPD;
extern idCVar	vr_manualHeight;
extern idCVar   vr_timewarp;
extern idCVar	vr_warp;
extern idCVar	vr_chromaCorrection;

extern idCVar	vr_showBody;
extern idCVar	vr_viewModelArms;
extern idCVar	vr_disableWeaponAnimation;
extern idCVar	vr_headKick;

extern idCVar	vr_hydraEnable;
extern idCVar	vr_hydraMode;

extern idCVar	vr_flashPitchAngle;
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

extern idCVar	vr_warp;
extern idCVar	vr_chromaCorrection;
extern idCVar	vr_timewarp;
extern idCVar	vr_overdrive;
extern idCVar	vr_overdriveEnable;

extern idCVar	vr_guiScale; 
extern idCVar	vr_hudScaleX; 
extern idCVar	vr_hudScaleY; 
extern idCVar	vr_listMonitorName;

extern idCVar	vr_enable;
extern idCVar	vr_FBOscale;
extern idCVar	vr_hydraEnable;
extern idCVar	vr_joystickMenuMapping;

extern iVr* vr;

/*
extern bool		VR_GAME_PAUSED;
extern bool		PDAforcetoggle;
extern bool		PDAforced;
extern bool		PDArising;
extern bool		renderingPDA;
extern idVec3	lastViewOrigin;
extern idMat3	lastViewAxis;
extern float	lastHMDYaw;

extern int		useFBO;
extern int		VR_USE_HYDRA;
extern int		VR_AAmode;

extern bool		PDAclipModelSet;

extern ovrHmd	hmd;
extern hmdEye_t hmdEye[2];
extern int		currentRiftEye;
extern float	hmdFovX;
extern float	hmdFovY;

extern float	oculusIPD;
extern float	oculusHeight;

extern idImage * hmdEyeImage[2];
extern idImage * hmdCurrentRender[2];
extern idImage * hmdPreviousRender[2];

extern bool vrIsBackgroundSaving;

extern float independentWeaponYaw;
extern float independentWeaponPitch;

extern bool hasHMD;
extern bool hasOculusRift;
*/
