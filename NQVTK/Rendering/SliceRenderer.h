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

	protected:
		GLProgram *shader;
		std::vector<GLAttributeInfo> shaderAttribs;

		Vector3 origin;
		Vector3 right;
		Vector3 up;
	};
}
