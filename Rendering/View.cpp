#include "View.h"

#include "Scene.h"
#include "Renderables/Renderable.h"

#include <cassert>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	View::View(Scene *scene) : scene(scene)
	{
	}

	// ------------------------------------------------------------------------
	View::View(View *sameSceneAs) : scene(0)
	{
		if (sameSceneAs)
		{
			scene = sameSceneAs->scene;
		}
	}

	// ------------------------------------------------------------------------
	View::~View()
	{
	}

	// ------------------------------------------------------------------------
	void View::SetScene(Scene *scene)
	{
		this->scene = scene;
	}

	// ------------------------------------------------------------------------
	void View::SetScene(NQVTK::View *sameSceneAs)
	{
		SetScene(sameSceneAs->scene);
	}

	// ------------------------------------------------------------------------
	Renderable *View::GetRenderable(unsigned int i)
	{
		if (!scene) return 0;
		return scene->GetRenderable(i);
	}

	// ------------------------------------------------------------------------
	unsigned int View::GetNumberOfRenderables()
	{
		if (!scene) return 0;
		return scene->GetNumberOfRenderables();
	}

	// ------------------------------------------------------------------------
	bool View::GetVisibility(unsigned int i)
	{
		if (!scene) return false;
		return scene->GetVisibility(i);
	}

	// ------------------------------------------------------------------------
	void View::SetVisibility(unsigned int i, bool visible)
	{
		if (!scene) return;
		scene->SetVisibility(i, visible);
	}
}
