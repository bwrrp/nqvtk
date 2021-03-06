#include "DepthPeeling.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLTextureManager.h"
#include "GLBlaat/GLUtility.h"

#include "Shaders.h"

#include <cassert>
#include <iostream>

namespace NQVTK
{
	namespace Styles
	{
		// --------------------------------------------------------------------
		DepthPeeling::DepthPeeling()
		{
		}

		// --------------------------------------------------------------------
		DepthPeeling::~DepthPeeling()
		{
		}

		// --------------------------------------------------------------------
		GLFramebuffer *DepthPeeling::CreateFBO(int w, int h)
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
			if (!fbo->IsOk()) 
			{
				std::cerr << "WARNING! fbo not ok!" << std::endl;
			}
			fbo->Unbind();

			return fbo;
		}

		// --------------------------------------------------------------------
		GLProgram *DepthPeeling::CreateScribe()
		{
			GLProgram *scribe = GLProgram::New();
			bool res = scribe->AddFragmentShader(
				Shaders::DepthPeelingScribeFS);
			if (res) res = scribe->Link();
			std::cout << scribe->GetInfoLogs() << std::endl;
			if (!res)
			{
				delete scribe;
				return 0;
			}
			return scribe;
		}

		// --------------------------------------------------------------------
		GLProgram *DepthPeeling::CreatePainter()
		{
			GLProgram *painter = GLProgram::New();
			bool res = painter->AddVertexShader(
				Shaders::GenericPainterVS);
			if (res) res = painter->AddFragmentShader(
				Shaders::DepthPeelingPainterFS);
			if (res) res = painter->Link();
			std::cout << painter->GetInfoLogs() << std::endl;
			if (!res) 
			{
				delete painter;
				return 0;
			}
			return painter;
		}

		// --------------------------------------------------------------------
		void DepthPeeling::RegisterScribeTextures(GLFramebuffer *previous) 
		{
			GLTexture *depthBuffer = previous->GetTexture2D(
				GL_DEPTH_ATTACHMENT_EXT);
			assert(depthBuffer);
			GLUtility::SetDefaultDepthTextureParameters(depthBuffer);
			glTexParameteri(depthBuffer->GetTextureTarget(), 
				GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
			depthBuffer->UnbindCurrent();
			tm->AddTexture("depthBuffer", depthBuffer, false);
		}

		// --------------------------------------------------------------------
		void DepthPeeling::RegisterPainterTextures(GLFramebuffer *current, 
			GLFramebuffer *previous) 
		{
			GLTexture *colors = current->GetTexture2D(
				GL_COLOR_ATTACHMENT0_EXT);
			assert(colors);
			tm->AddTexture("infoCurrent", colors, false);
		}
	}
}
