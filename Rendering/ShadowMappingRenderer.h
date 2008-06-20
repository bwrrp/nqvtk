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
			shadowBuffer = 0;
			shadowStyle = new NQVTK::Styles::ShadowMap();
			shadowRenderer = new NQVTK::Renderer();
		}

		virtual ~ShadowMappingRenderer() 
		{
			if (shadowBuffer) delete shadowBuffer;
			delete shadowStyle;
			// Clear the shadow renderables first
			shadowRenderer->SetRenderables(std::vector<Renderable*>());
			delete shadowRenderer;
		}

		virtual bool Initialize()
		{
			if (!Superclass::Initialize()) return false;
			
			// Set up the shadow renderer
			shadowRenderer->SetStyle(shadowStyle);
			return shadowRenderer->Initialize();
		}

		virtual void Resize(int w, int h)
		{
			// Resize buffers managed by the parent class
			Superclass::Resize(w, h);

			// Resize or recreate shadow buffer
			if (!shadowBuffer)
			{
				shadowBuffer = shadowStyle->CreateShadowBufferFBO(w, h);
				shadowRenderer->SetTarget(shadowBuffer);
			}
			else
			{
				if (!shadowBuffer->Resize(w, h)) qDebug("WARNING! shadowBuffer resize failed!");
			}

			// Resize the shadow renderer
			shadowRenderer->Resize(w, h);
		}

		virtual void Draw()
		{
			shadowRenderer->SetRenderables(renderables);
			shadowRenderer->Draw();
			
			// Get the shadow map
			GLTexture *shadowMap = shadowBuffer->GetTexture2D(GL_COLOR_ATTACHMENT0_EXT);
			tm->AddTexture("shadowMap", shadowMap, false);

			// Draw the normal pass
			Superclass::Draw();

			// DEBUG: show shadow buffer
			glDisable(GL_DEPTH_TEST);
			TestDrawTexture(shadowMap, 0.0, 1.0, 0.0, 1.0);
			glEnable(GL_DEPTH_TEST);
		}

	protected:
		GLFramebuffer *shadowBuffer;
		NQVTK::Styles::ShadowMap *shadowStyle;
		NQVTK::Renderer *shadowRenderer;
	};
}
