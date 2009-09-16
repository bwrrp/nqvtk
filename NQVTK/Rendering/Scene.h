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

		unsigned int GetNumberOfRenderables();

		void DeleteAllRenderables();

		void ResetRenderables();

		// Shortcuts for global visibility
		bool GetVisibility(unsigned int i);
		void SetVisibility(unsigned int i, bool visible);

	protected:
		std::vector<Renderable*> renderables;
	};
}
