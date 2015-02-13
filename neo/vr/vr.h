#ifndef VR_H
#define VR_H

/*#ifndef min
#define min(a,b)        (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b)        (((a) > (b)) ? (a) : (b))
#endif
*/
#include "..\..\\dependencies\LibOVR\Src\OVR_CAPI.h"
#include "renderer\RenderProgs.h"


extern idQuat idQuat_zero;  

typedef enum {
	EYE_LEFT = 0,
	EYE_RIGHT = 1,
	NUM_EYES = 2,
} vr_eye_t;

typedef struct {
	vr_eye_t eyes[NUM_EYES];
} eye_order_t;

typedef enum {
	VR_AIMMODE_DISABLE,
	VR_AIMMODE_HEAD_MYAW,
	VR_AIMMODE_HEAD_MYAW_MPITCH,
	VR_AIMMODE_MOUSE_MYAW,
	VR_AIMMODE_MOUSE_MYAW_MPITCH,
	VR_AIMMODE_TF2_MODE2,
	VR_AIMMODE_TF2_MODE3,
	VR_AIMMODE_TF2_MODE4,
	VR_AIMMODE_DECOUPLED,
	NUM_VR_AIMMODE
} vr_aimmode_t;

typedef struct {
	float scale;
	float offset;
} scaleOffset_t;

typedef struct {
	scaleOffset_t x;
	scaleOffset_t y;
} eyeScaleOffset_t;

typedef struct {
	GLuint vertBuffHandle;
	GLuint indexBuffHandle;
	GLenum usage;
	GLenum mode;
	GLsizei count;
	GLenum type;
	int numVerts;
	int numIndexes;
} vbo_t;

typedef struct {
	GLuint	framebuffer;
	GLuint	currentTexture;
	GLuint	depthbuffer;
	GLuint	colorbuffer;
	int		width;
	int		height;
	GLenum	format;
	bool	valid;
	idImage *FBOImage;
	int		MSAAsamples;
	bool	useFXAA;
} fbo_t;

typedef struct _hmdEye {

	float				projectionRift[16];
	float				projectionRiftCramZnear[16];
	idVec3				viewOffset;
	eyeScaleOffset_t	projection;
	ovrEyeRenderDesc	eyeRenderDesc;
	ovrSizei			renderTarget;
	ovrFovPort			eyeFov;
	ovrVector2f			UVScaleoffset[2];
	vbo_t				vbo;
} hmdEye_t;

/*
========================
Oculus Distortion Mesh Vertexes
=======================
*/

typedef struct _distortionVert {

	idVec2				pos;			// 8 bytes
	idVec2				texR;			// 8 bytes
	idVec2				texG;			// 8 bytes
	idVec2				texB;			// 8 bytes 
	byte				color[4];		// 4 bytes - total size 36 bytes

} distortionVert_t;

typedef struct _distortionMesh {
				
	int							numVerts;
	distortionVert_t *			verts;

	int 						numIndexes;
	unsigned short	*			indexes;
} distortionMesh_t;

#define DISTORTIONVERT_SIZE				36
#define DISTORTIONVERT_POS_OFFSET		(0*4)
#define DISTORTIONVERT_TEXR_OFFSET		(2*4)
#define DISTORTIONVERT_TEXG_OFFSET		(4*4)
#define DISTORTIONVERT_TEXB_OFFSET		(6*4)
#define DISTORTIONVERT_DCOLOR_OFFSET	(8*4)

#define VR_AA_NONE 0
#define VR_AA_FXAA 1
#define VR_AA_MSAA 2

typedef struct _hydraData { // koz 
		idVec3 position;
		idQuat hydraRotationQuat;
		float joystick_x;
		float joystick_y;
		float trigger;
		unsigned int buttons;
} hydraData;

#define hydra_zero {vec3_zero, \
					idQuat_zero, \
					0.0f, \
					0.0f, \
					0.0f, \
					0 } ;

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
bool VR_GenFBO( int width, int height, fbo_t &FBO, bool useDepth, bool useMSAA );
bool VR_GenMSAAFBO( int width, int height, fbo_t &FBO );
void VR_BindFBO( GLuint bufferEnum, fbo_t &FBO );
void VR_ResizeFBO ( fbo_t &FBO, int newWidth, int newHeight );
void VR_DeleteFBO( fbo_t &FBO );
void VR_ResolveMSAA ( void ); 
void VR_ResolveFXAA ( void );
void VR_SetFXAAUniforms( fbo_t FBO );
void VR_HUDRender( idImage *image0, idImage *image1 );
void VR_HMDRender ( idImage *leftCurrent, idImage *rightCurrent, idImage *leftLast, idImage *rightLast );
void VR_HMDTrackStatic();


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
extern idCVar   vr_timewarp;
extern idCVar	vr_warp;
extern idCVar	vr_chromaCorrection;

extern idCVar	pm_showBody;

extern bool		VR_GAME_PAUSED;
extern bool		PDAforcetoggle;
extern bool		PDAforced;
extern bool		PDArising;
extern bool		renderingPDA;
extern idVec3	lastViewOrigin;
extern idMat3	lastViewAxis;
extern float	lastHMDYaw;

extern int		VR_USE_FBO;
extern idCVar	vr_enable;
extern idCVar	vr_FBOscale;
extern idCVar	vr_hydraEnable;
extern int		VR_USE_HYDRA;
extern int		VR_AAmode;

extern fbo_t	VR_FBO;
extern fbo_t	VR_FullscreenFBO;
extern fbo_t	VR_ResolveAAFBO;

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

extern float independentWeaponYaw ;
extern float independentWeaponPitch ;

/*

extern idCVar vr_enabled;
extern idCVar vr_autoenable;
extern idCVar vr_autofov;
extern idCVar vr_autofov_scale;
extern idCVar vr_autoipd;
extern idCVar vr_ipd;
extern idCVar vr_hud_fov;
extern idCVar vr_hud_depth;
extern idCVar vr_hud_segments;
extern idCVar vr_hud_transparency;
extern idCVar vr_chromatic;
extern idCVar vr_aimlaser;
extern idCVar vr_aimmode;
extern idCVar vr_aimmode_deadzone_yaw;
extern idCVar vr_aimmode_deadzone_pitch;
extern idCVar vr_viewmove;
extern idCVar vr_hud_bounce;
extern idCVar vr_hud_bounce_falloff;
extern idCVar vr_nosleep;
extern idCVar vr_neckmodel;
extern idCVar vr_neckmodel_up;
extern idCVar vr_neckmodel_forward; 

*/


