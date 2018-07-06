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

#ifndef __VR_HMD_H__
#define __VR_HMD_H__

typedef enum
{
	EYE_LEFT = 0,
	EYE_RIGHT = 1,
	NUM_EYES = 2,
} vr_eye_t;

typedef struct
{
	vr_eye_t eyes[NUM_EYES];
} eye_order_t;

typedef struct
{
	float scale;
	float offset;
} scaleOffset_t;

typedef struct
{
	scaleOffset_t x;
	scaleOffset_t y;
} eyeScaleOffset_t;

typedef struct
{
	float projUp;
	float projDown;
	float projLeft;
	float projRight;
} eyeProjRaw_t;

typedef struct
{
	int x;
	int y;
} resolution_t;

#ifndef USE_OVR

typedef struct
{
	int x;
	int y;
} ovrEyeRenderDesc;

typedef struct
{
	int w;
	int h;
} ovrSizei;

typedef struct
{
	float x;
	float y;
} ovrVector2f;


typedef struct
{
	float LeftTan;
	float RightTan;
	float UpTan;
	float DownTan;
} ovrFovPort;

typedef struct
{
	idVec3 Position;
	idVec4 Orientation;
} ovrPosef;

#endif

typedef struct _hmdEye
{

	float				projectionHmd[16];
	idVec3				viewOffset;
	eyeProjRaw_t		projectionOpenVR;
	eyeScaleOffset_t	projectionOculus;
	ovrEyeRenderDesc	eyeRenderDesc;
	ovrSizei			renderTarget;
	resolution_t		renderTargetRes;
	ovrFovPort			eyeFov;
	ovrVector2f			UVScaleoffset[2];
} hmdEye_t;

#endif
