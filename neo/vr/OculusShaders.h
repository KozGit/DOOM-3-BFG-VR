#include "idlib\precompiled.h"

const idStr OculusMeshFragmentUniforms =		"";
const idStr OculusMeshVertexUniforms =		"";

const idStr OculusMeshVertexShader =	"#version 110\n" \


									"uniform vec2 EyeToSourceUVScale;\n"\
									"uniform vec2 EyeToSourceUVOffset;\n"\

									"attribute vec2 POS;\n"\
									"attribute vec4 DCOLOR;\n"\
									"attribute vec2 TEXR;\n"\

									"varying vec4 oColor;\n"\
									"varying vec2 oTexCoord;\n"\

									"void main()\n"\
									"{\n"\
									"	gl_Position.x = POS.x;\n"\
									"	gl_Position.y = POS.y;\n"\
									"	gl_Position.z = 0.5;\n"\
									"	gl_Position.w = 1.0;\n"\
									"// Vertex inputs are in TanEyeAngle space for the R,G,B channels (i.e. after chromatic aberration and distortion).\n"\
									"// Scale them into the correct [0-1],[0-1] UV lookup space (depending on eye)\n"\
									"	oTexCoord = TEXR * EyeToSourceUVScale + EyeToSourceUVOffset;\n"\
									"	oTexCoord.y = 1.0 - oTexCoord.y;\n"\
									"	oColor = DCOLOR;              // Used for vignette fade.\n"\
									"};\n";

const idStr OculusMeshFragmentShader = "#version 110\n"\

									"uniform sampler2D currentFrame;\n"\
									"uniform sampler2D lastFrame;\n"\

									"varying vec4 oColor;\n"\
									"varying vec2 oTexCoord;\n"\

									"uniform vec2 InverseResolution;\n"\
									"uniform vec2 OverdriveScales;\n"\
									"uniform float VignetteFade;\n"\

									"void main()\n"\
									"{\n"\
									"	vec3 newColor = texture2DLod( currentFrame, oTexCoord, 0.0 ).rgb;\n"\
									
									"	if ( VignetteFade > 0.0 );\n"\
									"	{\n"\
									"		newColor = newColor * oColor.xxx;\n"\
									"	}\n"\

									"	if ( OverdriveScales.x > 0.0 )\n"\
									"	{\n"\
									"		vec3 oldColor = texture2DLod( lastFrame, ( gl_FragCoord.xy * InverseResolution ), 0.0 ).rgb;\n"\
																		
									"		vec3 adjustedScales;\n"\
									"		adjustedScales.x = newColor.x > oldColor.x ? OverdriveScales.x : OverdriveScales.y;\n"\
									"		adjustedScales.y = newColor.y > oldColor.y ? OverdriveScales.x : OverdriveScales.y;\n"\
									"		adjustedScales.z = newColor.z > oldColor.z ? OverdriveScales.x : OverdriveScales.y;\n"\
									
									"		newColor = clamp( newColor + ( newColor - oldColor ) * adjustedScales , 0.0, 1.0 );\n"\
									
									"	}\n"\
									"	gl_FragColor = vec4( newColor, 1.0 );\n"\
									"};\n";


/*
======================
Oculus Shaders with chromatic abberation correction 
======================
*/

const idStr OculusMeshVertexShaderChrAb =		"#version 110\n" \
									
										"uniform vec2 EyeToSourceUVScale;\n"\
										"uniform vec2 EyeToSourceUVOffset;\n"\

										"attribute vec2 POS;\n"\
										"attribute vec4 DCOLOR;\n"\
										"attribute vec2 TEXR;\n"\
										"attribute vec2 TEXG;\n"\
										"attribute vec2 TEXB;\n"\
										
										"varying vec4 oColor;\n"\
										"varying vec2 oTexCoord0;\n"\
										"varying vec2 oTexCoord1;\n"\
										"varying vec2 oTexCoord2;\n"\
										
										"void main()\n"\
										"{\n"\
										"   gl_Position.x = POS.x;\n"\
										"   gl_Position.y = POS.y;\n"\
										"   gl_Position.z = 0.5;\n"\
										"   gl_Position.w = 1.0;\n"\
										"// Vertex inputs are in TanEyeAngle space for the R,G,B channels (i.e. after chromatic aberration and distortion).\n"\
										"// Scale them into the correct [0-1],[0-1] UV lookup space (depending on eye)\n"\
										"   oTexCoord0 = TEXR * EyeToSourceUVScale + EyeToSourceUVOffset;\n"\
										"   oTexCoord0.y = 1.0-oTexCoord0.y;\n"\
										"   oTexCoord1 = TEXG * EyeToSourceUVScale + EyeToSourceUVOffset;\n"\
										"   oTexCoord1.y = 1.0-oTexCoord1.y;\n"\
										"   oTexCoord2 = TEXB * EyeToSourceUVScale + EyeToSourceUVOffset;\n"\
										"   oTexCoord2.y = 1.0-oTexCoord2.y;\n"\
										"   oColor = DCOLOR; // Used for vignette fade.\n"\
										"};\n";



const idStr OculusMeshFragmentShaderChrAb =	"#version 110\n"\

										"uniform sampler2D currentFrame;\n"\
										"uniform sampler2D lastFrame;\n"\
										
										"varying vec4 oColor;\n"\
										"varying vec2 oTexCoord0;\n"\
										"varying vec2 oTexCoord1;\n"\
										"varying vec2 oTexCoord2;\n"\
										
										"uniform vec2 OverdriveScales;\n"\

										"void main()\n"\
										"{\n"\
										"	float ResultR = texture2DLod(currentFrame, oTexCoord0, 0.0).r;\n"\
										"	float ResultG = texture2DLod(currentFrame, oTexCoord1, 0.0).g;\n"\
										"	float ResultB = texture2DLod(currentFrame, oTexCoord2, 0.0).b;\n"\
										"	vec4 outColor = vec4(ResultR * oColor.r, ResultG * oColor.g, ResultB * oColor.b, 1.0);\n"\

										"	if( OverdriveScales.x > 0.0 )\n"\
										"	{\n"\
										"		vec3 oldColor = texture2DLod(lastFrame, (gl_FragCoord.xy * 0.5 + 0.5), 0.0).rgb;\n"\
										
										"		vec3 newColor = outColor.rgb;\n"\
										
										"		vec3 adjustedScales;\n"\
										"		adjustedScales.x = newColor.x > oldColor.x ? OverdriveScales.x : OverdriveScales.y;\n"\
										"		adjustedScales.y = newColor.y > oldColor.y ? OverdriveScales.x : OverdriveScales.y;\n"\
										"		adjustedScales.z = newColor.z > oldColor.z ? OverdriveScales.x : OverdriveScales.y;\n"\
										
										"		vec3 overdriveColor = clamp(newColor + (newColor - oldColor) * adjustedScales,0.0,1.0);\n"\
										"		outColor = vec4(overdriveColor, 1.0);\n"\
										"	}\n"\

										"	gl_FragColor = outColor;\n"\
										"};\n";


/*
======================
Oculus Vertex Shaders - Timewarp - with & without chromatic abberation correction 
======================
*/

