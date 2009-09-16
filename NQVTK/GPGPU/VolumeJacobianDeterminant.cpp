#include "VolumeJacobianDeterminant.h"

#include "Rendering/Volume.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLProgram.h"

#include "Shaders.h"

#include <cassert>
#include <iostream>

namespace NQVTK
{
	namespace GPGPU
	{
		// --------------------------------------------------------------------
		VolumeJacobianDeterminant::VolumeJacobianDeterminant()
			: scale(1.0), outDataShift(-5.0), outDataScale(10.0)
		{
		}

		// --------------------------------------------------------------------
		VolumeJacobianDeterminant::~VolumeJacobianDeterminant()
		{
		}

		// --------------------------------------------------------------------
		bool VolumeJacobianDeterminant::Setup(Volume *input)
		{
			if (!Superclass::Setup(input)) return false;

			// The intermediate results for the jacobian are too large to 
			// store. Therefore we implement the complete calculation using 
			// central differencing in a single pass. If the jacobian should 
			// be computed at a higher scale, blur the volume first using the 
			// VolumeGaussianFilter.

			return true;
		}

		// --------------------------------------------------------------------
		Volume *VolumeJacobianDeterminant::Execute(Volume *outVol)
		{
			Volume *output = outVol;
			if (output)
			{
				// Check output volume dimensions
				if (output->GetWidth() != input->GetWidth() || 
					output->GetHeight() != input->GetHeight() || 
					output->GetDepth() != input->GetDepth())
				{
					// We can't use this
					delete output;
					output = 0;
				}
			}

			if (!output)
			{
				// Create new output texture
				output = Volume::New(
					input->GetWidth(), input->GetHeight(), input->GetDepth(), 
					input->GetInternalFormat(), 
					input->GetDataFormat(), input->GetDataType(), 0);
			}
			if (!output)
			{
				std::cerr << "Could not create output texture!" << std::endl;
				return 0;
			}

			// Copy metadata from input
			output->SetDataScale(outDataScale);
			output->SetDataShift(outDataShift);
			output->SetOrigin(input->GetOrigin());
			output->SetOriginalSize(input->GetOriginalSize());

			// Prepare GL state
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);

			// Execute pass
			ExecutePass(input, output);

			glPopAttrib();

			// Use linear interpolation for the new volume
			output->BindToCurrent();
			glTexParameterf(output->GetTextureTarget(), 
				GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(output->GetTextureTarget(), 
				GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			output->UnbindCurrent();

			return output;
		}

		// --------------------------------------------------------------------------
		GLProgram *VolumeJacobianDeterminant::CreateProgram()
		{
			GLProgram *program = GLProgram::New();
			if (!program) return 0;
			bool res = program->AddVertexShader(
				Shaders::GenericPainterVS);
			if (res) res = program->AddFragmentShader(
				Shaders::JacobianDeterminant);
			if (res) res = program->Link();
			std::cout << program->GetInfoLogs() << std::endl;
			if (!res)
			{
				delete program;
				return 0;
			}
			return program;
		}

		// --------------------------------------------------------------------
		void VolumeJacobianDeterminant::SetupProgramParameters(
			GLProgram *program)
		{
			program->SetUniform1f("scale", scale);
			program->SetUniform1f("outDataShift", outDataShift);
			program->SetUniform1f("outDataScale", outDataScale);
		}
	}
}
