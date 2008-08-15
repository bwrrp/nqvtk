#pragma once

#include "GLBlaat/GL.h"

#include "Renderer.h"
#include "PolyData.h"

#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLTexture.h"
#include "GLBlaat/GLProgram.h"

#include "Shaders.h"

#include <QObject> // for qDebug
#include <cassert>

namespace NQVTK 
{
	class PointFilteringRenderer : public Renderer
	{
	public:
		typedef Renderer Superclass;

		PointFilteringRenderer() : Renderer() { }

		virtual ~PointFilteringRenderer() { }

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

			// TODO: make sure we have input points

			// TODO: figure out target FBO size
			int width = 2048;
			int height = 2048;

			// Create the target FBO
			fboTarget = GLFramebuffer::New(width, height);
			assert(fboTarget);
			fboTarget->CreateColorBuffer();

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

			return true;
		}

		virtual void Draw()
		{
			// Clear the target
			Clear();

			// Setup camera transforms
			DrawCamera();

			//fboTarget->Bind();

			pointFilter->Start();
			pointFilter->SetUniform1i("bufferWidth", fboTarget->GetWidth());
			pointFilter->SetUniform1i("bufferHeight", fboTarget->GetHeight());
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

			// TODO: get image from target and extract points

			//fboTarget->Unbind();
		}

	protected:
		// TODO: point storage (VTK PolyData?)

		GLProgram *pointFilter;

	private:
		// Not implemented
		PointFilteringRenderer(const PointFilteringRenderer&);
		void operator=(const PointFilteringRenderer&);
	};
}
