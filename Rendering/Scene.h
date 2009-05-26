#pragma once

#include <vector>

namespace NQVTK
{
	class Renderable;

	class Scene
	{
	public:
		Scene();
		~Scene();

		int AddRenderable(Renderable *obj);
		Renderable *GetRenderable(unsigned int i);
		Renderable *SetRenderable(unsigned int i, Renderable *obj);

		int GetNumberOfRenderables();

		void DeleteAllRenderables();

		void ResetRenderables();

	protected:
		std::vector<Renderable*> renderables;
	};
}
