#include "CustomLayeredRaycaster.h"

#include "GLBlaat/GLFrameBuffer.h"
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
		CustomLayeredRaycaster::CustomLayeredRaycaster()
		{
		}

		// --------------------------------------------------------------------
		void CustomLayeredRaycaster::SetScribeSource(
				std::string vertexSource, 
				std::string fragmentSource)
		{
			this->scribeVertexSource = vertexSource;
			this->scribeFragmentSource = fragmentSource;
		}

		// --------------------------------------------------------------------
		void CustomLayeredRaycaster::SetRaycasterSource(
				std::string vertexSource, 
				std::string fragmentSource)
		{
			this->raycasterVertexSource = vertexSource;
			this->raycasterFragmentSource = fragmentSource;
		}

		// --------------------------------------------------------------------
		void CustomLayeredRaycaster::SetRaycasterSource(
				std::string fragmentSource)
		{
			this->raycasterVertexSource = Shaders::GenericPainterVS;
			this->raycasterFragmentSource = fragmentSource;
		}

		// --------------------------------------------------------------------
		void CustomLayeredRaycaster::SetPainterSource(
				std::string vertexSource, 
				std::string fragmentSource)
		{
			this->painterVertexSource = vertexSource;
			this->painterFragmentSource = fragmentSource;
		}

		// --------------------------------------------------------------------
		void CustomLayeredRaycaster::SetPainterSource(
				std::string fragmentSource)
		{
			this->painterVertexSource = Shaders::GenericPainterVS;
			this->painterFragmentSource = fragmentSource;
		}

		// --------------------------------------------------------------------
		GLFramebuffer *CustomLayeredRaycaster::CreateFBO(int w, int h)
		{
			GLFramebuffer *fbo = GLFramebuffer::New(w, h);
			fbo->CreateDepthTextureRectangle();
			// 4 buffers should be enough for now, but we can go up to 
			// GL_MAX_COLOR_ATTACHMENTS (8 on GeForce8 hardware)
			int nBufs = 4;
			GLenum bufs[] = {
				GL_COLOR_ATTACHMENT0_EXT,
				GL_COLOR_ATTACHMENT1_EXT,
				GL_COLOR_ATTACHMENT2_EXT,
				GL_COLOR_ATTACHMENT3_EXT
			};
			for (int i = 0; i < nBufs; ++i)
			{
				fbo->CreateColorTextureRectangle(
					bufs[i], GL_RGBA16F_ARB, GL_RGBA, GL_FLOAT);
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
		GLProgram *CustomLayeredRaycaster::CreateScribe()
		{
			GLProgram *scribe = GLProgram::New();
			// Scribe vertex shaders
			bool res = scribe->AddVertexShader(
				AddShaderDefines(scribeVertexSource));
			// Scribe fragment shaders
			if (res) res = scribe->AddFragmentShader(
				AddShaderDefines(scribeFragmentSource));
			if (res) res = scribe->AddFragmentShader(
				AddShaderDefines(Shaders::LibUtility));
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
		GLProgram *CustomLayeredRaycaster::CreateRaycaster()
		{
			GLProgram *raycaster = GLProgram::New();
			// Raycaster vertex shaders
			bool res = raycaster->AddVertexShader(
				AddShaderDefines(raycasterVertexSource));
			// Raycaster fragment shaders
			if (res) res = raycaster->AddFragmentShader(
				AddShaderDefines(raycasterFragmentSource));
			if (res) res = raycaster->AddFragmentShader(
				AddShaderDefines(Shaders::LibUtility));
			if (res) res = raycaster->Link();
			std::cout << raycaster->GetInfoLogs() << std::endl;
			if (!res) 
			{
				delete raycaster;
				return 0;
			}
			return raycaster;
		}

		// --------------------------------------------------------------------
		GLProgram *CustomLayeredRaycaster::CreatePainter()
		{
			GLProgram *painter = GLProgram::New();
			// Painter vertex shaders
			bool res = painter->AddVertexShader(
				AddShaderDefines(painterVertexSource));
			// Painter fragment shaders
			if (res) res = painter->AddFragmentShader(
				AddShaderDefines(painterFragmentSource));
			if (res) res = painter->AddFragmentShader(
				AddShaderDefines(Shaders::LibUtility));
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
		void CustomLayeredRaycaster::RegisterScribeTextures(
			GLFramebuffer *previous) 
		{
			// Get the previous layer's depth buffer
			GLTexture *depthBuffer = previous->GetTexture2D(
				GL_DEPTH_ATTACHMENT_EXT);
			assert(depthBuffer);
			GLUtility::SetDefaultDepthTextureParameters(depthBuffer);
			glTexParameteri(depthBuffer->GetTextureTarget(), 
				GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
			depthBuffer->UnbindCurrent();
			tm->AddTexture("depthBuffer", depthBuffer, false);

			// Get the previous layer's info buffer
			GLTexture *infoBuffer = previous->GetTexture2D(
				GL_COLOR_ATTACHMENT0_EXT);
			assert(infoBuffer);
			tm->AddTexture("info0Previous", infoBuffer, false);

			infoBuffer = previous->GetTexture2D(
				GL_COLOR_ATTACHMENT1_EXT);
			assert(infoBuffer);
			tm->AddTexture("info1Previous", infoBuffer, false);

			infoBuffer = previous->GetTexture2D(
				GL_COLOR_ATTACHMENT2_EXT);
			assert(infoBuffer);
			tm->AddTexture("info2Previous", infoBuffer, false);

			infoBuffer = previous->GetTexture2D(
				GL_COLOR_ATTACHMENT3_EXT);
			assert(infoBuffer);
			tm->AddTexture("info3Previous", infoBuffer, false);
		}

		// --------------------------------------------------------------------
		// Used for both the painter and the raycaster passes
		void CustomLayeredRaycaster::RegisterPainterTextures(
			GLFramebuffer *current, GLFramebuffer *previous) 
		{
			// Pass all current and previous buffers (just in case...)
			GLTexture *tex = previous->GetTexture2D(
				GL_COLOR_ATTACHMENT0_EXT);
			assert(tex);
			tm->AddTexture("info0Previous", tex, false);

			tex = current->GetTexture2D(
				GL_COLOR_ATTACHMENT0_EXT);
			assert(tex);
			tm->AddTexture("info0Current", tex, false);

			tex = previous->GetTexture2D(
				GL_COLOR_ATTACHMENT1_EXT);
			assert(tex);
			tm->AddTexture("info1Previous", tex, false);

			tex = current->GetTexture2D(
				GL_COLOR_ATTACHMENT1_EXT);
			assert(tex);
			tm->AddTexture("info1Current", tex, false);

			tex = previous->GetTexture2D(
				GL_COLOR_ATTACHMENT2_EXT);
			assert(tex);
			tm->AddTexture("info2Previous", tex, false);

			tex = current->GetTexture2D(
				GL_COLOR_ATTACHMENT2_EXT);
			assert(tex);
			tm->AddTexture("info2Current", tex, false);

			tex = previous->GetTexture2D(
				GL_COLOR_ATTACHMENT3_EXT);
			assert(tex);
			tm->AddTexture("info3Previous", tex, false);

			tex = current->GetTexture2D(
				GL_COLOR_ATTACHMENT3_EXT);
			assert(tex);
			tm->AddTexture("info3Current", tex, false);
		}
	}
}
