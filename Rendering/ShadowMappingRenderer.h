#pragma once

#include "Renderer.h"

namespace NQVTK
{
	class ShadowMappingRenderer : public Renderer
	{
	public:
		typedef Renderer Superclass;

		ShadowMappingRenderer() : Renderer() 
		{
			shadowPass = false;
		}

		virtual ~ShadowMappingRenderer() { }

		virtual void Resize(int w, int h)
		{
			Superclass::Resize(w, h);
			// TODO: resize the shadow mapping FBO
		}

		virtual void DrawCamera()
		{
			// TODO: choose the right camera to draw (light or eye)
			Superclass::DrawCamera();
		}

		virtual void Draw()
		{
			// TODO: draw shadow pass
			// TODO: get and bind texture
			// Draw
			Superclass::Draw();
		}

	protected:
		bool shadowPass;
	};
}
