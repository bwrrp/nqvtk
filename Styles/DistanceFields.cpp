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
			SetOption("NQVTK_USE_DISTANCEFIELDS");

			// Set default parameters
			useDistanceColorMap = false;
			distanceThreshold = 0.0;
			useGridTexture = false;
			useGlyphTexture = false;
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
