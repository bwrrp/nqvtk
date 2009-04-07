#pragma once

#include "DeformationRaycaster.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLTexture3D.h"
#include "GLBlaat/GLTextureManager.h"
#include "GLBlaat/GLUtility.h"

#include "Shaders.h"

#include <iostream>

namespace NQVTK
{
	namespace Styles
	{
		// --------------------------------------------------------------------
		DeformationRaycaster::DeformationRaycaster() : noiseVol(0)
		{
			// Interest functions
			focusIFStart = 0.0;
			focusIFEnd = 1.0;
			staticIFStart = 0.0;
			staticIFEnd = 1.0;
			dynamicIFStart = 0.0;
			dynamicIFEnd = 1.0;

			testParam = 0.5;
			smearTFStart = 0.0;
			smearTFEnd = 1.0;
			smearDensity = 2.0;
			smearLength = 1.0;

			SetOption("NQVTK_RAYCASTER_VOLUMECOUNT", "4");

			//UnsetOption("NQVTK_RAYCASTER_CENTRALDIFFERENCES");
			UnsetOption("NQVTK_RAYCASTER_LIGHTING");
			//UnsetOption("NQVTK_RAYCASTER_DITHERPOS");

			//SetOption("NQVTK_DEFORMATION_MORPHING");
			//SetOption("NQVTK_DEFORMATION_TEXTURING");

			SetOption("NQVTK_DEFORMATION_FOCUS");
			//SetOption("NQVTK_DEFORMATION_FOCUS_MAGNITUDE");
			SetOption("NQVTK_DEFORMATION_STATIC");
			SetOption("NQVTK_DEFORMATION_DYNAMIC_CONTOURS");
			SetOption("NQVTK_DEFORMATION_DYNAMIC_VOLUME");
		}

		// --------------------------------------------------------------------
		DeformationRaycaster::~DeformationRaycaster()
		{
			delete noiseVol;
		}

		// --------------------------------------------------------------------
		GLProgram *DeformationRaycaster::CreatePainter()
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
			std::cout << painter->GetInfoLogs() << std::endl;
			if (!res) 
			{
				delete painter;
				return 0;
			}
			return painter;
		}

		// --------------------------------------------------------------------
		void DeformationRaycaster::RegisterPainterTextures(
			GLFramebuffer *current, GLFramebuffer *previous) 
		{
			Superclass::RegisterPainterTextures(current, previous);

			// Add the noise texture
			if (!noiseVol)
			{
				// Create the noise texture
				const unsigned int noiseDim = 64;
				unsigned char *buffer = new unsigned char[noiseDim * 
					noiseDim * noiseDim];
				for (unsigned int i = 0; i < noiseDim * noiseDim * noiseDim; 
					++i)
				{
					buffer[i] = static_cast<unsigned char>(std::rand() % 256);
				}
				noiseVol = GLTexture3D::New(noiseDim, noiseDim, noiseDim, 
					GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer);
				tm->AddTexture("noiseVol", noiseVol, false);
				delete [] buffer;
			}
		}

		// --------------------------------------------------------------------
		void DeformationRaycaster::UpdatePainterParameters(GLProgram *painter)
		{
			Superclass::UpdatePainterParameters(painter);

			painter->SetUniform1f("testParam", testParam);
			// Interest functions
			painter->SetUniform1f("focusIFStart", focusIFStart);
			painter->SetUniform1f("focusIFEnd", focusIFStart);
			painter->SetUniform1f("staticIFStart", staticIFStart);
			painter->SetUniform1f("staticIFEnd", staticIFEnd);
			painter->SetUniform1f("dynamicIFStart", dynamicIFStart);
			painter->SetUniform1f("dynamicIFEnd", dynamicIFEnd);
			// Other parameters
			painter->SetUniform1f("smearTFStart", smearTFStart);
			painter->SetUniform1f("smearTFEnd", smearTFEnd);
			painter->SetUniform1f("smearDensity", smearDensity);
			painter->SetUniform1f("smearLength", smearLength);
		}
	}
}