const idStr OculusMeshVertexShaderTimewarp =	"#version 110\n"\
												"uniform vec2 EyeToSourceUVScale;\n"\
												"uniform vec2 EyeToSourceUVOffset;\n"\
												"uniform mat4 EyeRotationStart;\n"\
												"uniform mat4 EyeRotationEnd;\n"\

												"attribute vec2 POS;\n"\
												"attribute vec4 DCOLOR;\n"\
												"attribute vec2 TEXR;\n"\

												"varying vec4 oColor;\n"\
												"varying vec2 oTexCoord;\n"\

												"void main()\n"\
												"{\n"\
												"   gl_Position.x = POS.x;\n"\
												"   gl_Position.y = POS.y;\n"\
												"   gl_Position.z = 0.0;\n"\
												"   gl_Position.w = 1.0;\n"\
												"// Vertex inputs are in TanEyeAngle space for the R,G,B channels (i.e. after chromatic aberration and distortion).\n"\
												"// These are now real world vectors in direction (x,y,1) relative to the eye of the HMD.\n"\
												"   vec3 TanEyeAngle = vec3 ( TEXR.x, TEXR.y, 1.0 );\n"\
												
												"// Accurate time warp lerp vs. faster\n"\
												"#if 1\n"\
												"// Apply the two 3x3 timewarp rotations to these vectors.\n"\
												"   vec3 TransformedStart = (EyeRotationStart * vec4(TanEyeAngle, 0)).xyz;\n"\
												"   vec3 TransformedEnd   = (EyeRotationEnd * vec4(TanEyeAngle, 0)).xyz;\n"\
												"// And blend between them.\n"\
												"   vec3 Transformed = mix ( TransformedStart, TransformedEnd, Color.a );\n"\
												"#else\n"\
												"   mat4 EyeRotation = mix ( EyeRotationStart, EyeRotationEnd, Color.a );\n"\
												"   vec3 Transformed   = EyeRotation * TanEyeAngle;\n"\
												"#endif\n"\

												"// Project them back onto the Z=1 plane of the rendered images.\n"\
												"   float RecipZ = 1.0 / Transformed.z;\n"\
												"   vec2 Flattened = vec2 ( Transformed.x * RecipZ, Transformed.y * RecipZ );\n"\
												"// These are now still in TanEyeAngle space.\n"\
												"// Scale them into the correct [0-1],[0-1] UV lookup space (depending on eye)\n"\
												"   vec2 SrcCoord = Flattened * EyeToSourceUVScale + EyeToSourceUVOffset;\n"\
												"   oTexCoord = SrcCoord;\n"\
												"   oTexCoord.y = 1.0-oTexCoord.y;\n"\
												"   oColor = vec4(DCOLOR.r, DCOLOR.r, DCOLOR.r, DCOLOR.r);              // Used for vignette fade.\n"\
												"}\n";


const idStr OculusMeshVertexShaderTimewarpChrAb =	"#version 110\n"\
													
													"uniform vec2 EyeToSourceUVScale;\n"\
													"uniform vec2 EyeToSourceUVOffset;\n"\
													"uniform mat4 EyeRotationStart;\n"\
													"uniform mat4 EyeRotationEnd;\n"\

													"attribute vec2 POS;\n"\
													"attribute vec4 DCOLOR;\n"\
													"attribute vec2 TEXR;\n"\
													"attribute vec2 TEXG;\n"\
													"attribute vec2 TEXB;\n"\

													"varying vec4 oColor;\n"\
													"varying vec2 oTexCoord0;\n"\
													"varying vec2 oTexCoord1;\n"\
													"varying vec2 oTexCoord2;\n"\

													"void main()\n"\
													"{\n"\
													"   gl_Position.x = POS.x;\n"\
													"   gl_Position.y = POS.y;\n"\
													"   gl_Position.z = 0.0;\n"\
													"   gl_Position.w = 1.0;\n"\
													
													"// Vertex inputs are in TanEyeAngle space for the R,G,B channels (i.e. after chromatic aberration and distortion).\n"\
													"// These are now real world vectors in direction (x,y,1) relative to the eye of the HMD.\n"\
													"   vec3 TanEyeAngleR = vec3 ( TEXR.x, TEXR.y, 1.0 );\n"\
													"   vec3 TanEyeAngleG = vec3 ( TEXG.x, TEXG.y, 1.0 );\n"\
													"   vec3 TanEyeAngleB = vec3 ( TEXB.x, TEXB.y, 1.0 );\n"\
													
													"// Accurate time warp lerp vs. faster\n"\
													"#if 1\n"\
													"// Apply the two 3x3 timewarp rotations to these vectors.\n"\
													"   vec3 TransformedRStart = (EyeRotationStart * vec4(TanEyeAngleR, 0)).xyz;\n"\
													"   vec3 TransformedGStart = (EyeRotationStart * vec4(TanEyeAngleG, 0)).xyz;\n"\
													"   vec3 TransformedBStart = (EyeRotationStart * vec4(TanEyeAngleB, 0)).xyz;\n"\
													"   vec3 TransformedREnd   = (EyeRotationEnd * vec4(TanEyeAngleR, 0)).xyz;\n"\
													"   vec3 TransformedGEnd   = (EyeRotationEnd * vec4(TanEyeAngleG, 0)).xyz;\n"\
													"   vec3 TransformedBEnd   = (EyeRotationEnd * vec4(TanEyeAngleB, 0)).xyz;\n"\
													
													"// And blend between them.\n"\
													"   vec3 TransformedR = mix ( TransformedRStart, TransformedREnd, DCOLOR.a );\n"\
													"   vec3 TransformedG = mix ( TransformedGStart, TransformedGEnd, DCOLOR.a );\n"\
													"   vec3 TransformedB = mix ( TransformedBStart, TransformedBEnd, DCOLOR.a );\n"\
													"#else\n"\
													"   mat3 EyeRotation;\n"\
													"   EyeRotation[0] = mix ( EyeRotationStart[0], EyeRotationEnd[0], DCOLOR.a ).xyz;\n"\
													"   EyeRotation[1] = mix ( EyeRotationStart[1], EyeRotationEnd[1], DCOLOR.a ).xyz;\n"\
													"   EyeRotation[2] = mix ( EyeRotationStart[2], EyeRotationEnd[2], DCOLOR.a ).xyz;\n"\
													"   vec3 TransformedR   = EyeRotation * TanEyeAngleR;\n"\
													"   vec3 TransformedG   = EyeRotation * TanEyeAngleG;\n"\
													"   vec3 TransformedB   = EyeRotation * TanEyeAngleB;\n"\
													"#endif\n"\

													"// Project them back onto the Z=1 plane of the rendered images.\n"\
													"   float RecipZR = 1.0 / TransformedR.z;\n"\
													"   float RecipZG = 1.0 / TransformedG.z;\n"\
													"   float RecipZB = 1.0 / TransformedB.z;\n"\
													"   vec2 FlattenedR = vec2 ( TransformedR.x * RecipZR, TransformedR.y * RecipZR );\n"\
													"   vec2 FlattenedG = vec2 ( TransformedG.x * RecipZG, TransformedG.y * RecipZG );\n"\
													"   vec2 FlattenedB = vec2 ( TransformedB.x * RecipZB, TransformedB.y * RecipZB );\n"\

													"// These are now still in TanEyeAngle space.\n"\
													"// Scale them into the correct [0-1],[0-1] UV lookup space (depending on eye)\n"\
													"   vec2 SrcCoordR = FlattenedR * EyeToSourceUVScale + EyeToSourceUVOffset;\n"\
													"   vec2 SrcCoordG = FlattenedG * EyeToSourceUVScale + EyeToSourceUVOffset;\n"\
													"   vec2 SrcCoordB = FlattenedB * EyeToSourceUVScale + EyeToSourceUVOffset;\n"\
													"   oTexCoord0 = SrcCoordR;\n"\
													"   oTexCoord0.y = 1.0-oTexCoord0.y;\n"\
													"   oTexCoord1 = SrcCoordG;\n"\
													"   oTexCoord1.y = 1.0-oTexCoord1.y;\n"\
													"   oTexCoord2 = SrcCoordB;\n"\
													"   oTexCoord2.y = 1.0-oTexCoord2.y;\n"\
													
													"   oColor = vec4(DCOLOR.r, DCOLOR.r, DCOLOR.r, DCOLOR.r);              // Used for vignette fade.\n"\
													"}\n";



/*
======================
Oculus Shaders with chromatic abberation correction & antialiasing
======================
*/

const idStr OculusMeshVertexShaderChrAb1 =		"#version 110\n" \
									
			"uniform vec2 EyeToSourceUVScale;\n"\
			"uniform vec2 EyeToSourceUVOffset;\n"\

			"attribute vec2 POS;\n"\
			"attribute vec4 DCOLOR;\n"\
			"attribute vec2 TEXR;\n"\
			"attribute vec2 TEXG;\n"\
			"attribute vec2 TEXB;\n"\
			
			"varying vec4 oPosition;\n"\
			"varying vec4 oColor;\n"\
			"varying vec2 oTexCoord0;\n"\
			"varying vec2 oTexCoord1;\n"\
			"varying vec2 oTexCoord2;\n"\

													
			"void main()\n"\
			"{\n"\
			"   gl_Position.x = POS.x;\n"\
			"   gl_Position.y = POS.y;\n"\
			"   gl_Position.z = 0.5;\n"\
			"   gl_Position.w = 1.0;\n"\
			"   oPosition.x = POS.x;\n"\
			"   oPosition.y = POS.y;\n"\
			"   oPosition.z = 0.5;\n"\
			"   oPosition.w = 1.0;\n"\


			"// Vertex inputs are in TanEyeAngle space for the R,G,B channels (i.e. after chromatic aberration and distortion).\n"\
			"// Scale them into the correct [0-1],[0-1] UV lookup space (depending on eye)\n"\
			"   oTexCoord0 =  EyeToSourceUVScale * TEXR + EyeToSourceUVOffset;\n"\
			"   oTexCoord0.y = 1.0-oTexCoord0.y;\n"\
			"   oTexCoord1 = EyeToSourceUVScale * TEXG + EyeToSourceUVOffset;\n"\
			"   oTexCoord1.y = 1.0-oTexCoord1.y;\n"\
			"   oTexCoord2 = EyeToSourceUVScale * TEXB + EyeToSourceUVOffset;\n"\
			"   oTexCoord2.y = 1.0-oTexCoord2.y;\n"\
			"   oColor = DCOLOR; // Used for vignette fade.\n"\
			"};\n";



