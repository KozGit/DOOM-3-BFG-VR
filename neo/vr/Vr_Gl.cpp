
#include "precompiled.h"
#pragma hdrstop

#include "d3xp\Game_local.h"
#include "vr.h"
#include "renderer\tr_local.h"
#include "sys\win32\win_local.h"
#include "libs\LibOVR\Include\OVR_CAPI_GL.h"
#include "libs\LibOVR\Include\Extras\OVR_Math.h"


	

void GLimp_SwapBuffers();
void GL_BlockingSwapBuffers();

extern PFNWGLSWAPINTERVALEXTPROC				wglSwapIntervalEXT;

/*
====================
iVr::FXAASetUniforms
====================
*/

void iVr::FXAASetUniforms( Framebuffer FBO ) 
{
	int progr = renderProgManager.GetGLSLCurrentProgram(); 
	
	//static GLuint scale = renderProgManager.GetUniformLocByName(progr, "Scale");
	static GLuint scale1 = glGetUniformLocation( progr,"texScale" );
	
	static GLuint resolution = glGetUniformLocation( progr, "buffersize" );
	
	//common->Printf("Setting FXAA uniforms resolution = %d texScale = %d\n",resolution,scale1);
	//glUniform2f(resolution, FBO.width, FBO.height);
	glUniform2f( resolution, FBO.GetWidth(), FBO.GetHeight() );
	glUniform2f( scale1, 1.0, 1.0 );

}

/*
====================
iVr::FXAAResolve
====================
*/

void iVr::FXAAResolve ( idImage * leftCurrent, idImage * rightCurrent ) 
{ 

	//VR_BindFBO ( GL_FRAMEBUFFER, VR_ResolveAAFBO ); // antialias image 0
	globalFramebuffers.resolveFBO->Bind();
	glClearColor( 0, 0, 0, 0 );
	glClear( GL_COLOR_BUFFER_BIT );
												
	renderProgManager.BindShader_VRFXAA();
	//VR_SetFXAAUniforms( VR_ResolveAAFBO );
						
	FXAASetUniforms( *globalFramebuffers.resolveFBO );
	GL_SelectTexture( 0 );
	leftCurrent->Bind();
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
						
	GL_ViewportAndScissor( 0, 0, globalFramebuffers.resolveFBO->GetWidth(), globalFramebuffers.resolveFBO->GetHeight() );
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
	
	GL_ViewportAndScissor( 0, 0, globalFramebuffers.resolveFBO->GetWidth(), globalFramebuffers.resolveFBO->GetHeight() );
	RB_DrawElementsWithCounters( &backEnd.unitSquareSurface ); // draw it
	
	//VR_BindFBO ( GL_READ_FRAMEBUFFER, VR_ResolveAAFBO );
	
	//rightCurrent->Bind();
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, rightCurrent->GetUploadWidth(), rightCurrent->GetUploadHeight(), 0 );
																		
	renderProgManager.Unbind();
					
} 
		


