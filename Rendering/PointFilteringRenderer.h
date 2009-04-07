#pragma once

#include "Renderer.h"

#include <vector>

class GLFramebuffer;
class GLProgram;
class GLTexture;

namespace NQVTK 
{
	class PointFilteringRenderer : public Renderer
	{
	public:
		typedef Renderer Superclass;

		PointFilteringRenderer();
		virtual ~PointFilteringRenderer();

		virtual bool Initialize();

		virtual void Draw();

		void SetMask(GLTexture *mask);

		std::vector<unsigned int> pointIds;

	protected:
		// TODO: point storage (VTK PolyData?)
		GLProgram *pointFilter;
		GLFramebuffer *renderBuffer;

	private:
		// Not implemented
		PointFilteringRenderer(const PointFilteringRenderer&);
		void operator=(const PointFilteringRenderer&);
	};
}