const idStr OculusMeshFragmentShaderChrAb1 =	"#version 110\n"\
			
			"uniform sampler2D lastFrame;\n"\
			"uniform sampler2D currentFrame;\n"\
			
			"uniform vec2 OverdriveScales;\n"\
			"uniform vec2 AaDerivMult;\n"\
			
			"varying vec4 oPosition;\n"\
			"varying vec4 oColor;\n"\
			"varying vec2 oTexCoord0;\n"\
			"varying vec2 oTexCoord1;\n"\
			"varying vec2 oTexCoord2;\n"\
			
			"vec4 outColor0;\n"\
			"vec4 outColor1;\n"\
						
			"vec3 ToLinear( vec3 inColor ) { return inColor * inColor; }\n"\
			"vec3 ToGamma( vec3 inColor ) { return sqrt( inColor ); }\n"\

			"vec3 ApplyHqAa( vec3 inColor, vec2 oTexCoord0, vec2 oTexCoord1, vec2 oTexCoord2 )\n"\
			"{\n"\
			"	vec2 uvDeriv = vec2(dFdx(oTexCoord1.x), dFdy(oTexCoord1.y)) * AaDerivMult.x;\n"\
			"	float ResultR = texture2DLod(currentFrame, oTexCoord0 + vec2(-uvDeriv.x, 0), 0.0).r;\n"\
			"	float ResultG = texture2DLod(currentFrame, oTexCoord1 + vec2(-uvDeriv.x, 0), 0.0).g;\n"\
			"	float ResultB = texture2DLod(currentFrame, oTexCoord2 + vec2(-uvDeriv.x, 0), 0.0).b;\n"\
			"	vec3 newColor0 = ToLinear( vec3( ResultR, ResultG, ResultB ) );\n"\

			"	ResultR = texture2DLod(currentFrame, oTexCoord0 + vec2(uvDeriv.x, 0), 0.0).r;\n"\
			"	ResultG = texture2DLod(currentFrame, oTexCoord1 + vec2(uvDeriv.x, 0), 0.0).g;\n"\
			"	ResultB = texture2DLod(currentFrame, oTexCoord2 + vec2(uvDeriv.x, 0), 0.0).b;\n"\
			"	vec3 newColor1 = ToLinear( vec3( ResultR, ResultG, ResultB ) );\n"\

			"	ResultR = texture2DLod(currentFrame, oTexCoord0 + vec2(0, uvDeriv.y), 0.0).r;\n"\
			"	ResultG = texture2DLod(currentFrame, oTexCoord1 + vec2(0, uvDeriv.y), 0.0).g;\n"\
			"	ResultB = texture2DLod(currentFrame, oTexCoord2 + vec2(0, uvDeriv.y), 0.0).b;\n"\
			"	vec3 newColor2 = ToLinear( vec3( ResultR, ResultG, ResultB ) );\n"\

			"	ResultR = texture2DLod(currentFrame, oTexCoord0 + vec2(0, -uvDeriv.y), 0.0).r;\n"\
			"	ResultG = texture2DLod(currentFrame, oTexCoord1 + vec2(0, -uvDeriv.y), 0.0).g;\n"\
			"	ResultB = texture2DLod(currentFrame, oTexCoord2 + vec2(0, -uvDeriv.y), 0.0).b;\n"\
			"	vec3 newColor3 = ToLinear( vec3( ResultR, ResultG, ResultB ) );\n"\

			"	vec3 outColor = ToLinear(inColor) + newColor0 + newColor1 + newColor2 + newColor3;\n"\
			"	outColor = ToGamma(outColor * 0.2);\n"\
			"	return outColor;\n"\
			"}\n"\
						
			"void main()\n"\
			"{\n"\
			"	float ResultR = texture2DLod(currentFrame, oTexCoord0, 0.0).r;\n"\
			"	float ResultG = texture2DLod(currentFrame, oTexCoord1, 0.0).g;\n"\
			"	float ResultB = texture2DLod(currentFrame, oTexCoord2, 0.0).b;\n"\
			"	vec3 newColor = vec3( ResultR , ResultG , ResultB );\n"\
			"	vec4 outColor = vec4( ResultR * oColor.r , ResultG * oColor.g , ResultB * oColor.b ,1.0 );\n"\
			"	//High quality anti-aliasing in distortion\n"\
			"	if ( AaDerivMult.x  > 0.0 )\n"\
			"	{\n"\
			"		//newColor = ApplyHqAa(newColor, oTexCoord0, oTexCoord1, oTexCoord2);\n"\
			"	}\n"\
			
			"	newColor = vec3(newColor.r * oColor.r, newColor.g * oColor.g,newColor.b * oColor.b);\n"\
			"	outColor0 = vec4(newColor, 1.0);\n"\
			"	outColor1 = outColor0;\n"\
						
			"	// pixel luminance overdrive\n"\

			"	if( OverdriveScales.x > 0.0 )\n"\
			"	{\n"\
			"		vec3 oldColor = texture2DLod(lastFrame, ( gl_FragCoord.xy * 0.5 + 0.5 ), 0.0).rgb;\n"\
			"		newColor = outColor.rgb;\n"\
			"		vec3 adjustedScales;\n"\
			"		adjustedScales.x = newColor.x > oldColor.x ? OverdriveScales.x : OverdriveScales.y;\n"\
			"		adjustedScales.y = newColor.y > oldColor.y ? OverdriveScales.x : OverdriveScales.y;\n"\
			"		adjustedScales.z = newColor.z > oldColor.z ? OverdriveScales.x : OverdriveScales.y;\n"\
										
			"		vec3 overdriveColor = clamp(newColor + (newColor - oldColor) * adjustedScales,0.0,1.0);\n"\
			"		outColor1 = vec4(overdriveColor, 1.0);\n"\
			"	}\n"\
			"	gl_FragColor = outColor1;\n"\
			"	//gl_FragData[0] = outColor0;\n"\
			"	//gl_FragData[1] = outColor1;\n"\
			"};\n";

/*
======================
VR Simple MVP Shader
======================
*/
const idStr VrSimpleVertexShader1 =	"#version 110\n" \
	
									"attribute vec4 POSITION;\n"\
									"attribute vec2 TEXCOORD;\n"\
									"attribute vec4 NORMAL;\n"\
									"attribute vec4 TANGENT;\n"\
									"attribute vec4 COLOR;\n"\
									

									"varying vec2 oTexCoord;\n"\

									"void main()\n"\
									"{\n"\
									"//	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix  * vec4( POSITION.x, POSITION.y, POSITION.z, 1.0);\n"\
									"	vec4 newPos = vec4( POSITION.x, POSITION.y, POSITION.z, 1.0);\n"\
									"	gl_Position = newPos;\n"\
									"	oTexCoord.x = TEXCOORD.x;\n"\
									"	oTexCoord.y = TEXCOORD.y;\n"\
									"};\n";

const idStr VrSimpleFragmentShader1 = "#version 110\n"\

									"varying vec2 oTexCoord;\n"\
									
									"uniform sampler2D currentTexture;\n"\
																											
									"void main()\n"\
									"{\n"\
									"	vec3 newColor = texture2D( currentTexture, oTexCoord.xy).rgb;\n"\
									"	vec4 outColor = vec4(newColor, 1 );\n"\
									
									"	gl_FragColor = outColor;\n"\
									"	//gl_FragColor = vec4(oTexCoord.x * 254.0,oTexCoord.y*254.0,0.0,1.0) ;\n"\
									"};\n";



