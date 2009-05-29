#pragma once

#include "Renderer.h"

#include "GLBlaat/GLProgram.h"

class GLFramebuffer;
class GLProgram;
class GLOcclusionQuery;

namespace NQVTK 
{
	class Renderable;
	class RenderStyle;

	class LayeredRenderer : public Renderer
	{
	public:
		typedef Renderer Superclass;

		LayeredRenderer();
		virtual ~LayeredRenderer();

		virtual bool Initialize();

		virtual void SetViewport(int x, int y, int w, int h);

		void SwapFramebuffers();

		virtual void DrawScribePass(int layer);

		virtual void DrawPainterPass(int layer);

		virtual void Draw();

		virtual void PrepareForRenderable(int objectId, Renderable *renderable);

		virtual void ApplyParamSetsArrays(GLProgram *program);

		virtual void DrawRenderables();

		virtual void SceneChanged();

		bool SetStyle(RenderStyle *style);

		int maxLayers;
		int skipLayers;
		bool drawBackground;

	protected:
		RenderStyle *style;
		std::vector<GLAttributeInfo> scribeAttribs;

		GLFramebuffer *fbo1;
		GLFramebuffer *fbo2;
		GLProgram *scribe;
		GLProgram *painter;
		GLOcclusionQuery *query;

	private:
		// Not implemented
		LayeredRenderer(const LayeredRenderer&);
		void operator=(const LayeredRenderer&);
	};
}
