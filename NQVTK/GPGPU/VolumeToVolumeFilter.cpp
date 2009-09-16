#include "VolumeToVolumeFilter.h"

#include "Rendering/Volume.h"

#include "GLBlaat/GL.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLTexture3D.h"
#include "GLBlaat/GLTextureManager.h"
#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLRenderTexture3DLayer.h"
#include "GLBlaat/GLUtility.h"

#include <cassert>
#include <iostream>

namespace NQVTK
{
	namespace GPGPU
	{
		// --------------------------------------------------------------------
		VolumeToVolumeFilter::VolumeToVolumeFilter()
			: tm(0), program(0), input(0)
		{
		}

		// --------------------------------------------------------------------
		VolumeToVolumeFilter::~VolumeToVolumeFilter()
		{
			delete tm;
			delete program;
		}

		// --------------------------------------------------------------------
		bool VolumeToVolumeFilter::Setup(Volume *input)
		{
            tm = GLTextureManager::New();

			assert(input);
			this->input = input;

			std::cout << "Creating filter..." << std::endl;
			program = CreateProgram();
			if (!program)
			{
				std::cerr << "Could not create filter program!" << std::endl;
				return false;
			}

			return true;
		}

		// --------------------------------------------------------------------------
		void VolumeToVolumeFilter::ExecutePass(Volume *input, Volume *output)
		{
			// Make sure the input uses nearest neighbor filtering
			input->BindToCurrent();
			glTexParameterf(input->GetTextureTarget(), 
				GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameterf(input->GetTextureTarget(), 
				GL_TEXTURE_MIN_FILTER, GL_NEAREST);
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
				
				SetupProgramParameters(program);
				
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

			// Restore linear interpolation for the input volume
			input->BindToCurrent();
			glTexParameterf(input->GetTextureTarget(), 
				GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(input->GetTextureTarget(), 
				GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			input->UnbindCurrent();
		}
	}
}
