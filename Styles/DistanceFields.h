#pragma once

#include "RenderStyle.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLTexture.h"
#include "GLBlaat/GLUtility.h"

#include "Rendering/ImageDataTexture3D.h"

#include <cassert>
#include <map>

namespace NQVTK
{
	namespace Styles
	{
		class DistanceFields : public NQVTK::RenderStyle
		{
		public:
			typedef NQVTK::RenderStyle Superclass;

			DistanceFields()
			{ 
				distanceFieldId = GLTextureManager::BAD_SAMPLER_ID;

				// Set default parameters
				useDistanceColorMap = false;
				classificationThreshold = 1.05;
				useGridTexture = false;
				useGlyphTexture = false;
				useFatContours = false;
				contourDepthEpsilon = 0.001;
				useFog = true;
				depthCueRange = 10.0;
			}
			
			virtual ~DistanceFields() 
			{ 
				for (std::map<int, GLTexture*>::iterator it = distanceFields.begin();
					it != distanceFields.end(); ++it)
				{
					delete it->second;
				}
			}

			virtual GLFramebuffer *CreateFBO(int w, int h)
			{
				GLFramebuffer *fbo = GLFramebuffer::New(w, h);
				fbo->CreateDepthTextureRectangle();
				int nBufs = 3;
				GLenum bufs[] = {
					GL_COLOR_ATTACHMENT0_EXT, 
					GL_COLOR_ATTACHMENT1_EXT, 
					GL_COLOR_ATTACHMENT2_EXT
				};
				for (int i = 0; i < nBufs; ++i)
				{
					fbo->CreateColorTextureRectangle(bufs[i]);
					GLUtility::SetDefaultColorTextureParameters(
						fbo->GetTexture2D(bufs[i]));
				}
				glDrawBuffers(nBufs, bufs);
				if (!fbo->IsOk()) qDebug("WARNING! fbo not ok!");
				fbo->Unbind();

				return fbo;
			}

