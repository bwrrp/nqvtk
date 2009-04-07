#pragma once

#include "Raycaster.h"

class GLTexture3D;

namespace NQVTK
{
	namespace Styles
	{
		class DeformationRaycaster : public Raycaster
		{
		public:
			typedef NQVTK::Styles::Raycaster Superclass;

			DeformationRaycaster();
			virtual ~DeformationRaycaster();

			virtual GLProgram *CreatePainter();

			virtual void RegisterPainterTextures(GLFramebuffer *current, 
				GLFramebuffer *previous);

			virtual void UpdatePainterParameters(GLProgram *painter);

			float testParam;

			float focusIFStart;
			float focusIFEnd;
			float staticIFStart;
			float staticIFEnd;
			float dynamicIFStart;
			float dynamicIFEnd;

			float smearTFStart;
			float smearTFEnd;
			float smearDensity;
			float smearLength;

		protected:
			GLTexture3D *noiseVol;

		private:
			// Not implemented
			DeformationRaycaster(const DeformationRaycaster&);
			void operator=(const DeformationRaycaster&);
		};
	}
}
