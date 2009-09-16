#pragma once

#include "RenderStyle.h"

namespace NQVTK
{
	namespace Styles
	{
		class DepthPeeling : public NQVTK::RenderStyle
		{
		public:
			typedef NQVTK::RenderStyle Superclass;

			DepthPeeling();
			virtual ~DepthPeeling();

			virtual GLFramebuffer *CreateFBO(int w, int h);
			virtual GLProgram *CreateScribe();
			virtual GLProgram *CreatePainter();

			virtual void RegisterScribeTextures(GLFramebuffer *previous);
			virtual void RegisterPainterTextures(GLFramebuffer *current, 
				GLFramebuffer *previous);

		private:
			// Not implemented
			DepthPeeling(const DepthPeeling&);
			void operator=(const DepthPeeling&);
		};
	}
}
