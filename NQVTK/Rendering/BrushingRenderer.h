#pragma once

#include "Renderer.h"

#include "NQVTK/Math/Vector3.h"

#include <queue>

namespace NQVTK
{
	class BrushingRenderer : public NQVTK::Renderer
	{
	public:
		typedef Renderer Superclass;

		BrushingRenderer();

		virtual void SetViewport(int x, int y, int w, int h);

		virtual void Draw();

		virtual void LineTo(int x, int y, int pen);

	protected:
		std::queue<NQVTK::Vector3> pointQueue;
		Vector3 lastPos;
		double leftover;

		virtual bool Initialize();

	private:
		// Not implemented
		BrushingRenderer(const BrushingRenderer&);
		void operator=(const BrushingRenderer&);
	};
}