			virtual GLProgram *CreateScribe()
			{
				GLProgram *scribe = GLProgram::New();
				bool res = scribe->AddVertexShader(
					"#define NQVTK_USE_SHADOWMAP\n"
					"uniform float farPlane;"
					"uniform float nearPlane;"
					"uniform int objectId;"
					// For shadow mapping
					"\n#ifdef NQVTK_USE_SHADOWMAP\n"
					"uniform float shadowNearPlane;"
					"uniform float shadowFarPlane;"
					"uniform mat4 shadowMVM;"
					"uniform mat4 shadowPM;"
					"varying vec4 shadowCoord;"
					"varying float depthInShadow;"
					"\n#endif\n"
					// Varyings
					"varying vec4 vertex;"
					"varying vec3 normal;"
					"varying vec4 color;"
					"varying float depthInCamera;"
					// Shader main function
					"void main() {"
					// HACK: find a better way to pass these transforms
					"  vertex = gl_TextureMatrixInverse[1 - objectId] * gl_TextureMatrix[objectId] * gl_Vertex;"
					"  normal = normalize(gl_NormalMatrix * gl_Normal);"
					"  color = gl_Color;"
					"  vec4 pos = gl_ModelViewMatrix * gl_Vertex;"
					"  float depthRange = (farPlane - nearPlane);"
					"  depthInCamera = (-pos.z / pos.w - nearPlane) / depthRange;"
					"  gl_Position = gl_ProjectionMatrix * pos;"
					"  gl_TexCoord[0] = gl_MultiTexCoord0;"
					// Shadow mapping again
					"\n#ifdef NQVTK_USE_SHADOWMAP\n"
					"  shadowCoord = gl_TextureMatrix[2] * gl_ModelViewMatrix * gl_Vertex;"
					"  vec4 shadowPos = shadowMVM * gl_TextureMatrix[objectId] * gl_Vertex;"
					"  float shadowDepthRange = (shadowFarPlane - shadowNearPlane);"
					"  depthInShadow = (-shadowPos.z / shadowPos.w - shadowNearPlane) / shadowDepthRange;"
					"\n#endif\n"
					"}");
				if (res) res = scribe->AddFragmentShader(
					"#define NQVTK_USE_SHADOWMAP\n"
					"#extension GL_ARB_texture_rectangle : enable\n"
					"#extension GL_ARB_draw_buffers : enable\n"
					"#ifdef GL_EXT_gpu_shader4\n"
					"#extension GL_EXT_gpu_shader4 : enable\n"
					"#endif\n"
					// Shadow mapping
					"\n#ifdef NQVTK_USE_SHADOWMAP\n"
					"uniform sampler2D shadowMap;"
					"varying vec4 shadowCoord;"
					"varying float depthInShadow;"
					"\n#endif\n"
					"uniform sampler2DRectShadow depthBuffer;"
					"uniform sampler2DRect infoBuffer;"
					"uniform sampler3D distanceField;"
					"uniform bool hasDistanceField;"
					"uniform float distanceFieldDataShift;"
					"uniform float distanceFieldDataScale;"
					"uniform vec3 distanceFieldOrigin;"
					"uniform vec3 distanceFieldSize;"
					"uniform int layer;"
					"uniform int objectId;"
					// Parameters
					"uniform bool useDistanceColorMap;"
					"uniform float classificationThreshold;" // = 1.05
					"uniform bool useGridTexture;"
					"uniform bool useGlyphTexture;"
					// Varying
					"varying vec4 vertex;"
					"varying vec3 normal;"
					"varying vec4 color;"
					"varying float depthInCamera;"
					// Rounds a float to the nearest integer
					"\n#ifndef GL_EXT_gpu_shader4\n"
					"float round(float x) {"
					"  return floor(x + 0.5);"
					"}"
					"\n#endif\n"
					// Encodes a bit set in a float, range [0..1]
					"float setBit(float byte, int bit, bool on) {"
					"\n#ifdef GL_EXT_gpu_shader4\n"
					// - gpu-shader4, use bitwise operators
					"  float N = 2.0;"
					"  float max = pow(2.0, N) - 1.0;"
					"  int pattern = int(round(byte * max));"
					"  int mask = 1 << bit;"
					"  if (on) {"
					"    pattern = pattern | mask;"
					"  } else {"
					"    pattern = pattern & ~mask;"
					"  }"
					"  return float(pattern) / max;"
					"\n#else\n"
					// - no gpu-shader4, use float arith for bit masks
					"  float f = 2.0;"
					"  float N = 2.0;"
					"  float max = pow(f, N) - 1.0;"
					"  byte = round(byte * max);"
					"  float bf = pow(f, float(bit));"
					"  float b = fract(byte / bf);"
					"  float af = bf * f;"
					"  float a = floor(byte / af);"
					"  float r = bf * b + af * a;"
					"  if (on) r += f / 2.0 * bf;"
					"  return r / max;"
					"\n#endif\n"
					"}"
					// Gets a single bit from a float-encoded bit set
					"\nbool getBit(float byte, int bit) {"
					"\n#ifdef GL_EXT_gpu_shader4\n"
					// - gpu-shader4, use bitwise operators
					"  float N = 2.0;"
					"  float max = pow(2.0, N) - 1.0;"
					"  int pattern = int(round(byte * max));"
					"  int mask = 1 << bit;"
					"  return (pattern & mask) != 0;"
					"\n#else\n"
					// - no gpu-shader4, use float arith for bit masks
					"  float f = 2.0;"
					"  float N = 2.0;"
					"  if (bit > int(N)) return false;"
					"  float mask = round(byte * (pow(f, N) - 1.0)) / f;"
					"  int i;"
					"  for (i = 0; i <= bit - 1; ++i) {"
					"    mask = floor(mask) / f;"
					"  }"
					"  return (fract(mask) > 0.25);"
					"\n#endif\n"
					"}"
					// Packs a float in two 8 bit channels
					"vec2 encodeDepth(float depth) {"
					"  depth = clamp(depth, 0.0, 1.0);"
					"  vec2 factors = vec2(1.0, 256.0);"
					"  vec2 result = fract(depth * factors);"
					"  result.x -= result.y / factors.y;"
					"  return result;"
					"}"
					// Unpacks a float from two 8 bit channels
					"float decodeDepth(vec2 coded) {"
					"  vec2 factors = vec2(1.0, 0.00390625);"
					"  return dot(coded, factors);"
					"}"
					// Unpacks a float from three 8 bit channels
					"float decodeShadow(vec3 coded) {"
					"  vec3 factors = vec3(1.0, 0.00390625, 0.0000152587890625);"
					"  return dot(coded, factors);"
					"}"
					// Shader main function
					"void main() {"
					"  vec4 r0 = gl_FragCoord;"
					"  vec4 col = color;"
					// Depth peeling
					"  if (layer > 0) {"
					"    float r1 = shadow2DRect(depthBuffer, r0.xyz).x;"
					"    r1 = r1 - 0.5;"
					"    if (r1 < 0.0) { discard; }"
					"  }"
					// Get the previous info buffer
					"  vec4 prevInfo = vec4(0.0);"
					"  if (layer > 0) {"
					"    prevInfo = texture2DRect(infoBuffer, r0.xy);"
					"  }"
					// Coplanarity peeling
					"  if (getBit(prevInfo.y, objectId) == gl_FrontFacing) {"
					"    discard;"
					"  }"
					// Distance classification
					"  float classification = 0.0;"
					"  if (objectId >= 0) {"
					"    classification = 0.25 + float(objectId) * 0.25;"
					"  }"
					"  if (hasDistanceField) {"
					"    vec3 p = vertex.xyz / vertex.w;"
					// HACK: Beware! Hack! Distance field alignment is wrong!
					//"    p = p + vec3(-3.5, -4.0, 9.0);" // msdata
					//"    p = p + vec3(-28.0, 57.0, 13.0);" // cartilage
					//"    p = p + vec3(98.0, 98.0, 100.0);" // test
					"    p = (p - distanceFieldOrigin) / distanceFieldSize;"
					"    float dist = texture3D(distanceField, p).x;"
					"    dist = abs(dist * distanceFieldDataScale + distanceFieldDataShift);"
					//*
					"    if (useDistanceColorMap) {"
					"      float d = clamp(dist / 7.0, 0.0, 1.0);"
					"      if (objectId == 0) {"
					"        col = vec4(1.0, 1.0 - d, 1.0 - d, 1.0);"
					"      } else {"
					"        col = vec4(1.0 - d, 1.0 - d, 1.0, 1.0);"
					"      }"
					"    }"
					//*/
					// TEST: saturation color map
					//"    float d = clamp(dist / classificationThreshold, 0.0, 1.0);"
					//"    col = vec4(col.rgb * d + vec3(0.5) * (1.0 - d), col.a);"
					// Thresholding
					"    if (dist < classificationThreshold) {"
					"      classification = 0.0;"
					"      col = vec4(1.0);"
					"    }"
					"  }"
					/* TEST: texcoord-less xy grid
					"  if (useGridTexture && (col.a < 1.0 || !hasDistanceField)) {"
					"    vec2 tc = fract(abs(0.03 * vertex.xy / vertex.w));"
					"    float grid = abs(2.0 * mod(tc.x * 3.0, 1.0) - 1.0);"
					"    grid = 1.0 - min(grid, abs(2.0 * mod(tc.y * 5.0, 1.0) - 1.0));"
					"    col = vec4(col.rgb, col.a + 0.5 * pow(grid, 5.0));"
					"  }"
					//*/
					//*
					// TEST: grid texture
					"  if (useGridTexture && (col.a < 1.0 || !hasDistanceField)) {"
					"    vec2 tc = fract(abs(gl_TexCoord[0].xy));"
					"    float grid = abs(2.0 * mod(tc.x * 3.0, 1.0) - 1.0);"
					"    grid = 1.0 - min(grid, abs(2.0 * mod(tc.y * 5.0, 1.0) - 1.0));"
					"    col = vec4(col.rgb, col.a + 0.5 * pow(grid, 5.0));"
					"  }"
					//*/
					//*/
					// TEST: glyph texture
					"  if (useGlyphTexture && (col.a < 1.0 || !hasDistanceField)) {"
					"    vec2 tc = abs(2.0 * gl_TexCoord[0].xy - vec2(1.0));"
					"    if ((tc.x < 0.1 && tc.y < 0.9) || (tc.y < 0.1 && tc.x < 0.6)) {"
					"      col.a = min(col.a + 0.3, 1.0);"
					"    } else if ((tc.x < 0.15 && tc.y < 0.95) || (tc.y < 0.15 && tc.x < 0.65)) {"
					"      col.a = min(col.a + 0.15, 1.0);"
					"    }"
					"  }"
					//*/
					// Shadow mapping
					"\n#ifdef NQVTK_USE_SHADOWMAP\n"
					"  vec4 shadow = texture2DProj(shadowMap, shadowCoord);"
					"  if (shadow.a < 1.0 && depthInShadow > decodeShadow(shadow.xyz)) {"
					"    col *= vec4(0.5, 0.5, 0.5, 1.0);"
					"  }"
					"\n#endif\n"
					// Encode in-out mask
					"  float inOutMask = prevInfo.y;"
					"  if (objectId >= 0) {"
					"    inOutMask = setBit(inOutMask, objectId, gl_FrontFacing);"
					"  }"
					// Encode depth
					"  vec2 depthVec = encodeDepth(depthInCamera);"
					// Encode normal
					"  vec3 n = (normalize(normal) + vec3(1.0)) / 2.0;"
					// Store data
					"  gl_FragData[0] = col;"
					"  gl_FragData[1] = vec4(n, 1.0);"
					"  gl_FragData[2] = vec4(classification, inOutMask, depthVec);"
					"}");
				if (res) res = scribe->Link();
				qDebug(scribe->GetInfoLogs().c_str());
				if (!res)
				{
					delete scribe;
					return 0;
				}
				return scribe;
			}

