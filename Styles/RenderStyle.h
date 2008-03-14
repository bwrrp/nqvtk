#pragma once

#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLProgram.h"

namespace NQVTK
{
	class Renderable;

	class RenderStyle
	{
	public:
		RenderStyle() { }
		virtual ~RenderStyle() { }

		virtual GLFramebuffer *CreateFBO(int w, int h) = 0;
		virtual GLProgram *CreateScribe() = 0;
		virtual GLProgram *CreatePainter() = 0;

		virtual void PrepareForObject(GLProgram *scribe, 
			int objectId, Renderable *renderable) 
		{
			scribe->SetUniform1i("objectId", objectId);
		}

		virtual void BindScribeTextures(GLProgram *scribe, 
			GLFramebuffer *previous) { }
		virtual void UnbindScribeTextures() { }

		virtual void BindPainterTextures(GLProgram *painter, 
			GLFramebuffer *current, GLFramebuffer *previous) { }
		virtual void UnbindPainterTextures() { }

	private:
		// Not implemented
		RenderStyle(const RenderStyle&);
		void operator=(const RenderStyle&);
	};
}