const idStr VrSimpleVertexShader =	"#version 150\n"\
									"#define PC\n"\
									"uniform mat4 mvpMat;\n"\
									"in vec4 in_Position;\n"\
									"in vec2 in_TexCoord;\n"\
									"in vec4 in_Normal;\n"\
									"in vec4 in_Tangent;\n"\
									"in vec4 in_Color;\n"\

									"out vec4 gl_Position;\n"\
									"out vec2 vofi_TexCoord0;\n"\

									"void main() {\n"\
									"	vec4 pos = vec4(in_Position.xyz , 1.0);\n"\
									"	//gl_Position = in_Position ;\n"\
									"	//gl_Position = pos;\n"\
									"	//vec4 newpos = gl_ProjectionMatrix * gl_ModelViewMatrix  * gl_Vertex;\n"\
									"	vec4 eye_position = mvpMat * pos;\n"\
									"	gl_Position = eye_position;\n"\
									"	vofi_TexCoord0 = in_TexCoord ;\n"\
									"}\n";

const idStr VrSimpleFragmentShader =	"#version 150\n"\
										"#define PC\n"\
										
										"vec4 tex2D( sampler2D sampler, vec2 texcoord ) { return texture( sampler, texcoord.xy ); }\n"\

										"uniform sampler2D samp0;\n"\

										"in vec4 gl_FragCoord;\n"\
										"in vec2 vofi_TexCoord0;\n"\

										"out vec4 gl_FragColor;\n"\

										"void main() {\n"\
										"	vec2 tCoords = vofi_TexCoord0 ;\n"\
										"	gl_FragColor = tex2D ( samp0 , tCoords ) ;\n"\
										"}\n";


/*
======================
VR FXAA Antialiasing shader
======================
*/

const idStr FXAAVertexShader = 	"#version 110\n"\
		"//attribute vec2 Position;\n"\
		"//attribute vec2 TexCoord;\n"\
		"uniform vec2 texScale;\n"\
		"attribute vec3 Position;\n"\
		"attribute vec4 Normal;\n"\
		"attribute vec4 Color1;\n"\
		"attribute vec4 Color2;\n"\
		"attribute vec2 TexCoord;\n"\
		"attribute vec4 tangent;\n"\
		"\n"\
		"\n"\
		"varying vec2 TexCoords;\n"\
		"\n"\
		"void main()\n"\
		"{\n"\
		"	gl_Position = vec4( Position.x,Position.y, 1.0, 1.0 );\n"\
		"	TexCoords = ( TexCoord * texScale ); \n"\
		"};\n";



