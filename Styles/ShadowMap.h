#pragma once

#include "RenderStyle.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLUtility.h"

#include <cassert>

namespace NQVTK
{
	namespace Styles
	{
		class ShadowMap : public NQVTK::RenderStyle
		{
		public:
			typedef NQVTK::RenderStyle Superclass;

			ShadowMap() 
			{
				useGridTexture = false;
				useGlyphTexture = false;
			}

			virtual ~ShadowMap() { }

			virtual GLFramebuffer *CreateFBO(int w, int h)
			{
				GLFramebuffer *fbo = GLFramebuffer::New(w, h);
				fbo->CreateDepthTextureRectangle();
				int nBufs = 1;
				GLenum bufs[] = {
					GL_COLOR_ATTACHMENT0_EXT
				};
				for (int i = 0; i < nBufs; ++i)
				{
					fbo->CreateColorTextureRectangle(
						bufs[i], GL_RGBA16F_ARB, GL_RGBA, GL_FLOAT);
					GLUtility::SetDefaultColorTextureParameters(
						fbo->GetTexture2D(bufs[i]));
				}
				glDrawBuffers(nBufs, bufs);
				if (!fbo->IsOk()) qDebug("WARNING! fbo not ok!");
				fbo->Unbind();

				return fbo;
			}

