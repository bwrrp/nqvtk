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

			ShadowMap() : depthBuffer(0), infoBuffer(0), infoCurrent(0), infoPrevious(0) 
			{
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
					fbo->CreateColorTextureRectangle(bufs[i]);
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
				fbo->CreateColorTextureRectangle();
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
					// Encode identity
					"  float id = float(objectId) + 1.0;"
					"  if (id < 0.0) id = 0.0;"
					"  float identity = id / 9.0;"
					// Encode in-out mask
					"  float inOutMask = prevInfo.y;"
					"  if (objectId >= 0) {"
					"    inOutMask = setBit(inOutMask, objectId, gl_FrontFacing);"
					"  }"
					// Encode depth
					"  vec2 depthVec = encodeDepth(depthInCamera);"
					// Store data
					"  gl_FragData[0] = vec4(identity, inOutMask, depthVec);"
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
					// Rounds a float to the nearest integer
					"\n#ifndef GL_EXT_gpu_shader4\n"
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
					// Main shader function
					"void main() {"
					"  vec4 r0 = gl_FragCoord;"
					"  r0.x -= viewportX;"
					"  r0.y -= viewportY;"
					"  vec4 info0 = texture2DRect(infoCurrent, r0.xy);"
					"  vec4 info1 = texture2DRect(infoPrevious, r0.xy);"
					"  if (layer == 0) info1 = vec4(0.0);"
					// Apply CSG
					"  float mask0 = info0.y;"
					"  float mask1 = info1.y;"
					"  if (CSG(mask0) == CSG(mask1)) {"
					// this is a transparent surface, which could be textured
					// TODO: store depths? accumulate opacity? 
					"    gl_FragColor = vec4(0.0);"
					"  } else {"
					// this is a solid surface, store the depth
					"    gl_FragColor = vec4(info0.zw, 0.0, 1.0);"
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

			virtual void BindScribeTextures(GLProgram *scribe, 
				GLFramebuffer *previous) 
			{
				assert(!depthBuffer && !infoBuffer);

				// Get the previous layer's info buffer
				infoBuffer = previous->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT);
				assert(infoBuffer);
				glActiveTexture(GL_TEXTURE1);
				infoBuffer->BindToCurrent();
				scribe->UseTexture("infoBuffer", 1);

				// Get the previous layer's depth buffer
				depthBuffer = previous->GetTexture2D(GL_DEPTH_ATTACHMENT_EXT);
				assert(depthBuffer);
				glActiveTexture(GL_TEXTURE0);
				depthBuffer->BindToCurrent();
				GLUtility::SetDefaultDepthTextureParameters(depthBuffer);
				glTexParameteri(depthBuffer->GetTextureTarget(), 
					GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
				scribe->UseTexture("depthBuffer", 0);
			}

			virtual void UnbindScribeTextures() 
			{
				assert(depthBuffer && infoBuffer);

				glActiveTexture(GL_TEXTURE1);
				infoBuffer->UnbindCurrent();
				infoBuffer = 0;
				glActiveTexture(GL_TEXTURE0);
				glTexParameteri(depthBuffer->GetTextureTarget(), 
						GL_TEXTURE_COMPARE_MODE, GL_NONE);
				depthBuffer->UnbindCurrent();
				depthBuffer = 0;
			}

			virtual void BindPainterTextures(GLProgram *painter, 
				GLFramebuffer *current, GLFramebuffer *previous) 
			{
				assert(!infoCurrent && !infoPrevious);

				// Previous layer info buffer
				infoPrevious = previous->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT);
				assert(infoPrevious);
				glActiveTexture(GL_TEXTURE1);
				infoPrevious->BindToCurrent();
				painter->UseTexture("infoPrevious", 1);
				// Current layer info buffer
				infoCurrent = current->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT);
				assert(infoCurrent);
				glActiveTexture(GL_TEXTURE0);
				infoCurrent->BindToCurrent();
				painter->UseTexture("infoCurrent", 0);
			}

			virtual void UnbindPainterTextures() 
			{
				assert(infoCurrent && infoPrevious);

				glActiveTexture(GL_TEXTURE1);
				infoPrevious->UnbindCurrent();
				infoPrevious = 0;
				glActiveTexture(GL_TEXTURE0);
				infoCurrent->UnbindCurrent();
				infoCurrent = 0;
			}

		protected:
			// Scribe textures
			GLTexture *depthBuffer;
			GLTexture *infoBuffer;
			// Painter textures
			GLTexture *infoCurrent;
			GLTexture *infoPrevious;

		private:
			// Not implemented
			ShadowMap(const ShadowMap&);
			void operator=(const ShadowMap&);
		};
	}
}
