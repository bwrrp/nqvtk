#include "View.h"

#include "Scene.h"
#include "Renderables/Renderable.h"

#include <cassert>

namespace NQVTK
{
	// ------------------------------------------------------------------------
	View::View(Scene *scene) : scene(scene)
	{
		assert(scene);
	}

	// ------------------------------------------------------------------------
	View::View(View *sameSceneAs) : scene(sameSceneAs->scene)
	{
		assert(sameSceneAs);
	}

	// ------------------------------------------------------------------------
	View::~View()
	{
	}

	// ------------------------------------------------------------------------
	Renderable *View::GetRenderable(unsigned int i)
	{
		return scene->GetRenderable(i);
	}

	// ------------------------------------------------------------------------
	int View::GetNumberOfRenderables()
	{
		return scene->GetNumberOfRenderables();
	}

	// ------------------------------------------------------------------------
	bool View::GetVisibility(unsigned int i)
	{
		Renderable *renderable = GetRenderable(i);
		if (!renderable) return false;
		return renderable->visible;
	}

	// ------------------------------------------------------------------------
	void View::SetVisibility(unsigned int i, bool visible)
	{
		Renderable *renderable = GetRenderable(i);
		if (!renderable) return;
		renderable->visible = visible;
	}
}