			virtual GLProgram *CreatePainter()
			{
				GLProgram *painter = GLProgram::New();
				bool res = painter->AddVertexShader(
					"void main() {"
					"  gl_Position = gl_Vertex;"
					"}");
				if (res) res = painter->AddFragmentShader(
					"#extension GL_ARB_texture_rectangle : enable\n"
					"\n#ifdef GL_EXT_gpu_shader4\n"
					"#extension GL_EXT_gpu_shader4 : enable\n"
					"\n#endif\n"
					"uniform sampler2DRect normals;"
					"uniform sampler2DRect colors;"
					"uniform sampler2DRect infoPrevious;"
					"uniform sampler2DRect infoCurrent;"
					"uniform int layer;"
					"uniform float farPlane;"
					"uniform float nearPlane;"
					"uniform float viewportX;"
					"uniform float viewportY;"
					// Parameters
					"uniform bool useFatContours;"
					"uniform float contourDepthEpsilon;" // = 0.001
					"uniform bool useFog;"
					"uniform float depthCueRange;" // = 10.0
					"\n#ifndef GL_EXT_gpu_shader4\n"
					// Rounds a float to the nearest integer
					"float round(float x) {"
					"  return floor(x + 0.5);"
					"}"
					"\n#endif\n"
					// Unpacks a float from two 8 bit channels
					"float decodeDepth(vec2 coded) {"
					"  vec2 factors = vec2(1.0, 0.00390625);"
					"  return dot(coded, factors);"
					"}"
					// CSG formula
					"\n#ifdef GL_EXT_gpu_shader4\n"
					// - gpu-shader4, use bitwise operators
					"bool getBit(float byte, int bit) {"
					"  float N = 2.0;"
					"  float max = pow(2.0, N) - 1.0;"
					"  int pattern = int(round(byte * max));"
					"  int mask = 1 << bit;"
					"  return (pattern & mask) != 0;"
					"}"
					"bool CSG(float mask) {"
					"  return getBit(mask, 0) && getBit(mask, 1);"
					"}"
					"\n#else\n"
					// - no gpu-shader4, use float arith for bit masks
					"bool CSG(float mask) {"
					"  float f = 2.0;"
					"  float N = 2.0;"
					"  mask = round(mask * (pow(f, N) - 1.0)) / f;"
					"  bool inActor0 = fract(mask) > 0.25;"
					"  mask = floor(mask) / f;"
					"  bool inActor1 = fract(mask) > 0.25;"
					"  mask = floor(mask) / f;"
					"  bool inActor2 = fract(mask) > 0.25;"
					"  return inActor0 && inActor1;"
					"}"
					"\n#endif\n"
					// CSG formula for fogging volumes
					"bool CSGFog(float mask) {"
					"\n#ifdef GL_EXT_gpu_shader4\n"
					// - gpu-shader4, use bitwise operators
					"  return getBit(mask, 0) || getBit(mask, 1);"
					"\n#else\n"
					// - no gpu-shader4, use float arith for bit masks
					"  float f = 2.0;"
					"  float N = 2.0;"
					"  mask = round(mask * (pow(f, N) - 1.0)) / f;"
					"  bool inActor0 = fract(mask) > 0.25;"
					"  mask = floor(mask) / f;"
					"  bool inActor1 = fract(mask) > 0.25;"
					"  mask = floor(mask) / f;"
					"  bool inActor2 = fract(mask) > 0.25;"
					"  return inActor0 || inActor1;"
					"\n#endif\n"
					"}"
					// Phong shading helper
					"vec3 phongShading(vec3 matColor, vec3 normal) {"
					"  vec3 l = normalize(vec3(gl_LightSource[0].position));"
					"  vec3 n = normalize(normal);"
					"  vec3 h = normalize(gl_LightSource[0].halfVector.xyz);"
					"  float NdotL = dot(n, l);"
					"  float diffuse = 0.5 + 0.5 * min(max(NdotL, 0.0), 1.0);"
					"  float specular = 0.0;"
					"  if (NdotL >= 0.0) {"
					"    specular = pow(max(dot(n, h), 0.0), 64.0);"
					"  }"
					"  return diffuse * matColor + specular * vec3(0.4);"
					"}"
					// Main shader function
					"void main() {"
					"  vec4 r0 = gl_FragCoord;"
					"  r0.x -= viewportX;"
					"  r0.y -= viewportY;"
					"  vec3 normal = texture2DRect(normals, r0.xy).rgb;"
					"  normal = (2.0 * normal) - vec3(1.0);"
					"  vec4 color = texture2DRect(colors, r0.xy);"
					"  vec4 info0 = texture2DRect(infoCurrent, r0.xy);"
					"  vec4 info1 = texture2DRect(infoPrevious, r0.xy);"
					"  if (layer == 0) info1 = vec4(0.0);"
					// Apply lighting
					"  vec3 litFragment = phongShading(color.rgb, normal);"
					// Apply CSG
					"  float mask0 = info0.y;"
					"  float mask1 = info1.y;"
					"  if (CSG(mask0) != CSG(mask1)) {"
					//"    litFragment.rgb -= 0.3 * color.a;"// TEST: grid
					"    color.a = 1.0;"
					//"  } else {"
					//"    if (color.a > 0.0) color.a = 1.5 - length(litFragment);"
					"  }"
					// Apply contouring
					"  vec4 left   = texture2DRect(infoCurrent, vec2(r0.x - 1.0, r0.y));"
					"  vec4 right  = texture2DRect(infoCurrent, vec2(r0.x + 1.0, r0.y));"
					"  vec4 top    = texture2DRect(infoCurrent, vec2(r0.x, r0.y - 1.0));"
					"  vec4 bottom = texture2DRect(infoCurrent, vec2(r0.x, r0.y + 1.0));"
					"  float depth = decodeDepth(info0.zw);"
					"  float diffL = abs(decodeDepth(left.zw) - depth);"
					"  float diffR = abs(decodeDepth(right.zw) - depth);"
					"  float diffT = abs(decodeDepth(top.zw) - depth);"
					"  float diffB = abs(decodeDepth(bottom.zw) - depth);"
					"  bool contourL = (left.x != info0.x && diffL < contourDepthEpsilon);"
					"  bool contourR = (right.x != info0.x && diffR < contourDepthEpsilon);"
					"  bool contourT = (top.x != info0.x && diffT < contourDepthEpsilon);"
					"  bool contourB = (bottom.x != info0.x && diffB < contourDepthEpsilon);"
					"  if (contourL || contourT || (useFatContours && (contourR || contourB))) {"
					"    litFragment = vec3(0.0);"
					"    color.a = 1.0;"
					"  }"
					// Apply fogging
					"  if (useFog && CSGFog(mask1)) {"
					"\n#ifdef GL_EXT_gpu_shader4\n"
					"    vec3 fogColor = vec3(1.0, 0.0, 0.2);"
					// TEST: colored fog
					//"    vec3 fogColor = vec3(0.0);"
					//"    if (getBit(mask1, 0) && getBit(mask1, 1)) {"
					//"      fogColor = vec3(1.0, 1.0, 1.0);"
					//"    } else if (getBit(mask1, 0)) {"
					//"      fogColor = vec3(1.0, 1.0, 0.0);"
					//"    } else {"
					//"      fogColor = vec3(0.2, 0.0, 1.0);"
					//"    }"
					// END TEST
					"\n#else\n"
					"    vec3 fogColor = vec3(1.0, 0.0, 0.2);"
					"\n#endif\n"
					"    float depthRange = (farPlane - nearPlane);"
					"    float front = decodeDepth(info1.zw) * depthRange;"
					"    float back = decodeDepth(info0.zw) * depthRange;"
					"    float diff = back - front;"
					"    float fogAlpha = 1.0 - "
					"      clamp(exp(-diff / depthCueRange), 0.0, 1.0);"
					"    litFragment = fogColor * fogAlpha + "
					"      litFragment * color.a * (1.0 - fogAlpha);"
					"    color.a = fogAlpha + color.a * (1.0 - fogAlpha);"
					"    litFragment /= color.a;"
					"  }"
					// Pre-multiply colors by alpha
					"  litFragment *= color.a;"
					"  gl_FragColor = vec4(litFragment, color.a);"
					"}");
				if (res) res = painter->Link();
				qDebug(painter->GetInfoLogs().c_str());
				if (!res) 
				{
					delete painter;
					return 0;
				}
				return painter;
			}

