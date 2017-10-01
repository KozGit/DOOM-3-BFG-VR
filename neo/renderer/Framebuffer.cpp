/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 2014 Robert Beckebans

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

#include "tr_local.h"
#include "Framebuffer.h"

#include "sys\win32\win_local.h"


//idList<Framebuffer*>	Framebuffer::framebuffers;

static idList<Framebuffer*>	framebuffers;

globalFramebuffers_t globalFramebuffers;

static void R_ListFramebuffers_f( const idCmdArgs& args )
{
	if( !glConfig.framebufferObjectAvailable )
	{
		common->Printf( "GL_EXT_framebuffer_object is not available.\n" );
		return;
	}
	if ( framebuffers.Num() != 0 )
	{
		for ( int i = 0; i < framebuffers.Num(); i++ )
		{
			Framebuffer* fb = framebuffers[i];
			idStr fbName = fb->GetName();
			common->Printf( "\nFramebuffer %s:\n", fbName.c_str() );
			common->Printf( "	Framebuffer # %d\n", fb->GetFramebuffer() );
			common->Printf( "	Width x Height: %d x %d\n", fb->GetWidth(), fb->GetHeight() );
			common->Printf( "	MSAA  " );
			if ( fb->IsMSAA() )
			{ 
				common->Printf( "enabled with %d samples.\n", fb->GetSamples() );
			}
			else
			{
				common->Printf( " disabled.\n" );
			}
			if ( fb->GetDepthBuffer() )
			{
				common->Printf( "	Depthbuffer %d format %x\n", fb->GetDepthBuffer(), fb->GetDepthFormat() );
			}
				for ( int j = 0; j < 16; j++ )
			{
				if ( fb->GetColorBuffer( j ) )
				{
					common->Printf( "	Colorbuffer[%d] # %d format %x\n", j, fb->GetColorBuffer( j ), fb->GetColorFormat() );
				}
			}
		}
	}
}

/*
====================
VR_ResolveMSAA
If primaryFBO is MSAA enabled, blit it to the resolveFBO to resolve antialiasing.
resolveFBO will be bound after this operation.
====================
*/

void ResolveMSAA( void ) 
{
	if ( commonVr->useFBO ) 
	{

		if ( globalFramebuffers.primaryFBO->IsMSAA() )
		{	// primary FBO is MSAA enabled, resolve to secondary FBO to perform antialiasing before copy
		
			glBindFramebuffer( GL_DRAW_FRAMEBUFFER, globalFramebuffers.resolveFBO->GetFramebuffer() );// bind resolve FBO for draw
			glBindFramebuffer( GL_READ_FRAMEBUFFER, globalFramebuffers.primaryFBO->GetFramebuffer() ); // bind primary FBO for read
			//glClearColor( 0, 0, 0, 0 );
			//glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
			//GL_Clear( true, true, true, STENCIL_SHADOW_TEST_VALUE, 0, 0, 0, 0 );
			glBlitFramebuffer( 0, 0, globalFramebuffers.primaryFBO->GetWidth(), globalFramebuffers.primaryFBO->GetHeight(), 0, 0, globalFramebuffers.resolveFBO->GetWidth(), globalFramebuffers.resolveFBO->GetHeight(), GL_COLOR_BUFFER_BIT, GL_LINEAR ); // blit from primary FBO to resolve MSAA antialiasing.
			globalFramebuffers.resolveFBO->Bind();
			
		}
	}
}


Framebuffer::Framebuffer( const char* name, int w, int h, bool msaa )
{
	fboName = name;
	
	frameBuffer = 0;
	
	memset( colorBuffers, 0, sizeof( colorBuffers ) );
	colorFormat = 0;
	
	depthBuffer = 0;
	depthFormat = 0;
	
	stencilBuffer = 0;
	stencilFormat = 0;
	
	width = w;
	height = h;

	// Koz
	useMsaa = msaa;
	msaaSamples = r_multiSamples.GetInteger();
	//if ( !useMsaa ) msaaSamples = 0;
	//common->Printf( "Use MSAA = %d Framebuffer msaaSamples = %d\n", useMsaa, msaaSamples ); // Koz fixme
	if ( msaaSamples == 0 ) useMsaa = false;
	// Koz end
	
	glGenFramebuffers( 1, &frameBuffer );
	framebuffers.Append( this );
}

