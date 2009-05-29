#pragma once

namespace NQVTK
{
	class Scene;
	class Renderable;

	// Default view, always uses global visibility
	// NOTE: Views do not own the Scene because it may be shared
	class View
	{
	public:
		View(Scene *scene);
		View(View *sameSceneAs);
		virtual ~View();

		virtual void SetScene(Scene *scene);
		void SetScene(View *sameSceneAs);

		// Passed on to the scene
		Renderable *GetRenderable(unsigned int i);
		unsigned int GetNumberOfRenderables();

		// Override to provide local visibility
		virtual bool GetVisibility(unsigned int i);
		virtual void SetVisibility(unsigned int i, bool visible);

	protected:
		Scene *scene;
	};
}
