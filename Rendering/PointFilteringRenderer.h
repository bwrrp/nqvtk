#pragma once

#include "GLBlaat/GL.h"

#include "Renderer.h"
#include "PolyData.h"

#include "Math/Vector3.h"

#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLTexture.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLUtility.h"

#include "Shaders.h"

#include <QObject> // for qDebug
#include <cassert>

namespace NQVTK 
{
	class PointFilteringRenderer : public Renderer
	{
	public:
		typedef Renderer Superclass;

		PointFilteringRenderer() : Renderer()
		{
			pointFilter = 0;
			renderBuffer = 0;
		}

		virtual ~PointFilteringRenderer()
		{
			// This renderer doesn't own its renderables or camera
			camera = 0;
			renderables.clear();

			if (pointFilter) delete pointFilter;
			if (renderBuffer) delete renderBuffer;
		}

		virtual bool Initialize()
		{
			if (!Superclass::Initialize()) return false;

			// Create the point extracting program
			pointFilter = GLProgram::New();
			bool res = pointFilter->AddVertexShader(
				Shaders::PointFilterVS);
			if (res) res = pointFilter->AddFragmentShader(
				Shaders::PointFilterFS);
			if (res) res = pointFilter->Link();
			if (!res)
			{
				delete pointFilter;
				qDebug("Error creating PointFilter program!");
				return false;
			}

			// TODO: get input, figure out target FBO size
			int width = 512;
			int height = 512;

			// Create the target FBO
			renderBuffer = GLFramebuffer::New(width, height);
			assert(renderBuffer);
			renderBuffer->CreateColorTexture();
			assert(renderBuffer->IsOk());
			renderBuffer->Unbind();

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

			return true;
		}

		virtual void Draw()
		{
			// Clear the target
			Clear();

			// Setup camera transforms
			DrawCamera();

			renderBuffer->Bind();

			pointFilter->Start();
			pointFilter->SetUniform1i("bufferWidth", renderBuffer->GetWidth());
			pointFilter->SetUniform1i("bufferHeight", renderBuffer->GetHeight());
			tm->SetupProgram(pointFilter);
			tm->Bind();

			// Draw the points
			NQVTK::PolyData *pd = dynamic_cast<NQVTK::PolyData*>(GetRenderable(0));
			if (pd)
			{
				pd->Draw(NQVTK::PolyData::DRAWMODE_POINTS, NQVTK::PolyData::DRAWPARTS_ALL);
			}

			tm->Unbind();
			pointFilter->Stop();

			renderBuffer->Unbind();

			// Get the image from the target
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			GLTexture *tex = renderBuffer->GetTexture2D();
			tex->BindToCurrent();
			const int bpp = 4;
			unsigned char *buffer = new unsigned char[tex->GetWidth() * tex->GetHeight() * bpp];
			glGetTexImage(tex->GetTextureTarget(), 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
			tex->UnbindCurrent();
			GLUtility::CheckOpenGLError("Grabbing filter result");
			glPopAttrib();

			// Extract the points
			for (int i = 0; i < tex->GetWidth() * tex->GetHeight(); ++i)
			{
				if (buffer[i * bpp] > 0 || 
					buffer[i * bpp + 1] > 0 || 
					buffer[i * bpp + 2] > 0)
				{
					pointIds.push_back(static_cast<unsigned int>(i));
				}
			}

			qDebug("Extracted %d points:", pointIds.size());
			for (std::vector<unsigned int>::iterator it = pointIds.begin();
				it != pointIds.end(); ++it)
			{
				qDebug("%d", *it);
			}

			delete [] buffer;
		}

		void SetMask(GLTexture *mask)
		{
			assert(tm);
			tm->AddTexture("mask", mask, false);
		}

		std::vector<unsigned int> pointIds;

	protected:
		// TODO: point storage (VTK PolyData?)
		GLProgram *pointFilter;
		GLFramebuffer *renderBuffer;

	private:
		// Not implemented
		PointFilteringRenderer(const PointFilteringRenderer&);
		void operator=(const PointFilteringRenderer&);
	};
}