void Framebuffer::Init()
{
	cmdSystem->AddCommand( "listFramebuffers", R_ListFramebuffers_f, CMD_FL_RENDERER, "lists framebuffers" );
	
	backEnd.glState.currentFramebuffer = NULL;
	
	int width, height;
	width = height = r_shadowMapImageSize.GetInteger();
	
	for( int i = 0; i < MAX_SHADOWMAP_RESOLUTIONS; i++ )
	{
		width = height = shadowMapResolutions[i];
		
		globalFramebuffers.shadowFBO[i] = new Framebuffer( va( "_shadowMap%i", i ) , width, height, false ); // Koz
		globalFramebuffers.shadowFBO[i]->Bind();
		glDrawBuffers( 0, NULL );
	}

	BindDefault();
}

void Framebuffer::Shutdown()
{
	// TODO
}

void Framebuffer::Bind()
{
#if 0
	if( r_logFile.GetBool() )
	{
		RB_LogComment( "--- Framebuffer::Bind( name = '%s' ) ---\n", fboName.c_str() );
	}
#endif
	

	if( backEnd.glState.currentFramebuffer != this )
	{
		glBindFramebuffer( GL_FRAMEBUFFER, frameBuffer );
		backEnd.glState.currentFramebuffer = this;
		wglSwapIntervalEXT( 0 );
	}
}

void Framebuffer::BindDefault()
{
	//if(backEnd.glState.framebuffer != NULL)
	{
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );
		backEnd.glState.currentFramebuffer = NULL;

		if ( commonVr->useFBO )
		{
			globalFramebuffers.primaryFBO->Bind();
		}
		else
		{
			glDrawBuffer( GL_BACK );
			glReadBuffer( GL_BACK );
		}
	}
}

void Framebuffer::AddColorBuffer( GLuint format, int index )
{
	if( index < 0 || index >= glConfig.maxColorAttachments )
	{
		common->Warning( "Framebuffer::AddColorBuffer( %s ): bad index = %i", fboName.c_str(), index );
		return;
	}
	GL_CheckErrors();
	colorFormat = format;
	
	bool notCreatedYet = colorBuffers[index] == 0;
	if( notCreatedYet )
	{
		glGenRenderbuffers( 1, &colorBuffers[index] );
		GL_CheckErrors();
	}
	
	glBindRenderbuffer( GL_RENDERBUFFER, colorBuffers[index] );
	GL_CheckErrors();
	if ( useMsaa ) {
		common->Printf( "GlRenderBufferStorage GL_RENDERBUFFER, %d, %d, %d, %d\n", msaaSamples, format, width, height );
		glRenderbufferStorageMultisample( GL_RENDERBUFFER, msaaSamples, format, width, height );
		GL_CheckErrors();
	}
	else
	{
		glRenderbufferStorage( GL_RENDERBUFFER, format, width, height );
		GL_CheckErrors();
	}
	
	if( notCreatedYet )
	{
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_RENDERBUFFER, colorBuffers[index] );
		GL_CheckErrors();
	}
	
	GL_CheckErrors();
}

void Framebuffer::AddColorTexture( GLuint format )
{
	
	colorFormat = format;
	
	bool notCreatedYet = colorTexnum == 0;
	if ( notCreatedYet )
	{
		glGenTextures( 1, &colorTexnum );
	}

	glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, colorTexnum );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, msaaSamples, format, width, height, false );
	
	glBindFramebuffer( GL_FRAMEBUFFER, frameBuffer );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, colorTexnum, 0 );
	GL_CheckErrors();
}

void Framebuffer::AddDepthBuffer( GLuint format )
{
	depthFormat = format;
	
	bool notCreatedYet = depthBuffer == 0;
	if ( notCreatedYet )
	{
		glGenRenderbuffers( 1, &depthBuffer );
	}
	
	glBindRenderbuffer( GL_RENDERBUFFER, depthBuffer );
	
	if ( useMsaa ) 
	{
		glRenderbufferStorageMultisample( GL_RENDERBUFFER, msaaSamples, format, width, height );
	}
	else 
	{
		glRenderbufferStorage( GL_RENDERBUFFER, format, width, height );
	}
	
	glBindRenderbuffer( GL_RENDERBUFFER, 0 );
	
	if( notCreatedYet )
	{
		
		glBindFramebuffer( GL_FRAMEBUFFER, frameBuffer );
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer );
				
	}
	
}

