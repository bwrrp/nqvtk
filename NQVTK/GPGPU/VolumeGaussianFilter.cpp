#include "VolumeGaussianFilter.h"

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
		VolumeGaussianFilter::VolumeGaussianFilter()
			: helperYZX(0), helperZXY(0), scale(1.0)
		{
		}

		// --------------------------------------------------------------------
		VolumeGaussianFilter::~VolumeGaussianFilter()
		{
			delete helperYZX;
			delete helperZXY;
		}

		// --------------------------------------------------------------------
		bool VolumeGaussianFilter::Setup(Volume *input)
		{
			if (!Superclass::Setup(input)) return false;

			// We use seperability of the kernel to perform convolution in 
			// three passes. In order to keep the shader simple we rotate the 
			// dimensions of the texture for each pass. This way, each pass 
			// convolves along its input's x-axis and writes the result in the 
			// y/z planes, which are actually the x/y planes for the next pass.

			// TODO: use hw linear interpolation for faster convolution

			// We'll need a few helpers for this, with rotated dimensions.
			delete helperYZX;
			helperYZX = Volume::New(
				input->GetHeight(), input->GetDepth(), input->GetWidth(), 
				input->GetInternalFormat(), 
				input->GetDataFormat(), input->GetDataType(), 0);

			delete helperZXY;
			helperZXY = Volume::New(
				input->GetDepth(), input->GetWidth(), input->GetHeight(), 
				input->GetInternalFormat(), 
				input->GetDataFormat(), input->GetDataType(), 0);

			if (!helperYZX || !helperZXY) 
			{
				std::cerr << "Could not create helper volumes!" << std::endl;
				return false;
			}

			// Copy metadata from input
			helperYZX->SetDataScale(input->GetDataScale());
			helperYZX->SetDataShift(input->GetDataShift());
			helperYZX->SetOriginalSize(input->GetOriginalSize().yzx());

			helperZXY->SetDataScale(input->GetDataScale());
			helperZXY->SetDataShift(input->GetDataShift());
			helperZXY->SetOriginalSize(input->GetOriginalSize().zxy());

			return true;
		}

		// --------------------------------------------------------------------
		Volume *VolumeGaussianFilter::Execute(Volume *outVol)
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
			output->SetDataScale(input->GetDataScale());
			output->SetDataShift(input->GetDataShift());
			output->SetOrigin(input->GetOrigin());
			output->SetOriginalSize(input->GetOriginalSize());

			// Prepare GL state
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);

			// Execute convolution passes
			ExecutePass(input, helperYZX);
			ExecutePass(helperYZX, helperZXY);
			ExecutePass(helperZXY, output);

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

		// --------------------------------------------------------------------
		GLProgram *VolumeGaussianFilter::CreateProgram()
		{
			GLProgram *program = GLProgram::New();
			if (!program) return 0;
			bool res = program->AddVertexShader(
				Shaders::GenericPainterVS);
			if (res) res = program->AddFragmentShader(
				Shaders::ConvolutionFilter);
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
		void VolumeGaussianFilter::SetupProgramParameters(GLProgram *program)
		{
			program->SetUniform1f("scale", scale);
		}
	}
}
