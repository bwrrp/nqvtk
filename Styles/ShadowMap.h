#pragma once

#include "RenderStyle.h"

namespace NQVTK
{
	namespace Styles
	{
		class ShadowMap : public NQVTK::RenderStyle
		{
		public:
			typedef NQVTK::RenderStyle Superclass;

			ShadowMap(NQVTK::RenderStyle *baseStyle);
			virtual ~ShadowMap();

			virtual GLFramebuffer *CreateFBO(int w, int h);
			virtual GLFramebuffer *CreateShadowBufferFBO(int w, int h);

			virtual GLProgram *CreateScribe();
			virtual GLProgram *CreatePainter();

			virtual void PrepareForObject(GLProgram *scribe, 
				int objectId, Renderable *renderable);

			virtual void RegisterScribeTextures(GLFramebuffer *previous);
			virtual void UpdateScribeParameters(GLProgram *scribe);
			virtual void RegisterPainterTextures(GLFramebuffer *current, 
				GLFramebuffer *previous);
			virtual void UpdatePainterParameters(GLProgram *painter);

			virtual void DrawBackground();

		protected:
			// Base style
			NQVTK::RenderStyle *baseStyle;

		private:
			// Not implemented
			ShadowMap(const ShadowMap&);
			void operator=(const ShadowMap&);
		};
	}
}