			virtual void PrepareForObject(GLProgram *scribe, 
				int objectId, NQVTK::Renderable *renderable)
			{
				Superclass::PrepareForObject(scribe, objectId, renderable);
				
				ImageDataTexture3D *distanceField = 
					dynamic_cast<ImageDataTexture3D*>(GetDistanceField(objectId));
				if (distanceField)
				{
					// Pass distance field information
					scribe->SetUniform1i("hasDistanceField", 1);
					scribe->SetUniform1f("distanceFieldDataShift", 
						distanceField->GetDataShift());
					scribe->SetUniform1f("distanceFieldDataScale", 
						distanceField->GetDataScale());
					Vector3 origin = distanceField->GetOrigin();
					scribe->SetUniform3f("distanceFieldOrigin", 
						origin.x, origin.y, origin.z);
					Vector3 size = distanceField->GetOriginalSize();
					scribe->SetUniform3f("distanceFieldSize", 
						size.x, size.y, size.z);
					// Update distance field texture
					tm->SwapTexture(distanceFieldId, distanceField);
				}
				else
				{
					scribe->SetUniform1i("hasDistanceField", 0);
				}
				tm->Bind();
			}

			virtual void RegisterScribeTextures(GLFramebuffer *previous) 
			{
				// Get the previous layer's depth buffer
				GLTexture *depthBuffer = previous->GetTexture2D(GL_DEPTH_ATTACHMENT_EXT);
				assert(depthBuffer);
				GLUtility::SetDefaultDepthTextureParameters(depthBuffer);
				glTexParameteri(depthBuffer->GetTextureTarget(), 
					GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
				depthBuffer->UnbindCurrent();
				tm->AddTexture("depthBuffer", depthBuffer, false);

				// Get the previous layer's info buffer
				GLTexture *infoBuffer = previous->GetTexture2D(GL_COLOR_ATTACHMENT2_EXT);
				assert(infoBuffer);
				GLUtility::SetDefaultColorTextureParameters(infoBuffer);
				infoBuffer->UnbindCurrent();
				tm->AddTexture("infoBuffer", infoBuffer, false);

				if (distanceFieldId == GLTextureManager::BAD_SAMPLER_ID)
				{
					// Register distance field sampler
					// This is set to different textures in PrepareForObject
					distanceFieldId = tm->AddTexture("distanceField", 0, false);
				}
			}

			virtual void UnregisterScribeTextures() 
			{
				//tm->RemoveTexture("depthBuffer");
				//tm->RemoveTexture("infoBuffer");
				//tm->RemoveTexture("distanceField");
			}

			virtual void UpdateScribeParameters(GLProgram *scribe)
			{
				// Set program parameters
				scribe->SetUniform1i("useDistanceColorMap", useDistanceColorMap);
				scribe->SetUniform1f("classificationThreshold", classificationThreshold);
				scribe->SetUniform1i("useGridTexture", useGridTexture);
				scribe->SetUniform1i("useGlyphTexture", useGlyphTexture);
			}

			virtual void RegisterPainterTextures(GLFramebuffer *current, GLFramebuffer *previous) 
			{
				// Previous layer info buffer
				GLTexture *infoPrevious = previous->GetTexture2D(GL_COLOR_ATTACHMENT2_EXT);
				assert(infoPrevious);
				tm->AddTexture("infoPrevious", infoPrevious);
				// Current layer info buffer
				GLTexture *infoCurrent = current->GetTexture2D(GL_COLOR_ATTACHMENT2_EXT);
				assert(infoCurrent);
				tm->AddTexture("infoCurrent", infoCurrent);
				// Current layer colors
				GLTexture *colors = current->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT);
				assert(colors);
				tm->AddTexture("colors", colors);
				// Current layer normals
				GLTexture *normals = current->GetTexture2D(GL_COLOR_ATTACHMENT1_EXT);
				assert(normals);
				tm->AddTexture("normals", normals);
			}

