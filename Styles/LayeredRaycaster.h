#pragma once

#include "Raycaster.h"

namespace NQVTK
{
	namespace Styles
	{
		// TODO: we may want to create an abstract superclass for raycaster styles
		// (they have an extra stage)
		class LayeredRaycaster : public NQVTK::Styles::Raycaster
		{
		public:
			typedef NQVTK::Styles::Raycaster Superclass;

			LayeredRaycaster()
			{
				testParam = 0.0;
				isoOpacity = 0.6;
				occlusionEdgeThreshold = 1.0;
				cornerEdgeThreshold = 0.0;
			}

			virtual GLFramebuffer *CreateFBO(int w, int h)
			{
				GLFramebuffer *fbo = GLFramebuffer::New(w, h);
				fbo->CreateDepthTextureRectangle();
				int nBufs = 3;
				GLenum bufs[] = {
					GL_COLOR_ATTACHMENT0_EXT, // Positions, in-out buffer
					GL_COLOR_ATTACHMENT1_EXT, // Normals / gradients, object IDs
					GL_COLOR_ATTACHMENT2_EXT  // Colors, opacities
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

			// Scribe stage peeling pass
			virtual GLProgram *CreateScribe()
			{
				GLProgram *scribe = GLProgram::New();
				// Scribe vertex shaders
				bool res = scribe->AddVertexShader(
					AddShaderDefines(Shaders::LayeredRaycasterScribePeelVS));
				// Scribe fragment shaders
				if (res) res = scribe->AddFragmentShader(
					AddShaderDefines(Shaders::LayeredRaycasterScribePeelFS));
				if (res) res = scribe->AddFragmentShader(
					AddShaderDefines(Shaders::LibUtility));
				if (res) res = scribe->Link();
				qDebug(scribe->GetInfoLogs().c_str());
				if (!res)
				{
					delete scribe;
					return 0;
				}
				return scribe;
			}

			// Scribe stage raycasting pass
			virtual GLProgram *CreateRaycaster()
			{
				GLProgram *raycaster = GLProgram::New();
				// Raycaster vertex shaders
				bool res = raycaster->AddVertexShader(
					Shaders::GenericPainterVS);
				// Raycaster fragment shaders
				if (res) res = raycaster->AddFragmentShader(
					AddShaderDefines(Shaders::LayeredRaycasterScribeCastFS));
				if (res) res = raycaster->AddFragmentShader(
					AddShaderDefines(Shaders::LibUtility));
				if (res) res = raycaster->Link();
				qDebug(raycaster->GetInfoLogs().c_str());
				if (!res) 
				{
					delete raycaster;
					return 0;
				}
				return raycaster;
			}

			// Painter stage
			virtual GLProgram *CreatePainter()
			{
				GLProgram *painter = GLProgram::New();
				// Painter vertex shaders
				bool res = painter->AddVertexShader(
					Shaders::GenericPainterVS);
				// Painter fragment shaders
				if (res) res = painter->AddFragmentShader(
					AddShaderDefines(Shaders::LayeredRaycasterPainterFS));
				if (res) res = painter->AddFragmentShader(
					AddShaderDefines(Shaders::LibUtility));
				if (res) res = painter->Link();
				qDebug(painter->GetInfoLogs().c_str());
				if (!res) 
				{
					delete painter;
					return 0;
				}
				return painter;
			}

			// Used for both the painter and the scribe raycaster passes
			virtual void RegisterPainterTextures(GLFramebuffer *current, GLFramebuffer *previous) 
			{
				// Register infobuffers and volumes
				Superclass::RegisterPainterTextures(current, previous);
				
				// Current layer normals and IDs
				GLTexture *normals = current->GetTexture2D(GL_COLOR_ATTACHMENT1_EXT);
				assert(normals);
				tm->AddTexture("normals", normals, false);
				// Current layer colors
				GLTexture *colors = current->GetTexture2D(GL_COLOR_ATTACHMENT2_EXT);
				assert(colors);
				tm->AddTexture("colors", colors, false);
			}

			virtual void UpdatePainterParameters(GLProgram *painter)
			{
				Superclass::UpdatePainterParameters(painter);
				painter->SetUniform1f("testParam", testParam);
				painter->SetUniform1f("isoOpacity", isoOpacity);
				painter->SetUniform1f("occlusionEdgeThreshold", occlusionEdgeThreshold);
				painter->SetUniform1f("cornerEdgeThreshold", cornerEdgeThreshold);
			}

			float testParam;

			float isoOpacity;
			float occlusionEdgeThreshold;
			float cornerEdgeThreshold;

		private:
			// Not implemented
			LayeredRaycaster(const LayeredRaycaster&);
			void operator=(const LayeredRaycaster&);
		};
	}
}
