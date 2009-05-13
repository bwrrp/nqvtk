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

			virtual void UpdateScribeParameters(GLProgram *scribe);

			// Program parameters
			// - Scribe
			bool useDistanceColorMap;
			float distanceThreshold;
			bool useGridTexture;
			bool useGlyphTexture;

		private:
			// Not implemented
			DistanceFields(const DistanceFields&);
			void operator=(const DistanceFields&);
		};
	}
}