			virtual void UnregisterPainterTextures() 
			{
				//tm->RemoveTexture("infoPrevious");
				//tm->RemoveTexture("infoCurrent");
				//tm->RemoveTexture("colors");
				//tm->RemoveTexture("normals");
			}

			virtual void UpdatePainterParameters(GLProgram *painter)
			{
				// Set program parameters
				painter->SetUniform1i("useFatContours", useFatContours);
				painter->SetUniform1f("contourDepthEpsilon", contourDepthEpsilon);
				painter->SetUniform1i("useFog", useFog);
				painter->SetUniform1f("depthCueRange", depthCueRange);
			}

			void SetDistanceField(int objectId, GLTexture *field)
			{
				// Replace the distance field for this object
				GLTexture *old = GetDistanceField(objectId);
				if (old) delete old;
				distanceFields[objectId] = field;
			}

			// Program parameters
			// - Scribe
			bool useDistanceColorMap;
			float classificationThreshold; // = 1.05
			bool useGridTexture;
			bool useGlyphTexture;
			// - Painter
			bool useFatContours;
			float contourDepthEpsilon; // = 0.001
			bool useFog;
			float depthCueRange; // = 10.0

		protected:
			// SamplerIds
			GLTextureManager::SamplerId distanceFieldId;

			// Distance fields
			std::map<int, GLTexture*> distanceFields;
			// Get the distance field to sample for the specified object
			GLTexture *GetDistanceField(int objectId)
			{
				// Look up the texture, return 0 if nothing's attached
				std::map<int, GLTexture*>::iterator it = distanceFields.find(objectId);
				if (it == distanceFields.end()) return 0;
				return it->second;
			}

		private:
			// Not implemented
			DistanceFields(const DistanceFields&);
			void operator=(const DistanceFields&);
		};
	}
}
