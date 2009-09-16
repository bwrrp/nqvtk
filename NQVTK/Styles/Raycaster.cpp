#include "Raycaster.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLTextureManager.h"
#include "GLBlaat/GLUtility.h"

#include "ParamSets/VolumeParamSet.h"
#include "Renderables/Renderable.h"
#include "Rendering/Volume.h"
#include "Rendering/View.h"

#include "Shaders.h"

#include <string>
#include <sstream>
#include <vector>
#include <cassert>
#include <iostream>

namespace NQVTK
{
	namespace Styles
	{
		// --------------------------------------------------------------------
		Raycaster::Raycaster()
		{
			// This is later set to the minimum spacing in any direction 
			// over all volumes
			unitSize = 1.0;

			// Step size in units
			stepSize = 10.0;
			kernelSize = 1.0;

			// This is recomputed whenever the scene changes
			SetOption("NQVTK_RAYCASTER_VOLUMECOUNT", 1);

			SetOption("NQVTK_RAYCASTER_LIGHTING");
			SetOption("NQVTK_RAYCASTER_DITHERPOS");
			SetOption("NQVTK_RAYCASTER_CENTRALDIFFERENCES");
		}

		// --------------------------------------------------------------------
		GLFramebuffer *Raycaster::CreateFBO(int w, int h)
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
			if (!fbo->IsOk()) 
			{
				std::cerr << "WARNING! fbo not ok!" << std::endl;
			}
			fbo->Unbind();

			return fbo;
		}

		// --------------------------------------------------------------------
		GLProgram *Raycaster::CreateScribe()
		{
			GLProgram *scribe = GLProgram::New();
			// Scribe vertex shaders
			bool res = scribe->AddVertexShader(
				AddShaderDefines(Shaders::RaycasterScribeVS));
			// Scribe fragment shaders
			if (res) res = scribe->AddFragmentShader(
				AddShaderDefines(Shaders::RaycasterScribeFS));
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
		GLProgram *Raycaster::CreatePainter()
		{
			GLProgram *painter = GLProgram::New();
			// Painter vertex shaders
			bool res = painter->AddVertexShader(
				Shaders::GenericPainterVS);
			// Painter fragment shaders
			if (res) res = painter->AddFragmentShader(
				AddShaderDefines(Shaders::RaycasterPainterFS));
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
		void Raycaster::RegisterScribeTextures(GLFramebuffer *previous) 
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
			tm->AddTexture("infoBuffer", infoBuffer, false);
		}

		// --------------------------------------------------------------------
		void Raycaster::RegisterPainterTextures(GLFramebuffer *current, 
			GLFramebuffer *previous) 
		{
			// Previous layer info buffer
			GLTexture *infoPrevious = previous->GetTexture2D(
				GL_COLOR_ATTACHMENT0_EXT);
			assert(infoPrevious);
			tm->AddTexture("infoPrevious", infoPrevious, false);
			// Current layer info buffer
			GLTexture *infoCurrent = current->GetTexture2D(
				GL_COLOR_ATTACHMENT0_EXT);
			assert(infoCurrent);
			tm->AddTexture("infoCurrent", infoCurrent, false);

			// TODO: make sure the volumes use linear interpolation
		}

		// --------------------------------------------------------------------
		void Raycaster::UpdatePainterParameters(GLProgram *painter)
		{
			// Volume parameters are set by the VolumeParamSet
			// Set other parameters
			painter->SetUniform1f("stepSize", 
				static_cast<float>(stepSize * unitSize));
			painter->SetUniform1f("kernelSize", 
				static_cast<float>(kernelSize * unitSize));
		}

		// --------------------------------------------------------------------
		void Raycaster::SceneChanged(View *view)
		{
			Superclass::SceneChanged(view);

			// Compute unitSize
			unitSize = 1000000.0;
			// Compute max objectId which has a non-null volume
			unsigned int maxVolumeId = 0;
			for (unsigned int i = 0; i < view->GetNumberOfRenderables(); ++i)
			{
				Renderable *renderable = view->GetRenderable(i);
				if (renderable)
				{
					VolumeParamSet *vps = dynamic_cast<VolumeParamSet*>(
						renderable->GetParamSet("volume"));
					if (vps)
					{
						Volume *volume = vps->GetVolume();
						if (volume)
						{
							maxVolumeId = i;
							// Compute spacings
							NQVTK::Vector3 size = 
								volume->GetOriginalSize();
							double spX = size.x / static_cast<double>(
								volume->GetWidth() - 1);
							double spY = size.y / static_cast<double>(
								volume->GetHeight() - 1);
							double spZ = size.z / static_cast<double>(
								volume->GetDepth() - 1);
							// Update unitSize if any are smaller
							if (spX < unitSize) unitSize = spX;
							if (spY < unitSize) unitSize = spY;
							if (spZ < unitSize) unitSize = spZ;
						}
					}
					else
					{
						// Add an empty volume ParamSet
						renderable->SetParamSet("volume", 
							new NQVTK::VolumeParamSet(0));
					}
				}
			}
			assert(unitSize > 0.0);
			// This can't be lower than 1
			unsigned int volumeCount = maxVolumeId + 1;
			if (volumeCount < 1) volumeCount = 1;
			SetOption("NQVTK_RAYCASTER_VOLUMECOUNT", volumeCount);
		}
	}
}
