#pragma once

#include "VolumeToVolumeFilter.h"

#include "Rendering/Volume.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLTexture3D.h"
#include "GLBlaat/GLTextureManager.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLRenderTexture3DLayer.h"
#include "GLBlaat/GLUtility.h"

#include "Shaders.h"

#include <cassert>
#include <iostream>

namespace NQVTK
{
	namespace GPGPU
	{
		// --------------------------------------------------------------------
		VolumeToVolumeFilter::VolumeToVolumeFilter()
			: tm(0), program(0), input(0), helperYZX(0), helperZXY(0), 
			scale(1.0)
		{
		}

		// --------------------------------------------------------------------
		VolumeToVolumeFilter::~VolumeToVolumeFilter()
		{
			delete tm;
			delete program;
			delete helperYZX;
			delete helperZXY;
		}

		// --------------------------------------------------------------------
		bool VolumeToVolumeFilter::Setup(Volume *input)
		{
            tm = GLTextureManager::New();

			assert(input);
			this->input = input;

			// We use seperability of the kernel to perform convolution in 
			// three passes. In order to keep the shader simple we rotate the 
			// dimensions of the texture for each pass. This way, each pass 
			// convolves along its input's x-axis and writes the result in the 
			// y/z planes, which are actually the x/y planes for the next pass.

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
				return 0;
			}

			// Copy metadata from input
			helperYZX->SetDataScale(input->GetDataScale());
			helperYZX->SetDataShift(input->GetDataShift());
			helperYZX->SetOriginalSize(input->GetOriginalSize().yzx());

			helperZXY->SetDataScale(input->GetDataScale());
			helperZXY->SetDataShift(input->GetDataShift());
			helperZXY->SetOriginalSize(input->GetOriginalSize().zxy());

			// TODO: program could be a parameter
			std::cout << "Creating filter..." << std::endl;
			program = CreateProgram();
			if (!program)
			{
				std::cerr << "Could not create filter program!" << std::endl;
				return false;
			}

			return true;
		}

		// --------------------------------------------------------------------
		Volume *VolumeToVolumeFilter::Execute(Volume *outVol)
		{
			// NOTE: hardcoded for seperable convolution for testing purposes
			// TODO: use inheritance to implement more complicated filters
			// such as convolution

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

			return output;
		}

		// --------------------------------------------------------------------------
		GLProgram *VolumeToVolumeFilter::CreateProgram()
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

		// --------------------------------------------------------------------------
		void VolumeToVolumeFilter::ExecutePass(Volume *input, Volume *output)
		{
			// Make sure the input uses nearest neighbor filtering
			// TODO: use hw linear interpolation for faster convolution
			GLUtility::SetDefaultColorTextureParameters(input);
			input->UnbindCurrent();

			// Set the input
			tm->AddTexture("volume", input, false);

			// Prepare an FBO for the output
			GLFramebuffer *fbo = GLFramebuffer::New(
				output->GetWidth(), output->GetHeight());

			// Run over the slices of the output
			// We render 4 slices simultaneously
			int numSlices = output->GetDepth();
			GLenum bufs[] = {
				GL_COLOR_ATTACHMENT0_EXT, 
				GL_COLOR_ATTACHMENT1_EXT, 
				GL_COLOR_ATTACHMENT2_EXT, 
				GL_COLOR_ATTACHMENT3_EXT
			};
			for (int slice = 0; slice < numSlices; slice += 4)
			{
				// Attach the slices
				int i;
				for (i = 0; i < 4 && slice + i < numSlices; ++i)
				{
					GLRenderTexture3DLayer *rt = 
						GLRenderTexture3DLayer::New(output, slice + i);
					GLRendertarget *rtOld = fbo->AttachRendertarget(
						bufs[i], rt);
					delete rtOld;
				}
				glDrawBuffers(i, bufs);
				// Check the FBO
				if (!fbo->IsOk())
				{
					std::cerr << "FBO for output volume not ok!" << std::endl;
					delete fbo;
					return;
				}

				// FBO should still be bound after this

				// Start the program
				program->Start();
				// TODO: pass volume metadata for spacing etc.
				Vector3 dims(
					static_cast<float>(input->GetWidth()), 
					static_cast<float>(input->GetHeight()), 
					static_cast<float>(input->GetDepth()));
				program->SetUniform3f("volumeDims", 
					static_cast<float>(dims.x), 
					static_cast<float>(dims.y), 
					static_cast<float>(dims.z));
				Vector3 size = input->GetOriginalSize();
				program->SetUniform3f("volumeSpacing", 
					static_cast<float>(size.x / dims.x), 
					static_cast<float>(size.y / dims.y), 
					static_cast<float>(size.z / dims.z));
				program->SetUniform1f("volumeDataShift", 
					static_cast<float>(input->GetDataShift()));
				program->SetUniform1f("volumeDataScale", 
					static_cast<float>(input->GetDataScale()));
				program->SetUniform1i("slice", slice);
				program->SetUniform1f("scale", scale);
				
				tm->SetupProgram(program);
				tm->Bind();

				// Render a full screen quad to process the slices
				glColor3d(1.0, 1.0, 1.0);
				glBegin(GL_QUADS);
				glVertex3d(-1.0, -1.0, 0.0);
				glVertex3d(1.0, -1.0, 0.0);
				glVertex3d(1.0, 1.0, 0.0);
				glVertex3d(-1.0, 1.0, 0.0);
				glEnd();

				program->Stop();
				tm->Unbind();
			}

			// Deleting the fbo will unbind it
			delete fbo;
		}
	}
}
