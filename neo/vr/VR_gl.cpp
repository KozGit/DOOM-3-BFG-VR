#pragma hdrstop

#include "../../idlib/precompiled.h"
#include "d3xp\Game_local.h"
#include "vr.h"
#include "renderer\tr_local.h"

#include "sys\win32\win_local.h"
static int VRFBOWidth = 0;
static int VRFBOHeight = 0;

idImage * hmdEyeImage[2];
idImage * hmdCurrentRender[2];
idImage * hmdPreviousRender[2];
	
idCVar vr_transz( "vr_transz", "-1", CVAR_RENDERER | CVAR_FLOAT, "test z trans" );
idCVar vr_transscale( "vr_transscale",".5",	CVAR_RENDERER | CVAR_FLOAT, "test trans scale" );

void GLimp_SwapBuffers();
void GL_BlockingSwapBuffers();

extern PFNWGLSWAPINTERVALEXTPROC				wglSwapIntervalEXT;
/*
====================
VR_MakeFBOImage
====================
*/

static void VR_MakeFBOImage( idImage *image ) {
	idImageOpts	opts;
	opts.width = VRFBOWidth;
	opts.height = VRFBOHeight;
	opts.numLevels = 1;
	opts.format = FMT_RGBA8;
	image->AllocImage( opts, TF_LINEAR, TR_CLAMP );
}

/*
====================
VR_DeleteFBO
====================
*/

void VR_DeleteFBO( fbo_t &FBO ) {
/*
	if ( FBO.FBOImage != NULL ) {
		FBO.FBOImage->PurgeImage();
		globalImages->images.Remove( FBO.FBOImage );
	}
*/
	
	if ( FBO.depthbuffer ) {
		glDeleteRenderbuffers(1, &FBO.depthbuffer );
	}
	
	if ( FBO.framebuffer ) {
		glDeleteFramebuffers(1, &FBO.framebuffer );
	}
	
	FBO.valid			= false;
	FBO.width			= 0;
	FBO.height			= 0;
	FBO.format			= 0;
	FBO.framebuffer		= 0;
	FBO.depthbuffer		= 0;
	FBO.currentTexture	= 0;
	FBO.FBOImage		= NULL;
	FBO.MSAAsamples		= 0;
	FBO.useFXAA			= false;
	FBO.colorbuffer		= 0;
}


/*
====================
VR_GenMSAAFBO
====================
*/

bool VR_GenMSAAFBO ( int width, int height, fbo_t &FBO) {
	
	FBO.framebuffer = 0;
	FBO.colorbuffer = 0; 
	FBO.depthbuffer = 0;
	
	int samples = r_multiSamples.GetInteger();//vr_FBOMSAALevel.GetInteger();

	GLuint glError;
	
	glGetError();
			
	glGenFramebuffers(1, &FBO.framebuffer);
	glBindFramebuffer( GL_FRAMEBUFFER, FBO.framebuffer );

	glGenRenderbuffers( 1, &FBO.colorbuffer );
	glBindRenderbuffer( GL_RENDERBUFFER, FBO.colorbuffer );
	glRenderbufferStorageMultisample( GL_RENDERBUFFER, samples, GL_RGBA8, width, height );
	
	//Make a depth multisample depth buffer with same samples, width, and height as the color RB
		
	glGenRenderbuffers(1, &FBO.depthbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, FBO.depthbuffer);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples , GL_DEPTH24_STENCIL8,  width, height ); 
	 
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, FBO.colorbuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, FBO.depthbuffer);
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, FBO.depthbuffer );
 
	//Make sure FBO is complete
	glError = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	glBindFramebuffer ( GL_FRAMEBUFFER, 0 );
	
	if ( glError != GL_FRAMEBUFFER_COMPLETE ) {
		
		// MSAA buffer is invalid, delete all attachments and return false.
		common->Warning( "VR_GenMSAAFBO: Error  0x%x creating %d x %d MSAA FBO with %d samples.\n", glError, width, height, samples );
		VR_DeleteFBO ( FBO );
		return false;
	
	}

	FBO.valid = 1;
	FBO.width = width;
	FBO.height = height;
	FBO.currentTexture = 0;
	FBO.MSAAsamples = samples;
	FBO.useFXAA = false;
	FBO.currentTexture = 0;
	FBO.FBOImage = NULL;
	return true;
}


