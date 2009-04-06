#pragma once

#include "Raycaster.h"

#include <cassert>
#include <iostream>

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
				occlusionEdgeThreshold = 0.1;
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
				if (!fbo->IsOk()) 
				{
					std::cerr << "WARNING! fbo not ok!" << std::endl;
				}
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
				std::cout << scribe->GetInfoLogs() << std::endl;
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
				std::cout << raycaster->GetInfoLogs() << std::endl;
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
				std::cout << painter->GetInfoLogs() << std::endl;
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
				painter->SetUniform1f("cornerEdgeThreshold", cornerEdgeThreshold);

				double maxRayLength = 0.0;
				for (unsigned int i = 0; i < volumes.size(); ++i)
				{
					NQVTK::DistanceFieldParamSet *dfps = volumes[i];
					if (dfps)
					{
						double w = dfps->distanceField->GetWidth();
						double h = dfps->distanceField->GetHeight();
						double d = dfps->distanceField->GetDepth();
						double diameter = sqrt(w * w + h * h + d * d);
						if (diameter > maxRayLength) maxRayLength = diameter;
					}
				}
				painter->SetUniform1f("occlusionEdgeThreshold", 
					occlusionEdgeThreshold * maxRayLength / 10.0);
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
