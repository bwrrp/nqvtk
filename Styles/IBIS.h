#pragma once

#include "RenderStyle.h"

namespace NQVTK
{
	namespace Styles
	{
		class IBIS : public NQVTK::RenderStyle
		{
		public:
			typedef NQVTK::RenderStyle Superclass;

			IBIS();
			virtual ~IBIS();

			virtual GLFramebuffer *CreateFBO(int w, int h);
			virtual GLProgram *CreateScribe();
			virtual GLProgram *CreatePainter();

			virtual void RegisterScribeTextures(GLFramebuffer *previous);
			virtual void RegisterPainterTextures(GLFramebuffer *current, 
				GLFramebuffer *previous);
			
			virtual void UpdatePainterParameters(GLProgram *painter);
			virtual void UpdateScribeParameters(GLProgram *scribe);

			virtual void SceneChanged(View *view);

			// Program parameters
			// - Painter
			bool useContours;
			bool useFatContours;
			float contourDepthEpsilon;
			bool useFog;
			float depthCueRange;
			int clipId;
			// - Scribe
			float pvalueThreshold;

		private:
			// Not implemented
			IBIS(const IBIS&);
			void operator=(const IBIS&);
		};
	}
}
