#pragma once

#include "IBIS.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLTexture.h"
#include "GLBlaat/GLUtility.h"

#include "Shaders.h"

#include "Rendering/DistanceFieldParamSet.h"

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

			virtual void PrepareForObject(GLProgram *scribe, 
				int objectId, NQVTK::Renderable *renderable)
			{
				scribe->SetUniform1i("hasDistanceField", 0);
				Superclass::PrepareForObject(scribe, objectId, renderable);

				
				DistanceFieldParamSet *dfps = dynamic_cast<DistanceFieldParamSet*>(
					renderable->GetParamSet("distancefield"));
				if (dfps)
				{
					ImageDataTexture3D *distanceField = dfps->distanceField;
					// HACK: the second condition is a hack for shadow map creation
					if (distanceField && distanceFieldId != GLTextureManager::BAD_SAMPLER_ID)
					{
						// Update distance field texture
						tm->SwapTexture(distanceFieldId, distanceField);
					}
					else
					{
						scribe->SetUniform1i("hasDistanceField", 0);
					}
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
			}

			// Program parameters
			// - Scribe
			bool useDistanceColorMap;
			float classificationThreshold;
			bool useGridTexture;
			bool useGlyphTexture;

		protected:
			// SamplerIds
			GLTextureManager::SamplerId distanceFieldId;

		private:
			// Not implemented
			DistanceFields(const DistanceFields&);
			void operator=(const DistanceFields&);
		};
	}
}
