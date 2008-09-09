#pragma once

#include "RenderStyle.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLUtility.h"

#include "Rendering/DistanceFieldParamSet.h"

#include "Shaders.h"

#include <sstream>
#include <vector>
#include <cassert>

namespace NQVTK
{
	namespace Styles
	{
		class Raycaster : public NQVTK::RenderStyle
		{
		public:
			typedef NQVTK::RenderStyle Superclass;

			Raycaster()
			{
				// Step size in world-space units
				// TODO: should probably depend on size and resolution of the volumes
				stepSize = 1.0;

				SetOption("NQVTK_RAYCASTER_LIGHTING");
			}

			virtual void PrepareForObject(GLProgram *scribe, 
				int objectId, NQVTK::Renderable *renderable)
			{
				// We re-use the DistanceFieldParamSet for describing volume data
				scribe->SetUniform1i("hasDistanceField", 0);
				Superclass::PrepareForObject(scribe, objectId, renderable);

				DistanceFieldParamSet *dfps = dynamic_cast<DistanceFieldParamSet*>(
					renderable->GetParamSet("distancefield"));
				if (dfps)
				{
					ImageDataTexture3D *volume = dfps->distanceField;
					if (volume)
					{
						// Make sure we have enough room
						while (static_cast<int>(volumes.size()) < objectId + 1) 
						{
							volumes.push_back(0);
						}
						// Store the paramset
						volumes[objectId] = dfps;
					}
					else
					{
						scribe->SetUniform1i("hasDistanceField", 0);
					}
				}
				tm->Bind();
			}

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

			virtual GLProgram *CreateScribe()
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
				// Painter vertex shaders
				bool res = painter->AddVertexShader(
					Shaders::GenericPainterVS);
				// Painter fragment shaders
				if (res) res = painter->AddFragmentShader(
					AddShaderDefines(Shaders::RaycasterPainterFS));
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
				GLTexture *infoBuffer = previous->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT);
				assert(infoBuffer);
				tm->AddTexture("infoBuffer", infoBuffer, false);
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

				// TODO: make sure bindings are initialized
				// TODO: find some way to handle max. number of volumes (also in shader)
				// TODO: getactiveuniforms seems to return "volume", size 4, not "volume[0]"
				for (int i = 0; i < 4; ++i)
				{
					tm->AddTexture(GetVarName("volume", i), 0, false);
				}

				// Add volumes to tm from stored paramsets
				for (unsigned int i = 0; i < volumes.size(); ++i)
				{
					NQVTK::DistanceFieldParamSet *dfps = volumes[i];
					if (dfps)
					{
						tm->AddTexture(GetVarName("volume", i), 
							dfps->distanceField, false);
					}
					else
					{
						// Ignore this object for now, the volume shouldn't be used
					}
				}
			}

			virtual void UpdatePainterParameters(GLProgram *painter)
			{
				// Set volume info from stored paramsets
				for (unsigned int i = 0; i < volumes.size(); ++i)
				{
					NQVTK::DistanceFieldParamSet *dfps = volumes[i];
					if (dfps)
					{
						// Set volume info
						NQVTK::ImageDataTexture3D *vol = dfps->distanceField;
						painter->SetUniform1f(
							GetVarName("volumeDataShift", i), 
							vol->GetDataShift());
						painter->SetUniform1f(
							GetVarName("volumeDataScale", i), 
							vol->GetDataScale());
						NQVTK::Vector3 origin = vol->GetOrigin();
						painter->SetUniform3f(
							GetVarName("volumeOrigin", i), 
							origin.x, origin.y, origin.z);
						NQVTK::Vector3 size = vol->GetOriginalSize();
						painter->SetUniform3f(
							GetVarName("volumeSize", i), 
							size.x, size.y, size.z);
					}
				}
				// Set other parameters
				painter->SetUniform1f("stepSize", stepSize);
			}

			float stepSize;

		protected:
			std::vector<NQVTK::DistanceFieldParamSet*> volumes;

			std::string GetVarName(const std::string &baseName, int index)
			{
				// TESTING: just return baseName
				//return baseName;
				// TODO: Later we'll want to use multiple volumes
				std::ostringstream name;
				name << baseName << "[" << index << "]";
				return name.str();
			}

		private:
			// Not implemented
			Raycaster(const Raycaster&);
			void operator=(const Raycaster&);
		};
	}
}
