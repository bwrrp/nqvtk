#pragma once

#include "GLBlaat/GLFramebuffer.h"
#include "GLBlaat/GLProgram.h"
#include "GLBlaat/GLTextureManager.h"

namespace NQVTK
{
	class Renderable;

	class RenderStyle
	{
	public:
		RenderStyle() 
		{ 
			tm = 0;
		}
		virtual ~RenderStyle() { }

		void Initialize(GLTextureManager *tm)
		{
			this->tm = tm;
		}

		virtual GLFramebuffer *CreateFBO(int w, int h) = 0;
		virtual GLProgram *CreateScribe() = 0;
		virtual GLProgram *CreatePainter() = 0;

		virtual void PrepareForObject(GLProgram *scribe, 
			int objectId, Renderable *renderable) 
		{
			scribe->SetUniform1i("objectId", objectId);
		}

		virtual void RegisterScribeTextures(GLFramebuffer *previous) { }
		virtual void UnregisterScribeTextures() { }
		virtual void UpdateScribeParameters(GLProgram *scribe) { }

		virtual void RegisterPainterTextures(GLFramebuffer *current, 
			GLFramebuffer *previous) { }
		virtual void UnregisterPainterTextures() { }
		virtual void UpdatePainterParameters(GLProgram *painter) { }

	protected:
		GLTextureManager *tm;

	private:
		// Not implemented
		RenderStyle(const RenderStyle&);
		void operator=(const RenderStyle&);
	};
}