/*
====================
VR_GenFBO
====================
*/

bool VR_GenFBO( int width, int height, fbo_t &FBO, bool useDepth, bool useMSAA ) {
	
	int glError	= 0;
	glGetError();

	if ( FBO.valid ) { // FBO is already valid, delete and recreate with current params.
		VR_DeleteFBO( FBO );
	}
	
	if ( useMSAA ) {
		return VR_GenMSAAFBO ( width, height, FBO );
	}
		
	// gen a new FBO
		
	FBO.framebuffer		= 0;
	FBO.currentTexture	= 0;
	FBO.depthbuffer		= 0;
		
	glGenFramebuffers( 1, &FBO.framebuffer );
	
	if ( useDepth ) {
		glGenRenderbuffers( 1, &FBO.depthbuffer );
	}
	
	//create an idImage for the FBO texture, and bind the texture.
	
	if ( FBO.FBOImage != NULL ) {
		FBO.FBOImage->Resize( width, height );
	} else {

		VRFBOWidth = width;
		VRFBOHeight = height;
		FBO.FBOImage = globalImages->ImageFromFunction( va("_FBOimage%d",FBO.framebuffer), VR_MakeFBOImage );
	}	
	
	FBO.currentTexture = FBO.FBOImage->GetTexNum();
	GL_SelectTexture( 0 );
	FBO.FBOImage->Bind();
		
	if ( useDepth ) {
		glBindRenderbuffer( GL_RENDERBUFFER, FBO.depthbuffer );
		glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height );
		//glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_STENCIL, width, height );
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );
	}
	
	glBindFramebuffer( GL_FRAMEBUFFER, FBO.framebuffer );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBO.currentTexture, 0 );
	
	if ( useDepth ) {
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, FBO.depthbuffer );
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, FBO.depthbuffer );
	}
	
	if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
	{
		common->Error( "VR_GenFBO: FBO failed to generate with width = %d, height = %d FBO\n", width, height );
		glBindFramebuffer(GL_FRAMEBUFFER, 0 );
		VR_DeleteFBO ( FBO );
		return false;
				
	} else {
		
		glError = glGetError();
		if (glError != GL_NO_ERROR) {
			common->Error(" VR_GenFBO: glGetError() = 0x%x\n", glError);
		} //else common->Printf("VR_GenFBO: FBO successfully generated with width = %d, height = %d \n", width, height );
				
		FBO.colorbuffer = 0;
		FBO.width	= width;
		FBO.height	= height;
		FBO.format	= FMT_RGBA8;
		FBO.valid	= 1;
		FBO.MSAAsamples = 0;
		FBO.useFXAA = false;
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		return true;
	}
}

/*
====================
VR_ResizeFBO
====================
*/

void VR_ResizeFBO ( fbo_t &FBO, int newWidth, int newHeight ) {
		
	bool depth = false;
	bool FXAA = FBO.useFXAA;
	bool success = false;
		
	common->Printf("Starting FBO resize, old FBO %d x %d, attempting %d x %d\n",FBO.width, FBO.height, newWidth, newHeight );
	if ( FBO.depthbuffer !=0 ) depth = true;
	
	if ( FBO.MSAAsamples > 0 ) {
		success = VR_GenFBO( newWidth, newHeight, FBO, depth , true );
	} else {
		success = VR_GenFBO( newWidth, newHeight, FBO, depth , false );
	}

	if ( success ) {
		FBO.useFXAA = FXAA;
		common->Printf("FBO successfully resized\n");
		
	} else {
		// oops.
		common->Printf("WARNING: FBO failed to resize!\n");
		VR_USE_FBO = FALSE;
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		glDrawBuffer( GL_BACK );
	}
}

/*
====================
VR_BindFBO
====================
*/

