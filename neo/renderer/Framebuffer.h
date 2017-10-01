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

#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

static const int MAX_SHADOWMAP_RESOLUTIONS = 5;
#if 1
static	int shadowMapResolutions[MAX_SHADOWMAP_RESOLUTIONS] = { 2048, 1024, 512, 512, 256 };
#else
static	int shadowMapResolutions[MAX_SHADOWMAP_RESOLUTIONS] = { 1024, 1024, 1024, 1024, 1024 };
#endif

void ResolveMSAA( void );

class Framebuffer
{
public:

	Framebuffer( const char* name, int width, int height, bool msaa ); // Koz add MSAA support
	
	static void				Init();
	static void				Shutdown();
	
	// deletes OpenGL object but leaves structure intact for reloading
	void					PurgeFramebuffer();
	
	void					Bind();
	static void				BindDefault();
	
	void					AddColorBuffer( GLuint format, int index );
	void					AddColorTexture( GLuint format );
	void					AddDepthBuffer( GLuint format );
	void					AddDepthStencilBuffer( GLuint format ); // Koz bind depth buffer as stencil attachment also.
	
	void					AttachImage2D( GLuint target, const idImage* image, int index );
	void					AttachImage3D( const idImage* image );
	void					AttachImageDepth( const idImage* image );
	void					AttachImageDepthLayer( const idImage* image, int layer );

	// Koz begin
	int						GetWidth() const { return width; }
	int						GetHeight() const { return height; }
	int						GetSamples() const { return msaaSamples; }
	int						GetDepthBuffer() const { return depthBuffer; }
	int						GetDepthFormat() const { return depthFormat; }
	int						GetStencilBuffer() const { return stencilBuffer; }
	int						GetStencilFormat() const { return stencilFormat; }
	int						GetColorFormat() const { return colorFormat; }
	int						GetColorBuffer( int cb ) const { return colorBuffers[cb]; }
	int						GetMSAATexnum() const { return colorTexnum; }
	bool					IsMSAA() const { return useMsaa; }
	idStr					GetName() const { return fboName; }
	// Koz end

	// check for OpenGL errors
	int						Check();
	void					Error( int status ); // Koz
	uint32_t				GetFramebuffer() const
	{
		return frameBuffer;
	}
	
private:
	idStr					fboName;
	
	// FBO object
	uint32_t				frameBuffer;
	
	uint32_t				colorBuffers[16];
	int						colorFormat;
	
	uint32_t				depthBuffer;
	int						depthFormat;
	
	uint32_t				stencilBuffer;
	int						stencilFormat;
	
	int						width;
	int						height;

	bool					useMsaa;
	int						msaaSamples;

	GLuint					colorTexnum;
	GLuint					depthTexnum;

	
	//static idList<Framebuffer*>	framebuffers;
};

struct globalFramebuffers_t
{
	Framebuffer*				shadowFBO[MAX_SHADOWMAP_RESOLUTIONS];
	Framebuffer*				primaryFBO;
	Framebuffer*				resolveFBO;
	Framebuffer*				fullscreenFBO;

};
extern globalFramebuffers_t globalFramebuffers;


#endif // __FRAMEBUFFER_H__