void VR_PerspectiveScale( eyeScaleOffset_t eye, GLfloat zNear, GLfloat zFar, float ( &out )[4][4] ) 
{
	
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

void VR_TranslationMatrix( float x, float y, float z, float ( &out )[4][4] ) 
{
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

void RotationMatrix( float angle, float x, float y, float z, float ( &out )[4][4] ) 
{
	
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

void VR_MatrixMultiply (float in1[4][4],float in2[4][4], float (&out)[4][4] ) 
{

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

void VR_QuatToRotation( idQuat q, float (&out)[4][4] ) 
{
	
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
static void VR_MakeStereoRenderImage( idImage *image ) 
{
	idImageOpts	opts;
	opts.width = renderSystem->GetWidth();
	opts.height = renderSystem->GetHeight();
	opts.numLevels = 1;
	opts.format = FMT_RGBA8;
	image->AllocImage( opts, TF_LINEAR, TR_CLAMP );
}

/*
====================
iVr::HUDRender
Render headtracked quad or hud mesh.

Source images: idImage image0 is left eye, image1 is right eye.
Destination images: idImage hmdEyeImage[0,1] 0 is left, 1 is right.

Original images are not modified ( can be called repeatedly with the same source textures to 
provide continual tracking for static images, e.g. during loading )

Does not perform hmd distortion correction.
====================
*/

void iVr::HUDRender( idImage *image0, idImage *image1 )
{

	//static idAngles imuAngles = { 0.0, 0.0, 0.0 };
	static idQuat imuRotation = { 0.0, 0.0, 0.0, 0.0 };
	static idQuat imuRotationGL = { 0.0, 0.0, 0.0, 0.0 };
	static idVec3 absolutePosition = vec3_zero;
	static float rot[4][4], trans[4][4], eye[4][4], proj[4][4], result[4][4] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	static float glMatrix[16] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	static float trax = 0.0f;
	static float tray = 0.0f;
	static float traz = 0.0f;
	
	for ( int i = 0; i < 2; i++ ) {
		if ( hmdEyeImage[i] == NULL ) {
			hmdEyeImage[i] = globalImages->ImageFromFunction( va( "_hmdEyeImage%i", i ), VR_MakeStereoRenderImage );
		}
		if ( hmdEyeImage[i]->GetUploadWidth() != renderSystem->GetWidth() ||
			hmdEyeImage[i]->GetUploadHeight() != renderSystem->GetHeight() ) {
			hmdEyeImage[i]->Resize( renderSystem->GetWidth(), renderSystem->GetHeight() );
		}
	}

	imuRotation = commonVr->poseHmdAngles.ToQuat();
		
	imuRotationGL.x = -imuRotation.y; // convert from id coord system to gl 
	imuRotationGL.y = imuRotation.z;
	if ( commonVr->hasOculusRift )
	{
		imuRotationGL.z = imuRotation.x;
		imuRotationGL.w = -imuRotation.w;
	} else {
		imuRotationGL.z = -imuRotation.x;
		imuRotationGL.w = imuRotation.w;
	}
	
	VR_QuatToRotation( imuRotationGL, rot );

	absolutePosition = commonVr->poseHmdAbsolutePosition;
	
	if ( commonVr->vrIsBackgroundSaving )
	{
		VR_TranslationMatrix( 0, 0, 0 /*vr_transz.GetFloat()*/, trans );
	}
	else
	{
		/*
		if ( commonVr->hasOculusRift )
			VR_TranslationMatrix( 0, 0, 1.5 , trans );
		else
			VR_TranslationMatrix( 0, 0, -1.5 , trans );
		*/

		if ( commonVr->hasOculusRift )
		{
			traz = 1.5 - absolutePosition.x / 50.0f;
			trax = absolutePosition.y / 50.0f;
			tray = -absolutePosition.z / 50.0f;
		}
		else
		{
			traz = -1.5 + absolutePosition.x / 300.0f;
			trax = absolutePosition.y / 300.0f;
			tray = -absolutePosition.z / 300.0f;
		}
		
		VR_TranslationMatrix( trax, tray, traz, trans );
		
	}

	if ( commonVr->useFBO ) // we dont want to render this into an MSAA FBO.
	{
		if ( globalFramebuffers.primaryFBO->IsMSAA() )
		{
			globalFramebuffers.resolveFBO->Bind();
		}
		else
		{
			globalFramebuffers.primaryFBO->Bind();
		}
	}
	else
	{
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		glDrawBuffer( GL_BACK );
	}

	glClearColor( 0, 0, 0, 0 );
	GL_SelectTexture( 0 );
	
	const float texS[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
	const float texT[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
	renderProgManager.SetRenderParm( RENDERPARM_TEXTUREMATRIX_S, texS );
	renderProgManager.SetRenderParm( RENDERPARM_TEXTUREMATRIX_T, texT );

	// disable any texgen
	const float texGenEnabled[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	renderProgManager.SetRenderParm( RENDERPARM_TEXGEN_0_ENABLED, texGenEnabled );

	renderProgManager.BindShader_Texture();

	for ( int index = 0; index < 2; index++ )
	{

		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		if ( index )
		{
			image1->Bind();
		}
		else
		{
			image0->Bind();
		}


		if ( commonVr->hasOculusRift )
		{
			memcpy( proj, hmdEye[index].projectionHmd, sizeof( float ) * 16 );
			proj[0][2] = hmdEye[index].projectionOculus.x.offset;

			VR_TranslationMatrix( -hmdEye[index].viewOffset[0], hmdEye[index].viewOffset[1], hmdEye[index].viewOffset[2], eye );
		} 
		else
		{
			float idx = 1.0f / (commonVr->hmdEye[index].projectionOpenVR.projRight - commonVr->hmdEye[index].projectionOpenVR.projLeft);
			float idy = 1.0f / (commonVr->hmdEye[index].projectionOpenVR.projDown - commonVr->hmdEye[index].projectionOpenVR.projUp);
			float sx = commonVr->hmdEye[index].projectionOpenVR.projRight + commonVr->hmdEye[index].projectionOpenVR.projLeft;
			float sy = commonVr->hmdEye[index].projectionOpenVR.projDown + commonVr->hmdEye[index].projectionOpenVR.projUp;
			float zNear = 0;
			float zFar = 10000;
			float depth = zFar - zNear;

			proj[0][0] = 2.0f * idx;
			proj[1][0] = 0.0f;
			proj[2][0] = sx * idx;
			proj[3][0] = 0.0f;

			proj[0][1] = 0.0f;
			proj[1][1] = 2.0f * idy;
			proj[2][1] = sy*idy;	// normally 0
			proj[3][1] = 0.0f;

			proj[0][2] = 0.0f;
			proj[1][2] = 0.0f;
			proj[2][2] = -(zFar + zNear) / depth;		// -0.999f; // adjust value to prevent imprecision issues
			proj[3][2] = -2 * zFar * zNear / depth;	// -2.0f * zNear;

			proj[0][3] = 0.0f;
			proj[1][3] = 0.0f;
			proj[2][3] = -1.0f;
			proj[3][3] = 0.0f;

			VR_TranslationMatrix( 0.0f, 0.0f, 0.0f, eye );
		}
		
		VR_MatrixMultiply( trans, rot, result );
		VR_MatrixMultiply( eye, result, result );
		VR_MatrixMultiply( result, proj, result );
		
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

		renderProgManager.SetRenderParms( RENDERPARM_MVPMATRIX_X, glMatrix, 4 );
		renderProgManager.CommitUniforms();

		// draw the hud for that eye
		RB_DrawElementsWithCounters( &backEnd.unitSquareSurface );

		hmdEyeImage[index]->Bind();

		glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, hmdEyeImage[index]->GetUploadWidth(), hmdEyeImage[index]->GetUploadHeight(), 0 );

	}
	renderProgManager.Unbind();

	globalFramebuffers.primaryFBO->Bind();
	
}



/*
====================
iVr::HMDRender

Draw the pre rendered eye textures to the back buffer.
Apply FXAA if enabled.
Apply HMD distortion correction.

eye textures: idImage leftCurrent, rightCurrent
====================
*/

void iVr::HMDRender ( idImage *leftCurrent, idImage *rightCurrent ) 
{
	using namespace OVR;

	static int FBOW;
	static int FBOH;

	wglSwapIntervalEXT( 0 ); 


	// final eye textures now in finalEyeImage[0,1]				
	
	if ( hasOculusRift )
	{
		static ovrLayerHeader	*layers = &oculusLayer.Header;
		static ovrPosef			eyeRenderPose[2];
		static ovrVector3f		viewOffset[2] = { hmdEye[0].eyeRenderDesc.HmdToEyeOffset, hmdEye[1].eyeRenderDesc.HmdToEyeOffset };
		static ovrViewScaleDesc viewScaleDesc;

		if ( commonVr->useFBO ) // if using FBOs, bind them, otherwise bind the default frame buffer.
		{

			FBOW = globalFramebuffers.primaryFBO->GetWidth();
			FBOH = globalFramebuffers.primaryFBO->GetHeight();
		}
		else
		{
			FBOW = renderSystem->GetNativeWidth();
			FBOH = renderSystem->GetNativeHeight();
			glBindFramebuffer( GL_FRAMEBUFFER, 0 ); // bind the default framebuffer if necessary
			glDrawBuffer( GL_BACK );
			backEnd.glState.currentFramebuffer = NULL;
		}

		renderProgManager.BindShader_PostProcess(); // pass thru shader

		if ( 1 ) //!vr_skipOvr.GetBool() )
		{
			wglSwapIntervalEXT( 0 );

			GLuint curTexId;
			int curIndex;

			ovr_GetTextureSwapChainCurrentIndex( hmdSession, oculusSwapChain[0], &curIndex );
			ovr_GetTextureSwapChainBufferGL( hmdSession, oculusSwapChain[0], curIndex, &curTexId );


			glBindFramebuffer( GL_FRAMEBUFFER, oculusFboId );
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0 );
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ocululsDepthTexID, 0 );

			glViewport( 0, 0, FBOW, FBOH );
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
			// koz GL_CheckErrors();

			// draw the left eye texture.				
			GL_SelectTexture( 0 );
			leftCurrent->Bind();
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
			RB_DrawElementsWithCounters( &backEnd.unitSquareSurface ); // draw it


			// right eye		
		
			ovr_GetTextureSwapChainCurrentIndex( hmdSession, oculusSwapChain[1], &curIndex );
			ovr_GetTextureSwapChainBufferGL( hmdSession, oculusSwapChain[1], curIndex, &curTexId );


			glBindFramebuffer( GL_FRAMEBUFFER, oculusFboId );
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0 );
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ocululsDepthTexID, 0 );
			glClearColor( 0, 0, 0, 0 );
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

			// draw the right eye texture
			glViewport( 0, 0, FBOW, FBOH );
			GL_SelectTexture( 0 );
			rightCurrent->Bind();

			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
			RB_DrawElementsWithCounters( &backEnd.unitSquareSurface ); // draw it

			renderProgManager.Unbind();

			// Submit frame with one layer we have.

			ovr_CommitTextureSwapChain( hmdSession, oculusSwapChain[0] );
			ovr_CommitTextureSwapChain( hmdSession, oculusSwapChain[1] );

			// Submit frame/layer to oculus compositor
			glBindFramebuffer( GL_FRAMEBUFFER, oculusFboId );
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0 );
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0 );

			ovr_CalcEyePoses( commonVr->hmdTrackingState.HeadPose.ThePose, viewOffset, eyeRenderPose );

			viewScaleDesc.HmdSpaceToWorldScaleInMeters = 0.0254f; // inches to meters
			viewScaleDesc.HmdToEyeOffset[0] = hmdEye[0].eyeRenderDesc.HmdToEyeOffset;
			viewScaleDesc.HmdToEyeOffset[1] = hmdEye[1].eyeRenderDesc.HmdToEyeOffset;

			oculusLayer.RenderPose[0] = eyeRenderPose[0];
			oculusLayer.RenderPose[1] = eyeRenderPose[1];

		
			//common->Printf( "Frame Submitting frame # %d\n", idLib::frameNumber );
			ovrResult result = ovr_SubmitFrame( hmdSession, idLib::frameNumber , &viewScaleDesc, &layers, 1 );
			if (result == ovrSuccess_NotVisible)
			{
			}
			else if (result == ovrError_DisplayLost)
			{
				common->Warning( "Vr_GL.cpp HMDRender : Display Lost when submitting oculus layer.\n" );
			}
			else if (OVR_FAILURE(result))
			{
				common->Warning( "Vr_GL.cpp HMDRender : Failed to submit oculus layer. (result %d) \n", result );
			}

			if ( vr_stereoMirror.GetBool() == true )
			{
				// Blit mirror texture to back buffer
				glBindFramebuffer( GL_READ_FRAMEBUFFER, oculusMirrorFboId );
				glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );

				glBlitFramebuffer( 0, mirrorH, mirrorW, 0, 0, 0, mirrorW, mirrorH, GL_COLOR_BUFFER_BIT, GL_NEAREST );
				glBindFramebuffer( GL_READ_FRAMEBUFFER, 0 );
			}
			else
			{
				renderProgManager.BindShader_PostProcess(); // pass thru shader
				glBindFramebuffer( GL_FRAMEBUFFER, 0 ); // bind the default framebuffer
				glDrawBuffer( GL_BACK );
				backEnd.glState.currentFramebuffer = NULL;
			
				// draw the left eye texture.				
				glViewport( 0, 0, commonVr->hmdWidth / 4, commonVr->hmdHeight / 2 );
				GL_SelectTexture( 0 );
				leftCurrent->Bind();
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
				RB_DrawElementsWithCounters( &backEnd.unitSquareSurface ); // draw it
			}

			// koz hack
			// for some reason, vsync will not disable unless wglSwapIntervalEXT( 0 )
			// is called at least once after ovr_SubmitFrame is called.
			// (at least on the two Nvidia cards I have to test with.)
			// this only seems to be the case when rendering to FBOs instead
			// of the default framebuffer.
			// if anyone has any ideas why this is, please tell!

			static int swapset = 0;
			if ( swapset == 0 )
			{
				//swapset = 1;
				wglSwapIntervalEXT( 0 );
			}

			globalFramebuffers.primaryFBO->Bind();
		}

		else
		{
			glBindFramebuffer( GL_FRAMEBUFFER, 0 ); // bind the default framebuffer
			glDrawBuffer( GL_BACK );
			backEnd.glState.currentFramebuffer = NULL;


			glViewport( 0, 0, commonVr->hmdWidth, commonVr->hmdHeight );
			glClearColor( 0, 0, 0, 0 );
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
			//koz GL_CheckErrors();

			// draw the left eye texture.				
			glViewport( 0, 0, commonVr->hmdWidth / 4, commonVr->hmdHeight / 2 );
			GL_SelectTexture( 0 );
			leftCurrent->Bind();
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
			RB_DrawElementsWithCounters( &backEnd.unitSquareSurface ); // draw it

			glViewport( commonVr->hmdWidth / 4, 0, commonVr->hmdWidth / 4, commonVr->hmdHeight / 2 );
			GL_SelectTexture( 0 );
			rightCurrent->Bind();

			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
			RB_DrawElementsWithCounters( &backEnd.unitSquareSurface ); // draw it

		}

	}
	else // openVR
	{
		vr::Texture_t leftEyeTexture = { (void*)leftCurrent->GetTexNum(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit( vr::Eye_Left, &leftEyeTexture );
		vr::Texture_t rightEyeTexture = { (void*)rightCurrent->GetTexNum(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit( vr::Eye_Right, &rightEyeTexture );
		
		wglSwapIntervalEXT( 0 ); //
		// Blit mirror texture to back buffer
		//renderProgManager.BindShader_PostProcess(); // pass thru shader

		renderProgManager.BindShader_Texture();
		GL_Color( 1, 1, 1, 1 );

		glBindFramebuffer( GL_READ_FRAMEBUFFER, 0 );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
		GL_ViewportAndScissor( 0, 0, commonVr->hmdWidth/2, commonVr->hmdHeight/2 );
		GL_SelectTexture( 0 );
		rightCurrent->Bind();
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
		RB_DrawElementsWithCounters( &backEnd.unitSquareSurface ); // draw it
		renderProgManager.Unbind();

		globalFramebuffers.primaryFBO->Bind();
	

		wglSwapIntervalEXT( 0 );//
	}
}

/* Carl
====================
iVr::HMDRenderQuad

Draw the pre rendered eye textures as a worldspace quad.
quad textures: idImage leftCurrent, rightCurrent
====================
*/

bool iVr::HMDRenderQuad(idImage *leftCurrent, idImage *rightCurrent)
{
	using namespace OVR;

	static int FBOW;
	static int FBOH;

	wglSwapIntervalEXT(0);


	// final eye textures now in finalEyeImage[0,1]				

	if (hasOculusRift)
	{
		static ovrLayerHeader	*layers = &oculusLayer.Header;
		static ovrPosef			eyeRenderPose[2];
		static ovrVector3f		viewOffset[2] = { hmdEye[0].eyeRenderDesc.HmdToEyeOffset, hmdEye[1].eyeRenderDesc.HmdToEyeOffset };
		static ovrViewScaleDesc viewScaleDesc;

		if (commonVr->useFBO) // if using FBOs, bind them, otherwise bind the default frame buffer.
		{

			FBOW = globalFramebuffers.primaryFBO->GetWidth();
			FBOH = globalFramebuffers.primaryFBO->GetHeight();
		}
		else
		{
			FBOW = renderSystem->GetNativeWidth();
			FBOH = renderSystem->GetNativeHeight();
			glBindFramebuffer(GL_FRAMEBUFFER, 0); // bind the default framebuffer if necessary
			glDrawBuffer(GL_BACK);
			backEnd.glState.currentFramebuffer = NULL;
		}

		renderProgManager.BindShader_PostProcess(); // pass thru shader

		if (1) //!vr_skipOvr.GetBool() )
		{
			wglSwapIntervalEXT(0);

			GLuint curTexId;
			int curIndex;

			ovr_GetTextureSwapChainCurrentIndex(hmdSession, oculusSwapChain[0], &curIndex);
			ovr_GetTextureSwapChainBufferGL(hmdSession, oculusSwapChain[0], curIndex, &curTexId);


			glBindFramebuffer(GL_FRAMEBUFFER, oculusFboId);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ocululsDepthTexID, 0);

			glViewport(0, 0, FBOW, FBOH);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			// koz GL_CheckErrors();

			// draw the left eye texture.				
			GL_SelectTexture(0);
			leftCurrent->Bind();
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			RB_DrawElementsWithCounters(&backEnd.unitSquareSurface); // draw it


			// right eye		

			ovr_GetTextureSwapChainCurrentIndex(hmdSession, oculusSwapChain[1], &curIndex);
			ovr_GetTextureSwapChainBufferGL(hmdSession, oculusSwapChain[1], curIndex, &curTexId);


			glBindFramebuffer(GL_FRAMEBUFFER, oculusFboId);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ocululsDepthTexID, 0);
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// draw the right eye texture
			glViewport(0, 0, FBOW, FBOH);
			GL_SelectTexture(0);
			rightCurrent->Bind();

			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			RB_DrawElementsWithCounters(&backEnd.unitSquareSurface); // draw it

			renderProgManager.Unbind();

			// Submit frame with one layer we have.

			ovr_CommitTextureSwapChain(hmdSession, oculusSwapChain[0]);
			ovr_CommitTextureSwapChain(hmdSession, oculusSwapChain[1]);

			// Submit frame/layer to oculus compositor
			glBindFramebuffer(GL_FRAMEBUFFER, oculusFboId);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);

			ovr_CalcEyePoses(commonVr->hmdTrackingState.HeadPose.ThePose, viewOffset, eyeRenderPose);

			ovrLayerQuad lg, lg2;
			lg.Header.Type = ovrLayerType_Quad;
			lg.Header.Flags = oculusLayer.Header.Flags;
			lg.ColorTexture = oculusSwapChain[0];
			lg.Viewport = oculusLayer.Viewport[0];
			lg.QuadSize.x = 3.0f; // metres
			lg.QuadSize.y = lg.QuadSize.x * 3.0f / 4.0f; // metres
			lg.QuadPoseCenter.Position.x = 0; // metres
			lg.QuadPoseCenter.Position.y = 0; // metres
			lg.QuadPoseCenter.Position.z = -1.5f; // metres (negative means in front of us)
			lg.QuadPoseCenter.Orientation.w = 1;
			lg.QuadPoseCenter.Orientation.x = 0;
			lg.QuadPoseCenter.Orientation.y = 0;
			lg.QuadPoseCenter.Orientation.z = 0;
			
			// draw another one behind us
			lg2 = lg;
			lg2.QuadPoseCenter.Orientation.w = 0;
			lg2.QuadPoseCenter.Orientation.y = 1;
			lg2.QuadPoseCenter.Position.z = -lg.QuadPoseCenter.Position.z;

			ovrLayerHeader* LayerList[2];
			LayerList[0] = &lg.Header;
			LayerList[1] = &lg2.Header;
			//common->Printf( "Frame Submitting 2D frame # %d\n", idLib::frameNumber );
			ovrResult result = ovr_SubmitFrame(hmdSession, idLib::frameNumber, NULL, LayerList, 2);
			if (result == ovrSuccess_NotVisible)
			{
			}
			else if (result == ovrError_DisplayLost)
			{
				common->Warning("Vr_GL.cpp HMDRender : Display Lost when submitting oculus layer.\n");
			}
			else if (OVR_FAILURE(result))
			{
				common->Warning("Vr_GL.cpp HMDRender : Failed to submit oculus layer. (result %d) \n", result);
			}

			if (vr_stereoMirror.GetBool() == true)
			{
				// Blit mirror texture to back buffer
				glBindFramebuffer(GL_READ_FRAMEBUFFER, oculusMirrorFboId);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

				glBlitFramebuffer(0, mirrorH, mirrorW, 0, 0, 0, mirrorW, mirrorH, GL_COLOR_BUFFER_BIT, GL_NEAREST);
				glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			}
			else
			{
				renderProgManager.BindShader_PostProcess(); // pass thru shader
				glBindFramebuffer(GL_FRAMEBUFFER, 0); // bind the default framebuffer
				glDrawBuffer(GL_BACK);
				backEnd.glState.currentFramebuffer = NULL;

				// draw the left eye texture.				
				glViewport(0, 0, commonVr->hmdWidth / 4, commonVr->hmdHeight / 2);
				GL_SelectTexture(0);
				leftCurrent->Bind();
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
				RB_DrawElementsWithCounters(&backEnd.unitSquareSurface); // draw it
			}

			// koz hack
			// for some reason, vsync will not disable unless wglSwapIntervalEXT( 0 )
			// is called at least once after ovr_SubmitFrame is called.
			// (at least on the two Nvidia cards I have to test with.)
			// this only seems to be the case when rendering to FBOs instead
			// of the default framebuffer.
			// if anyone has any ideas why this is, please tell!

			static int swapset = 0;
			if (swapset == 0)
			{
				//swapset = 1;
				wglSwapIntervalEXT(0);
			}

			globalFramebuffers.primaryFBO->Bind();
		}
		return true;
	}
	else // openVR
	{
		return false;
	}
}

/*
====================
iVr::HMDTrackStatic()
Draw tracked HUD to back buffer using the 
last fullscreen texture then force a buffer swap.
====================
*/

void iVr::HMDTrackStatic( bool is3D )
{
	
	
	if ( game->isVR )
	{
		
		//common->Printf( "HmdTrackStatic called idFrame #%d\n", idLib::frameNumber);
		if ( commonVr->hmdCurrentRender[0] == NULL || commonVr->hmdCurrentRender[1] == NULL )
		{
			common->Printf( "VR_HmdTrackStatic no images to render\n" );
			return;
		}


		if ( is3D || !HMDRenderQuad( commonVr->hmdCurrentRender[0], commonVr->hmdCurrentRender[1] ) )
		{
			//commonVr->hmdCurrentRender[0]->CopyFramebuffer(renderSystem->GetWidth(), renderSystem->GetHeight(), renderSystem->GetWidth(), renderSystem->GetHeight());
			//commonVr->hmdCurrentRender[1]->CopyFramebuffer(renderSystem->GetWidth(), renderSystem->GetHeight(), renderSystem->GetWidth(), renderSystem->GetHeight());
			HUDRender(commonVr->hmdCurrentRender[0], commonVr->hmdCurrentRender[1]);
			HMDRender( hmdEyeImage[0], hmdEyeImage[1] );
		}
				
	}
}