void VR_BindFBO( GLuint bufferEnum, fbo_t &FBO ) {
	static GLuint currentBufferEnum = 0;
	static GLuint currentFramebuffer = 0;
	GL_CheckErrors();
	if (currentFramebuffer != FBO.framebuffer || currentBufferEnum != bufferEnum)
	{
		
		currentBufferEnum = bufferEnum;
		currentFramebuffer = FBO.framebuffer;

		glBindFramebuffer(GL_FRAMEBUFFER /*bufferEnum*/, FBO.framebuffer);
		GL_CheckErrors();
	/*	if ( bufferEnum == GL_FRAMEBUFFER || bufferEnum == GL_DRAW_FRAMEBUFFER) 
		{
			//glReadBuffer( GL_BACK );
			GL_CheckErrors();
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			GL_CheckErrors();
		}

		if ( bufferEnum == GL_READ_FRAMEBUFFER ) 
		{
			//glDrawBuffer(GL_BACK);
			GL_CheckErrors();
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			GL_CheckErrors();
		} 
		GL_CheckErrors(); */
	} 
}

/*
====================
VR_ResolveMSAA
====================
*/

void VR_ResolveMSAA ( void ) { 
		
	if ( VR_USE_FBO  ) {
		
		if ( VR_FBO.MSAAsamples != 0 ) 
		{ // primary FBO is MSAA enabled, resolve to secondary FBO to perform antialiasing before copy
			
			glBindFramebuffer( GL_READ_FRAMEBUFFER, VR_FBO.framebuffer ) ; // bind primary FBO for read
			glReadBuffer( GL_COLOR_ATTACHMENT0 );
			glBindFramebuffer( GL_DRAW_FRAMEBUFFER, VR_ResolveAAFBO.framebuffer );// bind resolve FBO for draw
			glDrawBuffer( GL_COLOR_ATTACHMENT0 );
			glBlitFramebuffer( 0, 0, VR_FBO.width, VR_FBO.height, 0, 0, VR_ResolveAAFBO.width, VR_ResolveAAFBO.height, GL_COLOR_BUFFER_BIT, GL_LINEAR); // blit from primary FBO to resolve MSAA antialiasing.
			VR_BindFBO( GL_FRAMEBUFFER, VR_ResolveAAFBO ); 
		} 
	} 
}

void VR_SetFXAAUniforms( fbo_t FBO ) {
	int progr = renderProgManager.GetGLSLCurrentProgram(); 
	
	//static GLuint scale = renderProgManager.GetUniformLocByName(progr, "Scale");
	static GLuint scale1 = glGetUniformLocation( progr,"texScale" );
	
	static GLuint resolution = glGetUniformLocation( progr, "buffersize" );
	
	//common->Printf("Setting FXAA uniforms resolution = %d texScale = %d\n",resolution,scale1);
	glUniform2f(resolution, FBO.width, FBO.height);
	glUniform2f( scale1, 1.0, 1.0 );

}

/*
====================
VR_ResolveFXAA
====================
*/

void VR_ResolveFXAA ( idImage * leftCurrent, idImage * rightCurrent ) { 

	VR_BindFBO ( GL_FRAMEBUFFER, VR_ResolveAAFBO ); // antialias image 0
	glClearColor( 0, 0, 0, 0 );
	glClear( GL_COLOR_BUFFER_BIT );
												
	renderProgManager.BindShader_VRFXAA();
	VR_SetFXAAUniforms( VR_ResolveAAFBO );
						
	GL_SelectTexture( 0 );
	leftCurrent->Bind();
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
						
	GL_ViewportAndScissor( 0, 0, VR_ResolveAAFBO.width, VR_ResolveAAFBO.height );
	RB_DrawElementsWithCounters( &backEnd.unitSquareSurface ); // draw it
						
	//VR_BindFBO ( GL_READ_FRAMEBUFFER, VR_ResolveAAFBO );
											
	//leftCurrent->Bind();
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, leftCurrent->GetUploadWidth(), leftCurrent->GetUploadHeight(), 0 );
												
	//VR_BindFBO ( GL_DRAW_FRAMEBUFFER, VR_ResolveAAFBO );
	//glClearColor( 0, 0, 0, 0 );
	glClear( GL_COLOR_BUFFER_BIT );
																	
	//renderProgManager.BindShader_VRFXAA();
	//VR_SetFXAAUniforms( VR_ResolveAAFBO );
												
	rightCurrent->Bind();
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
						
	GL_ViewportAndScissor( 0, 0, VR_ResolveAAFBO.width, VR_ResolveAAFBO.height );
	RB_DrawElementsWithCounters( &backEnd.unitSquareSurface ); // draw it
	
	//VR_BindFBO ( GL_READ_FRAMEBUFFER, VR_ResolveAAFBO );
	
	//rightCurrent->Bind();
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, rightCurrent->GetUploadWidth(), rightCurrent->GetUploadHeight(), 0 );
																		
	renderProgManager.Unbind();
					
} 
		
	


