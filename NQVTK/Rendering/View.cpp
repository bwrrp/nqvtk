#include "View.h"

#include "Scene.h"
#include "Renderables/Renderable.h"

#include <cassert>
#include <limits>

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

	// ------------------------------------------------------------------------
	const double *View::GetVisibleBounds()
	{
		// Get bounds for all visible renderables
		const double inf = std::numeric_limits<double>::infinity();
		for (unsigned int i = 0; i < 3; ++i)
		{
			bounds[2 * i] = inf;
			bounds[2 * i + 1] = -inf;
		}

		unsigned int numRenderables = GetNumberOfRenderables();
		for (unsigned int i = 0; i < numRenderables; ++i)
		{
			Renderable *renderable = GetRenderable(i);
			if (GetVisibility(i))
			{
				double rbounds[6];
				renderable->GetBounds(rbounds);
				for (int i = 0; i < 3; ++i)
				{
					if (rbounds[i*2] < bounds[i*2]) 
						bounds[i*2] = rbounds[i*2];

					if (rbounds[i*2+1] > bounds[i*2+1]) 
						bounds[i*2+1] = rbounds[i*2+1];
				}
			}
		}

		return bounds;
	}

	// ------------------------------------------------------------------------
	const Vector3 View::GetVisibleCenter()
	{
		const double *bounds = GetVisibleBounds();
		return Vector3(
			0.5 * (bounds[0] + bounds[1]), 
			0.5 * (bounds[2] + bounds[3]), 
			0.5 * (bounds[4] + bounds[5]));
	}
}
