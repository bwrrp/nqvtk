#pragma once

#include "RenderStyle.h"

#include <string>
#include <vector>

namespace NQVTK
{
	class DistanceFieldParamSet;

	namespace Styles
	{
		class Raycaster : public NQVTK::RenderStyle
		{
		public:
			typedef NQVTK::RenderStyle Superclass;

			Raycaster();

			virtual void PrepareForObject(GLProgram *scribe, 
				int objectId, NQVTK::Renderable *renderable);

			virtual GLFramebuffer *CreateFBO(int w, int h);
			virtual GLProgram *CreateScribe();
			virtual GLProgram *CreatePainter();

			virtual void RegisterScribeTextures(GLFramebuffer *previous);
			virtual void RegisterPainterTextures(GLFramebuffer *current, 
				GLFramebuffer *previous);

			virtual void UpdatePainterParameters(GLProgram *painter);

			float stepSize;
			float kernelSize;

		protected:
			std::vector<NQVTK::DistanceFieldParamSet*> volumes;

			std::string GetVarName(const std::string &baseName, int index);

			// Unit used for stepSize and kernelSize
			double unitSize;

		private:
			// Not implemented
			Raycaster(const Raycaster&);
			void operator=(const Raycaster&);
		};
	}
}
