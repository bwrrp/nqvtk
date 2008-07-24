#pragma once

#include "Renderer.h"

#include "OrthoCamera.h"

#include "GLBlaat/GLUtility.h"

#include <queue>

#include <QObject> // for qDebug
#include <cassert>


namespace NQVTK
{
	class BrushingRenderer : public NQVTK::Renderer
	{
	public:
		typedef Renderer Superclass;

		BrushingRenderer()
		{
			leftover = 0.0;
		}

		virtual bool Initialize()
		{
			if (!camera) 
			{
				camera = new NQVTK::OrthoCamera();
				camera->nearZ = 1.0;
				camera->farZ = 20.0;
			}

			return Superclass::Initialize();
		}

		virtual void Resize(int w, int h)
		{
			Superclass::Resize(w, h);
			
			NQVTK::OrthoCamera *ocam = dynamic_cast<NQVTK::OrthoCamera*>(camera);
			if (ocam)
			{
				// Resize the camera view
				ocam->width = w;
				ocam->height = h;
				ocam->position = Vector3(ocam->width / 2.0, ocam->height / 2.0, 10.0);
				ocam->focus = Vector3(ocam->width / 2.0, ocam->height / 2.0, 0.0);
			}

			Clear();
		}

		virtual void Draw()
		{
			if (fboTarget)
			{
				fboTarget->Bind();
			}
			else
			{
				glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
			}

			DrawCamera();

			glPushAttrib(GL_ALL_ATTRIB_BITS);

			glDisable(GL_LIGHTING);
			glDisable(GL_DEPTH_TEST);

			while (pointQueue.size() > 0)
			{
				NQVTK::Vector3 toPos = pointQueue.front();
				pointQueue.pop();

				switch (static_cast<int>(toPos.z))
				{
				case 0:
					lastPos = toPos;
					leftover = 0.0;
					continue;
					break;

				case 1:
					glColor3d(1.0, 0.0, 0.0);
					break;

				default:
					glColor3d(0.0, 0.0, 0.0);
					break;
				}

				toPos.z = 0.0;

				Vector3 d = toPos - lastPos;
				double length = d.length();
				d = d.normalized();
				double stepsize = 2.0;
				double i = leftover;
				while (i <= length)
				{
					Vector3 pos = lastPos + i * d;

					// TESTING: draw simple cursor / brush
					glBegin(GL_QUADS);
					glVertex3dv(pos.V);
					glVertex3dv((pos + Vector3(0.0, -5.0, 0.0)).V);
					glVertex3dv((pos + Vector3(5.0, -5.0, 0.0)).V);
					glVertex3dv((pos + Vector3(5.0, 0.0, 0.0)).V);
					glEnd();

					i += stepsize;
				}

				lastPos = toPos;
				leftover = i - length;
			}

			glPopAttrib();

			if (fboTarget)
			{
				fboTarget->Unbind();
			}

			GLUtility::CheckOpenGLError("BrushingRenderer::Draw()");
		}

		virtual void LineTo(int x, int y, int pen)
		{
			pointQueue.push(Vector3(x, y, pen));
		}

	protected:
		std::queue<NQVTK::Vector3> pointQueue;
		Vector3 lastPos;
		double leftover;

	private:
		// Not implemented
		BrushingRenderer(const BrushingRenderer&);
		void operator=(const BrushingRenderer&);
	};
}
