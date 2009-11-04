#pragma once

#include "Renderer.h"

#include "GLBlaat/GLProgram.h"

#include <vector>

namespace NQVTK
{
	class SimpleRenderer : public Renderer
	{
	public:
		typedef Renderer Superclass;

		SimpleRenderer();
		virtual ~SimpleRenderer();

		virtual void UpdateLighting();

		virtual void PrepareForRenderable(int objectId, 
			Renderable *renderable);

		virtual void Clear();

		virtual void Draw();

		GLProgram *GetShader() { return shader; }
		GLProgram *SetShader(GLProgram *shader);

		void SetDrawBackground(bool draw);

	protected:
		GLProgram *shader;
		std::vector<GLAttributeInfo> shaderAttribs;

		bool drawBackground;

	private:
		// Not implemented
		SimpleRenderer(const SimpleRenderer&);
		void operator=(const SimpleRenderer&);
	};
}
