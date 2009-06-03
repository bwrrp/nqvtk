#pragma once

#include "RenderStyle.h"

#include <string>
#include <vector>

namespace NQVTK
{
	class VolumeParamSet;

	namespace Styles
	{
		class Raycaster : public NQVTK::RenderStyle
		{
		public:
			typedef NQVTK::RenderStyle Superclass;

			Raycaster();

			virtual GLFramebuffer *CreateFBO(int w, int h);
			virtual GLProgram *CreateScribe();
			virtual GLProgram *CreatePainter();

			virtual void RegisterScribeTextures(GLFramebuffer *previous);
			virtual void RegisterPainterTextures(GLFramebuffer *current, 
				GLFramebuffer *previous);

			virtual void UpdatePainterParameters(GLProgram *painter);

			virtual void SceneChanged(View *view);

			float stepSize;
			float kernelSize;

		protected:
			// Unit used for stepSize and kernelSize
			// TODO: add method to compute unitSize over all volumes
			double unitSize;

			// TODO: add method to set texture parameters (interpolation)

		private:
			// Not implemented
			Raycaster(const Raycaster&);
			void operator=(const Raycaster&);
		};
	}
}
