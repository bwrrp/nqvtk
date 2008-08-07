#pragma once

#include "RenderStyle.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLTexture.h"
#include "GLBlaat/GLUtility.h"

#include "Shaders.h"

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
				classificationThreshold = 0.0;
				useGridTexture = false;
				useGlyphTexture = false;
				useFatContours = false;
				contourDepthEpsilon = 0.005;
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
				// Scribe vertex shaders
				bool res = scribe->AddVertexShader(
					Shaders::DistanceFieldsScribeVS);
				// Scribe fragment shaders
				if (res) res = scribe->AddFragmentShader(
					Shaders::DistanceFieldsScribeFS);
				if (res) res = scribe->AddFragmentShader(
					Shaders::LibUtility);
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
					Shaders::DistanceFieldsPainterFS);
				if (res) res = painter->AddFragmentShader(
					Shaders::LibUtility);
				if (res) res = painter->AddFragmentShader(
					Shaders::LibCSG);
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
				tm->AddTexture("infoPrevious", infoPrevious, false);
				// Current layer info buffer
				GLTexture *infoCurrent = current->GetTexture2D(GL_COLOR_ATTACHMENT2_EXT);
				assert(infoCurrent);
				tm->AddTexture("infoCurrent", infoCurrent, false);
				// Current layer colors
				GLTexture *colors = current->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT);
				assert(colors);
				tm->AddTexture("colors", colors, false);
				// Current layer normals
				GLTexture *normals = current->GetTexture2D(GL_COLOR_ATTACHMENT1_EXT);
				assert(normals);
				tm->AddTexture("normals", normals, false);
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
