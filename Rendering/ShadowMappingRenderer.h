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
			shadowBuffer = 0;
			shadowPass = false;
		}

		virtual ~ShadowMappingRenderer() 
		{
			if (shadowBuffer) delete shadowBuffer;
		}

		virtual void Resize(int w, int h)
		{
			Superclass::Resize(w, h);
			shadowBuffer->Resize(w, h);
		}

		virtual void DrawCamera()
		{
			if (shadowPass)
			{
				// TODO: use the light's viewpoint
			}
			else
			{
				// Draw the normal camera
				Superclass::DrawCamera();
			}
		}

		virtual void Draw()
		{
			// Draw the shadow pass to our shadow buffer
			GLFramebuffer *oldTarget = SetTarget(shadowBuffer);
			// TODO: switch to shadow mapping style (without all the reinitialization...)
			Superclass::Draw();
			// TODO: get and bind texture
			// Draw the final pass to the original target
			SetTarget(oldTarget);
			Superclass::Draw();
		}

	protected:
		GLFramebuffer *shadowBuffer;
		bool shadowPass;
	};
}
