#pragma once

class GLFramebuffer;
class GLProgram;

namespace NQVTK
{
	class RenderStyle
	{
	public:
		RenderStyle() { }
		virtual ~RenderStyle() { }

		virtual GLFramebuffer *CreateFBO(int w, int h) = 0;
		virtual GLProgram *CreateScribe() = 0;
		virtual GLProgram *CreatePainter() = 0;

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
