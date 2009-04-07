#pragma once

#include "IBIS.h"

namespace NQVTK
{
	namespace Styles
	{
		class DistanceFields : public IBIS
		{
		public:
			typedef IBIS Superclass;

			DistanceFields();

			virtual void PrepareForObject(GLProgram *scribe, 
				int objectId, NQVTK::Renderable *renderable);

			virtual void RegisterScribeTextures(GLFramebuffer *previous);
			virtual void UpdateScribeParameters(GLProgram *scribe);

			// Program parameters
			// - Scribe
			bool useDistanceColorMap;
			float distanceThreshold;
			bool useGridTexture;
			bool useGlyphTexture;

		protected:
			// SamplerIds
			int distanceFieldId;

		private:
			// Not implemented
			DistanceFields(const DistanceFields&);
			void operator=(const DistanceFields&);
		};
	}
}
