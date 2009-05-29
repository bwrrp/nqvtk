#pragma once

#include "Raycaster.h"

namespace NQVTK
{
	namespace Styles
	{
		// TODO: we may want to create an abstract superclass
		// (layered raycaster styles have an extra stage)
		class LayeredRaycaster : public NQVTK::Styles::Raycaster
		{
		public:
			typedef NQVTK::Styles::Raycaster Superclass;

			LayeredRaycaster();

			virtual GLFramebuffer *CreateFBO(int w, int h);
			
			// Scribe stage peeling pass
			virtual GLProgram *CreateScribe();
			// Scribe stage raycasting pass
			virtual GLProgram *CreateRaycaster();
			// Painter stage
			virtual GLProgram *CreatePainter();

			// Used for both the painter and the scribe raycaster passes
			virtual void RegisterPainterTextures(GLFramebuffer *current, 
				GLFramebuffer *previous);
			virtual void UpdatePainterParameters(GLProgram *painter);

			virtual void SceneChanged(View *view);

			float testParam;

			float isoOpacity;
			float occlusionEdgeThreshold;
			float cornerEdgeThreshold;

		protected:
			// Maximum length of a ray through the volume
			double maxRayLength;

		private:
			// Not implemented
			LayeredRaycaster(const LayeredRaycaster&);
			void operator=(const LayeredRaycaster&);
		};
	}
}