void VR_PerspectiveScale( eyeScaleOffset_t eye, GLfloat zNear, GLfloat zFar, float ( &out )[4][4] ) {
	
	GLfloat nf = 1.0f / (zNear - zFar);
	
	out[0][0] = eye.x.scale;
	out[0][1] = 0;
	out[0][2] = 0;
	out[0][3] = 0;

	out[1][0] = 0;
	out[1][1] = eye.y.scale;
	out[1][2] = 0;
	out[1][3] = 0;

	out[2][0] = -eye.x.offset;
	out[2][1] = eye.y.offset;
	out[2][2] = (zFar + zNear) * nf;
	out[2][3] = -1;

	out[3][0] = 0;
	out[3][1] = 0;
	out[3][2] = (2.0f * zFar * zNear) * nf;
	out[3][3] = 0;
		
}

/*
================
VR_TranslationMatrix
================
*/

void VR_TranslationMatrix( float x, float y, float z, float ( &out )[4][4] ) {
	// build translation matrix
	memset( out, 0, sizeof( float ) * 16 );
	out[0][0] = out[1][1] = out[2][2] = 1;
	out[3][0] = x;
	out[3][1] = y;
	out[3][2] = z;
	out[3][3] = 1;
}

/*
================
RotationMatrix
================
*/

void RotationMatrix( float angle, float x, float y, float z, float ( &out )[4][4] ) {
	
	float phi = DEG2RAD(angle);
	float c = cosf(phi); // cosine
	float s = sinf(phi); // sine
	float xx = x * x;
	float xy = x * y;
	float xz = x * z;
	float yy = y * y;
	float yz = y * z;
	float zz = z * z;
	// build rotation matrix
	out[0][0] = xx * (1 - c) + c;
	out[1][0] = xy * (1 - c) - z * s;
	out[2][0] = xz * (1 - c) + y * s;
	out[3][0] = 0;
	out[0][1] = xy * (1 - c) + z * s;
	out[1][1] = yy * (1 - c) + c;
	out[2][1] = yz * (1 - c) - x * s;
	out[3][1] = 0;
	out[0][2] = xz * (1 - c) - y * s;
	out[1][2] = yz * (1 - c) + x * s;
	out[2][2] = zz * (1 - c) + c;
	out[3][2] = out[0][3] = out[1][3] = out[2][3] = 0;
	out[3][3] = 1;
}

/*
================
VR_MatrixMultiply
================
*/

