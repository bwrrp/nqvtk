#pragma once

#include "Renderer.h"

namespace NQVTK
{
	class CrossEyedStereoRenderer : public Renderer
	{
	public:
		typedef Renderer Superclass;

		CrossEyedStereoRenderer() : Renderer() 
		{
			leftEye = true;
			eyeSpacing = 0.1;
		}

		virtual ~CrossEyedStereoRenderer() { }

		virtual void Resize(int w, int h)
		{
			// We render at half width
			Superclass::Resize(w / 2, h);
		}

		virtual void Clear()
		{
			// Only clear for the left eye and when rendering to FBO
			if (leftEye || fbo1->IsBound()) Superclass::Clear();
		}

		virtual void DrawCamera()
		{
			Vector3 offset = Vector3(0.0, 0.0, 1.0);
			if (leftEye)
			{
				offset.x = -eyeSpacing / 2.0;
			}
			else
			{
				offset.x = eyeSpacing / 2.0;
			}

			// TODO: replace this, add camera reset to focus on all renderables
			Renderable *renderable = renderables[0];
			camera->position = renderable->GetCenter() - offset;
			camera->FocusOn(renderable);

			// Set up the camera (matrices)
			camera->Draw();
		}

		virtual void Draw()
		{
			// Draw left eye (on the right)
			leftEye = true;
			// Offset viewport
			viewportX = viewportWidth;
			glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
			// Draw
			Superclass::Draw();

			// Draw right eye
			leftEye = false;
			// Offset viewport
			viewportX = 0;
			glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
			// Draw
			Superclass::Draw();
		}

		double eyeSpacing;

	protected:
		bool leftEye;
	};
}