			virtual GLFramebuffer *CreateShadowBufferFBO(int w, int h)
			{
				GLFramebuffer *fbo = GLFramebuffer::New(w, h);
				// We only need a color texture to store the shadow map
				fbo->CreateColorTexture(
					GL_COLOR_ATTACHMENT0_EXT, 
					GL_RGBA16F_ARB, GL_RGBA, GL_FLOAT);
				GLTexture *tex = fbo->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT);
				// With variance shadow mapping we can linearly interpolate the texture
				tex->BindToCurrent();
				glTexParameteri(tex->GetTextureTarget(), 
					GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(tex->GetTextureTarget(), 
					GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				tex->UnbindCurrent();
				if (!fbo->IsOk()) qDebug("WARNING! shadow buffer fbo not ok!");
				fbo->Unbind();

				return fbo;
			}

			virtual GLProgram *CreateScribe()
			{
				GLProgram *scribe = GLProgram::New();
				bool res = scribe->AddVertexShader(
					"uniform float farPlane;"
					"uniform float nearPlane;"
					"varying vec3 normal;"
					"varying vec4 color;"
					"varying float depthInCamera;"
					// Shader main function
					"void main() {"
					"  normal = normalize(gl_NormalMatrix * gl_Normal);"
					"  color = gl_Color;"
					"  vec4 pos = gl_ModelViewMatrix * gl_Vertex;"
					"  float depthRange = (farPlane - nearPlane);"
					"  depthInCamera = (-pos.z / pos.w - nearPlane) / depthRange;"
					"  gl_Position = ftransform();"
					"  gl_TexCoord[0] = gl_MultiTexCoord0;"
					"}");
				if (res) res = scribe->AddFragmentShader(
					"#extension GL_ARB_texture_rectangle : enable\n"
					"#extension GL_ARB_draw_buffers : enable\n"
					"#ifdef GL_EXT_gpu_shader4\n"
					"#extension GL_EXT_gpu_shader4 : enable\n"
					"#endif\n"
					"uniform sampler2DRectShadow depthBuffer;"
					"uniform sampler2DRect infoBuffer;"
					"uniform int layer;"
					"uniform int objectId;"
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
					"  float N = 4.0;"
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
					"  float N = 4.0;"
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
					"  float N = 4.0;"
					"  float max = pow(2.0, N) - 1.0;"
					"  int pattern = int(round(byte * max));"
					"  int mask = 1 << bit;"
					"  return (pattern & mask) != 0;"
					"\n#else\n"
					// - no gpu-shader4, use float arith for bit masks
					"  float f = 2.0;"
					"  float N = 4.0;"
					"  if (bit > int(N)) return false;"
					"  float mask = round(byte * (pow(f, N) - 1.0)) / f;"
					"  int i;"
					"  for (i = 0; i <= bit - 1; ++i) {"
					"    mask = floor(mask) / f;"
					"  }"
					"  return (fract(mask) > 0.25);"
					"\n#endif\n"
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
					"  float inOutMask = prevInfo.x;"
					"  if (getBit(inOutMask, objectId) == gl_FrontFacing) {"
					"    discard;"
					"  }"
					// Encode in-out mask
					"  if (objectId >= 0) {"
					"    inOutMask = setBit(inOutMask, objectId, gl_FrontFacing);"
					"  }"
					// Encode depth
					//"  vec3 depthVec = encodeShadow(depthInCamera);"
					// Store data
					"  gl_FragData[0] = vec4(inOutMask, depthInCamera, gl_TexCoord[0].xy);"
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
					"uniform sampler2DRect infoPrevious;"
					"uniform sampler2DRect infoCurrent;"
					"uniform int layer;"
					"uniform float farPlane;"
					"uniform float nearPlane;"
					"uniform float viewportX;"
					"uniform float viewportY;"
					// Parameters
					"uniform bool useGridTexture;"
					"uniform bool useGlyphTexture;"
					// Rounds a float to the nearest integer
					"\n#ifndef GL_EXT_gpu_shader4\n"
					"float round(float x) {"
					"  return floor(x + 0.5);"
					"}"
					"\n#endif\n"
					// Gets a single bit from a float-encoded bit set
					"\nbool getBit(float byte, int bit) {"
					"\n#ifdef GL_EXT_gpu_shader4\n"
					// - gpu-shader4, use bitwise operators
					"  float N = 4.0;"
					"  float max = pow(2.0, N) - 1.0;"
					"  int pattern = int(round(byte * max));"
					"  int mask = 1 << bit;"
					"  return (pattern & mask) != 0;"
					"\n#else\n"
					// - no gpu-shader4, use float arith for bit masks
					"  float f = 2.0;"
					"  float N = 4.0;"
					"  if (bit > int(N)) return false;"
					"  float mask = round(byte * (pow(f, N) - 1.0)) / f;"
					"  int i;"
					"  for (i = 0; i <= bit - 1; ++i) {"
					"    mask = floor(mask) / f;"
					"  }"
					"  return (fract(mask) > 0.25);"
					"\n#endif\n"
					"}"
					// CSG formula
					"bool CSG(float mask) {"
					"\n#ifdef GL_EXT_gpu_shader4\n"
					// - gpu-shader4, use bitwise operators
					"  return getBit(mask, 0) && getBit(mask, 1);"
					"\n#else\n"
					// - no gpu-shader4, use float arith for bit masks
					"  float f = 2.0;"
					"  float N = 4.0;"
					"  mask = round(mask * (pow(f, N) - 1.0)) / f;"
					"  bool inActor0 = fract(mask) > 0.25;"
					"  mask = floor(mask) / f;"
					"  bool inActor1 = fract(mask) > 0.25;"
					"  mask = floor(mask) / f;"
					"  bool inActor2 = fract(mask) > 0.25;"
					"  return inActor0 && inActor1;"
					"\n#endif\n"
					"}"
					// Main shader function
					"void main() {"
					"  vec4 r0 = gl_FragCoord;"
					"  r0.x -= viewportX;"
					"  r0.y -= viewportY;"
					"  vec4 info0 = texture2DRect(infoCurrent, r0.xy);"
					"  vec4 info1 = texture2DRect(infoPrevious, r0.xy);"
					"  if (layer == 0) info1 = vec4(0.0);"
					// Apply CSG
					"  float mask0 = info0.x;"
					"  float mask1 = info1.x;"
					"  if (CSG(mask0) == CSG(mask1)) {"
					// this is a transparent surface, which could be textured
					"    gl_FragColor = vec4(0.0);"
					// - Grid texture
					"    if (useGridTexture) {"
					"      vec2 tc = fract(abs(info0.zw));"
					"      float grid = abs(2.0 * mod(tc.x * 3.0, 1.0) - 1.0);"
					"      grid = 1.0 - min(grid, abs(2.0 * mod(tc.y * 5.0, 1.0) - 1.0));"
					"      if (pow(grid, 5.0) > 0.5) {"
					"        gl_FragColor = vec4(info0.y, info0.y * info0.y, 0.0, 1.0);"
					"      }"
					"    }"
					// - Glyph texture
					"    if (useGlyphTexture) {"
					"      vec2 tc = abs(2.0 * info0.zw - vec2(1.0));"
					"      if ((tc.x < 0.1 && tc.y < 0.9) || (tc.y < 0.1 && tc.x < 0.6)) {"
					"        gl_FragColor = vec4(info0.y, info0.y * info0.y, 0.0, 1.0);"
					"      }"
					"    }"
					"  }"
					// otherwise this is a solid surface, store the depth and depth^2
					"  else gl_FragColor = vec4(info0.y, info0.y * info0.y, 0.0, 1.0);"
					// Clipping: objectId 2 is our clipping object
					// TODO: Clipping object should probably be configurable
					"  if ((getBit(mask0, 2) || getBit(mask1, 2))) {"
					"    discard;" // Nothing to render for this slab
					"  }"
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

			virtual void RegisterScribeTextures(GLFramebuffer *previous) 
			{
				// Get the previous layer's info buffer
				GLTexture *infoBuffer = previous->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT);
				assert(infoBuffer);
				tm->AddTexture("infoBuffer", infoBuffer, false);

				// Get the previous layer's depth buffer
				GLTexture *depthBuffer = previous->GetTexture2D(GL_DEPTH_ATTACHMENT_EXT);
				assert(depthBuffer);
				GLUtility::SetDefaultDepthTextureParameters(depthBuffer);
				glTexParameteri(depthBuffer->GetTextureTarget(), 
					GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
				depthBuffer->UnbindCurrent();
				tm->AddTexture("depthBuffer", depthBuffer, false);
			}

			virtual void UnregisterScribeTextures() 
			{
				//tm->RemoveTexture("infoBuffer");
				//tm->RemoveTexture("depthBuffer");
			}

			virtual void RegisterPainterTextures(GLFramebuffer *current, GLFramebuffer *previous) 
			{
				// Previous layer info buffer
				GLTexture *infoPrevious = previous->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT);
				assert(infoPrevious);
				tm->AddTexture("infoPrevious", infoPrevious, false);
				// Current layer info buffer
				GLTexture *infoCurrent = current->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT);
				assert(infoCurrent);
				tm->AddTexture("infoCurrent", infoCurrent, false);
			}

			virtual void UnregisterPainterTextures() 
			{
				//tm->RemoveTexture("infoPrevious");
				//tm->RemoveTexture("infoCurrent");
			}

			virtual void UpdatePainterParameters(GLProgram *scribe)
			{
				scribe->SetUniform1i("useGridTexture", useGridTexture);
				scribe->SetUniform1i("useGlyphTexture", useGlyphTexture);
			}

			virtual void DrawBackground()
			{
				glDisable(GL_LIGHTING);
				glEnable(GL_BLEND);

				glBegin(GL_QUADS);
				glColor4d(1.0, 1.0, 0.0, 1.0);
				glVertex3d(-1.0, -1.0, 0.0);
				glVertex3d(1.0, -1.0, 0.0);
				glVertex3d(1.0, 1.0, 0.0);
				glVertex3d(-1.0, 1.0, 0.0);
				glEnd();

				glDisable(GL_BLEND);
			}

			// Program parameters
			// - Painter
			bool useGridTexture;
			bool useGlyphTexture;

		private:
			// Not implemented
			ShadowMap(const ShadowMap&);
			void operator=(const ShadowMap&);
		};
	}
}