void VR_MatrixMultiply (float in1[4][4],float in2[4][4], float (&out)[4][4] ) {

	float result[4][4];

	result[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] + in1[0][2] * in2[2][0] + in1[0][3] * in2[3][0];
	result[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] + in1[0][2] * in2[2][1] + in1[0][3] * in2[3][1];
	result[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] + in1[0][2] * in2[2][2] + in1[0][3] * in2[3][2];
	result[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] + in1[0][2] * in2[2][3] + in1[0][3] * in2[3][3];
	result[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] + in1[1][2] * in2[2][0] + in1[1][3] * in2[3][0];
	result[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] + in1[1][2] * in2[2][1] + in1[1][3] * in2[3][1];
	result[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] + in1[1][2] * in2[2][2] + in1[1][3] * in2[3][2];
	result[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] + in1[1][2] * in2[2][3] + in1[1][3] * in2[3][3];
	result[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] + in1[2][2] * in2[2][0] + in1[2][3] * in2[3][0];
	result[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] + in1[2][2] * in2[2][1] + in1[2][3] * in2[3][1];
	result[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] + in1[2][2] * in2[2][2] + in1[2][3] * in2[3][2];
	result[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] + in1[2][2] * in2[2][3] + in1[2][3] * in2[3][3];
	result[3][0] = in1[3][0] * in2[0][0] + in1[3][1] * in2[1][0] + in1[3][2] * in2[2][0] + in1[3][3] * in2[3][0];
	result[3][1] = in1[3][0] * in2[0][1] + in1[3][1] * in2[1][1] + in1[3][2] * in2[2][1] + in1[3][3] * in2[3][1];
	result[3][2] = in1[3][0] * in2[0][2] + in1[3][1] * in2[1][2] + in1[3][2] * in2[2][2] + in1[3][3] * in2[3][2];
	result[3][3] = in1[3][0] * in2[0][3] + in1[3][1] * in2[1][3] + in1[3][2] * in2[2][3] + in1[3][3] * in2[3][3];
	
	memcpy (out, result, sizeof(float) *16 );
}

/*
================
VR_QuatToRotation
================
*/

void VR_QuatToRotation( idQuat q, float (&out)[4][4] ) {
	
	float xx = q.x * q.x;
	float xy = q.x * q.y;
	float xz = q.x * q.z;
	float xw = q.x * q.w;
	float yy = q.y * q.y;
	float yz = q.y * q.z;
	float yw = q.y * q.w;
	float zz = q.z * q.z;
	float zw = q.z * q.w;
	out[0][0] = 1 - 2 * (yy + zz);
	out[1][0] = 2 * (xy - zw);
	out[2][0] = 2 * (xz + yw);
	out[0][1] = 2 * (xy + zw);
	out[1][1] = 1 - 2 * (xx + zz);
	out[2][1] = 2 * (yz - xw);
	out[0][2] = 2 * (xz - yw);
	out[1][2] = 2 * (yz + xw);
	out[2][2] = 1 - 2 * (xx + yy);
	out[3][0] = out[3][1] = out[3][2] = out[0][3] = out[1][3] = out[2][3] = 0;
	out[3][3] = 1;
}

/*
====================
VR_MakeStereoRenderImage
====================
*/
static void VR_MakeStereoRenderImage( idImage *image ) {
	idImageOpts	opts;
	opts.width = renderSystem->GetWidth();
	opts.height = renderSystem->GetHeight();
	opts.numLevels = 1;
	opts.format = FMT_RGBA8;
	image->AllocImage( opts, TF_LINEAR, TR_CLAMP );
}

/*
====================
VR_HUDRender
Render headtracked quad or hud mesh.

Source images: idImage image0 is left eye, image1 is right eye.
Destination images: idImage hmdEyeImage[0,1] 0 is left, 1 is right.

Original images are not modified ( can be called repeatedly with the same source textures to 
provide continual tracking for static images, e.g. during loading )

Does not perform hmd distortion correction.
====================
*/

