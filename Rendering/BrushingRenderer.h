#pragma once

#include "Renderer.h"

#include "Math/Vector3.h"

#include <queue>

namespace NQVTK
{
	class BrushingRenderer : public NQVTK::Renderer
	{
	public:
		typedef Renderer Superclass;

		BrushingRenderer();

		virtual bool Initialize();

		virtual void Resize(int w, int h);

		virtual void Draw();

		virtual void LineTo(int x, int y, int pen);

	protected:
		std::queue<NQVTK::Vector3> pointQueue;
		Vector3 lastPos;
		double leftover;

	private:
		// Not implemented
		BrushingRenderer(const BrushingRenderer&);
		void operator=(const BrushingRenderer&);
	};
}
