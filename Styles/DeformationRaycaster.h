#pragma once

#include "Raycaster.h"

#include "GLBlaat/GLTexture3D.h"

#include <cstdlib>

namespace NQVTK
{
	namespace Styles
	{
		class DeformationRaycaster : public NQVTK::Styles::Raycaster
		{
		public:
			typedef NQVTK::Styles::Raycaster Superclass;

			DeformationRaycaster() : noiseVol(0)
			{
				testParam = 0.5;

				SetOption("NQVTK_RAYCASTER_VOLUMECOUNT", "4");

				//UnsetOption("NQVTK_RAYCASTER_CENTRALDIFFERENCES");
				UnsetOption("NQVTK_RAYCASTER_LIGHTING");

				//SetOption("NQVTK_RAYCASTER_STRIPING");
				SetOption("NQVTK_RAYCASTER_DEFORM");
				//SetOption("NQVTK_RAYCASTER_SMEAR");

				SetOption("NQVTK_RAYCASTER_FOCUS");
				SetOption("NQVTK_RAYCASTER_NOISESMEAR");
				SetOption("NQVTK_RAYCASTER_STREAMLINESHADING");
			}

			virtual ~DeformationRaycaster()
			{
				delete noiseVol;
			}

			virtual GLProgram *CreatePainter()
			{
				GLProgram *painter = GLProgram::New();
				// Painter vertex shaders
				bool res = painter->AddVertexShader(
					Shaders::GenericPainterVS);
				// Painter fragment shaders
				if (res) res = painter->AddFragmentShader(
					AddShaderDefines(Shaders::DeformationRaycasterPainterFS));
				if (res) res = painter->AddFragmentShader(
					AddShaderDefines(Shaders::LibUtility));
				if (res) res = painter->Link();
				qDebug(painter->GetInfoLogs().c_str());
				if (!res) 
				{
					delete painter;
					return 0;
				}
				return painter;
			}

			virtual void RegisterPainterTextures(GLFramebuffer *current, GLFramebuffer *previous) 
			{
				Superclass::RegisterPainterTextures(current, previous);

				// Add the noise texture
				if (!noiseVol)
				{
					// Create the noise texture
					const unsigned int noiseDim = 64;
					unsigned char *buffer = new unsigned char[noiseDim * noiseDim * noiseDim];
					for (unsigned int i = 0; i < noiseDim * noiseDim * noiseDim; ++i)
					{
						buffer[i] = static_cast<unsigned char>(std::rand() % 256);
					}
					noiseVol = GLTexture3D::New(noiseDim, noiseDim, noiseDim, 
						GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer);
					tm->AddTexture("noiseVol", noiseVol, false);
					delete [] buffer;
				}
			}

			virtual void UpdatePainterParameters(GLProgram *painter)
			{
				Superclass::UpdatePainterParameters(painter);

				painter->SetUniform1f("testParam", testParam);
			}

			float testParam;

		protected:
			GLTexture3D *noiseVol;

		private:
			// Not implemented
			DeformationRaycaster(const DeformationRaycaster&);
			void operator=(const DeformationRaycaster&);
		};
	}
}
