#pragma once

#include "LayeredRenderer.h"

namespace NQVTK
{
	class CrossEyedStereoRenderer : public LayeredRenderer
	{
	public:
		typedef LayeredRenderer Superclass;

		CrossEyedStereoRenderer();
		virtual ~CrossEyedStereoRenderer();

		virtual void Resize(int w, int h);

		virtual void Clear();

		virtual void DrawCamera();

		virtual void Draw();

		double eyeSpacing;

	protected:
		bool leftEye;
	};
}
