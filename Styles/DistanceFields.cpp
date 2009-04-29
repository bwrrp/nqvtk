#pragma once

#include "DistanceFields.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLTexture.h"
#include "GLBlaat/GLTextureManager.h"
#include "GLBlaat/GLUtility.h"

#include "Shaders.h"

#include "ParamSets/VolumeParamSet.h"
#include "Renderables/Renderable.h"
#include "Rendering/Volume.h"

#include <sstream>

#include <cassert>
#include <map>

namespace NQVTK
{
	namespace Styles
	{
		// --------------------------------------------------------------------
		DistanceFields::DistanceFields()
		{ 
			distanceFieldId = GLTextureManager::BAD_SAMPLER_ID;

			SetOption("NQVTK_USE_DISTANCEFIELDS");

			// Set default parameters
			useDistanceColorMap = false;
			distanceThreshold = 0.0;
			useGridTexture = false;
			useGlyphTexture = false;
		}

		// --------------------------------------------------------------------
		void DistanceFields::PrepareForObject(GLProgram *scribe, 
			int objectId, NQVTK::Renderable *renderable)
		{
			scribe->SetUniform1i("hasVolume", 0);
			Superclass::PrepareForObject(scribe, objectId, renderable);

			VolumeParamSet *vps = dynamic_cast<VolumeParamSet*>(
				renderable->GetParamSet("volume"));
			if (vps)
			{
				Volume *distanceField = vps->GetVolume();
				// HACK: the second condition is a hack for shadow map creation
				if (distanceField && distanceFieldId != 
					GLTextureManager::BAD_SAMPLER_ID)
				{
					// Update distance field texture
					tm->SwapTexture(distanceFieldId, distanceField);
				}
				else
				{
					scribe->SetUniform1i("hasVolume", 0);
				}
			}
			tm->Bind();
		}

		// --------------------------------------------------------------------
		void DistanceFields::RegisterScribeTextures(GLFramebuffer *previous) 
		{
			Superclass::RegisterScribeTextures(previous);

			if (distanceFieldId == GLTextureManager::BAD_SAMPLER_ID)
			{
				// Register distance field sampler
				// This is set to different textures in PrepareForObject
				distanceFieldId = tm->AddTexture("volume", 0, false);
			}
		}

		// --------------------------------------------------------------------
		void DistanceFields::UpdateScribeParameters(GLProgram *scribe)
		{
			Superclass::UpdateScribeParameters(scribe);
			// Set program parameters
			scribe->SetUniform1i("useDistanceColorMap", useDistanceColorMap);
			scribe->SetUniform1f("distanceThreshold", distanceThreshold);
			scribe->SetUniform1i("useGridTexture", useGridTexture);
			scribe->SetUniform1i("useGlyphTexture", useGlyphTexture);
		}
	}
}