void Framebuffer::AddDepthStencilBuffer( GLuint format )
{
	depthFormat = format;
	AddDepthBuffer( depthFormat );
	
	bool notCreatedYet = stencilBuffer == 0;
		
	if ( notCreatedYet )
	{
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer );
		stencilBuffer = depthBuffer;
		stencilFormat = depthFormat;
	}

}

void Framebuffer::AttachImage2D( GLuint target, const idImage* image, int index )
{
	if( ( target != GL_TEXTURE_2D ) && ( target < GL_TEXTURE_CUBE_MAP_POSITIVE_X || target > GL_TEXTURE_CUBE_MAP_NEGATIVE_Z ) )
	{
		common->Warning( "Framebuffer::AttachImage2D( %s ): invalid target", fboName.c_str() );
		return;
	}
	
	if( index < 0 || index >= glConfig.maxColorAttachments )
	{
		common->Warning( "Framebuffer::AttachImage2D( %s ): bad index = %i", fboName.c_str(), index );
		return;
	}
	
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, target, image->texnum, 0 );
}

void Framebuffer::AttachImageDepth( const idImage* image )
{
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, image->texnum, 0 );
}

void Framebuffer::AttachImageDepthLayer( const idImage* image, int layer )
{
	glFramebufferTextureLayer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, image->texnum, 0, layer );
}

int Framebuffer::Check()
// Koz changed to return status of framebuffer and print message if found,
// instead of just throwing an error.  
// added Framebuffer::Error ( int stautus ) that can be called
// with the result of this check to throw error if needed.
{
	int prev;
	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &prev );
	
	glBindFramebuffer( GL_FRAMEBUFFER, frameBuffer );
	
	int status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	if( status == GL_FRAMEBUFFER_COMPLETE )
	{
		glBindFramebuffer( GL_FRAMEBUFFER, prev );
		return status;
	}
	
	// something went wrong
	switch( status )
	{
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			common->Printf( "Framebuffer::Check( %s ): Framebuffer incomplete, incomplete attachment", fboName.c_str() );
			break;
			
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			common->Printf( "Framebuffer::Check( %s ): Framebuffer incomplete, missing attachment", fboName.c_str() );
			break;
			
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			common->Printf( "Framebuffer::Check( %s ): Framebuffer incomplete, missing draw buffer", fboName.c_str() );
			break;
			
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			common->Printf( "Framebuffer::Check( %s ): Framebuffer incomplete, missing read buffer", fboName.c_str() );
			break;
			
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			common->Printf( "Framebuffer::Check( %s ): Framebuffer incomplete, missing layer targets", fboName.c_str() );
			break;
			
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			common->Printf( "Framebuffer::Check( %s ): Framebuffer incomplete, missing multisample", fboName.c_str() );
			break;
			
		case GL_FRAMEBUFFER_UNSUPPORTED:
			common->Printf( "Framebuffer::Check( %s ): Unsupported framebuffer format", fboName.c_str() );
			break;
			
		default:
			common->Printf( "Framebuffer::Check( %s ): Unknown error 0x%X", fboName.c_str(), status );
			break;
	};
	
	glBindFramebuffer( GL_FRAMEBUFFER, prev );
	return status;
}

void Framebuffer::Error( int status ) // Koz throw error if needed.
{
	if ( status == GL_FRAMEBUFFER_COMPLETE )
	{
		return;
	}
	
	// something went wrong
	switch ( status )
	{
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		common->Error( "Framebuffer::Check( %s ): Framebuffer incomplete, incomplete attachment", fboName.c_str() );
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		common->Error( "Framebuffer::Check( %s ): Framebuffer incomplete, missing attachment", fboName.c_str() );
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		common->Error( "Framebuffer::Check( %s ): Framebuffer incomplete, missing draw buffer", fboName.c_str() );
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		common->Error( "Framebuffer::Check( %s ): Framebuffer incomplete, missing read buffer", fboName.c_str() );
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
		common->Error( "Framebuffer::Check( %s ): Framebuffer incomplete, missing layer targets", fboName.c_str() );
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		common->Error( "Framebuffer::Check( %s ): Framebuffer incomplete, missing multisample", fboName.c_str() );
		break;

	case GL_FRAMEBUFFER_UNSUPPORTED:
		common->Error( "Framebuffer::Check( %s ): Unsupported framebuffer format", fboName.c_str() );
		break;

	default:
		common->Error( "Framebuffer::Check( %s ): Unknown error 0x%X", fboName.c_str(), status );
		break;
	};
	
}