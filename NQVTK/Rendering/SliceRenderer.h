#pragma once

#include "Renderer.h"

#include "NQVTK/Math/Vector3.h"

#include <vector>

#include "GLBlaat/GLProgram.h"

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

		GLProgram *GetShader() { return shader; }
		GLProgram *SetShader(GLProgram *shader);
		bool CreateDefaultShader();

		void SetPlane(const Vector3 &origin, 
			const Vector3 &right, 
			const Vector3 &up);
		Vector3 GetPlaneOrigin() { return origin; }
		Vector3 GetPlaneRight() { return right; }
		Vector3 GetPlaneUp() { return up; }
		Vector3 GetViewportPlaneOrigin() { return vporigin; }
		Vector3 GetViewportPlaneRight() { return vpright; }
		Vector3 GetViewportPlaneUp() { return vpup; }

	protected:
		GLProgram *shader;
		std::vector<GLAttributeInfo> shaderAttribs;

		// User-defined slice plane
		Vector3 origin;
		Vector3 right;
		Vector3 up;

		// Adjusted plane representing the entire viewport
		Vector3 vporigin;
		Vector3 vpright;
		Vector3 vpup;

		void UpdateViewportPlane();
	};
}
