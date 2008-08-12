#pragma once

#include "IBIS.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLTexture.h"
#include "GLBlaat/GLUtility.h"

#include "Shaders.h"

#include "Rendering/ImageDataTexture3D.h"

#include <sstream>

#include <cassert>
#include <map>

namespace NQVTK
{
	namespace Styles
	{
		class DistanceFields : public IBIS
		{
		public:
			typedef IBIS Superclass;

			DistanceFields()
			{ 
				distanceFieldId = GLTextureManager::BAD_SAMPLER_ID;

				SetOption("NQVTK_USE_DISTANCEFIELDS");

				// Set default parameters
				useDistanceColorMap = false;
				classificationThreshold = 0.0;
				useGridTexture = false;
				useGlyphTexture = false;
			}
			
			virtual ~DistanceFields() 
			{ 
				for (std::map<int, GLTexture*>::iterator it = distanceFields.begin();
					it != distanceFields.end(); ++it)
				{
					delete it->second;
				}
			}

			virtual void PrepareForObject(GLProgram *scribe, 
				int objectId, NQVTK::Renderable *renderable)
			{
				Superclass::PrepareForObject(scribe, objectId, renderable);
				
				ImageDataTexture3D *distanceField = 
					dynamic_cast<ImageDataTexture3D*>(GetDistanceField(objectId));
				// HACK: the second condition is a hack for shadow map creation
				if (distanceField && distanceFieldId != GLTextureManager::BAD_SAMPLER_ID)
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
				Superclass::RegisterScribeTextures(previous);

				if (distanceFieldId == GLTextureManager::BAD_SAMPLER_ID)
				{
					// Register distance field sampler
					// This is set to different textures in PrepareForObject
					distanceFieldId = tm->AddTexture("distanceField", 0, false);
				}
			}

			virtual void UpdateScribeParameters(GLProgram *scribe)
			{
				// Set program parameters
				scribe->SetUniform1i("useDistanceColorMap", useDistanceColorMap);
				scribe->SetUniform1f("classificationThreshold", classificationThreshold);
				scribe->SetUniform1i("useGridTexture", useGridTexture);
				scribe->SetUniform1i("useGlyphTexture", useGlyphTexture);

				// HACK: set weights
				for (unsigned int i = 0; i < weights.size(); ++i)
				{
					std::ostringstream name;
					name << "weights[" << i << "]";
					scribe->SetUniform1f(name.str(), weights[i]);
				}
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
			float classificationThreshold;
			bool useGridTexture;
			bool useGlyphTexture;

			std::vector<float> weights;

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