void VR_HUDRender( idImage *image0, idImage *image1 ) {
	
	static idAngles imuAngles = { 0.0, 0.0, 0.0 } ;
	static idQuat imuRotation = { 0.0, 0.0, 0.0, 0.0 };
	static idQuat imuRotationGL = { 0.0, 0.0, 0.0, 0.0 };
	extern bool hmdPositionTracked;
	static idVec3 lastValidHmdTranslation = vec3_zero;
	static idVec3 hmdTranslation = vec3_zero;
			
	static float rot[4][4], trans[4][4], eye[4][4], proj[4][4], result[4][4] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	static float glMatrix[16] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
		
	static float xx,yy,zz = 0.0;
	
	for ( int i = 0; i < 2; i++ ) {
			if ( hmdEyeImage[i] == NULL ) {
				hmdEyeImage[i] = globalImages->ImageFromFunction( va("_hmdEyeImage%i", i ), VR_MakeStereoRenderImage );
			}
			if ( hmdEyeImage[i]->GetUploadWidth() != renderSystem->GetWidth() ||
				hmdEyeImage[i]->GetUploadHeight() != renderSystem->GetHeight() ) {
				hmdEyeImage[i]->Resize( renderSystem->GetWidth(), renderSystem->GetHeight() );
			}
	}
	
	VR_GetHMDOrientation( imuAngles[ROLL], imuAngles[PITCH], imuAngles[YAW], hmdTranslation );
	
	imuRotation = imuAngles.ToQuat();
		
	imuRotationGL.x = -imuRotation.y; // convert from id coord system to gl 
	imuRotationGL.y =  imuRotation.z;
	imuRotationGL.z = -imuRotation.x;
	imuRotationGL.w =  imuRotation.w; 
	
	//imuRotationGL.x = imuRotation.y; // convert from id coord system to gl 
	//imuRotationGL.y = imuRotation.z;
	//imuRotationGL.z = imuRotation.x;
	//imuRotationGL.w = imuRotation.w;
	
	VR_QuatToRotation( imuRotationGL, rot );
	VR_TranslationMatrix( 0, 0, vr_transz.GetFloat() , trans );
		
	if (VR_USE_FBO)
	{
		//IF( VR_USE)
	}
	VR_BindFBO(GL_FRAMEBUFFER, VR_ResolveAAFBO );
	GL_ViewportAndScissor(0, 0, VR_ResolveAAFBO.width, VR_ResolveAAFBO.height );
	glClearColor( 0, 0, 0, 0 );
	
	GL_SelectTexture( 0 );
		
	// We just want to do a quad pass - so make sure we disable any texgen and
	// set the texture matrix to the identity so we don't get anomalies from 
	// any stale uniform data being present from a previous draw call
	const float texS[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
	const float texT[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
	renderProgManager.SetRenderParm(RENDERPARM_TEXTUREMATRIX_S, texS);
	renderProgManager.SetRenderParm(RENDERPARM_TEXTUREMATRIX_T, texT);

	// disable any texgen
	const float texGenEnabled[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	renderProgManager.SetRenderParm(RENDERPARM_TEXGEN_0_ENABLED, texGenEnabled);

	renderProgManager.BindShader_Texture();
			
	for (int index = 0; index < 2; index++)	{
		
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );			
		
		if ( index )  {
			image1->Bind();
		} else {
			image0->Bind();
		}
				
		// copy the perspective matrix for this eye
		memcpy ( proj, hmdEye[index].projectionRift, sizeof(float) *16 );
				
		// move the camera for this eye.
		VR_TranslationMatrix( -hmdEye[index].viewOffset[0], hmdEye[index].viewOffset[1], hmdEye[index].viewOffset[2], eye );
		//common->Printf("-vo0 %f v01 %f vo2 %f\n", hmdEye[index].viewOffset[0], hmdEye[index].viewOffset[1], hmdEye[index].viewOffset[2]);
		//idMat3 transMat = idVec3(-hmdEye[index].viewOffset[0], hmdEye[index].viewOffset[1], hmdEye[index].viewOffset[2]).ToMat3;
		//float viewOffset = (6.5f / 20.0f);
		//VR_TranslationMatrix((-1 + index * 2) * -viewOffset, 0, 0, temp);
		/*	if (vr_autoipd->value)
		{
			TranslationMatrix(-vrState.renderParams[index].viewOffset[0], vrState.renderParams[index].viewOffset[1], vrState.renderParams[index].viewOffset[2], temp);
		} else {
			float viewOffset = (vr_ipd->value / 2000.0);
			TranslationMatrix((-1 + index * 2) * -viewOffset, 0, 0, temp);
		}
     */
		
		VR_MatrixMultiply( trans, rot, result);
		VR_MatrixMultiply( eye, result, result);
		VR_MatrixMultiply(result, proj, result );
				
		//memcpy( glMatrix, result, sizeof(float) * 16 );
	
		glMatrix[0] = result[0][0];
		glMatrix[1] = result[1][0];
		glMatrix[2] = result[2][0];
		glMatrix[3] = result[3][0];
		glMatrix[4] = result[0][1];
		glMatrix[5] = result[1][1];
		glMatrix[6] = result[2][1];
		glMatrix[7] = result[3][1];
		glMatrix[8] = result[0][2];
		glMatrix[9] = result[1][2];
		glMatrix[10] = result[2][2];
		glMatrix[11] = result[3][2];
		glMatrix[12] = result[0][3];
		glMatrix[13] = result[1][3];
		glMatrix[14] = result[2][3];
		glMatrix[15] = result[3][3];



		//glUniformMatrix4fv( mvpmat, 1, GL_FALSE, (GLfloat*) glMatrix );
		
		renderProgManager.SetRenderParms(RENDERPARM_MVPMATRIX_X, glMatrix, 4);
		renderProgManager.CommitUniforms();
		
		// draw the hud for that eye
		//RB_DrawStripWithCounters( &backEnd.hudSurface );
		RB_DrawElementsWithCounters( &backEnd.unitSquareSurface );
		
		hmdEyeImage[ index ]->Bind();
		
		glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, hmdEyeImage[ index ]->GetUploadWidth(), hmdEyeImage[ index ]->GetUploadHeight(), 0 );
		
	}
	renderProgManager.Unbind();

}

/*
====================
VR_HMDRender

Draw the pre rendered eye textures to the back buffer.
Apply FXAA if enabled.
Apply HMD distortion correction.

eye textures: idImage leftCurrent, rightCurrent, leftLast, rightLast   - current and last refer to frame
====================
*/

void VR_HMDRender ( idImage *leftCurrent, idImage *rightCurrent, idImage *leftLast, idImage *rightLast ) {

	extern bool hmdInFrame;
	extern ovrFrameTiming hmdFrameTime;
	bool timewarp;
	static int FBOW ;
	static int FBOH ;
	static int ocuframe = 0;
	static idImage * finalEyeImage[2];
	ovrPosef thePose;
	GL_CheckErrors();
	VR_GetFrameHMDData( ocuframe, thePose );
	VR_FrameStart( ocuframe );
		
	
	timewarp = ( 0 && hmdInFrame && vr_timewarp.GetInteger() ); // koz fixme
		

	if (VR_USE_FBO) // if using FBOs, bind them, otherwise bind the default frame buffer. 
	{
		if (VR_AAmode == VR_AA_FXAA) // perform post process FXAA antialiasing on stereoRenderImages
		{
			VR_ResolveFXAA(leftCurrent, rightCurrent);
		}
	}
/*		VR_BindFBO( GL_FRAMEBUFFER, VR_FullscreenFBO );
		GL_ViewportAndScissor( 0, 0, VR_FullscreenFBO.width, VR_FullscreenFBO.height );
	}
	else
	{
		glBindFramebuffer( GL_FRAMEBUFFER, 0 ); // bind the default framebuffer if necessary
		glDrawBuffer( GL_BACK );
		GL_ViewportAndScissor( 0, 0, renderSystem->GetNativeWidth(), renderSystem->GetNativeHeight() );
	}*/
		
	// AA is done,  FBO is bound, render headtracked quads
	
	if ( 0 && !game->IsInGame() ) 
	{
		VR_HUDRender( leftCurrent, rightCurrent );
		// headtracked textures now in hmdEyeImage[0,1]		
		finalEyeImage[0] = hmdEyeImage[0];
		finalEyeImage[1] = hmdEyeImage[1];
	}
	else {
		finalEyeImage[0] = leftCurrent;
		finalEyeImage[1] = rightCurrent;
	}
		
	// final eye textures now in finalEyeImage[0,1]				

	if ( VR_USE_FBO ) // if using FBOs, bind them, otherwise bind the default frame buffer.
	{ 

		VR_BindFBO( GL_FRAMEBUFFER, VR_FullscreenFBO );
		GL_ViewportAndScissor( 0, 0, VR_FullscreenFBO.width, VR_FullscreenFBO.height );
		GL_CheckErrors();

	}
	else	{

		glBindFramebuffer( GL_FRAMEBUFFER, 0 ); // bind the default framebuffer if necessary
		glDrawBuffer( GL_BACK );
		GL_ViewportAndScissor( 0, 0, renderSystem->GetNativeWidth(), renderSystem->GetNativeHeight() );
	}
		

	// this is the Rift warp
	// Updated shaders support oculus distortion meshes, timewarp,
	// and chromatic abberation correction.

	if (0 && timewarp && vr_chromaCorrection.GetInteger()) {
		common->Printf("Timewarp chroma\n");
		renderProgManager.BindShader_OculusWarpTimewarpChrAb();
		// koz fixme ovr_WaitTillTime( hmdFrameTime.TimewarpPointSeconds);

	}
	else if ( vr_chromaCorrection.GetInteger() != 0  ) 
	{
		//common->Printf("Chroma\n");
		renderProgManager.BindShader_OculusWarpChrAb();
	}
	else if ( timewarp ) 
	{
		common->Printf("Timewarp\n");
		//renderProgManager.BindShader_OculusWarpTimewarp();
		//ovr_WaitTillTime( hmdFrameTime.TimewarpPointSeconds);
	}
	else 
	{
		renderProgManager.BindShader_OculusWarp();
	}
	
	glClearColor( 0, 0, 0, 0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	
	GL_CheckErrors();
	// draw the left eye texture.				
	GL_SelectTexture(0);
	finalEyeImage[0]->Bind();
	//leftCurrent->Bind();
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	GL_SelectTexture(1); // bind the last rendered eye texture for overdrive
	leftLast->Bind();
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	
	RB_DrawDistortionMesh( 0 , timewarp, ocuframe, thePose ); // draw left eye
	GL_CheckErrors();
	// draw the right eye texture
	GL_SelectTexture(0);
	finalEyeImage[1]->Bind();
	//rightCurrent->Bind();
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GL_CheckErrors();
	GL_SelectTexture(1); // bind the last rendered eye texture for overdrive calc
	rightLast->Bind();
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	
	RB_DrawDistortionMesh( 1 , timewarp, ocuframe, thePose ); // draw right eye
		
	if ( VR_USE_FBO ) 
	{	// distortion corrected textures have been rendered to fullscreen FBO  
		// now draw a fullscreen quad to the default buffer		

		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		glClearColor( 0, 0, 0, 0 );
		glClear( GL_COLOR_BUFFER_BIT );

		GL_ViewportAndScissor(0, 0, renderSystem->GetNativeWidth(), renderSystem->GetNativeHeight());

		renderProgManager.BindShader_PostProcess(); // pass thru shader

		GL_SelectTexture( 0 );
		VR_FullscreenFBO.FBOImage->Bind();
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );

		RB_DrawElementsWithCounters( &backEnd.unitSquareSurface ); // draw it

	} 

	renderProgManager.Unbind();
	
}

/*
====================
VR_HMDTrackStatic
Draw tracked HUD to back buffer using the 
last fullscreen texture and force buffer swap.
====================
*/

void VR_HMDTrackStatic()
{
	
	if ( hasHMD && hasOculusRift && vr_enable.GetBool() ) {

		if (hmdCurrentRender[0] == NULL ||
			hmdCurrentRender[1] == NULL ||
			hmdPreviousRender[0] == NULL ||
			hmdPreviousRender[1] == NULL)
		{
			common->Printf("VR_HmdTrackStatic no images to render\n");
			return;
		}

		VR_HUDRender(hmdCurrentRender[0], hmdCurrentRender[1]);
		VR_HMDRender(hmdEyeImage[0], hmdEyeImage[1], hmdPreviousRender[0], hmdPreviousRender[1]);
	}

	
}
