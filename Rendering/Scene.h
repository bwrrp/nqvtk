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

		virtual bool GetVisibility(unsigned int i);
		virtual void SetVisibility(unsigned int i, bool visible);

	protected:
		std::vector<Renderable*> renderables;
	};
}
