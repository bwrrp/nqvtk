#pragma once

#include "RenderStyle.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLTexture.h"
#include "GLBlaat/GLUtility.h"

#include <cassert>

namespace NQVTK
{
	namespace Styles
	{
		class DepthPeeling : public NQVTK::RenderStyle
		{
		public:
			typedef NQVTK::RenderStyle Superclass;

			DepthPeeling() { }
			virtual ~DepthPeeling() { }

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

			virtual GLProgram *CreateScribe()
			{
				GLProgram *scribe = GLProgram::New();
				bool res = scribe->AddFragmentShader(
					"#extension GL_ARB_texture_rectangle : enable\n"
					"uniform sampler2DRectShadow depthBuffer;"
					"uniform int layer;"
					// Shader main function
					"void main() {"
					"  vec4 r0 = gl_FragCoord;"
					// Depth peeling
					"  if (layer > 0) {"
					"    float r1 = shadow2DRect(depthBuffer, r0.xyz).x;"
					"    r1 = r1 - 0.5;"
					"    if (r1 < 0.0) { discard; }"
					"  }"
					// Store data
					"  gl_FragData[0] = gl_Color;"
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
					"uniform sampler2DRect colors;"
					"uniform int layer;"
					"uniform float viewportX;"
					"uniform float viewportY;"
					// Main shader function
					"void main() {"
					"  vec4 r0 = gl_FragCoord;"
					"  r0.x -= viewportX;"
					"  r0.y -= viewportY;"
					"  vec4 color = texture2DRect(colors, r0.xy);"
					// Pre-multiply colors by alpha
					"  vec3 litFragment = color.rgb;"
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

			virtual void RegisterScribeTextures(GLFramebuffer *previous) 
			{
				GLTexture *depthBuffer = previous->GetTexture2D(GL_DEPTH_ATTACHMENT_EXT);
				assert(depthBuffer);
				GLUtility::SetDefaultDepthTextureParameters(depthBuffer);
				glTexParameteri(depthBuffer->GetTextureTarget(), 
					GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
				depthBuffer->UnbindCurrent();
				tm->AddTexture("depthBuffer", depthBuffer, false);
			}

			virtual void UnregisterScribeTextures() 
			{
				//tm->RemoveTexture("depthBuffer");
			}

			virtual void RegisterPainterTextures(GLFramebuffer *current, GLFramebuffer *previous) 
			{
				GLTexture *colors = current->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT);
				assert(colors);
				tm->AddTexture("colors", colors, false);
			}

			virtual void UnregisterPainterTextures() 
			{
				//tm->RemoveTexture("colors");
			}

		private:
			// Not implemented
			DepthPeeling(const DepthPeeling&);
			void operator=(const DepthPeeling&);
		};
	}
}
