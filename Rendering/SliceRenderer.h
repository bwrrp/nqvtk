#pragma once

#include "Renderer.h"

#include "Math/Vector3.h"

namespace NQVTK
{
	class SliceRenderer : public Renderer
	{
	public:
		typedef Renderer Superclass;

		SliceRenderer();
		virtual ~SliceRenderer();

		virtual void PrepareForRenderable(int objectId, 
			Renderable *renderable);

		virtual void Draw();

		void SetPlane(const Vector3 &origin, 
			const Vector3 &up, 
			const Vector3 &right);

	protected:
		Vector3 origin;
		Vector3 up;
		Vector3 right;
	};
}
