#pragma once

#include "Renderer.h"
#include "Styles/ShadowMap.h"

namespace NQVTK
{
	class ShadowMappingRenderer : public Renderer
	{
	public:
		typedef Renderer Superclass;

		ShadowMappingRenderer() : Renderer() 
		{
			otherStyle = new NQVTK::Styles::ShadowMap();
			otherFbo1 = otherFbo2 = 0;
			shadowBuffer = 0;
			shadowPass = false;
		}

		virtual ~ShadowMappingRenderer() 
		{
			if (shadowBuffer) delete shadowBuffer;
			// Cleanup our shadow mapping style
			if (shadowPass) SwapStyles();
			if (otherStyle) delete otherStyle;
			if (otherFbo1) delete otherFbo1;
			if (otherFbo2) delete otherFbo2;
		}

		virtual bool Initialize()
		{
			if (!Superclass::Initialize()) return false;

			// Force recreation of the shadow map FBO
			if (shadowBuffer) delete shadowBuffer;
			shadowBuffer = 0;

			// Clean up the auxilary buffers
			if (otherFbo1) delete otherFbo1;
			otherFbo1 = 0;
			if (otherFbo2) delete otherFbo2;
			otherFbo2 = 0;

			// The RenderStyle may have changed. If this was a shadow pass, we lost our style.
			if (shadowPass)
			{
				otherStyle = new NQVTK::Styles::ShadowMap();
			}
			else
			{
				// The next pass should be a shadow pass (we lost our shadow buffer)
				SwapStyles();
			}

			return true;
		}

		virtual void Resize(int w, int h)
		{
			// Resize buffers managed by the parent class
			Superclass::Resize(w, h);

			// Resize or recreate shadow buffer
			if (!shadowBuffer)
			{
				NQVTK::Styles::ShadowMap* shadowStyle;
				if (shadowPass)
				{
					shadowStyle = dynamic_cast<NQVTK::Styles::ShadowMap*>(style);
				}
				else
				{
					shadowStyle = dynamic_cast<NQVTK::Styles::ShadowMap*>(otherStyle);
				}
				shadowBuffer = shadowStyle->CreateShadowBufferFBO(w, h);
			}
			else
			{
				if (!shadowBuffer->Resize(w, h)) qDebug("WARNING! shadowBuffer resize failed!");
			}

			// Resize or recreate the auxilary buffers
			if (!otherFbo1)
			{
				otherFbo1 = otherStyle->CreateFBO(w, h);
			}
			else
			{
				if (!otherFbo1->Resize(w, h)) qDebug("WARNING! otherFbo1 resize failed!");
			}
			if (!otherFbo2)
			{
				otherFbo2 = otherStyle->CreateFBO(w, h);
			}
			else
			{
				if (!otherFbo2->Resize(w, h)) qDebug("WARNING! otherFbo2 resize failed!");
			}
		}

		virtual void DrawCamera()
		{
			if (shadowPass)
			{
				// TODO: use the light's viewpoint
				Superclass::DrawCamera();
			}
			else
			{
				// Draw the normal camera
				Superclass::DrawCamera();
			}
		}

		virtual void Draw()
		{
			// Draw both passes
			DrawPass();
			DrawPass();
		}

		virtual void DrawPass()
		{
			GLFramebuffer *oldTarget = 0;
			if (shadowPass)
			{
				// Draw the shadow pass to our shadow buffer
				oldTarget = SetTarget(shadowBuffer);
			}
			else
			{
				// Get the shadow map
				GLTexture *shadowMap = shadowBuffer->GetTexture2D(
					GL_COLOR_ATTACHMENT0_EXT);
			}

			// Draw the pass
			Superclass::Draw();

			if (shadowPass)
			{
				// Restore the target for the next pass
				SetTarget(oldTarget);
			}

			// Swap styles for the next pass
			SwapStyles();
		}

		virtual void SwapStyles()
		{
			// Swap styles
			RenderStyle *style = this->style;
			this->style = otherStyle;
			otherStyle = style;
			// Swap fbos
			GLFramebuffer *fbo = this->fbo1;
			fbo1 = otherFbo1;
			otherFbo1 = fbo;
			fbo = fbo2;
			fbo2 = otherFbo2;
			otherFbo2 = fbo;
			// Toggle pass flag
			shadowPass = !shadowPass;
		}

	protected:
		GLFramebuffer *shadowBuffer;
		GLFramebuffer *otherFbo1;
		GLFramebuffer *otherFbo2;
		NQVTK::RenderStyle *otherStyle;
		bool shadowPass;
	};
}