const idStr FXAAFragmentShader = "\n"\
		"/*---------------------------------------------------------------------------------------------------------\n"\
		"\n"\
		"FXAA 3.11 for LeadWerks engine converted by Franck Poulain and originaly made by TIMOTHY LOTTES\n"\
		"---------------------------------------------------------------------------------------------------------*/\n"\
		"\n"\
		"#extension GL_EXT_gpu_shader4 : enable \n"\
		"\n"\
		"// PARAMETERS\n"\
		"#define FXAA_QUALITY__PRESET 13\n"\
		"\n"\
		"// IMPORTANT: You can find other parameters to tweak in the main() function of this shader\n"\
		"\n"\
		"\n"\
		"// Dont touch this\n"\
		"#define FXAA_GLSL_120\n"\
		"#define FXAA_GREEN_AS_LUMA 1\n"\
		"#define FXAA_FAST_PIXEL_OFFSET 0\n"\
		"#define FXAA_GATHER4_ALPHA 0\n"\
		"\n"\
		"#define COLOR Texture0\n"\
		"uniform sampler2D COLOR;\n"\
		"uniform vec2 buffersize;\n"\
		"\n"\
		"\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"#ifndef FXAA_GLSL_120\n"\
		"        #define FXAA_GLSL_120 0\n"\
		"#endif\n"\
		"\n"\
		"\n"\
		"/*==========================================================================*/\n"\
		"#ifndef FXAA_GREEN_AS_LUMA\n"\
		"        #define FXAA_GREEN_AS_LUMA 0\n"\
		"#endif\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"\n"\
		"#ifndef FXAA_DISCARD\n"\
		"        #define FXAA_DISCARD 0\n"\
		"#endif\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"#ifndef FXAA_FAST_PIXEL_OFFSET\n"\
		"        //\n"\
		"        // Used for GLSL 120 only.\n"\
		"        //\n"\
		"        // 1 = GL API supports fast pixel offsets\n"\
		"        // 0 = do not use fast pixel offsets\n"\
		"        //\n"\
		"        #ifdef GL_EXT_gpu_shader4\n"\
		"\n"\
		"#define FXAA_FAST_PIXEL_OFFSET 1\n"\
		"        #endif\n"\
		"        #ifdef GL_NV_gpu_shader5\n"\
		"                #define FXAA_FAST_PIXEL_OFFSET 1\n"\
		"        #endif\n"\
		"\n"\
		"        #ifdef GL_ARB_gpu_shader5\n"\
		"                #define FXAA_FAST_PIXEL_OFFSET 1\n"\
		"        #endif\n"\
		"        #ifndef FXAA_FAST_PIXEL_OFFSET\n"\
		"                #define FXAA_FAST_PIXEL_OFFSET 0\n"\
		"        #endif\n"\
		"#endif\n"\
		"\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"#ifndef FXAA_GATHER4_ALPHA\n"\
		"        //\n"\
		"        // 1 = API supports gather4 on alpha channel.\n"\
		"        // 0 = API does not support gather4 on alpha channel.\n"\
		"        //\n"\
		"        #if (FXAA_HLSL_5 == 1)\n"\
		"                #define FXAA_GATHER4_ALPHA 1\n"\
		"        #endif\n"\
		"        #ifdef GL_ARB_gpu_shader5\n"\
		"                #define FXAA_GATHER4_ALPHA 1\n"\
		"\n"\
		"        #endif\n"\
		"        #ifdef GL_NV_gpu_shader5\n"\
		"                #define FXAA_GATHER4_ALPHA 1\n"\
		"        #endif\n"\
		"        #ifndef FXAA_GATHER4_ALPHA\n"\
		"                #define FXAA_GATHER4_ALPHA 0\n"\
		"        #endif\n"\
		"#endif\n"\
		"\n"\
		"\n"\
		"\n"\
		"#ifndef FXAA_QUALITY__PRESET\n"\
		"        #define FXAA_QUALITY__PRESET 13\n"\
		"#endif\n"\
		"\n"\
		"\n"\
		"/*============================================================================\n"\
		"\n"\
		"                                                  FXAA QUALITY - PRESETS\n"\
		"\n"\
		"============================================================================*/\n"\
		"\n"\
		"/*============================================================================\n"\
		"                                         FXAA QUALITY - MEDIUM DITHER PRESETS\n"\
		"============================================================================*/\n"\
		"\n"\
		"#if (FXAA_QUALITY__PRESET == 10)\n"\
		"        #define FXAA_QUALITY__PS 3\n"\
		"        #define FXAA_QUALITY__P0 1.5\n"\
		"        #define FXAA_QUALITY__P1 3.0\n"\
		"        #define FXAA_QUALITY__P2 12.0\n"\
		"#endif\n"\
		"\n"\
		"#if (FXAA_QUALITY__PRESET == 11)\n"\
		"        #define FXAA_QUALITY__PS 4\n"\
		"        #define FXAA_QUALITY__P0 1.0\n"\
		"        #define FXAA_QUALITY__P1 1.5\n"\
		"        #define FXAA_QUALITY__P2 3.0\n"\
		"        #define FXAA_QUALITY__P3 12.0\n"\
		"#endif\n"\
		"\n"\
		"#if (FXAA_QUALITY__PRESET == 12)\n"\
		"        #define FXAA_QUALITY__PS 5\n"\
		"        #define FXAA_QUALITY__P0 1.0\n"\
		"        #define FXAA_QUALITY__P1 1.5\n"\
		"        #define FXAA_QUALITY__P2 2.0\n"\
		"        #define FXAA_QUALITY__P3 4.0\n"\
		"        #define FXAA_QUALITY__P4 12.0\n"\
		"#endif\n"\
		"\n"\
		"#if (FXAA_QUALITY__PRESET == 13)\n"\
		"        #define FXAA_QUALITY__PS 6\n"\
		"        #define FXAA_QUALITY__P0 1.0\n"\
		"        #define FXAA_QUALITY__P1 1.5\n"\
		"        #define FXAA_QUALITY__P2 2.0\n"\
		"        #define FXAA_QUALITY__P3 2.0\n"\
		"        #define FXAA_QUALITY__P4 4.0\n"\
		"        #define FXAA_QUALITY__P5 12.0\n"\
		"#endif\n"\
		"\n"\
		"#if (FXAA_QUALITY__PRESET == 14)\n"\
		"        #define FXAA_QUALITY__PS 7\n"\
		"        #define FXAA_QUALITY__P0 1.0\n"\
		"        #define FXAA_QUALITY__P1 1.5\n"\
		"        #define FXAA_QUALITY__P2 2.0\n"\
		"        #define FXAA_QUALITY__P3 2.0\n"\
		"        #define FXAA_QUALITY__P4 2.0\n"\
		"        #define FXAA_QUALITY__P5 4.0\n"\
		"        #define FXAA_QUALITY__P6 12.0\n"\
		"#endif\n"\
		"\n"\
		"#if (FXAA_QUALITY__PRESET == 15)\n"\
		"        #define FXAA_QUALITY__PS 8\n"\
		"        #define FXAA_QUALITY__P0 1.0\n"\
		"        #define FXAA_QUALITY__P1 1.5\n"\
		"        #define FXAA_QUALITY__P2 2.0\n"\
		"        #define FXAA_QUALITY__P3 2.0\n"\
		"        #define FXAA_QUALITY__P4 2.0\n"\
		"        #define FXAA_QUALITY__P5 2.0\n"\
		"        #define FXAA_QUALITY__P6 4.0\n"\
		"        #define FXAA_QUALITY__P7 12.0\n"\
		"#endif\n"\
		"\n"\
		"\n"\
		"\n"\
		"#if (FXAA_QUALITY__PRESET == 20)\n"\
		"        #define FXAA_QUALITY__PS 3\n"\
		"        #define FXAA_QUALITY__P0 1.5\n"\
		"        #define FXAA_QUALITY__P1 2.0\n"\
		"        #define FXAA_QUALITY__P2 8.0\n"\
		"#endif\n"\
		"\n"\
		"#if (FXAA_QUALITY__PRESET == 21)\n"\
		"\n"\
		"        #define FXAA_QUALITY__PS 4\n"\
		"        #define FXAA_QUALITY__P0 1.0\n"\
		"        #define FXAA_QUALITY__P1 1.5\n"\
		"        #define FXAA_QUALITY__P2 2.0\n"\
		"        #define FXAA_QUALITY__P3 8.0\n"\
		"#endif\n"\
		"\n"\
		"#if (FXAA_QUALITY__PRESET == 22)\n"\
		"        #define FXAA_QUALITY__PS 5\n"\
		"        #define FXAA_QUALITY__P0 1.0\n"\
		"        #define FXAA_QUALITY__P1 1.5\n"\
		"        #define FXAA_QUALITY__P2 2.0\n"\
		"        #define FXAA_QUALITY__P3 2.0\n"\
		"        #define FXAA_QUALITY__P4 8.0\n"\
		"#endif\n"\
		"\n"\
		"#if (FXAA_QUALITY__PRESET == 23)\n"\
		"        #define FXAA_QUALITY__PS 6\n"\
		"        #define FXAA_QUALITY__P0 1.0\n"\
		"        #define FXAA_QUALITY__P1 1.5\n"\
		"        #define FXAA_QUALITY__P2 2.0\n"\
		"        #define FXAA_QUALITY__P3 2.0\n"\
		"        #define FXAA_QUALITY__P4 2.0\n"\
		"        #define FXAA_QUALITY__P5 8.0\n"\
		"#endif\n"\
		"\n"\
		"#if (FXAA_QUALITY__PRESET == 24)\n"\
		"        #define FXAA_QUALITY__PS 7\n"\
		"        #define FXAA_QUALITY__P0 1.0\n"\
		"        #define FXAA_QUALITY__P1 1.5\n"\
		"        #define FXAA_QUALITY__P2 2.0\n"\
		"        #define FXAA_QUALITY__P3 2.0\n"\
		"        #define FXAA_QUALITY__P4 2.0\n"\
		"        #define FXAA_QUALITY__P5 3.0\n"\
		"        #define FXAA_QUALITY__P6 8.0\n"\
		"#endif\n"\
		"\n"\
		"#if (FXAA_QUALITY__PRESET == 25)\n"\
		"        #define FXAA_QUALITY__PS 8\n"\
		"        #define FXAA_QUALITY__P0 1.0\n"\
		"        #define FXAA_QUALITY__P1 1.5\n"\
		"        #define FXAA_QUALITY__P2 2.0\n"\
		"        #define FXAA_QUALITY__P3 2.0\n"\
		"        #define FXAA_QUALITY__P4 2.0\n"\
		"        #define FXAA_QUALITY__P5 2.0\n"\
		"        #define FXAA_QUALITY__P6 4.0\n"\
		"        #define FXAA_QUALITY__P7 8.0\n"\
		"#endif\n"\
		"\n"\
		"#if (FXAA_QUALITY__PRESET == 26)\n"\
		"        #define FXAA_QUALITY__PS 9\n"\
		"        #define FXAA_QUALITY__P0 1.0\n"\
		"        #define FXAA_QUALITY__P1 1.5\n"\
		"        #define FXAA_QUALITY__P2 2.0\n"\
		"        #define FXAA_QUALITY__P3 2.0\n"\
		"        #define FXAA_QUALITY__P4 2.0\n"\
		"        #define FXAA_QUALITY__P5 2.0\n"\
		"        #define FXAA_QUALITY__P6 2.0\n"\
		"        #define FXAA_QUALITY__P7 4.0\n"\
		"        #define FXAA_QUALITY__P8 8.0\n"\
		"#endif\n"\
		"\n"\
		"#if (FXAA_QUALITY__PRESET == 27)\n"\
		"        #define FXAA_QUALITY__PS 10\n"\
		"        #define FXAA_QUALITY__P0 1.0\n"\
		"        #define FXAA_QUALITY__P1 1.5\n"\
		"        #define FXAA_QUALITY__P2 2.0\n"\
		"        #define FXAA_QUALITY__P3 2.0\n"\
		"        #define FXAA_QUALITY__P4 2.0\n"\
		"        #define FXAA_QUALITY__P5 2.0\n"\
		"        #define FXAA_QUALITY__P6 2.0\n"\
		"        #define FXAA_QUALITY__P7 2.0\n"\
		"        #define FXAA_QUALITY__P8 4.0\n"\
		"        #define FXAA_QUALITY__P9 8.0\n"\
		"#endif\n"\
		"\n"\
		"#if (FXAA_QUALITY__PRESET == 28)\n"\
		"        #define FXAA_QUALITY__PS 11\n"\
		"        #define FXAA_QUALITY__P0 1.0\n"\
		"        #define FXAA_QUALITY__P1 1.5\n"\
		"        #define FXAA_QUALITY__P2 2.0\n"\
		"        #define FXAA_QUALITY__P3 2.0\n"\
		"        #define FXAA_QUALITY__P4 2.0\n"\
		"        #define FXAA_QUALITY__P5 2.0\n"\
		"        #define FXAA_QUALITY__P6 2.0\n"\
		"        #define FXAA_QUALITY__P7 2.0\n"\
		"        #define FXAA_QUALITY__P8 2.0\n"\
		"        #define FXAA_QUALITY__P9 4.0\n"\
		"        #define FXAA_QUALITY__P10 8.0\n"\
		"#endif\n"\
		"\n"\
		"#if (FXAA_QUALITY__PRESET == 29)\n"\
		"        #define FXAA_QUALITY__PS 12\n"\
		"        #define FXAA_QUALITY__P0 1.0\n"\
		"        #define FXAA_QUALITY__P1 1.5\n"\
		"        #define FXAA_QUALITY__P2 2.0\n"\
		"        #define FXAA_QUALITY__P3 2.0\n"\
		"        #define FXAA_QUALITY__P4 2.0\n"\
		"        #define FXAA_QUALITY__P5 2.0\n"\
		"        #define FXAA_QUALITY__P6 2.0\n"\
		"        #define FXAA_QUALITY__P7 2.0\n"\
		"        #define FXAA_QUALITY__P8 2.0\n"\
		"        #define FXAA_QUALITY__P9 2.0\n"\
		"        #define FXAA_QUALITY__P10 4.0\n"\
		"        #define FXAA_QUALITY__P11 8.0\n"\
		"#endif\n"\
		"\n"\
		"#if (FXAA_QUALITY__PRESET == 39)\n"\
		"        #define FXAA_QUALITY__PS 12\n"\
		"        #define FXAA_QUALITY__P0 1.0\n"\
		"        #define FXAA_QUALITY__P1 1.0\n"\
		"        #define FXAA_QUALITY__P2 1.0\n"\
		"        #define FXAA_QUALITY__P3 1.0\n"\
		"        #define FXAA_QUALITY__P4 1.0\n"\
		"        #define FXAA_QUALITY__P5 1.5\n"\
		"        #define FXAA_QUALITY__P6 2.0\n"\
		"        #define FXAA_QUALITY__P7 2.0\n"\
		"        #define FXAA_QUALITY__P8 2.0\n"\
		"        #define FXAA_QUALITY__P9 2.0\n"\
		"        #define FXAA_QUALITY__P10 4.0\n"\
		"        #define FXAA_QUALITY__P11 8.0\n"\
		"#endif\n"\
		"\n"\
		"\n"\
		"\n"\
		"/*============================================================================\n"\
		"\n"\
		"                                                                GLSL DEFINES\n"\
		"\n"\
		"============================================================================*/\n"\
		"\n"\
		"        #define FxaaSat(x) clamp(x, 0.0, 1.0)\n"\
		"        #define LumaHack(x) vec4(vec3(x),dot(vec3(x), vec3(0.299, 0.587, 0.114)))\n"\
		"		#define FxaaTexTop(t, p) LumaHack(texture2DLod(t, p, 0.0))\n"\
		"        \n"\
		"        \n"\
		"        #if (FXAA_FAST_PIXEL_OFFSET == 1)\n"\
		"                #define FxaaTexOff(t, p, o, r) texture2DLodOffset(t, p, 0.0, o)\n"\
		"        #else\n"\
		"                #define FxaaTexOff(t, p, o, r) texture2DLod(t, p + (o * r), 0.0)\n"\
		"        #endif\n"\
		"   \n"\
		"\n"\
		"   #if (FXAA_GATHER4_ALPHA == 1)\n"\
		"                // use #extension GL_ARB_gpu_shader5 : enable\n"\
		"                #define FxaaTexAlpha4(t, p) textureGather(t, p, 3)\n"\
		"                #define FxaaTexOffAlpha4(t, p, o) textureGatherOffset(t, p, o, 3)\n"\
		"                #define FxaaTexGreen4(t, p) textureGather(t, p, 1)\n"\
		"                #define FxaaTexOffGreen4(t, p, o) textureGatherOffset(t, p, o, 1)\n"\
		"        #endif\n"\
		"\n"\
		"\n"\
		"\n"\
		"\n"\
		"\n"\
		"\n"\
		"\n"\
		"\n"\
		"/*============================================================================\n"\
		"                                   GREEN AS LUMA OPTION SUPPORT FUNCTION\n"\
		"============================================================================*/\n"\
		"#if (FXAA_GREEN_AS_LUMA == 0)\n"\
		"        float FxaaLuma(vec4 rgba) { return rgba.w; }\n"\
		"#else\n"\
		"        float FxaaLuma(vec4 rgba) { return rgba.y; }\n"\
		"#endif  \n"\
		"\n"\
		"\n"\
		"\n"\
		"\n"\
		"/*============================================================================\n"\
		"\n"\
		"                                                        FXAA3 QUALITY - PC\n"\
		"\n"\
		"============================================================================*/\n"\
		"\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"vec4 FxaaPixelShader(vec2 pos, vec4 fxaaConsolePosPos, sampler2D tex, sampler2D fxaaConsole360TexExpBiasNegOne, sampler2D fxaaConsole360TexExpBiasNegTwo, vec2 fxaaQualityRcpFrame, vec4 fxaaConsoleRcpFrameOpt, vec4 fxaaConsoleRcpFrameOpt2, vec4 fxaaConsole360RcpFrameOpt2, float fxaaQualitySubpix, float fxaaQualityEdgeThreshold, float fxaaQualityEdgeThresholdMin, float fxaaConsoleEdgeSharpness, float fxaaConsoleEdgeThreshold, float fxaaConsoleEdgeThresholdMin, vec4 fxaaConsole360ConstDir) \n"\
		"{\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"        vec2 posM;\n"\
		"        posM.x = pos.x;\n"\
		"        posM.y = pos.y;\n"\
		"        #if (FXAA_GATHER4_ALPHA == 1)\n"\
		"                #if (FXAA_DISCARD == 0)\n"\
		"                        vec4 rgbyM = FxaaTexTop(tex, posM);\n"\
		"                        #if (FXAA_GREEN_AS_LUMA == 0)\n"\
		"                                #define lumaM rgbyM.w\n"\
		"                        #else\n"\
		"                                #define lumaM rgbyM.y\n"\
		"                        #endif\n"\
		"                #endif\n"\
		"                #if (FXAA_GREEN_AS_LUMA == 0)\n"\
		"                        vec4 luma4A = FxaaTexAlpha4(tex, posM);\n"\
		"                        vec4 luma4B = FxaaTexOffAlpha4(tex, posM, vec2(-1.0, -1.0));\n"\
		"                #else\n"\
		"                        vec4 luma4A = FxaaTexGreen4(tex, posM);\n"\
		"                        vec4 luma4B = FxaaTexOffGreen4(tex, posM, vec2(-1.0, -1.0));\n"\
		"                #endif\n"\
		"                #if (FXAA_DISCARD == 1)\n"\
		"                        #define lumaM luma4A.w\n"\
		"                #endif\n"\
		"                #define lumaE luma4A.z\n"\
		"                #define lumaS luma4A.x\n"\
		"                #define lumaSE luma4A.y\n"\
		"                #define lumaNW luma4B.w\n"\
		"                #define lumaN luma4B.z\n"\
		"                #define lumaW luma4B.x\n"\
		"        #else\n"\
		"                vec4 rgbyM = FxaaTexTop(tex, posM);\n"\
		"                #if (FXAA_GREEN_AS_LUMA == 0)\n"\
		"                        #define lumaM rgbyM.w\n"\
		"                #else\n"\
		"                        #define lumaM rgbyM.y\n"\
		"                #endif\n"\
		"                float lumaS = FxaaLuma(FxaaTexOff(tex, posM, vec2( 0.0, 1.0), fxaaQualityRcpFrame.xy));\n"\
		"                float lumaE = FxaaLuma(FxaaTexOff(tex, posM, vec2( 1.0, 0.0), fxaaQualityRcpFrame.xy));\n"\
		"                float lumaN = FxaaLuma(FxaaTexOff(tex, posM, vec2( 0.0,-1.0), fxaaQualityRcpFrame.xy));\n"\
		"                float lumaW = FxaaLuma(FxaaTexOff(tex, posM, vec2(-1.0, 0.0), fxaaQualityRcpFrame.xy));\n"\
		"        #endif\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"        float maxSM = max(lumaS, lumaM);\n"\
		"        float minSM = min(lumaS, lumaM);\n"\
		"        float maxESM = max(lumaE, maxSM);\n"\
		"        float minESM = min(lumaE, minSM);\n"\
		"        float maxWN = max(lumaN, lumaW);\n"\
		"        float minWN = min(lumaN, lumaW);\n"\
		"        float rangeMax = max(maxWN, maxESM);\n"\
		"        float rangeMin = min(minWN, minESM);\n"\
		"        float rangeMaxScaled = rangeMax * fxaaQualityEdgeThreshold;\n"\
		"        float range = rangeMax - rangeMin;\n"\
		"        float rangeMaxClamped = max(fxaaQualityEdgeThresholdMin, rangeMaxScaled);\n"\
		"        bool earlyExit = range < rangeMaxClamped;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"        if(earlyExit)\n"\
		"                #if (FXAA_DISCARD == 1)\n"\
		"                        discard;\n"\
		"                #else\n"\
		"                        return rgbyM;\n"\
		"                #endif\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"        #if (FXAA_GATHER4_ALPHA == 0)\n"\
		"                float lumaNW = FxaaLuma(FxaaTexOff(tex, posM, vec2(-1.0,-1.0), fxaaQualityRcpFrame.xy));\n"\
		"                float lumaSE = FxaaLuma(FxaaTexOff(tex, posM, vec2( 1.0, 1.0), fxaaQualityRcpFrame.xy));\n"\
		"                float lumaNE = FxaaLuma(FxaaTexOff(tex, posM, vec2( 1.0,-1.0), fxaaQualityRcpFrame.xy));\n"\
		"                float lumaSW = FxaaLuma(FxaaTexOff(tex, posM, vec2(-1.0, 1.0), fxaaQualityRcpFrame.xy));\n"\
		"        #else\n"\
		"                float lumaNE = FxaaLuma(FxaaTexOff(tex, posM, vec2(1.0, -1.0), fxaaQualityRcpFrame.xy));\n"\
		"                float lumaSW = FxaaLuma(FxaaTexOff(tex, posM, vec2(-1.0, 1.0), fxaaQualityRcpFrame.xy));\n"\
		"        #endif\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"        float lumaNS = lumaN + lumaS;\n"\
		"        float lumaWE = lumaW + lumaE;\n"\
		"        float subpixRcpRange = 1.0/range;\n"\
		"        float subpixNSWE = lumaNS + lumaWE;\n"\
		"        float edgeHorz1 = (-2.0 * lumaM) + lumaNS;\n"\
		"        float edgeVert1 = (-2.0 * lumaM) + lumaWE;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"        float lumaNESE = lumaNE + lumaSE;\n"\
		"        float lumaNWNE = lumaNW + lumaNE;\n"\
		"        float edgeHorz2 = (-2.0 * lumaE) + lumaNESE;\n"\
		"        float edgeVert2 = (-2.0 * lumaN) + lumaNWNE;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"        float lumaNWSW = lumaNW + lumaSW;\n"\
		"        float lumaSWSE = lumaSW + lumaSE;\n"\
		"        float edgeHorz4 = (abs(edgeHorz1) * 2.0) + abs(edgeHorz2);\n"\
		"        float edgeVert4 = (abs(edgeVert1) * 2.0) + abs(edgeVert2);\n"\
		"        float edgeHorz3 = (-2.0 * lumaW) + lumaNWSW;\n"\
		"        float edgeVert3 = (-2.0 * lumaS) + lumaSWSE;\n"\
		"        float edgeHorz = abs(edgeHorz3) + edgeHorz4;\n"\
		"        float edgeVert = abs(edgeVert3) + edgeVert4;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"        float subpixNWSWNESE = lumaNWSW + lumaNESE;\n"\
		"        float lengthSign = fxaaQualityRcpFrame.x;\n"\
		"        bool horzSpan = edgeHorz >= edgeVert;\n"\
		"        float subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"        if(!horzSpan) lumaN = lumaW;\n"\
		"        if(!horzSpan) lumaS = lumaE;\n"\
		"        if(horzSpan) lengthSign = fxaaQualityRcpFrame.y;\n"\
		"        float subpixB = (subpixA * (1.0/12.0)) - lumaM;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"        float gradientN = lumaN - lumaM;\n"\
		"        float gradientS = lumaS - lumaM;\n"\
		"        float lumaNN = lumaN + lumaM;\n"\
		"        float lumaSS = lumaS + lumaM;\n"\
		"        bool pairN = abs(gradientN) >= abs(gradientS);\n"\
		"        float gradient = max(abs(gradientN), abs(gradientS));\n"\
		"        if(pairN) lengthSign = -lengthSign;\n"\
		"        float subpixC = FxaaSat(abs(subpixB) * subpixRcpRange);\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"        vec2 posB;\n"\
		"        posB.x = posM.x;\n"\
		"        posB.y = posM.y;\n"\
		"        vec2 offNP;\n"\
		"        offNP.x = (!horzSpan) ? 0.0 : fxaaQualityRcpFrame.x;\n"\
		"        offNP.y = ( horzSpan) ? 0.0 : fxaaQualityRcpFrame.y;\n"\
		"        if(!horzSpan) posB.x += lengthSign * 0.5;\n"\
		"        if( horzSpan) posB.y += lengthSign * 0.5;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"        vec2 posN;\n"\
		"        posN.x = posB.x - offNP.x * FXAA_QUALITY__P0;\n"\
		"        posN.y = posB.y - offNP.y * FXAA_QUALITY__P0;\n"\
		"        vec2 posP;\n"\
		"        posP.x = posB.x + offNP.x * FXAA_QUALITY__P0;\n"\
		"        posP.y = posB.y + offNP.y * FXAA_QUALITY__P0;\n"\
		"        float subpixD = ((-2.0)*subpixC) + 3.0;\n"\
		"        float lumaEndN = FxaaLuma(FxaaTexTop(tex, posN));\n"\
		"        float subpixE = subpixC * subpixC;\n"\
		"        float lumaEndP = FxaaLuma(FxaaTexTop(tex, posP));\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"        if(!pairN) lumaNN = lumaSS;\n"\
		"        float gradientScaled = gradient * 1.0/4.0;\n"\
		"        float lumaMM = lumaM - lumaNN * 0.5;\n"\
		"        float subpixF = subpixD * subpixE;\n"\
		"        bool lumaMLTZero = lumaMM < 0.0;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"        lumaEndN -= lumaNN * 0.5;\n"\
		"        lumaEndP -= lumaNN * 0.5;\n"\
		"        bool doneN = abs(lumaEndN) >= gradientScaled;\n"\
		"        bool doneP = abs(lumaEndP) >= gradientScaled;\n"\
		"        if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P1;\n"\
		"        if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P1;\n"\
		"        bool doneNP = (!doneN) || (!doneP);\n"\
		"        if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P1;\n"\
		"       if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P1;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"        if(doneNP) {\n"\
		"                if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"\
		"                if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"\
		"                if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"\
		"                if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"\
		"                doneN = abs(lumaEndN) >= gradientScaled;\n"\
		"                doneP = abs(lumaEndP) >= gradientScaled;\n"\
		"                if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P2;\n"\
		"                if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P2;\n"\
		"                doneNP = (!doneN) || (!doneP);\n"\
		"                if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P2;\n"\
		"                if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P2;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"                #if (FXAA_QUALITY__PS > 3)\n"\
		"                if(doneNP) {\n"\
		"                        if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"\
		"                        if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"\
		"                        if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"\
		"                        if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"\
		"                        doneN = abs(lumaEndN) >= gradientScaled;\n"\
		"                        doneP = abs(lumaEndP) >= gradientScaled;\n"\
		"                        if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P3;\n"\
		"                        if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P3;\n"\
		"                        doneNP = (!doneN) || (!doneP);\n"\
		"                        if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P3;\n"\
		"                        if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P3;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"                        #if (FXAA_QUALITY__PS > 4)\n"\
		"                        if(doneNP) {\n"\
		"                                if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"\
		"                                if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"\
		"                                if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"\
		"                                if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"\
		"                                doneN = abs(lumaEndN) >= gradientScaled;\n"\
		"                                doneP = abs(lumaEndP) >= gradientScaled;\n"\
		"                                if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P4;\n"\
		"                                if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P4;\n"\
		"                                doneNP = (!doneN) || (!doneP);\n"\
		"                                if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P4;\n"\
		"                                if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P4;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"                                #if (FXAA_QUALITY__PS > 5)\n"\
		"                                if(doneNP) {\n"\
		"                                        if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"\
		"                                        if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"\
		"                                        if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"\
		"                                        if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"\
		"                                        doneN = abs(lumaEndN) >= gradientScaled;\n"\
		"                                        doneP = abs(lumaEndP) >= gradientScaled;\n"\
		"                                        if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P5;\n"\
		"                                        if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P5;\n"\
		"                                        doneNP = (!doneN) || (!doneP);\n"\
		"                                        if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P5;\n"\
		"                                        if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P5;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"                                        #if (FXAA_QUALITY__PS > 6)\n"\
		"                                        if(doneNP) {\n"\
		"                                                if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"\
		"                                                if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"\
		"                                                if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"\
		"                                                if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"\
		"                                                doneN = abs(lumaEndN) >= gradientScaled;\n"\
		"                                                doneP = abs(lumaEndP) >= gradientScaled;\n"\
		"                                                if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P6;\n"\
		"                                                if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P6;\n"\
		"                                                doneNP = (!doneN) || (!doneP);\n"\
		"                                                if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P6;\n"\
		"                                                if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P6;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"                                                #if (FXAA_QUALITY__PS > 7)\n"\
		"                                                if(doneNP) {\n"\
		"                                                        if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"\
		"                                                        if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"\
		"                                                        if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"\
		"                                                        if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"\
		"                                                        doneN = abs(lumaEndN) >= gradientScaled;\n"\
		"                                                        doneP = abs(lumaEndP) >= gradientScaled;\n"\
		"                                                        if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P7;\n"\
		"                                                        if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P7;\n"\
		"                                                        doneNP = (!doneN) || (!doneP);\n"\
		"                                                        if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P7;\n"\
		"                                                        if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P7;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"        #if (FXAA_QUALITY__PS > 8)\n"\
		"        if(doneNP) {\n"\
		"                if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"\
		"                if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"\
		"                if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"\
		"                if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"\
		"                doneN = abs(lumaEndN) >= gradientScaled;\n"\
		"                doneP = abs(lumaEndP) >= gradientScaled;\n"\
		"                if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P8;\n"\
		"                if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P8;\n"\
		"                doneNP = (!doneN) || (!doneP);\n"\
		"                if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P8;\n"\
		"                if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P8;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"                #if (FXAA_QUALITY__PS > 9)\n"\
		"                if(doneNP) {\n"\
		"                        if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"\
		"                        if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"\
		"                        if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"\
		"                        if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"\
		"                        doneN = abs(lumaEndN) >= gradientScaled;\n"\
		"                        doneP = abs(lumaEndP) >= gradientScaled;\n"\
		"                        if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P9;\n"\
		"                        if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P9;\n"\
		"                        doneNP = (!doneN) || (!doneP);\n"\
		"                        if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P9;\n"\
		"                        if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P9;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"                        #if (FXAA_QUALITY__PS > 10)\n"\
		"                        if(doneNP) {\n"\
		"                                if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"\
		"                                if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"\
		"                                if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"\
		"                                if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"\
		"                                doneN = abs(lumaEndN) >= gradientScaled;\n"\
		"                                doneP = abs(lumaEndP) >= gradientScaled;\n"\
		"                                if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P10;\n"\
		"                                if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P10;\n"\
		"                                doneNP = (!doneN) || (!doneP);\n"\
		"                                if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P10;\n"\
		"                                if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P10;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"                                #if (FXAA_QUALITY__PS > 11)\n"\
		"                                if(doneNP) {\n"\
		"                                        if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"\
		"                                        if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"\
		"                                        if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"\
		"                                        if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"\
		"                                        doneN = abs(lumaEndN) >= gradientScaled;\n"\
		"                                        doneP = abs(lumaEndP) >= gradientScaled;\n"\
		"                                        if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P11;\n"\
		"                                        if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P11;\n"\
		"                                        doneNP = (!doneN) || (!doneP);\n"\
		"                                        if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P11;\n"\
		"                                        if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P11;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"                                        #if (FXAA_QUALITY__PS > 12)\n"\
		"                                        if(doneNP) {\n"\
		"                                                if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"\
		"                                                if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"\
		"                                                if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"\
		"                                                if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"\
		"                                                doneN = abs(lumaEndN) >= gradientScaled;\n"\
		"                                                doneP = abs(lumaEndP) >= gradientScaled;\n"\
		"                                                if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P12;\n"\
		"                                                if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P12;\n"\
		"                                                doneNP = (!doneN) || (!doneP);\n"\
		"                                                if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P12;\n"\
		"                                                if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P12;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"                                        }\n"\
		"                                        #endif\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"                                }\n"\
		"                                #endif\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"                        }\n"\
		"                        #endif\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"                }\n"\
		"                #endif\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"        }\n"\
		"        #endif\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"                                                }\n"\
		"                                                #endif\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"                                        }\n"\
		"                                        #endif\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"                                }\n"\
		"                                #endif\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"                        }\n"\
		"                        #endif\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"                }\n"\
		"                #endif\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"        }\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"  \n"\
		"        float dstN = posM.x - posN.x;\n"\
		"        float dstP = posP.x - posM.x;\n"\
		"        if(!horzSpan) dstN = posM.y - posN.y;\n"\
		"        if(!horzSpan) dstP = posP.y - posM.y;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"        bool goodSpanN = (lumaEndN < 0.0) != lumaMLTZero;\n"\
		"        float spanLength = (dstP + dstN);\n"\
		"        bool goodSpanP = (lumaEndP < 0.0) != lumaMLTZero;\n"\
		"        float spanLengthRcp = 1.0/spanLength;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"        bool directionN = dstN < dstP;\n"\
		"        float dst = min(dstN, dstP);\n"\
		"        bool goodSpan = directionN ? goodSpanN : goodSpanP;\n"\
		"        float subpixG = subpixF * subpixF;\n"\
		"        float pixelOffset = (dst * (-spanLengthRcp)) + 0.5;\n"\
		"        float subpixH = subpixG * fxaaQualitySubpix;\n"\
		"/*--------------------------------------------------------------------------*/\n"\
		"        float pixelOffsetGood = goodSpan ? pixelOffset : 0.0;\n"\
		"        float pixelOffsetSubpix = max(pixelOffsetGood, subpixH);\n"\
		"        if(!horzSpan) posM.x += pixelOffsetSubpix * lengthSign;\n"\
		"        if( horzSpan) posM.y += pixelOffsetSubpix * lengthSign;\n"\
		"        #if (FXAA_DISCARD == 1)\n"\
		"                return FxaaTexTop(tex, posM);\n"\
		"        #else\n"\
		"                return vec4(FxaaTexTop(tex, posM).xyz, lumaM);\n"\
		"        #endif\n"\
		"}\n"\
		"/*==========================================================================*/\n"\
		"\n"\
		"void main( void )\n"\
		"{\n"\
		"        \n"\
		"        vec2 rcpFrame; \n"\
		"        \n"\
		"        rcpFrame.x = 1.0 / buffersize.x;\n"\
		"        rcpFrame.y = 1.0 / buffersize.y;\n"\
		" \n"\
		"        vec2 pos;\n"\
		"        pos.xy= gl_FragCoord.xy / buffersize.xy;\n"\
		"\n"\
		"        vec4 ConsolePosPos = vec4(0.0,0.0,0.0,0.0);\n"\
		"        vec4 ConsoleRcpFrameOpt = vec4(0.0,0.0,0.0,0.0);\n"\
		"        vec4 ConsoleRcpFrameOpt2 = vec4(0.0,0.0,0.0,0.0);\n"\
		"        vec4 Console360RcpFrameOpt2 = vec4(0.0,0.0,0.0,0.0);\n"\
		"\n"\
		"        \n"\
		"        // Only used on FXAA Quality.\n"\
		"        // Choose the amount of sub-pixel aliasing removal.\n"\
		"        // This can effect sharpness.\n"\
		"        //   1.00 - upper limit (softer)\n"\
		"        //   0.75 - default amount of filtering\n"\
		"        //   0.50 - lower limit (sharper, less sub-pixel aliasing removal)\n"\
		"        //   0.25 - almost off\n"\
		"        //   0.00 - completely off\n"\
		"        float QualitySubpix = 1.0;\n"\
		"        \n"\
		"        // The minimum amount of local contrast required to apply algorithm.\n"\
		"        //   0.333 - too little (faster)\n"\
		"        //   0.250 - low quality\n"\
		"        //   0.166 - default\n"\
		"        //   0.125 - high quality \n"\
		"        //   0.033 - very high quality (slower)\n"\
		"        float QualityEdgeThreshold = 0.033;\n"\
		"        \n"\
		"        // You dont need to touch theses variables it have no visible effect\n"\
		"        float QualityEdgeThresholdMin = 0.0;\n"\
		"        float ConsoleEdgeSharpness = 8.0;\n"\
		"        float ConsoleEdgeThreshold = 0.125;\n"\
		"        float ConsoleEdgeThresholdMin = 0.05;\n"\
		"        vec4  Console360ConstDir = vec4(1.0, -1.0, 0.25, -0.25);\n"\
		"  \n"\
		"        gl_FragColor=FxaaPixelShader(pos, ConsolePosPos, COLOR, COLOR, COLOR, rcpFrame, ConsoleRcpFrameOpt, ConsoleRcpFrameOpt2, Console360RcpFrameOpt2, QualitySubpix, QualityEdgeThreshold, QualityEdgeThresholdMin, ConsoleEdgeSharpness, ConsoleEdgeThreshold, ConsoleEdgeThresholdMin, Console360ConstDir);\n"\
		"}\n";
